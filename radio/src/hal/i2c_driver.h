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

#include <stdint.h>

typedef uint8_t etx_i2c_bus_t;

int i2c_init(etx_i2c_bus_t bus);
int i2c_deinit(etx_i2c_bus_t bus);

int i2c_dev_ready(etx_i2c_bus_t bus, uint16_t addr);

int i2c_read(uint8_t bus, uint16_t addr, uint16_t reg, uint16_t reg_size,
              uint8_t* data, uint16_t len);

int i2c_write(uint8_t bus, uint16_t addr, uint16_t reg, uint16_t reg_size,
              uint8_t* data, uint16_t len);

