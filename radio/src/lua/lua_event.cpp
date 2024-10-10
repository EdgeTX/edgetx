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

#include "lua_event.h"
#include "lua_api.h"
#include "edgetx_helpers.h"
#include "board_common.h"
#include "timers_driver.h"
#include "keys.h"

extern "C" {
  #include "lua.h"
};

static LuaEventData _lua_event_buffer[EVENT_BUFFER_SIZE];

LuaEventData::LuaEventData()
{
  memclear(this, sizeof(LuaEventData));
}

void luaPushEvent(event_t evt)
{
  for (int i = 0; i < EVENT_BUFFER_SIZE; i++) {
    if (_lua_event_buffer[i].event == 0) {
      _lua_event_buffer[i].event = evt;
      return;
    }
  }
}

void luaNextEvent(LuaEventData* evt)
{
  if (!evt) return;
  *evt = _lua_event_buffer[0];

  if (evt->event != 0) {
    size_t len = sizeof(_lua_event_buffer) - sizeof(LuaEventData);
    memmove(_lua_event_buffer, _lua_event_buffer + 1, len);
    memclear(_lua_event_buffer + (EVENT_BUFFER_SIZE - 1), sizeof(LuaEventData));
  }
}

void luaEmptyEventBuffer()
{
  memclear(_lua_event_buffer, sizeof(_lua_event_buffer));
}

// Look for a slot in the event buffer that is either unused (zero) or matches event
LuaEventData* luaGetEventSlot(event_t event)
{
  for (int i = 0; i < EVENT_BUFFER_SIZE; i++) {
    if (_lua_event_buffer[i].event == event || _lua_event_buffer[i].event == 0)
      return &_lua_event_buffer[i];
  }

  return nullptr;
}

#if defined(HARDWARE_TOUCH)

#define EVT_TOUCH_SWIPE_LOCK     4
#define EVT_TOUCH_SWIPE_SPEED   60
#define EVT_TOUCH_SWIPE_TIMEOUT 50

static tmr10ms_t _swipeTimeOut = 0;

void luaPushTouchEventTable(lua_State* ls, LuaEventData* evt)
{
  lua_newtable(ls);
  l_pushtableint(ls, "x", evt->touchX);
  l_pushtableint(ls, "y", evt->touchY);
  l_pushtableint(ls, "tapCount", evt->tapCount);

  if (evt->event == EVT_TOUCH_SLIDE) {
    l_pushtableint(ls, "startX", evt->startX);
    l_pushtableint(ls, "startY", evt->startY);
    l_pushtableint(ls, "slideX", evt->slideX);
    l_pushtableint(ls, "slideY", evt->slideY);

    // Do we have a swipe? Only one at a time!
    if (get_tmr10ms() > _swipeTimeOut) {
      coord_t absX = (evt->slideX < 0) ? -(evt->slideX) : evt->slideX;
      coord_t absY = (evt->slideY < 0) ? -(evt->slideY) : evt->slideY;
      bool swiped = false;

      if (absX > EVT_TOUCH_SWIPE_LOCK * absY) {
        if ((swiped = (evt->slideX > EVT_TOUCH_SWIPE_SPEED)))
          l_pushtablebool(ls, "swipeRight", true);
        else if ((swiped = (evt->slideX < -EVT_TOUCH_SWIPE_SPEED)))
          l_pushtablebool(ls, "swipeLeft", true);
      } else if (absY > EVT_TOUCH_SWIPE_LOCK * absX) {
        if ((swiped = (evt->slideY > EVT_TOUCH_SWIPE_SPEED)))
          l_pushtablebool(ls, "swipeDown", true);
        else if ((swiped = (evt->slideY < -EVT_TOUCH_SWIPE_SPEED)))
          l_pushtablebool(ls, "swipeUp", true);
      }

      if (swiped) _swipeTimeOut = get_tmr10ms() + EVT_TOUCH_SWIPE_TIMEOUT;
    }
  }
}
#endif
