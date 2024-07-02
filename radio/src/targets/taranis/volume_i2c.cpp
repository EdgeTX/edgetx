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

#if !defined(SOFTWARE_VOLUME)

#include "hal/i2c_driver.h"
#include "stm32_i2c_driver.h"

#include "board.h"

static const uint8_t volumeScale[VOLUME_LEVEL_MAX+1] = {
  0,  1,  2,  3,  5,  9,  13,  17,  22,  27,  33,  40,
  64, 82, 96, 105, 112, 117, 120, 122, 124, 125, 126, 127
};

int32_t getVolume()
{
  uint8_t value = 0;
  if (i2c_read(VOLUME_I2C_BUS, VOLUME_I2C_ADDRESS, 0, 1, &value, 1) < 0)
    return -1;

  return value;
}

void setVolume(uint8_t volume)
{
  i2c_init(VOLUME_I2C_BUS);
  i2c_write(VOLUME_I2C_BUS, VOLUME_I2C_ADDRESS, 0, 1, &volume, 1);
}

void setScaledVolume(uint8_t volume)
{
  if (volume > VOLUME_LEVEL_MAX) {
    volume = VOLUME_LEVEL_MAX;
  }

  setVolume(volumeScale[volume]);
}

#endif
