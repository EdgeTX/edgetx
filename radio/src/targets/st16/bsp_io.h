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

// Input port expander
#define BSP_TR1U PCA95XX_PIN_8
#define BSP_TR1D PCA95XX_PIN_9
#define BSP_TR3L PCA95XX_PIN_10
#define BSP_TR3R PCA95XX_PIN_11
#define BSP_TR2U PCA95XX_PIN_12
#define BSP_TR2D PCA95XX_PIN_13
#define BSP_TR4L PCA95XX_PIN_14
#define BSP_TR4R PCA95XX_PIN_15

// Output port expander
#define BSP_AUDIO_RST PCA95XX_PIN_0
#define BSP_PA_NMUTE  PCA95XX_PIN_1
#define BSP_INT_PWR   PCA95XX_PIN_2
#define BSP_EXT_PWR   PCA95XX_PIN_3
#define BSP_CHARGE_EN PCA95XX_PIN_4
#define BSP_LCD_NRST  PCA95XX_PIN_5
#define BSP_USB_SW    PCA95XX_PIN_6
#define BSP_PWR_LED   PCA95XX_PIN_7
#define BSP_LCD_CS    PCA95XX_PIN_8
#define BSP_RF_BOOT0  PCA95XX_PIN_9
#define BSP_USB_SWB   PCA95XX_PIN_10

int bsp_io_init();

void bsp_output_set(uint16_t pin);
void bsp_output_clear(uint16_t pin);

uint16_t bsp_input_get();
