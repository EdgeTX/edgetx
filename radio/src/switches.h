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

#include <cinttypes>

enum LogicalSwitchFamilies {
  LS_FAMILY_OFS,
  LS_FAMILY_BOOL,
  LS_FAMILY_COMP,
  LS_FAMILY_DIFF,
  LS_FAMILY_TIMER,
  LS_FAMILY_STICKY,
  LS_FAMILY_RANGE,
  LS_FAMILY_EDGE
};

typedef int16_t delayval_t;

uint8_t lswFamily(uint8_t func);
int16_t lswTimerValue(delayval_t val);

bool isSwitchWarningRequired(uint16_t &bad_pots);
