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

#define GYRO_SAMPLES_EXPONENT  3
#define GYRO_SAMPLES_COUNT     (2 ^ GYRO_SAMPLES_EXPONENT)

class GyroBuffer {
  protected:
    union {
      int16_t values[GYRO_VALUES_COUNT];
      uint8_t raw[GYRO_BUFFER_LENGTH];
    } samples[GYRO_SAMPLES_COUNT];

    int32_t sums[GYRO_VALUES_COUNT];

    uint8_t index;

  public:
    int read(int32_t values[GYRO_VALUES_COUNT]);
};

class Gyro {
  protected:
    GyroBuffer gyroBuffer;
    uint8_t errors = 0;

  public:
    int16_t outputs[2];
    void wakeup();

    int16_t scaledX()
    {
      return limit(-RESX, (int)(outputs[0] - g_eeGeneral.gyroOffset * RESX/180) * (180 / (GYRO_MAX_DEFAULT + g_eeGeneral.gyroMax)), RESX);
    }

    int16_t scaledY()
    {
      return limit(-RESX, outputs[1] * (180 / (GYRO_MAX_DEFAULT + g_eeGeneral.gyroMax)), RESX);
    }
};

extern Gyro gyro;
