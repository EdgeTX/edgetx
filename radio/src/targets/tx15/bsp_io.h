/*
 * Copyright (C) EdgeTx
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
#include "hal/switch_driver.h"

// Port expander 1 (0x74)
#define SWITCH_F_H PCA95XX_PIN_4
#define SWITCH_F_L PCA95XX_PIN_5

#define SWITCH_D_L PCA95XX_PIN_6
#define SWITCH_D_H PCA95XX_PIN_7

#define SWITCH_C_L PCA95XX_PIN_8
#define SWITCH_C_H PCA95XX_PIN_9

#define SWITCH_B_L PCA95XX_PIN_10
#define SWITCH_B_H PCA95XX_PIN_11

#define SWITCH_A_L PCA95XX_PIN_12
#define SWITCH_A_H PCA95XX_PIN_13

#define SWITCH_E_L PCA95XX_PIN_15
#define SWITCH_E_H PCA95XX_PIN_14


// Port expander 2 (0x75)
#define BSP_TR1U PCA95XX_PIN_13
#define BSP_TR1D PCA95XX_PIN_12

#define BSP_TR2U PCA95XX_PIN_11
#define BSP_TR2D PCA95XX_PIN_10

#define BSP_TR3L PCA95XX_PIN_15
#define BSP_TR3R PCA95XX_PIN_14

#define BSP_TR4L PCA95XX_PIN_9
#define BSP_TR4R PCA95XX_PIN_8

#define FSSWITCH_1 PCA95XX_PIN_0
#define FSSWITCH_2 PCA95XX_PIN_1
#define FSSWITCH_3 PCA95XX_PIN_2
#define FSSWITCH_4 PCA95XX_PIN_3
#define FSSWITCH_5 PCA95XX_PIN_4
#define FSSWITCH_6 PCA95XX_PIN_5

#define IO_INT_GPIO GPIO_PIN(GPIOD, 3)
#define IO_RESET_GPIO GPIO_PIN(GPIOG, 10)

int bsp_io_init();
uint32_t bsp_io_read_switches();
uint32_t bsp_io_read_fs_switches();

uint16_t bsp_input_get();
struct stm32_switch_t;
SwitchHwPos bsp_get_switch_position(const stm32_switch_t *sw, SwitchCategory cat, uint8_t idx);

