/*
 * Copyright (C) EdgeTx
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

#include <stdint.h>

#define BACKLIGHT_LEVEL_MAX 100

bool boardBacklightOn = false;
static uint8_t _backlightLevel = 0;

bool isBacklightEnabled()
{
  return boardBacklightOn && (_backlightLevel > 0);
}
void backlightFullOn() { boardBacklightOn = true; }

void backlightInit() {}

void backlightEnable(uint8_t level)
{
  boardBacklightOn = true;
#if defined(COLORLCD)
  _backlightLevel = level;
#else
  _backlightLevel = 100 - level;
#endif
}

void backlightEnable(uint8_t level, unsigned char) { backlightEnable(level); }

void backlightDisable()
{
  boardBacklightOn = false;
}
