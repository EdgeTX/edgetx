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

#include <stdlib.h>
#include <assert.h>
#include "switches.h"

struct hw_switch_def {
  const char*  name;
  SwitchHwType type;
};

#include "simu_switches.inc"

int8_t switchesStates[MAX_SWITCHES] = { -1 };

void simuSetSwitch(uint8_t swtch, int8_t state)
{
  assert(swtch < switchGetMaxSwitches() + switchGetMaxFctSwitches());
  switchesStates[swtch] = state;
}

void boardInitSwitches() {}

static uint8_t get_switch_index(uint8_t cat, uint8_t idx)
{
  switch(cat) {
  case SWITCH_PHYSICAL:
    assert(idx < n_switches);
    return idx;

  case SWITCH_FUNCTION:
    assert(idx < n_fct_switches);
    return idx + n_switches;

  default:
    assert(0);
    return 0;
  }  
}

SwitchHwPos boardSwitchGetPosition(uint8_t cat, uint8_t idx)
{
  idx = get_switch_index(cat, idx);

#if defined(FUNCTION_SWITCHES)
  if (IS_SWITCH_FS(idx + n_switches)) {
    if (bfSingleBitGet(functionSwitchFunctionState, idx))
      return SWITCH_HW_DOWN;
  }
#endif

  if (switchesStates[idx] < 0)
    return SWITCH_HW_UP;
  else if (switchesStates[idx] == 0)
    return SWITCH_HW_MID;
  else
    return SWITCH_HW_DOWN;
}

const char* boardSwitchGetName(uint8_t cat, uint8_t idx)
{
  idx = get_switch_index(cat, idx);
  return _switch_defs[idx].name;
}

SwitchHwType boardSwitchGetType(uint8_t cat, uint8_t idx)
{
  idx = get_switch_index(cat, idx);
  return _switch_defs[idx].type;
}

uint8_t boardGetMaxSwitches() { return n_switches; }
uint8_t boardGetMaxFctSwitches() { return n_fct_switches; }

swconfig_t boardSwitchGetDefaultConfig() { return _switch_default_config; }

switch_display_pos_t switchGetDisplayPosition(uint8_t idx)
{
  if (idx >= DIM(_switch_display)) return {0, 0};

  return _switch_display[idx];
}
