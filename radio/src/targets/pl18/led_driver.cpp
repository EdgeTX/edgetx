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

#include "board.h"
#include "boards/generic_stm32/rgb_leds.h"

void ledInit()
{
  // Do nothing
}

void ledOff()
{
  for (uint8_t i = 0; i < LED_STRIP_LENGTH; i++) {
    rgbSetLedColor(i, 0, 0, 0);
  }
  rgbLedColorApply();
}

void ledRed()
{
  for (uint8_t i = 0; i < LED_STRIP_LENGTH; i++) {
    rgbSetLedColor(i, 50, 0, 0);
  }
  rgbLedColorApply();
}

void ledGreen()
{
  for (uint8_t i = 0; i < LED_STRIP_LENGTH; i++) {
    rgbSetLedColor(i, 0, 50, 0);
  }
  rgbLedColorApply();
}

void ledBlue()
{
  for (uint8_t i = 0; i < LED_STRIP_LENGTH; i++) {
    rgbSetLedColor(i, 0, 0, 50);
  }
  rgbLedColorApply();
}
