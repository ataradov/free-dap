// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024, Alex Taradov <alex@taradov.com>. All rights reserved.

#ifndef _HAL_CONFIG_H_
#define _HAL_CONFIG_H_

/*- Includes ----------------------------------------------------------------*/
#include "samd21.h"
#include "hal_gpio.h"

/*- Definitions -------------------------------------------------------------*/
#define HAL_BOARD_GENERIC

#if defined(HAL_BOARD_CUSTOM)
  // Externally supplied board configuration takes precedence
  #include HAL_BOARD_CUSTOM

#elif defined(HAL_BOARD_GENERIC)
  #define HAL_CONFIG_ENABLE_VCP
  #define DAP_CONFIG_ENABLE_JTAG

  HAL_GPIO_PIN(SWCLK_TCK,          B, 0)
  HAL_GPIO_PIN(SWDIO_TMS,          B, 1)
  HAL_GPIO_PIN(TDI,                B, 2)
  HAL_GPIO_PIN(TDO,                B, 3)
  HAL_GPIO_PIN(nRESET,             B, 4)

  HAL_GPIO_PIN(VCP_STATUS,         A, 10);
  HAL_GPIO_PIN(DAP_STATUS,         B, 30);
  HAL_GPIO_PIN(BOOT_ENTER,         A, 31);

  HAL_GPIO_PIN(UART_TX,            A, 4);
  HAL_GPIO_PIN(UART_RX,            A, 5);

  #define UART_SERCOM              SERCOM0
  #define UART_SERCOM_PMUX         PORT_PMUX_PMUXE_D_Val
  #define UART_SERCOM_GCLK_ID      SERCOM0_GCLK_ID_CORE
  #define UART_SERCOM_APBCMASK     PM_APBCMASK_SERCOM0
  #define UART_SERCOM_IRQ_INDEX    SERCOM0_IRQn
  #define UART_SERCOM_IRQ_HANDLER  irq_handler_sercom0
  #define UART_SERCOM_TXPO         0 // PAD[0]
  #define UART_SERCOM_RXPO         1 // PAD[1]

#else
  #error No board defined
#endif

#endif // _HAL_CONFIG_H_

