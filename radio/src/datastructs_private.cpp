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
 #include "hal/switch_driver.h"

SwitchConfig ModelData::getSwitchConfig(uint8_t n)
{
#if defined(FUNCTION_SWITCHES)
  if (switchIsCustomSwitch(n))
    return cfsType(n);
#endif
  return g_eeGeneral.getSwitchConfig(n);
}

char* ModelData::getSwitchCustomName(uint8_t n)
{
#if defined(FUNCTION_SWITCHES)
  if (switchIsCustomSwitch(n)) // Switch is a customisable switch
    return g_model.cfsName(n);
#endif
  return g_eeGeneral.getSwitchCustomName(n);
}

bool ModelData::switchHasCustomName(uint8_t n)
{
#if defined(FUNCTION_SWITCHES)
  if (switchIsCustomSwitch(n)) // Switch is a customisable switch
    return g_model.cfsName(n)[0] != 0;
#endif
  return g_eeGeneral.switchHasCustomName(n);
}

uint8_t ModelData::getSwitchStateForWarning(uint8_t n)
{
  extern swarnstate_t switches_states;
#if defined(FUNCTION_SWITCHES)
  if (switchIsCustomSwitch(n))
    return cfsState(switchGetCustomSwitchIdx(n)) ? 3 : 1;
#endif
  return (switches_states >> (n * 2)) & 3;
}

#if defined(FUNCTION_SWITCHES)
SwitchConfig ModelData::cfsType(uint8_t n) { return (SwitchConfig)customSwitches[switchGetCustomSwitchIdx(n)].type; }
void ModelData::cfsSetType(uint8_t n, SwitchConfig v) { customSwitches[switchGetCustomSwitchIdx(n)].type = v; }
uint8_t ModelData::cfsGroup(uint8_t n) { return (SwitchConfig)customSwitches[switchGetCustomSwitchIdx(n)].group; }
void ModelData::cfsSetGroup(uint8_t n, uint8_t v) { customSwitches[switchGetCustomSwitchIdx(n)].group = v; }
fsStartPositionType ModelData::cfsStart(uint8_t n) { return (fsStartPositionType)customSwitches[switchGetCustomSwitchIdx(n)].start; }
void ModelData::cfsSetStart(uint8_t n, fsStartPositionType v) { customSwitches[switchGetCustomSwitchIdx(n)].start = v; }
bool ModelData::cfsState(uint8_t n) { return customSwitches[switchGetCustomSwitchIdx(n)].state; }
void ModelData::cfsSetState(uint8_t n, bool v) { customSwitches[switchGetCustomSwitchIdx(n)].state = v; }
bool ModelData::cfsSFState(uint8_t n) { return customSwitches[switchGetCustomSwitchIdx(n)].sfState; }
char* ModelData::cfsName(uint8_t n) { return customSwitches[switchGetCustomSwitchIdx(n)].name; }
#if defined(FUNCTION_SWITCHES_RGB_LEDS)
RGBLedColor& ModelData::cfsOnColor(uint8_t n) { return customSwitches[switchGetCustomSwitchIdx(n)].onColor; }
RGBLedColor& ModelData::cfsOffColor(uint8_t n) { return customSwitches[switchGetCustomSwitchIdx(n)].offColor; }
#endif
#endif

SwitchConfig RadioData::getSwitchConfig(uint8_t n)
{
  return (SwitchConfig)(bfGet<swconfig_t>(switchConfig, SW_CFG_BITS * n, SW_CFG_BITS));
}

void RadioData::setSwitchConfig(uint8_t n, SwitchConfig v)
{
  switchConfig = bfSet<swconfig_t>(switchConfig, v, SW_CFG_BITS * n, SW_CFG_BITS);
}

char* RadioData::getSwitchCustomName(uint8_t n)
{
  return switchNames[n];
}

bool RadioData::switchHasCustomName(uint8_t n)
{
  return switchNames[n][0] != 0;
}
