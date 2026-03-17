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

#include "hal/switch_driver.h"
#include "definitions.h"
#include "myeeprom.h"
#include "switches.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct hw_switch_def {
  const char*   name;
  SwitchHwType  type;
  SwitchConfig  defaultType;
#if defined(FUNCTION_SWITCHES)
  bool          isCustomSwitch;
  uint8_t       customSwitchIdx;
#endif
};

#include "simu_switches.inc"

int8_t switchesStates[MAX_SWITCHES];

#if defined(RADIO_GX12)
void _poll_switches() {}
#endif

void simuSetSwitch(uint8_t swtch, int8_t state)
{
  assert(swtch < switchGetMaxAllSwitches());
  switchesStates[swtch] = state;
}

void boardInitSwitches() {
  memset(switchesStates, -1, sizeof(switchesStates));
}

SwitchHwPos boardSwitchGetPosition(uint8_t idx)
{
  if (switchesStates[idx] < 0)
    return SWITCH_HW_UP;
  else if (switchesStates[idx] == 0)
    return SWITCH_HW_MID;
  else
    return SWITCH_HW_DOWN;
}

const char* boardSwitchGetName(uint8_t idx)
{
  return _switch_defs[idx].name;
}

SwitchHwType boardSwitchGetType(uint8_t idx)
{
  return _switch_defs[idx].type;
}

uint8_t boardGetMaxSwitches() { return n_switches; }

SwitchConfig boardSwitchGetDefaultConfig(uint8_t idx) { return _switch_defs[idx].defaultType; }

#if defined(FUNCTION_SWITCHES)
bool boardIsCustomSwitch(uint8_t idx) { return (idx < n_switches) ? _switch_defs[idx].isCustomSwitch : false; }
uint8_t boardGetCustomSwitchIdx(uint8_t idx) { return _switch_defs[idx].customSwitchIdx; }
#endif

#if !defined(COLORLCD)
switch_display_pos_t switchGetDisplayPosition(uint8_t idx)
{
  if (idx >= DIM(_switch_display)) return {0, 0};

  return _switch_display[idx];
}
#endif
