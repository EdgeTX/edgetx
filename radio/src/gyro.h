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

#include <inttypes.h>
#include "hal/imu.h"

#define IMU_MAX_DEFAULT       30
#define IMU_MAX_RANGE         60
#define IMU_OFFSET_MIN       -30
#define IMU_OFFSET_MAX        10

void gyroStart(imu_read_fn fn);
void gyroWakeup();
void gyroSetIMU_X(int16_t offset, int16_t range);
void gyroSetIMU_Y(int16_t offset, int16_t range);
int16_t gyroScaledX();
int16_t gyroScaledY();

extern int16_t gyroOutputs[2];

// implemented by the board
imu_read_fn gyroInit();
