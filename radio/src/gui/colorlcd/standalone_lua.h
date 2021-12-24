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

#if defined(LUA)
#include "libopenui.h"
#include "lua/lua_api.h"

struct LuaPopup
{
  rect_t rect;
  LuaPopup(rect_t r) : rect(r) {}
  void paint(BitmapBuffer* dc, uint8_t type, const char* text, const char* info);
};

class StandaloneLuaWindow : public Window
{
  static StandaloneLuaWindow* _instance;

  explicit StandaloneLuaWindow();

public:
  static StandaloneLuaWindow* instance();

  void attach(Window* newParent);
  void deleteLater(bool detach = true, bool trash = true) override;
  void paint(BitmapBuffer* dc) override;
  void checkEvents() override;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "StandaloneLuaWindow"; }
#endif

#if defined(HARDWARE_KEYS)
  void onEvent(event_t evt) override;
#endif

#if defined(HARDWARE_TOUCH)
  bool onTouchStart(coord_t x, coord_t y) override;
  bool onTouchEnd(coord_t x, coord_t y) override;
  bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY) override;
#endif

  bool displayPopup(event_t event, uint8_t type, const char* text,
                    const char* info, bool& result);

protected:
  // GFX
  BitmapBuffer lcdBuffer;

  // pop-ups
  LuaPopup popup;

  // run LUA code
  void runLua(event_t evt);
  event_t event = 0;
#if defined(HARDWARE_TOUCH)
  static bool fingerDown;
  LuaTouchData touch;
#endif
};
#endif
