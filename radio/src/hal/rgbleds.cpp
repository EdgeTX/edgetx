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

#include "edgetx.h"
#include "rgbleds.h"
#include "boards/generic_stm32/rgb_leds.h"
#include "definitions.h"
#include "dataconstants.h"

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
void setFSLedOFF(uint8_t index) {
  fsLedRGB(index, g_model.functionSwitchLedOFFColor[index].getColor());
}

void setFSLedON(uint8_t index) {
  fsLedRGB(index, g_model.functionSwitchLedONColor[index].getColor());
}

bool getFSLedState(uint8_t index) {
  return rgbGetLedColor(index) == g_model.functionSwitchLedONColor[index].getColor();
}
#else
void setFSLedOFF(uint8_t index) {
  fsLedOff(index);
}

void setFSLedON(uint8_t index) {
  fsLedOn(index);
}

bool getFSLedState(uint8_t index) {
  return fsLedState(index);
}
#endif
