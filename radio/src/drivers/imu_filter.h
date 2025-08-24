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

// IMU data structure
typedef struct {
  float accel_x, accel_y, accel_z;    // Accelerometer data (m/s²)
  float gyro_x, gyro_y, gyro_z;       // Gyroscope data (rad/s)
  float mag_x, mag_y, mag_z;          // Magnetometer data (optional)
} IMU_RawData_t;

typedef struct {
  float roll, pitch, yaw;             // Filtered attitude (radians)
  float accel_x, accel_y, accel_z;    // Filtered acceleration
  float gyro_x, gyro_y, gyro_z;       // Filtered gyroscope
} IMU_FilteredData_t;

void process_imu_data(IMU_RawData_t *raw_data);
IMU_FilteredData_t* get_filtered_imu_data(void);