// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2017-2022, Alex Taradov <alex@taradov.com>. All rights reserved.

#ifndef _HAL_CONFIG_H_
#define _HAL_CONFIG_H_

/*- Includes ----------------------------------------------------------------*/
#include "samd21.h"
#include "hal_gpio.h"

/*- Definitions -------------------------------------------------------------*/
//#define HAL_BOARD_STD
#define HAL_BOARD_JEFF_PROBE
//#define HAL_BOARD_VCP_V1
//#define HAL_BOARD_VCP_V3
//#define HAL_BOARD_OBD

#if defined(HAL_BOARD_CUSTOM)
  // Externally supplied board configuration takes precedence
  #include HAL_BOARD_CUSTOM

#elif defined(HAL_BOARD_STD)
  #define DAP_CONFIG_ENABLE_JTAG

  HAL_GPIO_PIN(SWCLK_TCK,          A, 14)
  HAL_GPIO_PIN(SWDIO_TMS,          A, 15)
  HAL_GPIO_PIN(TDO,                A, 9)
  HAL_GPIO_PIN(TDI,                A, 8)
  HAL_GPIO_PIN(nRESET,             A, 5)

  HAL_GPIO_PIN(DAP_STATUS,         A, 4);
  HAL_GPIO_PIN(BOOT_ENTER,         A, 31);

#elif defined(HAL_BOARD_VCP_V1)
  #define HAL_CONFIG_ENABLE_VCP

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

#elif defined(HAL_BOARD_JEFF_PROBE)
  #define HAL_CONFIG_ENABLE_VCP
  #define HAL_CONFIG_HAS_PWR_STATUS
  #define DAP_CONFIG_SUPPLY_PWR
  #define DAP_CONFIG_ENABLE_JTAG
  #define DAP_CONFIG_HAS_TMS_DIR
  #define DAP_CONFIG_RST_ACTIVE_HIGH
  #define DAP_CONFIG_HAS_RST_SENSE

  HAL_GPIO_PIN(SWCLK_TCK,          A, 6)
  HAL_GPIO_PIN(SWDIO_TMS,          A, 0)
  HAL_GPIO_PIN(SWDIO_TMS_DIR,      A, 15)
  HAL_GPIO_PIN(TDI,                A, 16)
  HAL_GPIO_PIN(TDO,                A, 19)
  HAL_GPIO_PIN(nRESET,             A, 8)
  HAL_GPIO_PIN(nRESET_SENSE,       A, 9)
  HAL_GPIO_PIN(SUPPLY_PWR,         A, 28);

  HAL_GPIO_PIN(VCP_STATUS,         A, 10);
  HAL_GPIO_PIN(DAP_STATUS,         A, 11);
  HAL_GPIO_PIN(PWR_STATUS,         A, 14);
  HAL_GPIO_PIN(BOOT_ENTER,         A, 31);

  HAL_GPIO_PIN(UART_TX,            A, 4);
  HAL_GPIO_PIN(UART_RX,            A, 7);

  #define UART_SERCOM              SERCOM0
  #define UART_SERCOM_PMUX         PORT_PMUX_PMUXE_D_Val
  #define UART_SERCOM_GCLK_ID      SERCOM0_GCLK_ID_CORE
  #define UART_SERCOM_APBCMASK     PM_APBCMASK_SERCOM0
  #define UART_SERCOM_IRQ_INDEX    SERCOM0_IRQn
  #define UART_SERCOM_IRQ_HANDLER  irq_handler_sercom0
  #define UART_SERCOM_TXPO         0
  #define UART_SERCOM_RXPO         3

#elif defined(HAL_BOARD_OBD)
  #define HAL_CONFIG_ENABLE_VCP

  HAL_GPIO_PIN(SWCLK_TCK,          A, 9)
  HAL_GPIO_PIN(SWDIO_TMS,          A, 14)
  HAL_GPIO_PIN(nRESET,             A, 15)

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

#else
  #error No board defined
#endif

#endif // _HAL_CONFIG_H_
