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

#include <stdlib.h>
#include <assert.h>

const char* _hw_switch_name[] = {
  #include "hw_switches.inc"
};

constexpr uint8_t NUM_SWITCHES = DIM(_hw_switch_name);

int8_t switchesStates[NUM_SWITCHES] = { -1 };
void simuSetSwitch(uint8_t swtch, int8_t state)
{
  assert(swtch < DIM(switchesStates));
  switchesStates[swtch] = state;
}

uint8_t switchGetMaxSwitches()
{
  return DIM(_hw_switch_name);
}

const char* switchGetName(uint8_t idx)
{
  assert(idx < NUM_SWITCHES);
  return _hw_switch_name[idx];
}

uint32_t switchState(uint8_t pos)
{
  assert(pos < NUM_SWITCHES * 3);

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
  assert(idx < NUM_SWITCHES);

  if (switchesStates[idx] < 0)
    return SWITCH_HW_UP;
  else if (switchesStates[idx] == 0)
    return SWITCH_HW_MID;
  else
    return SWITCH_HW_DOWN;
}
