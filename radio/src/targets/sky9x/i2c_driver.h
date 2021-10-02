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

#ifndef _I2C_DRIVER_H_
#define _I2C_DRIVER_H_

#include "stdint.h"

uint32_t fromBCD(uint8_t bcd_value);
uint8_t toBCD(uint32_t value);

bool i2cInit();
uint32_t i2cReadBuffer(uint8_t chip, uint8_t *addr, uint8_t addr_len, uint8_t *buffer, uint32_t length);
uint32_t i2cWriteBuffer(uint8_t chip, uint8_t *addr, uint8_t addr_len, uint8_t *buffer, uint32_t length);

#endif
