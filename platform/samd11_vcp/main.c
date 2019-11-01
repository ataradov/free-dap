/*
 * Copyright (c) 2017, Alex Taradov <alex@taradov.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*- Includes ----------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include <string.h>
#include "samd11.h"
#include "hal_gpio.h"
#include "nvm_data.h"
#include "usb.h"
#include "uart.h"
#include "dap.h"
#include "dap_config.h"

/*- Definitions -------------------------------------------------------------*/
#define USB_BUFFER_SIZE        64
#define UART_WAIT_TIMEOUT      10 // ms
#define STATUS_TIMEOUT         250 // ms

HAL_GPIO_PIN(VCP_STATUS,       A, 2);
HAL_GPIO_PIN(DAP_STATUS,       A, 4);

/*- Variables ---------------------------------------------------------------*/
static alignas(4) uint8_t app_request_buffer[DAP_CONFIG_PACKET_SIZE];
static alignas(4) uint8_t app_response_buffer[DAP_CONFIG_PACKET_SIZE];
static alignas(4) uint8_t app_recv_buffer[USB_BUFFER_SIZE];
static alignas(4) uint8_t app_send_buffer[USB_BUFFER_SIZE];
static int app_recv_buffer_size = 0;
static int app_recv_buffer_ptr = 0;
static int app_send_buffer_ptr = 0;
static bool app_send_buffer_free = true;
static bool app_send_zlp = false;
static uint64_t app_system_time = 0;
static uint64_t app_uart_timeout = 0;
static uint64_t app_status_timeout;
static bool app_dap_event = false;
static bool app_vcp_event = false;
static bool app_vcp_open = false;

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static void sys_init(void)
{
  uint32_t coarse, fine;
  uint32_t sn = 0;

  NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_RWS(1);

  SYSCTRL->INTFLAG.reg = SYSCTRL_INTFLAG_BOD33RDY | SYSCTRL_INTFLAG_BOD33DET |
      SYSCTRL_INTFLAG_DFLLRDY;

  coarse = NVM_READ_CAL(NVM_DFLL48M_COARSE_CAL);
  fine = NVM_READ_CAL(NVM_DFLL48M_FINE_CAL);

  SYSCTRL->DFLLCTRL.reg = 0; // See Errata 9905
  while (0 == (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY));

  SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_MUL(48000);
  SYSCTRL->DFLLVAL.reg = SYSCTRL_DFLLVAL_COARSE(coarse) | SYSCTRL_DFLLVAL_FINE(fine);

  SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE | SYSCTRL_DFLLCTRL_USBCRM |
      SYSCTRL_DFLLCTRL_MODE | SYSCTRL_DFLLCTRL_CCDIS;

  while (0 == (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY));

  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(0) | GCLK_GENCTRL_SRC(GCLK_SOURCE_DFLL48M) |
      GCLK_GENCTRL_RUNSTDBY | GCLK_GENCTRL_GENEN;
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

  sn ^= *(volatile uint32_t *)0x0080a00c;
  sn ^= *(volatile uint32_t *)0x0080a040;
  sn ^= *(volatile uint32_t *)0x0080a044;
  sn ^= *(volatile uint32_t *)0x0080a048;

  for (int i = 0; i < 8; i++)
    usb_serial_number[i] = "0123456789ABCDEF"[(sn >> (i * 4)) & 0xf];

  usb_serial_number[9] = 0;
}

//-----------------------------------------------------------------------------
static void sys_time_init(void)
{
  SysTick->VAL = 0;
  SysTick->LOAD = F_CPU / 1000ul;
  SysTick->CTRL = SysTick_CTRL_ENABLE_Msk;
  app_system_time = 0;
}

//-----------------------------------------------------------------------------
static void sys_time_task(void)
{
  if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
    app_system_time++;
}

//-----------------------------------------------------------------------------
static uint64_t get_system_time(void)
{
  return app_system_time;
}

//-----------------------------------------------------------------------------
void usb_cdc_send_callback(void)
{
  app_send_buffer_free = true;
}

//-----------------------------------------------------------------------------
static void send_buffer(void)
{
  app_send_buffer_free = false;
  app_send_zlp = (USB_BUFFER_SIZE == app_send_buffer_ptr);

  usb_cdc_send(app_send_buffer, app_send_buffer_ptr);

  app_send_buffer_ptr = 0;
}

