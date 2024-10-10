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

// Match with /companion/src/simulation/simulateduiwidget.h
enum EnumKeys {
  KEY_MENU,
  KEY_EXIT,
  KEY_ENTER,

  KEY_PAGEUP,
  KEY_PAGEDN,

  KEY_UP,
  KEY_DOWN,

  KEY_LEFT,
  KEY_RIGHT,

  KEY_PLUS,
  KEY_MINUS,

  KEY_MODEL,
  KEY_TELE,
  KEY_SYS,

  KEY_SHIFT,
  KEY_BIND,

  MAX_KEYS
};

// returns a bit field with each key set as (1 << KEY_xxx)
uint32_t readKeys();

// returns a bit field with each trim key
uint32_t readTrims();

// Init GPIO ports
void keysInit();

uint32_t keysGetSupported();

uint8_t keysGetMaxKeys();
uint8_t keysGetMaxTrims();

const char* keysGetLabel(EnumKeys key);
