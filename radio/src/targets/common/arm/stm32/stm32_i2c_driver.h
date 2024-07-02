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

#include "stm32_cmsis.h"
#include "stm32_gpio.h"
#include "hal/gpio.h"

#include <stdint.h>

// Maxmimum 2 I2C bus instances
enum { I2C_Bus_1 = 0, I2C_Bus_2 };

typedef struct {
  I2C_TypeDef* I2Cx;
  gpio_t       SCL_GPIO;
  gpio_t       SDA_GPIO;
  uint32_t     GPIO_AF;
  void (*set_pwr)(bool enable);
} stm32_i2c_hw_def_t;

// Init I2C bus
// @return -1 if error, 0 if already running, 1 otherwise
int stm32_i2c_init(uint8_t bus, uint32_t clock_rate,
                   const stm32_i2c_hw_def_t* hw_def);

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

// Read memory from address 'reg' (blocking mode)
int stm32_i2c_read(uint8_t bus, uint16_t addr, uint16_t reg, uint16_t reg_size,
                   uint8_t* data, uint16_t len, uint32_t timeout);

// Write memory from address 'reg' (blocking mode)
int stm32_i2c_write(uint8_t bus, uint16_t addr, uint16_t reg, uint16_t reg_size,
                    uint8_t* data, uint16_t len, uint32_t timeout);

int stm32_i2c_is_dev_ready(uint8_t bus, uint16_t addr, uint32_t retries, uint32_t timeout);
int stm32_i2c_is_dev_ready(uint8_t bus, uint16_t addr, uint32_t timeout);

