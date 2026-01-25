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

#define IO_EXPANDER_I2C_BUS     I2C_Bus_1
#define IO_EXPANDER1_I2C_ADDR	0x74
#define IO_EXPANDER2_I2C_ADDR	0x75

// Port expander 2 (0x74)
#define BSP_TR6U PCA95XX_PIN_7
#define BSP_TR6D PCA95XX_PIN_6
#define BSP_TR4R PCA95XX_PIN_8
#define BSP_TR4L PCA95XX_PIN_9
#define BSP_TR2D PCA95XX_PIN_10
#define BSP_TR2U PCA95XX_PIN_11
#define BSP_TR1D PCA95XX_PIN_12
#define BSP_TR1U PCA95XX_PIN_13
#define BSP_TR3R PCA95XX_PIN_14
#define BSP_TR3L PCA95XX_PIN_15

extern uint8_t isSwitch3Pos(uint8_t idx);

int bsp_io_init();
uint32_t bsp_io_read_switches();
uint32_t bsp_io_read_fs_switches();
uint32_t bsp_get_fs_switches();

uint16_t bsp_input_get();
struct stm32_switch_t;

//TODO compute from json
constexpr uint32_t IO_EXPANDER1_MASK = 0xFFFF;
constexpr uint32_t IO_EXPANDER2_MASK = 0xFFFF;
