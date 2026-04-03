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

#include "hal/gpio.h"
#include "hal/rgbleds.h"
#include "stm32_gpio.h"
#include "boards/generic_stm32/rgb_leds.h"
#include "board.h"

#define GET_RED(color) (((color) & 0xFF0000) >>16)
#define GET_GREEN(color) (((color) & 0x00FF00) >> 8)
#define GET_BLUE(color) ((color) & 0x0000FF)


void fsLedRGB(uint8_t index, uint32_t color)
{
  index = (index * 2) + CFS_LED_STRIP_START;
  rgbSetLedColor(index, GET_RED(color), GET_GREEN(color),GET_BLUE(color));
  rgbSetLedColor(index+1, GET_RED(color), GET_GREEN(color),GET_BLUE(color));
}

uint32_t fsGetLedRGB(uint8_t index)
{
  return rgbGetLedColor((index * 2) + CFS_LED_STRIP_START);
}

