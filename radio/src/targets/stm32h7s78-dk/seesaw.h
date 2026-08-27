/*
 * Copyright (C) EdgeTX
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
  // uint16_t      polarity;
  // uint16_t      direction;
  // uint16_t      output;
} seesaw_t;

enum seesaw_input_mode {
  SEESAW_INPUT,
  SEESAW_OUTPUT,
  SEESAW_INPUT_PULLUP,
  SEESAW_INPUT_PULLDOWN,
};

int seesaw_init(seesaw_t* dev, etx_i2c_bus_t bus, uint16_t addr);
int seesaw_pin_mode(seesaw_t* dev, uint32_t mask, seesaw_input_mode mode);
int seesaw_digital_read(seesaw_t* dev, uint32_t pins, uint32_t* value);
