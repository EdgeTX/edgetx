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
#include "hal/adc_driver.h"
#include "hal/switch_driver.h"
#include "tasks/mixer_task.h"

char* RadioData::getSwitchCustomName(uint8_t n)
{
  return switchConfig[n].name;
}

bool RadioData::switchHasCustomName(uint8_t n)
{
  return switchConfig[n].name[0] != 0;
}

SwitchConfig RadioData::switchType(uint8_t n) {
  return (SwitchConfig)switchConfig[n].type;
}

void RadioData::switchSetType(uint8_t n, SwitchConfig v) {
  switchConfig[n].type = v;
  storageDirty(EE_GENERAL);
}

char* RadioData::switchName(uint8_t n) {
  return switchConfig[n].name;
}

#if defined(FUNCTION_SWITCHES)
fsStartPositionType RadioData::switchStart(uint8_t n) {
  return (fsStartPositionType)switchConfig[n].start;
}

void RadioData::switchSetStart(uint8_t n, fsStartPositionType v) {
  switchConfig[n].start = v;
  storageDirty(EE_GENERAL);
}

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
RGBLedColor& RadioData::switchOnColor(uint8_t n) {
  return switchConfig[n].onColor;
}

RGBLedColor& RadioData::switchOffColor(uint8_t n) {
  return switchConfig[n].offColor;
}

bool RadioData::cfsOnColorLuaOverride(uint8_t n) {
  return switchConfig[n].onColorLuaOverride;
}

bool RadioData::cfsOffColorLuaOverride(uint8_t n) {
  return switchConfig[n].offColorLuaOverride;
}

void RadioData::cfsSetOnColorLuaOverride(uint8_t n, bool v) {
  switchConfig[n].onColorLuaOverride = v;
  storageDirty(EE_GENERAL);
}

void RadioData::cfsSetOffColorLuaOverride(uint8_t n, bool v) {
  switchConfig[n].offColorLuaOverride = v;
  storageDirty(EE_GENERAL);
}
#endif
#endif
