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

#include "edgetx_types.h"

#define EVENT_BUFFER_SIZE 4

struct LuaEventData {
  event_t event;
#if defined(HARDWARE_TOUCH)
  coord_t touchX;
  coord_t touchY;
  coord_t startX;
  coord_t startY;
  coord_t slideX;
  coord_t slideY;
  short tapCount;
#endif
  LuaEventData();
};

void luaPushEvent(event_t evt);
void luaNextEvent(LuaEventData* evt);
void luaEmptyEventBuffer();

// Look for a slot in the event buffer that is either unused (zero) or matches event
LuaEventData* luaGetEventSlot(event_t event = 0);

struct lua_State;
typedef struct lua_State lua_State;

void luaPushTouchEventTable(lua_State* ls, LuaEventData* evt);

