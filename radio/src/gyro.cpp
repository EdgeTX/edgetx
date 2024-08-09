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
#define _USE_MATH_DEFINES
#include <math.h>

#define ACC_LSB_VALUE	0.000488  // 0.488 mg/LSB

#define ALPHA 0.98
#define DT    0.01
#define SCALE_FACT_ACC  0.005
#define SCALE_FACT_GYRO 0.0078 // 250deg/s / 32000

Gyro gyro;

static float deg2RESX(float deg)
{
  // [-90 : 90] -> [-RESX : RESX]
  return (deg * float(RESX)) / 90.0;
}

void Gyro::wakeup()
{
  static tmr10ms_t gyroWakeupTime = 0;

  tmr10ms_t now = get_tmr10ms();
  if (errors >= 100 || now < gyroWakeupTime)
    return;

  gyroWakeupTime = now + 1; /* 10ms default */

  int16_t values[IMU_VALUES_COUNT];
  if (gyroRead((uint8_t*)values) < 0) {
    ++errors;
    return;
  }

  // reset error count on each
  // successful query to avoid
  // stopping the sensor forever
  errors = 0;

  int16_t gx = values[0];
  int16_t gy = values[1];
  // int16_t gz = values[2];

  int16_t ax = values[3];
  int16_t ay = values[4];
  int16_t az = values[5];

  // integrate gyro
  roll  -= gx * SCALE_FACT_GYRO * DT;
  pitch += gy * SCALE_FACT_GYRO * DT;

  int32_t magn = abs(ax) + abs(ay) + abs(az);
  if (magn > 8192 && magn < 32768) {

    if (az < 0) az = -az;
        
    float rollAcc  = atan2f((float)ay,(float)az) * 57.3 /* 180/PI */;
    float pitchAcc = atan2f((float)ax,(float)az) * 57.3 /* 180/PI */;

    roll  = roll  * ALPHA + rollAcc  * (1.0-ALPHA);
    pitch = pitch * ALPHA + pitchAcc * (1.0-ALPHA);
  }

  outputs[0] = deg2RESX(roll);
  outputs[1] = deg2RESX(pitch);
}

int16_t Gyro::scaledX()
{
  // return limit(-RESX,
  //              (int)(outputs[0] - g_eeGeneral.imuOffset * RESX / 180) *
  //                  (180 / (IMU_MAX_DEFAULT + g_eeGeneral.imuMax)),
  //              RESX);
  return limit<int16_t>(-RESX, outputs[0], RESX);
}

int16_t Gyro::scaledY()
{
  // return limit(-RESX,
  //              outputs[1] * (180 / (IMU_MAX_DEFAULT + g_eeGeneral.imuMax)),
  //              RESX);
  return limit<int16_t>(-RESX, outputs[1], RESX);
}
