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
#include "hal/i2c_driver.h"

struct etx_imu_data_t {
  float accel_x, accel_y, accel_z;
  float gyro_x, gyro_y, gyro_z;
};

typedef int (*imu_init_fn)(etx_i2c_bus_t bus, uint16_t addr);
typedef int (*imu_read_fn)(etx_imu_data_t* data);

struct etx_imu_driver_t {
  imu_init_fn init;
  imu_read_fn read;
  const char* name;
};

struct etx_imu_t {
  const etx_imu_driver_t* driver;
  etx_i2c_bus_t bus;
  uint16_t addr;
};

// Generic detection: iterates candidates, returns read fn and bus on success
imu_read_fn imuDetect(const etx_imu_t* candidates, uint8_t count,
                       etx_i2c_bus_t* detected_bus = nullptr);

// Returns the name of the detected IMU, or nullptr if none
const char* imuGetName();
