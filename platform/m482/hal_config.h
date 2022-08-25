// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022, Alex Taradov <alex@taradov.com>. All rights reserved.

#ifndef _HAL_CONFIG_H_
#define _HAL_CONFIG_H_

/*- Includes ----------------------------------------------------------------*/
#include "M480.h"
#include "hal_gpio.h"

/*- Definitions -------------------------------------------------------------*/
//#define HAL_BOARD_GENERIC
#define HAL_BOARD_M482_DAP

HAL_GPIO_PIN(SWCLK_TCK,      B, 0)
HAL_GPIO_PIN(SWDIO_TMS,      B, 1)
HAL_GPIO_PIN(TDI,            B, 2)
HAL_GPIO_PIN(TDO,            B, 3)
HAL_GPIO_PIN(nRESET,         B, 4)
HAL_GPIO_PIN(DAP_STATUS,     A, 15);
HAL_GPIO_PIN(BOOT_ENTER,     A, 3);

#endif // _HAL_CONFIG_H_

