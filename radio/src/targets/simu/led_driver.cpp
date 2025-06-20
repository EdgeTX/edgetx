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
#include "hal/rgbleds.h"
#include "definitions.h"

#include "board.h"

bool usbChargerLed() { return true; }
void ledRed() {}
void ledGreen() {}
void ledBlue() {}
void ledOff() {}

void rgbSetLedColor(uint8_t, uint8_t, uint8_t, uint8_t) {}
void rgbLedColorApply() {}

uint8_t getRGBColorIndex(uint32_t color)
{
  for (uint8_t i = 0; i < DIM(colorTable); i++) {
    if (color == colorTable[i])
      return(i + 1);
  }
  return 0; // Custom value set with Companion
}

#if NUM_FUNCTIONS_SWITCHES > 0
static uint32_t _fs_switch_colors[NUM_FUNCTIONS_SWITCHES] = {0};
static uint32_t _fs_switch_color_mask = 0;
static uint32_t _fs_switch_mask = 0;

void fsLedRGB(uint8_t index, uint32_t col)
{
  _fs_switch_color_mask |= (1 << index);
  _fs_switch_colors[index] = col;
}

uint32_t fsGetLedRGB(uint8_t index)
{
  return _fs_switch_colors[index];
}

bool fsLedIsColorSet(uint8_t index)
{
  return _fs_switch_color_mask & (1 << index);
}

bool fsLedState(uint8_t index)
{
  return _fs_switch_mask & (1 << index);
}

void fsLedOn(uint8_t index)
{
  _fs_switch_mask |= (1 << index);
}

void fsLedOff(uint8_t index)
{
  _fs_switch_mask &= ~(1 << index);
}
#endif
