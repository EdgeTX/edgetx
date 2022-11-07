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

#include "opentx_constants.h"

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

void evalLogicalSwitches(bool isCurrentFlightmode=true);
void logicalSwitchesCopyState(uint8_t src, uint8_t dst);
void logicalSwitchesReset();
void logicalSwitchesTimerTick();

bool isSwitchWarningRequired(uint16_t &bad_pots);

void getSwitchesPosition(bool startup);

extern swarnstate_t switches_states;
swsrc_t getMovedSwitch();

#define GETSWITCH_MIDPOS_DELAY   1
bool getSwitch(swsrc_t swtch, uint8_t flags=0);
uint8_t getXPotPosition(uint8_t idx);

div_t switchInfo(int switchPosition);

// Lookup switch index by letter ('A' to 'Z')
// note: no function switches
int switchLookupIdx(char c);

// Lookup switch index by name ('SA' to 'SZ')
// note: no function switches
int switchLookupIdx(const char* name, size_t len);

// Get switch letter ('A' to 'Z')
// note: no function switches
char switchGetLetter(uint8_t idx);

// note: no function switches
const char* switchGetCanonicalName(uint8_t idx);

// function switches supported
void switchSetCustomName(uint8_t idx, const char* str, size_t len);
const char* switchGetCustomName(uint8_t idx);
bool switchHasCustomName(uint8_t idx);

SwitchConfig switchGetMaxType(uint8_t idx);
