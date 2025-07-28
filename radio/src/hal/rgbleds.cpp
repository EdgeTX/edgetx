/*
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
static bool hasLedOverride[NUM_FUNCTIONS_SWITCHES] = { false };
static RGBLedColor ledOverride[NUM_FUNCTIONS_SWITCHES];

void setFSLedOverride(uint8_t index, bool state, uint8_t r, uint8_t g, uint8_t b)
{
  hasLedOverride[index] = state;
  ledOverride[index].r = r;
  ledOverride[index].g = g;
  ledOverride[index].b = b;
}

void setFSLedOFF(uint8_t index) {
  if (g_model.getSwitchType(index) != SWITCH_NONE) {
    uint8_t cfsIdx = switchGetCustomSwitchIdx(index);
    if (hasLedOverride[cfsIdx] && g_model.getSwitchOffColorLuaOverride(index))
      fsLedRGB(cfsIdx, ledOverride[cfsIdx].getColor());
    else
      fsLedRGB(cfsIdx, g_model.getSwitchOffColor(index).getColor());
  }
}

void setFSLedON(uint8_t index) {
  if (g_model.getSwitchType(index) != SWITCH_NONE) {
    uint8_t cfsIdx = switchGetCustomSwitchIdx(index);
    if (hasLedOverride[cfsIdx] && g_model.getSwitchOnColorLuaOverride(index))
      fsLedRGB(cfsIdx, ledOverride[cfsIdx].getColor());
    else
      fsLedRGB(cfsIdx, g_model.getSwitchOnColor(index).getColor());
  }
}

bool getFSLedState(uint8_t index) {
  uint8_t cfsIdx = switchGetCustomSwitchIdx(index);
  return rgbGetLedColor(cfsIdx) == g_model.getSwitchOnColor(index).getColor();
}

uint32_t getFSLedRGBColor(uint8_t index)
{
    return fsGetLedRGB(index);
}
#else
void setFSLedOFF(uint8_t index) {
  index = switchGetCustomSwitchIdx(index);
  fsLedOff(index);
}

void setFSLedON(uint8_t index) {
  index = switchGetCustomSwitchIdx(index);
  fsLedOn(index);
}

bool getFSLedState(uint8_t index) {
  index = switchGetCustomSwitchIdx(index);
  return fsLedState(index);
}
#endif
