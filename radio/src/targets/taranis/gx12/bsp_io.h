/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#pragma once

#include "drivers/pca95xx.h"

// Port expander 1 (0x74)
#define SWITCH_H PCA95XX_PIN_0
#define SWITCH_D PCA95XX_PIN_1

#define SWITCH_F_H PCA95XX_PIN_3
#define SWITCH_F_L PCA95XX_PIN_2

#define SWITCH_C_L PCA95XX_PIN_4
#define SWITCH_C_H PCA95XX_PIN_5

#define SWITCH_B_H PCA95XX_PIN_10
#define SWITCH_B_L PCA95XX_PIN_11

#define SWITCH_E_H PCA95XX_PIN_12
#define SWITCH_E_L PCA95XX_PIN_13

#define SWITCH_A PCA95XX_PIN_14
#define SWITCH_G PCA95XX_PIN_15

#define IO_INT_GPIO GPIO_PIN(GPIOE, 14)
#define IO_RESET_GPIO GPIO_PIN(GPIOE, 15)

// Port expander 1 (0x75)
#define FSSWITCH_1 PCA95XX_PIN_0
#define FSSWITCH_2 PCA95XX_PIN_1
#define FSSWITCH_3 PCA95XX_PIN_2
#define FSSWITCH_4 PCA95XX_PIN_3
#define FSSWITCH_5 PCA95XX_PIN_4
#define FSSWITCH_6 PCA95XX_PIN_5


int bsp_io_init();
uint32_t bsp_io_read_switches();
uint32_t bsp_io_read_fs_switches();
