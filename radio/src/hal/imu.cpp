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

#include "hal/imu.h"

// Holds a copy of the matched candidate so imuDetect() can return a pointer
// that outlives the (often stack-local) candidates array. A null driver means
// nothing was detected.
static etx_imu_t s_detected = {};

const etx_imu_t* imuDetect(const etx_imu_t* candidates, uint8_t count)
{
  for (uint8_t i = 0; i < count; i++) {
    if (candidates[i].driver->init(candidates[i].bus, candidates[i].addr) == 0) {
      s_detected = candidates[i];
      return &s_detected;
    }
  }
  s_detected = {};
  return nullptr;
}

const char* imuGetName()
{
  return s_detected.driver ? s_detected.driver->name : nullptr;
}
