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

static const char* s_imu_name = nullptr;

imu_read_fn imuDetect(const etx_imu_t* candidates, uint8_t count,
                       etx_i2c_bus_t* detected_bus)
{
  for (uint8_t i = 0; i < count; i++) {
    if (candidates[i].driver->init(candidates[i].bus, candidates[i].addr) == 0) {
      s_imu_name = candidates[i].driver->name;
      if (detected_bus) *detected_bus = candidates[i].bus;
      return candidates[i].driver->read;
    }
  }
  return nullptr;
}

const char* imuGetName()
{
  return s_imu_name;
}
