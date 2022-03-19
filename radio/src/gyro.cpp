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

#include "opentx.h"
#define _USE_MATH_DEFINES
#include <math.h>

#define ACC_LSB_VALUE	0.000488  // 0.488 mg/LSB

Gyro gyro;

int GyroBuffer::read(int32_t values[IMU_VALUES_COUNT])
{
  index = (index + 1) & (IMU_SAMPLES_COUNT - 1);

  for (uint8_t i = 0; i < IMU_VALUES_COUNT; i++) {
    sums[i] -= samples[index].values[i];
  }

  if (gyroRead(samples[index].raw) < 0)
    return -1;

  for (uint8_t i = 0; i < IMU_VALUES_COUNT; i++) {
    sums[i] += samples[index].values[i];
    values[i] = sums[i] >> IMU_SAMPLES_EXPONENT;
  }

  return 0;
}

float rad2RESX(float rad)
{
  return (rad * float(RESX)) / M_PI;
}

void Gyro::wakeup()
{
  static tmr10ms_t gyroWakeupTime = 0;

  tmr10ms_t now = get_tmr10ms();
  if (errors >= 100 || now < gyroWakeupTime)
    return;

  gyroWakeupTime = now + 1; /* 10ms default */

  int32_t values[IMU_VALUES_COUNT];
  if (gyroBuffer.read(values) < 0)
    ++errors;

  float accValues[3]; // m^2 / s
  accValues[0] = -9.81 * float(values[3]) * ACC_LSB_VALUE;
  accValues[1] = 9.81 * float(values[4]) * ACC_LSB_VALUE;
  accValues[2] = 9.81 * float(values[5]) * ACC_LSB_VALUE;

  outputs[0] = rad2RESX(atan2f(accValues[1], accValues[2]));
  outputs[1] = rad2RESX(atan2f(-accValues[0], accValues[2]));

  // TRACE("%d %d", values[0], values[1]);
}

int16_t Gyro::scaledX()
{
  return limit(-RESX,
               (int)(outputs[0] - g_eeGeneral.imuOffset * RESX / 180) *
                   (180 / (IMU_MAX_DEFAULT + g_eeGeneral.imuMax)),
               RESX);
}

int16_t Gyro::scaledY()
{
  return limit(-RESX,
               outputs[1] * (180 / (IMU_MAX_DEFAULT + g_eeGeneral.imuMax)),
               RESX);
}
