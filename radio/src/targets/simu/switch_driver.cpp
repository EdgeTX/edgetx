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

struct hw_switch_def {
  const char*  name;
  SwitchHwType type;
};

#include "simu_switches.inc"

int8_t switchesStates[n_total_switches] = { -1 };

void simuSetSwitch(uint8_t swtch, int8_t state)
{
  assert(swtch < n_total_switches);
  switchesStates[swtch] = state;
}

swconfig_t switchGetDefaultConfig()
{
  return _switch_default_config;
}

switch_display_pos_t switchGetDisplayPosition(uint8_t idx)
{
  if (idx >= DIM(_switch_display))
    return {0, 0};

  return _switch_display[idx];
}

uint8_t switchGetMaxSwitches()
{
  return n_switches;
}

uint8_t getSwitchCount()
{
  int count = 0;
  for (int i = 0; i < switchGetMaxSwitches(); ++i) {
    if (SWITCH_EXISTS(i)) {
      ++count;
    }
  }
  return count;
}

uint8_t switchGetMaxRow(uint8_t col)
{
  uint8_t lastrow = 0;
  for (int i = 0; i < switchGetMaxSwitches(); ++i) {
    if (SWITCH_EXISTS(i)) {
      auto switch_display = switchGetDisplayPosition(i);
      if (switch_display.col == col)
        lastrow = switch_display.row > lastrow ? switch_display.row : lastrow;
    }
  }
  return lastrow;
}

uint8_t switchGetMaxFctSwitches()
{
  return n_fct_switches;
}

const char* switchGetName(uint8_t idx)
{
  assert(idx < n_total_switches);
  return _hw_switch_defs[idx].name;
}

SwitchHwType switchGetHwType(uint8_t idx)
{
  assert(idx < n_total_switches);
  return _hw_switch_defs[idx].type;
}

uint32_t switchState(uint8_t pos)
{
  assert(pos < n_total_switches * 3);

  div_t qr = div(pos, 3);
  int state = switchesStates[qr.quot];
  switch (qr.rem) {
    case SWITCH_HW_UP:
      return state < 0;
    case SWITCH_HW_DOWN:
      return state > 0;
    default:
      return state == 0;
  }
}

SwitchHwPos switchGetPosition(uint8_t idx)
{
  assert(idx < n_total_switches);

  if (switchesStates[idx] < 0)
    return SWITCH_HW_UP;
  else if (switchesStates[idx] == 0)
    return SWITCH_HW_MID;
  else
    return SWITCH_HW_DOWN;
}
