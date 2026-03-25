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
#include "hal.h"
#include "hal/usb_driver.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define COMPLEMENTARY_ALPHA  0.92f
#define LP_FILTER_ALPHA      0.9f
#define SAMPLE_TIME_S        0.005f

int16_t gyroOutputs[2];

static imu_read_fn readFn;
static uint8_t errors;
static int16_t offset_x, offset_y;
static int16_t range_x = 8192, range_y = 8192;
static int16_t raw_ax, raw_ay;

// filter state
static bool filterInitialized;
static float roll, pitch, yaw;
static float filteredAccel[3];
static float prevAccel[3];
static float filteredGyro[3];
static float prevGyro[3];

static float lowPass(float val, float prev, float alpha)
{
  return alpha * prev + (1.0f - alpha) * val;
}

static void gyroFilter(etx_imu_data_t* raw)
{
  // low-pass
  filteredAccel[0] = lowPass(raw->accel_x, prevAccel[0], LP_FILTER_ALPHA);
  filteredAccel[1] = lowPass(raw->accel_y, prevAccel[1], LP_FILTER_ALPHA);
  filteredAccel[2] = lowPass(raw->accel_z, prevAccel[2], LP_FILTER_ALPHA);

  filteredGyro[0] = lowPass(raw->gyro_x, prevGyro[0], LP_FILTER_ALPHA);
  filteredGyro[1] = lowPass(raw->gyro_y, prevGyro[1], LP_FILTER_ALPHA);
  filteredGyro[2] = lowPass(raw->gyro_z, prevGyro[2], LP_FILTER_ALPHA);

  prevAccel[0] = filteredAccel[0];
  prevAccel[1] = filteredAccel[1];
  prevAccel[2] = filteredAccel[2];

  prevGyro[0] = filteredGyro[0];
  prevGyro[1] = filteredGyro[1];
  prevGyro[2] = filteredGyro[2];

  // complementary filter for attitude
  float ax = raw->accel_x;
  float ay = raw->accel_y;
  float az = raw->accel_z;

  float norm = sqrtf(ax * ax + ay * ay + az * az);
  if (norm == 0.0f) return;
  ax /= norm;
  ay /= norm;
  az /= norm;

  float accelRoll  = atan2f(ay, sqrtf(ax * ax + az * az));
  float accelPitch = atan2f(-ax, sqrtf(ay * ay + az * az));

  if (!filterInitialized) {
    roll  = accelRoll;
    pitch = accelPitch;
    yaw   = 0.0f;
    filterInitialized = true;
  } else {
    float dt = SAMPLE_TIME_S;
    roll  = COMPLEMENTARY_ALPHA * (roll  + raw->gyro_x * dt) +
            (1.0f - COMPLEMENTARY_ALPHA) * accelRoll;
    pitch = COMPLEMENTARY_ALPHA * (pitch + raw->gyro_y * dt) +
            (1.0f - COMPLEMENTARY_ALPHA) * accelPitch;
    yaw  += raw->gyro_z * dt;

    if (yaw > (float)M_PI) yaw -= 2.0f * (float)M_PI;
    if (yaw < -(float)M_PI) yaw += 2.0f * (float)M_PI;
  }
}

void gyroStart(imu_read_fn fn)
{
  readFn = fn;
}

void gyroWakeup()
{
  static tmr10ms_t gyroWakeupTime = 0;

  tmr10ms_t now = get_tmr10ms();
  if (!readFn || errors >= 100 || now < gyroWakeupTime ||
      usbPluggedInStorageMode())
    return;

  gyroWakeupTime = now + 1; /* 10ms default */

  etx_imu_data_t raw;
  if (readFn(&raw) < 0) {
    ++errors;
    return;
  }

  errors = 0;

  gyroFilter(&raw);

  raw_ax = (int16_t)filteredAccel[0];
  raw_ay = (int16_t)filteredAccel[1];

  int16_t ax = raw_ax - offset_x;
  int16_t ay = raw_ay - offset_y;

  gyroOutputs[0] = (ax * float(RESX)) / range_x;
  gyroOutputs[1] = (ay * float(RESX)) / range_y;
}

int16_t gyroScaledX()
{
  return limit<int16_t>(-RESX, gyroOutputs[0], RESX);
}

int16_t gyroScaledY()
{
  return limit<int16_t>(-RESX, gyroOutputs[1], RESX);
}

void gyroSetIMU_X(int16_t offset, int16_t range)
{
  if (offset == -1) offset_x = raw_ax;
  else offset_x = offset;
  range_x = range * 8192 / 180;
}

void gyroSetIMU_Y(int16_t offset, int16_t range)
{
  if (offset == -1) offset_y = raw_ay;
  else offset_y = offset;
  range_y = range * 8192 / 180;
}