//-----------------------------------------------------------------------------
void usb_cdc_recv_callback(int size)
{
  app_recv_buffer_ptr = 0;
  app_recv_buffer_size = size;
}

//-----------------------------------------------------------------------------
void usb_configuration_callback(int config)
{
  usb_cdc_recv(app_recv_buffer, sizeof(app_recv_buffer));
  usb_hid_recv(app_request_buffer, sizeof(app_request_buffer));

  app_send_buffer_free = true;

  (void)config;
}

//-----------------------------------------------------------------------------
void usb_cdc_line_coding_updated(usb_cdc_line_coding_t *line_coding)
{
  uart_init(line_coding);
}

//-----------------------------------------------------------------------------
void usb_cdc_control_line_state_update(int line_state)
{
  bool status = line_state & USB_CDC_CTRL_SIGNAL_DTE_PRESENT;

  // TODO: actually open/close the port?
  app_vcp_open = status;
}

//-----------------------------------------------------------------------------
static void tx_task(void)
{
  while (app_recv_buffer_size)
  {
    if (!uart_write_byte(app_recv_buffer[app_recv_buffer_ptr]))
      break;

    app_recv_buffer_ptr++;
    app_recv_buffer_size--;
    app_vcp_event = true;

    if (0 == app_recv_buffer_size)
      usb_cdc_recv(app_recv_buffer, sizeof(app_recv_buffer));
  }
}

//-----------------------------------------------------------------------------
static void rx_task(void)
{
  int byte;

  if (!app_send_buffer_free)
    return;

  while (uart_read_byte(&byte))
  {
    app_uart_timeout = get_system_time() + UART_WAIT_TIMEOUT;
    app_send_buffer[app_send_buffer_ptr++] = byte;
    app_vcp_event = true;

    if (USB_BUFFER_SIZE == app_send_buffer_ptr)
    {
      send_buffer();
      break;
    }
  }
}

//-----------------------------------------------------------------------------
static void uart_timer_task(void)
{
  if (app_uart_timeout && get_system_time() > app_uart_timeout)
  {
    if (app_send_zlp || app_send_buffer_ptr)
      send_buffer();

    app_uart_timeout = 0;
  }
}

//-----------------------------------------------------------------------------
void uart_serial_state_update(int state)
{
  usb_cdc_set_state(state);
}

//-----------------------------------------------------------------------------
void usb_hid_send_callback(void)
{
  usb_hid_recv(app_request_buffer, sizeof(app_request_buffer));
}

//-----------------------------------------------------------------------------
void usb_hid_recv_callback(int size)
{
  app_dap_event = true;
  dap_process_request(app_request_buffer, app_response_buffer);
  usb_hid_send(app_response_buffer, sizeof(app_response_buffer));
  (void)size;
}

//-----------------------------------------------------------------------------
bool usb_class_handle_request(usb_request_t *request)
{
  if (usb_cdc_handle_request(request))
    return true;
  else if (usb_hid_handle_request(request))
    return true;
  else 
    return false;
}

//-----------------------------------------------------------------------------
static void status_timer_task(void)
{
  if (get_system_time() < app_status_timeout)
    return;

  app_status_timeout = get_system_time() + STATUS_TIMEOUT;

  if (app_dap_event)
    HAL_GPIO_DAP_STATUS_toggle();
  else
    HAL_GPIO_DAP_STATUS_set();

  if (app_vcp_event)
    HAL_GPIO_VCP_STATUS_toggle();
  else
    HAL_GPIO_VCP_STATUS_write(app_vcp_open);

  app_dap_event = false;
  app_vcp_event = false;
}

//-----------------------------------------------------------------------------
int main(void)
{
  sys_init();
  sys_time_init();
  dap_init();
  usb_init();
  usb_cdc_init();
  usb_hid_init();

  app_status_timeout = STATUS_TIMEOUT;

  HAL_GPIO_VCP_STATUS_out();
  HAL_GPIO_VCP_STATUS_clr();

  HAL_GPIO_DAP_STATUS_out();
  HAL_GPIO_DAP_STATUS_set();

  while (1)
  {
    sys_time_task();
    usb_task();
    tx_task();
    rx_task();
    uart_timer_task();
    status_timer_task();
  }

  return 0;
}

