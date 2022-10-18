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

#pragma once

#include <stdint.h>

enum SwitchHwType {
  SWITCH_HW_2POS = 0,
  SWITCH_HW_3POS,
  SWITCH_HW_ADC,
};

enum SwitchHwPos {
  SWITCH_HW_UP = 0,
  SWITCH_HW_MID,
  SWITCH_HW_DOWN,
};

#define SWITCH_HW_INVERTED 1

// returns the maximum number of switches supported in hardware
uint8_t switchGetMaxSwitches();

// returns state (0 / 1) of a specific switch position
uint32_t switchState(uint8_t pos_idx);

// returns a position for a switch index
SwitchHwPos switchGetPosition(uint8_t idx);

const char* switchGetName(uint8_t idx);

const char** switchGetNames();
