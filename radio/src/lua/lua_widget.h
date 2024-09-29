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

#include <vector>

#include "window.h"
#include "widget.h"
#include "lua_api.h"
#include "lua_lvgl_widget.h"

#include "edgetx_types.h"

#define LUA_TAP_TIME 250 // 250 ms

class LuaWidgetFactory;

class LuaLvglManager
{
 public:
  LuaLvglManager() = default;

  std::vector<int> getLvglObjectRefs() const { return lvglObjectRefs; }
  void saveLvglObjectRef(int ref);
  void clearRefs(lua_State *L);
  bool callRefs(lua_State *L);

  void setTempParent(LvglWidgetObjectBase *p) { tempParent = p; }
  LvglWidgetObjectBase* getTempParent() const { return tempParent; }

  virtual Window* getCurrentParent() const = 0;
  virtual void clear() = 0;
  virtual bool useLvglLayout() const = 0;
  virtual bool isAppMode() const = 0;

  virtual void luaShowError() = 0;

  virtual bool isWidget() = 0;

  uint8_t refreshInstructionsPercent;

 protected:
  std::vector<int> lvglObjectRefs;
  LvglWidgetObjectBase* tempParent = nullptr;
};

class LuaEventHandler
{
#if defined(HARDWARE_TOUCH)
  // "tap" handling
  static uint32_t downTime;
  static uint32_t tapTime;
  static uint32_t tapCount;
  // "swipe" / "slide" handling
  static tmr10ms_t swipeTimeOut;
  static bool _sliding;
  static coord_t _startX;
  static coord_t _startY;
#endif
  static void event_cb(lv_event_t* e);

protected:
  void onClicked();
  void onCancel();
  void onEvent(event_t event);

public:
  LuaEventHandler() = default;
  void setupHandler(Window* w);
  void removeHandler(Window* w);
};

class LuaWidget : public Widget, public LuaEventHandler, public LuaLvglManager
{
  friend class LuaWidgetFactory;

 public:
  LuaWidget(const WidgetFactory* factory, Window* parent, const rect_t& rect,
            WidgetPersistentData* persistentData, int luaWidgetDataRef, int zoneRectDataRef,
            int optionsDataRef);
  ~LuaWidget() override;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "LuaWidget"; }
#endif

  void setErrorMessage(const char* funcName);

  // Widget interface
  const char* getErrorMessage() const override;
  void update() override;
  void background() override;

  void clear() override;

  LuaWidgetFactory* luaFactory() const { return (LuaWidgetFactory*)factory; }

  Window* getCurrentParent() const override { return (tempParent && tempParent->getWindow()) ? tempParent->getWindow() : (Window*)this; }

  bool useLvglLayout() const override;
  bool isAppMode() const override;

  void luaShowError() override {}

  void pushOptionsTable();

  bool isWidget() override { return !inSettings; }

 protected:
  bool inSettings = false;
  lv_obj_t* errorLabel = nullptr;
  int zoneRectDataRef;
  int optionsDataRef;
  int luaWidgetDataRef = 0;
  char* errorMessage;
  bool refreshed = false;

  // Window interface
  void onClicked() override;
  void onCancel() override;
  void checkEvents() override;
  void onEvent(event_t event) override;

  // Widget interface
  void onFullscreen(bool enable) override;

  // Update 'zone' data
  void updateZoneRect(rect_t rect, bool updateUI = true) override;
  bool updateTable(const char* idx, int val);

  // Calls LUA widget 'refresh' method
  void refresh(BitmapBuffer* dc);

  static void redraw_cb(lv_event_t *e);
};
