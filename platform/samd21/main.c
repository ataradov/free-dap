// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024, Alex Taradov <alex@taradov.com>. All rights reserved.

/*- Includes ----------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include <string.h>
#include "samd21.h"
#include "hal_config.h"
#include "nvm_data.h"
#include "usb.h"
#include "uart.h"
#include "dap.h"
#include "dap_config.h"

/*- Definitions -------------------------------------------------------------*/
#define USB_BUFFER_SIZE        64
#define UART_WAIT_TIMEOUT      10 // ms
#define STATUS_TIMEOUT         250 // ms

/*- Variables ---------------------------------------------------------------*/
static alignas(4) uint8_t app_req_buf_hid[DAP_CONFIG_PACKET_SIZE];
static alignas(4) uint8_t app_req_buf_bulk[DAP_CONFIG_PACKET_SIZE];
static alignas(4) uint8_t app_req_buf[DAP_CONFIG_PACKET_SIZE];
static alignas(4) uint8_t app_resp_buf[DAP_CONFIG_PACKET_SIZE];
static int app_req_buf_hid_size = 0;
static int app_req_buf_bulk_size = 0;
static bool app_resp_free = true;
static uint64_t app_system_time = 0;
static uint64_t app_status_timeout = 0;
static bool app_dap_event = false;

#ifdef HAL_CONFIG_ENABLE_VCP
static alignas(4) uint8_t app_recv_buffer[USB_BUFFER_SIZE];
static alignas(4) uint8_t app_send_buffer[USB_BUFFER_SIZE];
static int app_recv_buffer_size = 0;
static int app_recv_buffer_ptr = 0;
static int app_send_buffer_ptr = 0;
static bool app_send_buffer_free = true;
static bool app_send_zlp = false;
static uint64_t app_uart_timeout = 0;
static uint64_t app_break_timeout = 0;
static bool app_vcp_event = false;
static bool app_vcp_open = false;
#endif

#ifdef HAL_CONFIG_CUSTOM_LED

// based upon 3MHz for TCC
#define LED_PWM_PERIOD    3000
// percentage of LED brigtness, 100 = Full brightness
#define LED_BRIGHTNESS	  12.5

extern void custom_hal_gpio_dap_status_toggle();
extern void custom_hal_gpio_dap_status_set();
static void led_custom_init();
#ifdef HAL_CONFIG_ENABLE_VCP
extern void custom_hal_gpio_vcp_status_toggle();
extern void custom_hal_gpio_vcp_status_write(int val);
#endif
#endif

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
#ifdef HAL_CONFIG_CUSTOM_LED
static void led_custom_init()
{
#ifdef HAL_BOARD_JEFF_PROBE
  /* Enable the APB clock for TCC0 */
  PM->APBCMASK.reg |= PM_APBCMASK_TCC0;
  /* Enable GCLK1 and wire it up to TCC0 and TCC1. */
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |GCLK_CLKCTRL_ID_TCC0_TCC1 |
                    GCLK_CLKCTRL_GEN(0);
  /* Wait until the clock bus is synchronized. */
  while (GCLK->STATUS.bit.SYNCBUSY) {};

  /* Configure the clock prescaler for each TCC.
     This lets you divide up the clocks frequency to make the TCC count slower
     than the clock. In this case, I'm dividing the 48MHz clock by 16 making the
     TCC operate at 3MHz. This means each count (or "tick") is 0.33us.
  */
  TCC0->CTRLA.reg |= TCC_CTRLA_PRESCALER(TCC_CTRLA_PRESCALER_DIV16_Val);

  TCC0->PER.reg = LED_PWM_PERIOD;
  while (TCC0->SYNCBUSY.bit.PER) {};

  /* Use "Normal PWM" */
  TCC0->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
  /* Wait for bus synchronization */
  while (TCC0->SYNCBUSY.bit.WAVE) {};

  /* n for CC[n] is determined by n = x % 4 where x is from WO[x]
   WO[x] comes from the peripheral multiplexer - we'll get to that in a second.
  */
  // CC3 used by DAP
  TCC0->CC[3].reg = LED_PWM_PERIOD * LED_BRIGHTNESS /100;
  while (TCC0->SYNCBUSY.bit.CC3) {};

  // CC2 used by VCP
#if defined(HAL_CONFIG_ENABLE_VCP)
  TCC0->CC[2].reg = LED_PWM_PERIOD * LED_BRIGHTNESS /100 ;
  while (TCC0->SYNCBUSY.bit.CC2) {};
#endif

  // CC0 used by Power LED
#if defined(HAL_CONFIG_HAS_PWR_STATUS)
  TCC0->CC[0].reg = LED_PWM_PERIOD * LED_BRIGHTNESS /100 ;
  while (TCC0->SYNCBUSY.bit.CC0) {};
#endif

  TCC0->CTRLA.reg |= (TCC_CTRLA_ENABLE);
  while (TCC0->SYNCBUSY.bit.ENABLE) {};

  HAL_GPIO_DAP_STATUS_pmuxen( HAL_GPIO_PMUX_F );
  HAL_GPIO_DAP_STATUS_clr();

#if defined(HAL_CONFIG_ENABLE_VCP)
  /* set alt func */
  HAL_GPIO_VCP_STATUS_pmuxen( HAL_GPIO_PMUX_F );
  HAL_GPIO_VCP_STATUS_clr();
#endif

#if defined(HAL_CONFIG_HAS_PWR_STATUS)
  /* set alt func */
  HAL_GPIO_PWR_STATUS_pmuxen( HAL_GPIO_PMUX_F );
#endif

#endif
}

