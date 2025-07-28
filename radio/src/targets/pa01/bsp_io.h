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

#include "drivers/aw9523b.h"
#include "stm32_switch_driver.h"
#include "hal/switch_driver.h"

bool getFctKeyState(int index);

// Input port expander
#define BSP_KEY_IN1         AW9523B_PIN_0
#define BSP_KEY_IN2         AW9523B_PIN_1
#define BSP_KEY_IN3         AW9523B_PIN_2
#define BSP_KEY_IN4         AW9523B_PIN_3

// Output port expander
#define BSP_INTMOD_PWR_EN   AW9523B_PIN_7
#define BSP_CHARGE_EN       AW9523B_PIN_8
#define BSP_INTMOD_BOOTCMD  AW9523B_PIN_9
#define BSP_KEY_OUT1       AW9523B_PIN_10
#define BSP_KEY_OUT2       AW9523B_PIN_11
#define BSP_KEY_OUT3       AW9523B_PIN_12
#define BSP_KEY_OUT4       AW9523B_PIN_13
#define BSP_U6_SELECT      AW9523B_PIN_14
#define BSP_EXTMOD_PWR_EN  AW9523B_PIN_15

int bsp_io_init();
bool bsp_get_shouldReadKeys();


void bsp_output_set(uint16_t pin);
void bsp_output_set(uint16_t mask, uint16_t pin);
void bsp_output_clear(uint16_t pin);

uint16_t bsp_input_get();
