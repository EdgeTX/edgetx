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

#include <stdint.h>

enum { I2C_Bus_1 = 0, I2C_Bus_2 };

// Init I2C bus
// @return -1 if error, 0 if already running, 1 otherwise
int stm32_i2c_init(uint8_t bus, uint32_t clock_rate);

// DeInit I2C bus
// @return -1 if error, 0 otherwise
int stm32_i2c_deinit(uint8_t bus);

// Transmit in master mode (blocking mode)
// @return -1 if error, 0 otherwise
int stm32_i2c_master_tx(uint8_t bus, uint16_t addr, uint8_t *data, uint16_t len,
                        uint32_t timeout);

// Receive in master mode (blocking mode)
// @return -1 if error, 0 otherwise
int stm32_i2c_master_rx(uint8_t bus, uint16_t addr, uint8_t *data, uint16_t len,
                        uint32_t timeout);
