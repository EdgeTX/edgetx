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

#include "hal/i2c_driver.h"

typedef struct {
  etx_i2c_bus_t bus;
  uint16_t      addr;
} wm8904_t;

int wm8904_probe(wm8904_t* dev, etx_i2c_bus_t bus, uint16_t addr);
int wm8904_init(wm8904_t* dev);

int wm8904_write_reg(wm8904_t* dev, uint8_t reg, uint16_t data);
int wm8904_read_reg(wm8904_t* dev, uint8_t reg, uint16_t* value);
