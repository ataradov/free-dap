/*
 * Copyright (c) 2021, Alex Taradov <alex@taradov.com>
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

#ifndef _HAL_CONFIG_H_
#define _HAL_CONFIG_H_

/*- Includes ----------------------------------------------------------------*/
#include "hal_gpio.h"

/*- Definitions -------------------------------------------------------------*/
#define HAL_BOARD_V1
//#define HAL_BOARD_V3

#if defined(HAL_BOARD_V1)
  HAL_GPIO_PIN(SWCLK_TCK,          A, 14)
  HAL_GPIO_PIN(SWDIO_TMS,          A, 15)
  HAL_GPIO_PIN(nRESET,             A, 9)

  HAL_GPIO_PIN(VCP_STATUS,         A, 2);
  HAL_GPIO_PIN(DAP_STATUS,         A, 4);
  HAL_GPIO_PIN(BOOT_ENTER,         A, 31);

  HAL_GPIO_PIN(UART_TX,            A, 8);
  HAL_GPIO_PIN(UART_RX,            A, 5);

  #define UART_SERCOM              SERCOM0
  #define UART_SERCOM_PMUX         PORT_PMUX_PMUXE_D_Val
  #define UART_SERCOM_GCLK_ID      SERCOM0_GCLK_ID_CORE
  #define UART_SERCOM_APBCMASK     PM_APBCMASK_SERCOM0
  #define UART_SERCOM_IRQ_INDEX    SERCOM0_IRQn
  #define UART_SERCOM_IRQ_HANDLER  irq_handler_sercom0
  #define UART_SERCOM_TXPO         1
  #define UART_SERCOM_RXPO         1

#elif defined(HAL_BOARD_V3)
  #define DAP_CONFIG_ENABLE_JTAG

  HAL_GPIO_PIN(SWCLK_TCK,          A, 9)
  HAL_GPIO_PIN(SWDIO_TMS,          A, 8)
  HAL_GPIO_PIN(TDI,                A, 14)
  HAL_GPIO_PIN(TDO,                A, 10)
  HAL_GPIO_PIN(nRESET,             A, 15)

  HAL_GPIO_PIN(VCP_STATUS,         A, 3);
  HAL_GPIO_PIN(DAP_STATUS,         A, 6);
  HAL_GPIO_PIN(BOOT_ENTER,         A, 31);

  HAL_GPIO_PIN(UART_TX,            A, 16);
  HAL_GPIO_PIN(UART_RX,            A, 17);

  #define UART_SERCOM              SERCOM1
  #define UART_SERCOM_PMUX         PORT_PMUX_PMUXE_C_Val
  #define UART_SERCOM_GCLK_ID      SERCOM1_GCLK_ID_CORE
  #define UART_SERCOM_APBCMASK     PM_APBCMASK_SERCOM1
  #define UART_SERCOM_IRQ_INDEX    SERCOM1_IRQn
  #define UART_SERCOM_IRQ_HANDLER  irq_handler_sercom1
  #define UART_SERCOM_TXPO         1
  #define UART_SERCOM_RXPO         3
#else
  #error No board defined
#endif

#endif // _HAL_CONFIG_H_

