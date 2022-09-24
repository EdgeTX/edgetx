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

#include <inttypes.h>
#include "myeeprom.h"

#define IMU_VALUES_COUNT      6
#define IMU_BUFFER_LENGTH     (IMU_VALUES_COUNT * sizeof(int16_t))

#define IMU_MAX_DEFAULT       30
#define IMU_MAX_RANGE         60
#define IMU_OFFSET_MIN       -30
#define IMU_OFFSET_MAX        10

#define IMU_SAMPLES_EXPONENT  3
#define IMU_SAMPLES_COUNT     (2 ^ IMU_SAMPLES_EXPONENT)

class Gyro
{
 protected:
  uint8_t errors = 0;
  float roll = 0.0;
  float pitch = 0.0;

 public:
  int16_t outputs[2];

  void wakeup();

  int16_t scaledX();
  int16_t scaledY();
};

extern Gyro gyro;

// Gyro driver
int gyroInit();
int gyroRead(uint8_t buffer[IMU_BUFFER_LENGTH]);