void custom_hal_gpio_dap_status_toggle()
{
  static int last_value = 0;
  last_value = ! last_value;
  if ( last_value) {
    HAL_GPIO_DAP_STATUS_pmuxen( HAL_GPIO_PMUX_F );
  }else{
      HAL_GPIO_DAP_STATUS_clr();
    HAL_GPIO_DAP_STATUS_pmuxdis();
  }
}
void custom_hal_gpio_dap_status_set()
{
  HAL_GPIO_DAP_STATUS_pmuxen( HAL_GPIO_PMUX_F );
}

#if defined(HAL_CONFIG_ENABLE_VCP)
void custom_hal_gpio_vcp_status_toggle()
{
  static int last_value = 0;
  last_value = ! last_value;
  custom_hal_gpio_vcp_status_write(last_value);
}

void custom_hal_gpio_vcp_status_write(int val)
{
  if ( val ) {
    HAL_GPIO_VCP_STATUS_pmuxen( HAL_GPIO_PMUX_F );
  }else{
      HAL_GPIO_VCP_STATUS_clr();
    HAL_GPIO_VCP_STATUS_pmuxdis();
  }
}

#endif

#if defined(HAL_CONFIG_HAS_PWR_STATUS)
void custom_hal_gpio_pwr_status_set()
{
  HAL_GPIO_PWR_STATUS_pmuxen( HAL_GPIO_PMUX_F );
}
#endif

#endif

static void custom_init(void)
{
#ifdef HAL_CONFIG_CUSTOM_LED
  led_custom_init();
#endif

#ifdef DAP_CONFIG_SUPPLY_PWR
  HAL_GPIO_SUPPLY_PWR_out();
  HAL_GPIO_SUPPLY_PWR_clr();
#endif

#ifdef HAL_CONFIG_HAS_PWR_STATUS
  HAL_GPIO_PWR_STATUS_out();
#ifdef HAL_CONFIG_CUSTOM_LED
  custom_hal_gpio_pwr_status_set();
#else
  HAL_GPIO_PWR_STATUS_set();
#endif
#endif
}

static void sys_init(void)
{
  uint32_t coarse, fine;

  custom_init();

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
}

