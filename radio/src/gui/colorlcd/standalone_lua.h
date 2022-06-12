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
#include "lua/lua_widget.h"

struct LuaPopup
{
  rect_t rect;
  LuaPopup(rect_t r) : rect(r) {}
  void paint(BitmapBuffer* dc, uint8_t type, const char* text, const char* info);
};

class StandaloneLuaWindow : public Window, public LuaEventHandler
{
  static StandaloneLuaWindow* _instance;

  explicit StandaloneLuaWindow();

public:
  static StandaloneLuaWindow* instance();

  void attach();
  void deleteLater(bool detach = true, bool trash = true) override;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "StandaloneLuaWindow"; }
#endif

  bool displayPopup(event_t event, uint8_t type, const char* text,
                    const char* info, bool& result);

protected:
  lv_obj_t* prevScreen = nullptr;

  // GFX
  BitmapBuffer lcdBuffer;

  // pop-ups
  LuaPopup popup;

  // run LUA code
  void runLua(event_t evt);

  void paint(BitmapBuffer* dc) override;
  void onEvent(event_t evt) override;
  void checkEvents() override;
  void onClicked() override;
  void onCancel() override;
};
#endif
