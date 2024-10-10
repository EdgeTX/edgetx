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
#include "edgetx_types.h"

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

enum SwitchCategory {
  SWITCH_PHYSICAL=0,
  SWITCH_FUNCTION,
};

#define SWITCH_HW_INVERTED 1

// init hardware for switches
void switchInit();

// returns the factory configuration for all switches
swconfig_t switchGetDefaultConfig();

// returns the maximum number of regular switches supported in hardware (customizable switches NOT included)
uint8_t switchGetMaxSwitches();

// returns the number of configured switches
uint8_t getSwitchCount();

// returns the highest row for a specified column
uint8_t switchGetMaxRow(uint8_t col);

// returns the maximum number of customizable switches supported in hardware
uint8_t switchGetMaxFctSwitches();

struct switch_display_pos_t {
  uint8_t col;
  uint8_t row;
};

switch_display_pos_t switchGetDisplayPosition(uint8_t idx);

// Configure Flex switch (using unified switch index)
void switchConfigFlex(uint8_t idx, int8_t channel);
int8_t switchGetFlexConfig(uint8_t idx);

bool switchIsFlex(uint8_t idx);
bool switchIsFlexValid(uint8_t idx);
bool switchIsFlexInputAvailable(uint8_t idx, uint8_t channel);

void switchFixFlexConfig();

// "Raw" variants indexing only flex switches (used for storage)
void switchConfigFlex_raw(uint8_t idx, int8_t channel);
int8_t switchGetFlexConfig_raw(uint8_t idx);
bool switchIsFlexValid_raw(uint8_t idx);

// The functions bellow support regular as well as customizable switches.
//
// Regular switches are indexed from `0` to `switchGetMaxSwitches() - 1`
// Customizable switches are indexed above the previous range.
//
// The total number of switches (and max index + 1) is:
//   switchGetMaxSwitches() + switchGetMaxFctSwitches()
//

// returns state (0 / 1) of a specific switch position
uint32_t switchState(uint8_t pos_idx);

// returns a position for a switch index
SwitchHwPos switchGetPosition(uint8_t idx);

const char* switchGetName(uint8_t idx);
SwitchHwType switchGetHwType(uint8_t idx);
