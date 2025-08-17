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

#include "hal/i2c_driver.h"

#define AW9523B_PIN_0 (1 << 0)
#define AW9523B_PIN_1 (1 << 1)
#define AW9523B_PIN_2 (1 << 2)
#define AW9523B_PIN_3 (1 << 3)
#define AW9523B_PIN_4 (1 << 4)
#define AW9523B_PIN_5 (1 << 5)
#define AW9523B_PIN_6 (1 << 6)
#define AW9523B_PIN_7 (1 << 7)
#define AW9523B_PIN_8 (1 << 8)
#define AW9523B_PIN_9 (1 << 9)
#define AW9523B_PIN_10 (1 << 10)
#define AW9523B_PIN_11 (1 << 11)
#define AW9523B_PIN_12 (1 << 12)
#define AW9523B_PIN_13 (1 << 13)
#define AW9523B_PIN_14 (1 << 14)
#define AW9523B_PIN_15 (1 << 15)

typedef struct {
  etx_i2c_bus_t bus;
  uint16_t      addr;
  // uint16_t      interrupt; // unused, the direction config is used to enable interrupts for all input pins
  uint16_t      direction;
  uint16_t      output;
} aw9523b_t;

int aw9523b_init(aw9523b_t* dev, etx_i2c_bus_t bus, uint16_t addr);
int aw9523b_set_direction(aw9523b_t* dev, uint16_t mask, uint16_t dir);
int aw9523b_set_polarity(aw9523b_t* dev, uint16_t mask, uint16_t dir);
int aw9523b_write(aw9523b_t* dev, uint16_t mask, uint16_t value);
int aw9523b_read(aw9523b_t* dev, uint16_t mask, uint16_t* value);
