/*
 * Copyright (c) 2016, Alex Taradov <alex@taradov.com>
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

#ifndef _DAP_CONFIG_H_
#define _DAP_CONFIG_H_

/*- Includes ----------------------------------------------------------------*/
#include "samd21.h"
#include "hal_gpio.h"

/*- Definitions -------------------------------------------------------------*/
HAL_GPIO_PIN(SWCLK_TCK,    B, 11)
HAL_GPIO_PIN(SWDIO_TMS,    B, 17)
HAL_GPIO_PIN(TDI,          A, 8)
HAL_GPIO_PIN(TDO,          A, 9)
HAL_GPIO_PIN(nTRST,        B, 23)
HAL_GPIO_PIN(nRESET,       A, 2)

HAL_GPIO_PIN(LED,          B, 30)

#define DAP_CONFIG_ENABLE_JTAG

#define DAP_CONFIG_DEFAULT_PORT        DAP_PORT_SWD
#define DAP_CONFIG_DEFAULT_CLOCK       1000000 // Hz

#define DAP_CONFIG_PACKET_SIZE         64
#define DAP_CONFIG_PACKET_COUNT        1

#define DAP_CONFIG_JTAG_DEV_COUNT      8

// Set the value to NULL if you want to disable a string
// DAP_CONFIG_PRODUCT_STR must contain "CMSIS-DAP" to be compatible with the standard
#define DAP_CONFIG_VENDOR_STR          "Alex Taradov"
#define DAP_CONFIG_PRODUCT_STR         "Generic CMSIS-DAP Adapter"
#define DAP_CONFIG_SER_NUM_STR         "123456"
#define DAP_CONFIG_FW_VER_STR          "v0.1"
#define DAP_CONFIG_DEVICE_VENDOR_STR   NULL
#define DAP_CONFIG_DEVICE_NAME_STR     NULL

//#define DAP_CONFIG_RESET_TARGET_FN     target_specific_reset_function
//#define DAP_CONFIG_VENDOR_FN           vendor_command_handler_function

// Attribute to use for performance-critical functions
#define DAP_CONFIG_PERFORMANCE_ATTR    __attribute__((section(".ramfunc")))

// A value at which dap_clock_test() produces 1 kHz output on the SWCLK pin
#define DAP_CONFIG_DELAY_CONSTANT      4700

// A threshold for switching to fast clock (no added delays)
// This is the frequency produced by dap_clock_test(1) on the SWCLK pin
#define DAP_CONFIG_FAST_CLOCK          3600000 // Hz

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_write(int value)
{
  HAL_GPIO_SWCLK_TCK_write(value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_write(int value)
{
  HAL_GPIO_SWDIO_TMS_write(value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_TDI_write(int value)
{
  HAL_GPIO_TDI_write(value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_TDO_write(int value)
{
  HAL_GPIO_TDO_write(value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_nTRST_write(int value)
{
  (void)value;
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_nRESET_write(int value)
{
  HAL_GPIO_nRESET_write(value);
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWCLK_TCK_read(void)
{
  return HAL_GPIO_SWCLK_TCK_read();
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWDIO_TMS_read(void)
{
  return HAL_GPIO_SWDIO_TMS_read();
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_TDO_read(void)
{
  return HAL_GPIO_TDO_read();
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_TDI_read(void)
{
  return HAL_GPIO_TDI_read();
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_nTRST_read(void)
{
  return 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_nRESET_read(void)
{
  return HAL_GPIO_nRESET_read();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_set(void)
{
  HAL_GPIO_SWCLK_TCK_set();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_clr(void)
{
  HAL_GPIO_SWCLK_TCK_clr();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_in(void)
{
  HAL_GPIO_SWDIO_TMS_in();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_out(void)
{
  HAL_GPIO_SWDIO_TMS_out();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SETUP(void)
{
  HAL_GPIO_SWCLK_TCK_in();
  HAL_GPIO_SWDIO_TMS_in();
  HAL_GPIO_nRESET_in();
  HAL_GPIO_TDO_in();
  HAL_GPIO_TDI_in();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_DISCONNECT(void)
{
  HAL_GPIO_SWCLK_TCK_in();
  HAL_GPIO_SWDIO_TMS_in();
  HAL_GPIO_nRESET_in();
  HAL_GPIO_TDO_in();
  HAL_GPIO_TDI_in();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_SWD(void)
{
  HAL_GPIO_SWDIO_TMS_out();
  HAL_GPIO_SWDIO_TMS_set();

  HAL_GPIO_SWCLK_TCK_out();
  HAL_GPIO_SWCLK_TCK_set();

  HAL_GPIO_nRESET_out();
  HAL_GPIO_nRESET_set();

  HAL_GPIO_TDO_in();
  HAL_GPIO_TDI_in();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_JTAG(void)
{
  HAL_GPIO_SWDIO_TMS_out();
  HAL_GPIO_SWDIO_TMS_set();

  HAL_GPIO_SWCLK_TCK_out();
  HAL_GPIO_SWCLK_TCK_set();

  HAL_GPIO_TDO_in();

  HAL_GPIO_TDI_out();
  HAL_GPIO_TDI_set();

  HAL_GPIO_nRESET_out();
  HAL_GPIO_nRESET_set();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_LED(int index, int state)
{
  if (0 == index)
    HAL_GPIO_LED_write(!state);
}

//-----------------------------------------------------------------------------
__attribute__((always_inline))
static inline void DAP_CONFIG_DELAY(uint32_t cycles)
{
  asm volatile (
    "1: sub %[cycles], %[cycles], #1 \n"
    "   bne 1b \n"
    : [cycles] "+l"(cycles)
  );
}

#endif // _DAP_CONFIG_H_

