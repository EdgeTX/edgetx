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

class StandaloneLuaWindow : public Window, public LuaEventHandler, public LuaLvglManager
{
  static StandaloneLuaWindow* _instance;

  explicit StandaloneLuaWindow(bool useLvgl);

public:
  static StandaloneLuaWindow* instance();
  static void setup(bool useLvgl);

  void attach();
  void deleteLater(bool detach = true, bool trash = true) override;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "StandaloneLuaWindow"; }
#endif

  bool displayPopup(event_t event, uint8_t type, const char* text,
                    const char* info, bool& result);

  Window* getCurrentParent() const override { return tempParent ? tempParent : (Window*)this; }

  void clear() override;
  bool useLvglLayout() const override { return useLvgl; }

  void luaShowError() override;

  void showError(bool firstCall, const char* title, const char* msg);

  bool isWidget() override { return false; }

  static LAYOUT_VAL(POPUP_HEADER_HEIGHT, 30, 30);
  static LAYOUT_VAL(POPUP_X, 50, 40);
  static LAYOUT_VAL(POPUP_Y, 70, 110);

protected:
  lv_obj_t* prevScreen = nullptr;
  lv_obj_t* errorModal = nullptr;
  lv_obj_t* errorTitle = nullptr;
  lv_obj_t* errorMsg = nullptr;
  bool hasError = false;
  bool useLvgl = false;

  // GFX
  BitmapBuffer *lcdBuffer = nullptr;

  // pop-ups
  void popupPaint(BitmapBuffer* dc, coord_t x, coord_t y, coord_t w, coord_t h,
                  const char* text, const char* info);

  // run LUA code
  void runLua(event_t evt);

  void onEvent(event_t evt) override;
  void checkEvents() override;
  void onClicked() override;
  void onCancel() override;

  static void redraw_cb(lv_event_t *e);
};
#endif
