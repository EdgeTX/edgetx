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
struct imu_cmdpck_t {
  int16_t IMU_TEMP;
  int16_t IMU_ACCEL_X;
  int16_t IMU_ACCEL_Y;
  int16_t IMU_ACCEL_Z;
  int16_t IMU_GYRO_X;
  int16_t IMU_GYRO_Y;
  int16_t IMU_GYRO_Z;
};

extern void IICimu42627init(void);
extern void imu42627Init(void);

extern void GetIMU42627(uint16_t);
extern uint8_t GetIMUID(void);

#define MU42627ID 0x20
#define MU6050ID 0x68

extern float x_angle;
extern float y_angle;

extern float x_Gyro_angle;
extern float y_Gyro_angle;
extern float z_Gyro_angle;

extern bool IMUSTATUS;
extern bool IMURUNFLAG;

extern bool gyroinitflag;
