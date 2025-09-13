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

#include "board.h"

bool boardBacklightOn = false;
bool isBacklightEnabled() { return boardBacklightOn; }

void backlightInit() {}

#if !defined(COLORLCD)

void backlightFullOn() { boardBacklightOn = true; }

void backlightEnable(unsigned char)
{
  boardBacklightOn = true;
}

void backlightEnable(unsigned char, unsigned char)
{
  boardBacklightOn = true;  
}

void backlightDisable()
{
  boardBacklightOn = false;
}

#else

void backlightFullOn() { backlightEnable(BACKLIGHT_LEVEL_MAX); }
void backlightEnable(uint8_t) {}

#endif