//-----------------------------------------------------------------------------
static void serial_number_init(void)
{
  uint32_t wuid[4];
  uint8_t *uid = (uint8_t *)wuid;
  uint32_t sn = 5381;

  wuid[0] = *(volatile uint32_t *)0x0080a00c;
  wuid[1] = *(volatile uint32_t *)0x0080a040;
  wuid[2] = *(volatile uint32_t *)0x0080a044;
  wuid[3] = *(volatile uint32_t *)0x0080a048;

  for (int i = 0; i < 16; i++)
    sn = ((sn << 5) + sn) ^ uid[i];

  for (int i = 0; i < 8; i++)
    usb_serial_number[i] = "0123456789ABCDEF"[(sn >> (i * 4)) & 0xf];

  usb_serial_number[8] = 0;
}

//-----------------------------------------------------------------------------
static void sys_time_init(void)
{
  SysTick->VAL  = 0;
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

#ifdef HAL_CONFIG_ENABLE_VCP
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
static void send_buffer(void)
{
  app_send_buffer_free = false;
  app_send_zlp = (USB_BUFFER_SIZE == app_send_buffer_ptr);

  usb_cdc_send(app_send_buffer, app_send_buffer_ptr);

  app_send_buffer_ptr = 0;
}

//-----------------------------------------------------------------------------
static void rx_task(void)
{
  int byte;

  if (!app_send_buffer_free)
    return;

  while (uart_read_byte(&byte))
  {
    int state = (byte >> 8) & 0xff;

    app_uart_timeout = app_system_time + UART_WAIT_TIMEOUT;
    app_vcp_event = true;

    if (state)
    {
      usb_cdc_set_state(state);
    }
    else
    {
      app_send_buffer[app_send_buffer_ptr++] = byte;

      if (USB_BUFFER_SIZE == app_send_buffer_ptr)
      {
        send_buffer();
        break;
      }
    }
  }
}

//-----------------------------------------------------------------------------
static void break_task(void)
{
  if (app_break_timeout && app_system_time > app_break_timeout)
  {
    uart_set_break(false);
    app_break_timeout = 0;
  }
}

//-----------------------------------------------------------------------------
static void uart_timer_task(void)
{
  if (app_uart_timeout && app_system_time > app_uart_timeout)
  {
    if (app_send_zlp || app_send_buffer_ptr)
      send_buffer();

    app_uart_timeout = 0;
  }
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

  app_vcp_open        = status;
  app_send_buffer_ptr = 0;
  app_uart_timeout    = 0;
  app_break_timeout   = 0;

  if (app_vcp_open)
    uart_init(usb_cdc_get_line_coding());
  else
    uart_close();
}

//-----------------------------------------------------------------------------
void usb_cdc_send_break(int duration)
{
  if (USB_CDC_BREAK_DURATION_DISABLE == duration)
  {
    app_break_timeout = 0;
    uart_set_break(false);
  }
  else if (USB_CDC_BREAK_DURATION_INFINITE == duration)
  {
    app_break_timeout = 0;
    uart_set_break(true);
  }
  else
  {
    app_break_timeout = app_system_time + duration;
    uart_set_break(true);
  }
}

//-----------------------------------------------------------------------------
void usb_cdc_send_callback(void)
{
  app_send_buffer_free = true;
}

//-----------------------------------------------------------------------------
void usb_cdc_recv_callback(int size)
{
  app_recv_buffer_ptr = 0;
  app_recv_buffer_size = size;
}
#endif // HAL_CONFIG_ENABLE_VCP

//-----------------------------------------------------------------------------
void usb_hid_send_callback(void)
{
  app_resp_free = true;
}

//-----------------------------------------------------------------------------
void usb_hid_recv_callback(int size)
{
  app_req_buf_hid_size = size;
}

//-----------------------------------------------------------------------------
static void usb_bulk_send_callback(void)
{
  app_resp_free = true;
}

//-----------------------------------------------------------------------------
static void usb_bulk_recv_callback(int size)
{
  app_req_buf_bulk_size = size;
}

//-----------------------------------------------------------------------------
static void dap_task(void)
{
  int interface, size;

  if (!app_resp_free)
    return;

  if (app_req_buf_hid_size)
  {
    interface = USB_INTF_HID;
    size = app_req_buf_hid_size;
    app_req_buf_hid_size = 0;

    memcpy(app_req_buf, app_req_buf_hid, size);

    usb_hid_recv(app_req_buf_hid, sizeof(app_req_buf_hid));
  }
  else if (app_req_buf_bulk_size)
  {
    interface = USB_INTF_BULK;
    size = app_req_buf_bulk_size;
    app_req_buf_bulk_size = 0;

    memcpy(app_req_buf, app_req_buf_bulk, size);

    usb_recv(USB_BULK_EP_RECV, app_req_buf_bulk, sizeof(app_req_buf_bulk));
  }
  else
  {
    return;
  }

  size = dap_process_request(app_req_buf, size, app_resp_buf, sizeof(app_resp_buf));

  if (USB_INTF_BULK == interface)
    usb_send(USB_BULK_EP_SEND, app_resp_buf, size);
  else
    usb_hid_send(app_resp_buf, sizeof(app_resp_buf));

  app_resp_free = false;
  app_dap_event = true;
}

//-----------------------------------------------------------------------------
void usb_configuration_callback(int config)
{
  app_resp_free = true;
  app_req_buf_hid_size = 0;
  app_req_buf_bulk_size = 0;

  usb_set_send_callback(USB_BULK_EP_SEND, usb_bulk_send_callback);
  usb_set_recv_callback(USB_BULK_EP_RECV, usb_bulk_recv_callback);

  usb_hid_recv(app_req_buf_hid, sizeof(app_req_buf_hid));
  usb_recv(USB_BULK_EP_RECV, app_req_buf_bulk, sizeof(app_req_buf_bulk));

#ifdef HAL_CONFIG_ENABLE_VCP
  usb_cdc_recv(app_recv_buffer, sizeof(app_recv_buffer));

  app_send_buffer_free = true;
  app_send_buffer_ptr = 0;
#endif

  (void)config;
}

//-----------------------------------------------------------------------------
static void status_timer_task(void)
{
  if (app_system_time < app_status_timeout)
    return;

  app_status_timeout = app_system_time + STATUS_TIMEOUT;

  if (app_dap_event)
#ifdef HAL_CONFIG_CUSTOM_LED
    custom_hal_gpio_dap_status_toggle();
#else
    HAL_GPIO_DAP_STATUS_toggle();
#endif
  else
#ifdef HAL_CONFIG_CUSTOM_LED
    custom_hal_gpio_dap_status_set();
#else
    HAL_GPIO_DAP_STATUS_set();
#endif

  app_dap_event = false;

#ifdef HAL_CONFIG_ENABLE_VCP
  if (app_vcp_event)
#ifdef HAL_CONFIG_CUSTOM_LED
    custom_hal_gpio_vcp_status_toggle();
#else
    HAL_GPIO_VCP_STATUS_toggle();
#endif
  else
#ifdef HAL_CONFIG_CUSTOM_LED
    custom_hal_gpio_vcp_status_write(app_vcp_open);
#else
    HAL_GPIO_VCP_STATUS_write(app_vcp_open);
#endif

  app_vcp_event = false;
#endif
}

//-----------------------------------------------------------------------------
int main(void)
{
  sys_init();
  sys_time_init();
  dap_init();
  usb_init();
#ifdef HAL_CONFIG_ENABLE_VCP
  usb_cdc_init();
#endif
  usb_hid_init();
  serial_number_init();

  app_status_timeout = STATUS_TIMEOUT;

#ifdef HAL_CONFIG_ENABLE_VCP
  HAL_GPIO_VCP_STATUS_out();
  HAL_GPIO_VCP_STATUS_clr();
#endif

  HAL_GPIO_DAP_STATUS_out();
  HAL_GPIO_DAP_STATUS_set();

  HAL_GPIO_BUTTON_in();
//  HAL_GPIO_BUTTON_pullup(); Jeff Probe has an external pullup

  while (1)
  {
    sys_time_task();
    status_timer_task();
    usb_task();
    dap_task();

#ifdef HAL_CONFIG_ENABLE_VCP
    tx_task();
    rx_task();
    break_task();
    uart_timer_task();
#endif

//    if (0 == HAL_GPIO_BOOT_ENTER_read())
//      NVIC_SystemReset();
  }

  return 0;
}

