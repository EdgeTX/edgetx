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
#include "imu_icm426xx.h"

int gyroInit(void)
{
  TRACE("ICM426xx I2C Init");

  if (i2c_init(IMU_I2C_BUS) < 0) {
    TRACE("ICM426xx ERROR: i2c_init bus error");
    return -1;
  }

  if (i2c_dev_ready(IMU_I2C_BUS, ICM426xx_I2C_ADDR) < 0) {
    TRACE("ICM426xx device init error");
    return -1;
  }

  uint8_t who_am_i = 0;
  if (stm32_i2c_read(IMU_I2C_BUS, ICM426xx_I2C_ADDR, 0x75, 1, &who_am_i, 1, 1000) < 0) {
    TRACE("ICM426xx ERROR: i2c read error");
    return -1;
  }
  TRACE("ICM426xx Who am I: 0x%x", who_am_i);

  return 0;
}

int gyroRead(unsigned char*)
{
  return true;
}