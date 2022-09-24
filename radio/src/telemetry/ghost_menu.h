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

#include <inttypes.h>

constexpr uint8_t GHST_MENU_LINES = 6;
constexpr uint8_t GHST_MENU_CHARS = 20;

// GHST_DL_MENU_DESC (27 bytes)
struct GhostMenuFrame
{
  uint8_t address;
  uint8_t length ;
  uint8_t packetId;
  uint8_t menuStatus;    // GhostMenuStatus
  uint8_t lineFlags;     // GhostLineFlags
  uint8_t lineIndex;     // 0 = first line
  unsigned char menuText[GHST_MENU_CHARS];
  uint8_t crc;
};

struct GhostMenuData {
  uint8_t menuStatus;    // Update Line, Clear Menu, etc.
  uint8_t lineFlags;     // Carat states, Inverse, Bold for each of Menu Label, and Value
  uint8_t splitLine;     // Store beginning of Value substring
  char menuText[GHST_MENU_CHARS + 1];
};
