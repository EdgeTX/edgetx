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

#include "edgetx.h"
#include "hal/i2c_driver.h"
#include "stm32_i2c_driver.h"
#include "imu_icm42607p.h"

int imu_i2c_read(uint32_t reg, uint8_t regsize, uint8_t * data, uint16_t len, uint32_t timeout)
{
  stm32_i2c_read(IMU_I2C_BUS, ICM42670_I2C_ADDR, reg, regsize, data, len, timeout);
}

bool imu_icm42607p_init(void)
{
  TRACE("ICM62607P I2C Init");

  if (i2c_init(IMU_I2C_BUS) < 0) {
    TRACE("ICM62607P ERROR: i2c_init bus error");
    return false;
  }

  if (i2c_dev_ready(IMU_I2C_BUS, ICM42670_I2C_ADDR) < 0) {
    TRACE("ICM62607P device init error");
    return -1;
  }
}

bool imu_icm42607p_read()
{
  return true;
}