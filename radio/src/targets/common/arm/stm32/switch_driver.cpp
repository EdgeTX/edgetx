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
#include "stm32_switch_driver.h"
#include "stm32_gpio_driver.h"

#include "definitions.h"

// generated switch structs
#include "stm32_switches.inc"

#include <stdlib.h>

void switchInit()
{
  for (uint8_t i = 0; i < DIM(_switch_GPIOs); i++) {
    stm32_gpio_enable_clock(_switch_GPIOs[i]);
  }
}

uint8_t switchGetMaxSwitches()
{
  return n_switches;
}

uint8_t switchGetMaxFctSwitches()
{
  return n_fct_switches;
}

// returns state (0 / 1) of a specific switch position
uint32_t switchState(uint8_t pos_idx)
{
  auto d = div(pos_idx, 3);
  return stm32_switch_get_state(&_switch_defs[d.quot], (SwitchHwPos)d.rem);
}

SwitchHwPos switchGetPosition(uint8_t idx)
{
  if (idx >= n_total_switches) return SWITCH_HW_UP;
  return stm32_switch_get_position(&_switch_defs[idx]);
}

const char* switchGetName(uint8_t idx)
{
  if (idx >= n_total_switches) return "";
  return _switch_defs[idx].name;
}

SwitchHwType switchGetHwType(uint8_t idx)
{
  if (idx >= n_total_switches) return SWITCH_HW_2POS;
  return _switch_defs[idx].type;
}
