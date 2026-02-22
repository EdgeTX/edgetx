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

#include "lua_widget.h"

#include "lua_api.h"
#include "lua_event.h"
#include "lua_widget_factory.h"
#include "lua_states.h"

#include "touch.h"
#include "view_main.h"
#include "os/time.h"
#include "keys.h"

#define MAX_INSTRUCTIONS (20000 / 100)

LuaScriptManager *luaScriptManager = nullptr;

#if defined(HARDWARE_TOUCH)
uint32_t LuaEventHandler::downTime = 0;
uint32_t LuaEventHandler::tapTime = 0;
uint32_t LuaEventHandler::tapCount = 0;
tmr10ms_t LuaEventHandler::swipeTimeOut = 0;
coord_t LuaEventHandler::_startX;
coord_t LuaEventHandler::_startY;
bool LuaEventHandler::_sliding = false;
#endif

void LuaEventHandler::event_cb(lv_event_t* e)
{
  auto obj = lv_event_get_target(e);
  auto win = (Window*)lv_obj_get_user_data(obj);
  if (!win) return;

  const lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_KEY) {
    uint32_t key = *(uint32_t*)lv_event_get_param(e);
    if (key == LV_KEY_LEFT) {
      win->onEvent(EVT_ROTARY_LEFT);
    } else if (key == LV_KEY_RIGHT) {
      win->onEvent(EVT_ROTARY_RIGHT);
    }
  } else if (code == LV_EVENT_LONG_PRESSED) {
    lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());
    if (indev_type != LV_INDEV_TYPE_POINTER) {
      // Do not use `lv_indev_wait_release()` as some LUA scripts
      // rely on EVT_KEY_BREAK(KEY_ENTER) being generated on key release
      luaPushEvent(EVT_KEY_LONG(KEY_ENTER));
    }
  }
#if defined(HARDWARE_TOUCH)
  else if (code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING) {

    lv_area_t obj_coords;
    lv_obj_get_coords(obj, &obj_coords);

    lv_point_t point_act;
    lv_indev_t* click_source = (lv_indev_t*)lv_event_get_param(e);
    lv_indev_get_point(click_source, &point_act);

    // Ignore event from keypad
    if (point_act.x < 0 || point_act.y < 0) return;

    lv_point_t rel_pos;
    rel_pos.x = point_act.x - obj_coords.x1;
    rel_pos.y = point_act.y - obj_coords.y1;

    if (code == LV_EVENT_PRESSING) {
      lv_point_t vect_act;
      lv_indev_get_vect(click_source, &vect_act);

      if ((abs(vect_act.x) >= SLIDE_RANGE) || (abs(vect_act.y) >= SLIDE_RANGE))
        _sliding = true;

      if (!_sliding) return;
      TRACE_WINDOWS("LuaWidget touch slide");

      // If we already have a SLIDE going, then accumulate slide values instead
      // of allocating an empty slot
      LuaEventData* const es = luaGetEventSlot(EVT_TOUCH_SLIDE);
      if (es) {
        es->event = EVT_TOUCH_SLIDE;
        es->touchX = rel_pos.x;
        es->touchY = rel_pos.y;
        es->startX = _startX;
        es->startY = _startY;
        es->slideX += vect_act.x;
        es->slideY += vect_act.y;
        TRACE("EVT_TOUCH_SLIDE [%d,%d]", rel_pos.x, rel_pos.y);
      }
    } else {
      LuaEventData* const es = luaGetEventSlot();
      if (es) {
        es->event = EVT_TOUCH_FIRST;
        es->touchX = rel_pos.x;
        es->touchY = rel_pos.y;
        TRACE("EVT_TOUCH_FIRST [%d,%d]", rel_pos.x, rel_pos.y);
      }

      _startX = rel_pos.x;
      _startY = rel_pos.y;

      downTime = time_get_ms();
    }
  } else if (code == LV_EVENT_RELEASED) {
    // tap count handling
    uint32_t now = time_get_ms();
    if (now - downTime <= LUA_TAP_TIME) {
      if (now - tapTime > LUA_TAP_TIME) {
        tapCount = 1;
      } else {
        tapCount++;
      }
      tapTime = now;
    } else {
      tapCount = 0;
    }

    LuaEventData* const es = luaGetEventSlot();
    if (es && downTime) {
      downTime = 0;
      es->event = EVT_TOUCH_BREAK;
      TRACE("EVT_TOUCH_BREAK");
    }
  }
#endif
}

void LuaEventHandler::onClickedEvent()
{
#if defined(HARDWARE_TOUCH)
  auto click_source = lv_indev_get_act();
  bool is_pointer = lv_indev_get_type(click_source) == LV_INDEV_TYPE_POINTER;
  if (is_pointer) {
    lv_point_t point_act;
    lv_indev_get_point(click_source, &point_act);

    LuaEventData* const es = luaGetEventSlot();
    if (!es) return;

    if (tapCount > 0) {
      es->event = EVT_TOUCH_TAP;
      es->tapCount = tapCount;
      es->touchX = point_act.x;
      es->touchY = point_act.y;
    }

    _sliding = false;
    return;
  }
#endif

  luaPushEvent(EVT_KEY_BREAK(KEY_ENTER));
}

void LuaEventHandler::onCancelEvent() {
  luaPushEvent(EVT_KEY_BREAK(KEY_EXIT));
}

void LuaEventHandler::onLuaEvent(event_t event)
{
  luaPushEvent(event);
}

void LuaEventHandler::setupHandler(Window* w)
{
  lv_obj_t* obj = w->getLvObj();
  lv_obj_add_event_cb(obj, LuaEventHandler::event_cb, LV_EVENT_ALL, nullptr);
}

void LuaEventHandler::removeHandler(Window* w)
{
  lv_obj_t* obj = w->getLvObj();
  lv_obj_remove_event_cb(obj, LuaEventHandler::event_cb);
}

void LuaWidget::redraw_cb(lv_event_t* e)
{
  lv_obj_t* target = lv_event_get_target(e);

  LuaWidget* widget = (LuaWidget*)lv_obj_get_user_data(target);

  if (widget && !widget->useLvglLayout()) {
    lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e);

    lv_area_t a, clipping, obj_coords;
    lv_area_copy(&a, draw_ctx->buf_area);
    lv_area_copy(&clipping, draw_ctx->clip_area);
    lv_obj_get_coords(target, &obj_coords);

    auto w = a.x2 - a.x1 + 1;
    auto h = a.y2 - a.y1 + 1;

    TRACE_WINDOWS("Draw %s", widget->getWindowDebugString().c_str());

    BitmapBuffer buf = {BMP_RGB565, (uint16_t)w, (uint16_t)h,
                        (uint16_t*)draw_ctx->buf};

    buf.setDrawCtx(draw_ctx);

    buf.setOffset(obj_coords.x1 - a.x1, obj_coords.y1 - a.y1);
    buf.setClippingRect(clipping.x1 - a.x1, clipping.x2 + 1 - a.x1,
                        clipping.y1 - a.y1, clipping.y2 + 1 - a.y1);

    auto save = luaScriptManager;
    luaScriptManager = widget;
    widget->refresh(&buf);
    luaScriptManager = save;
  }
}

LuaWidget::LuaWidget(const WidgetFactory* factory, Window* parent,
                     const rect_t& rect, int screenNum, int zoneNum,
                     int zoneRectDataRef, int optionsDataRef,
                     int createFunctionRef, const std::string& path) :
    Widget(factory, parent, rect, screenNum, zoneNum),
    zoneRectDataRef(zoneRectDataRef), optionsDataRef(optionsDataRef),
    errorMessage(nullptr)
{
  // Push create function
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, createFunctionRef);
  // Push stored zone for 'create' call
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, zoneRectDataRef);
  // Push stored options for 'create' call
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, optionsDataRef);
  // Push widget folder path
  lua_pushstring(lsWidgets, path.c_str());

  auto save = luaScriptManager;
  luaScriptManager = this;

  if (lua_pcall(lsWidgets, 3, 1, 0) == LUA_OK) {
    luaScriptContextRef = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
  } else {
    luaScriptContextRef = LUA_REFNIL;
    setErrorMessage("create()");
  }

  luaScriptManager = save;

  if (useLvglLayout()) {
    update();
  } else {
    lv_obj_add_event_cb(lvobj, LuaWidget::redraw_cb, LV_EVENT_DRAW_MAIN,
                        nullptr);
  }
}

LuaWidget::~LuaWidget()
{
  luaL_unref(lsWidgets, LUA_REGISTRYINDEX, zoneRectDataRef);
  if (errorMessage)
    free(errorMessage);
}

void LuaWidget::onClicked()
{
  if (!fullscreen) {
    ButtonBase::onClicked();
    return;
  }

  LuaScriptManager::onClickedEvent();
}

void LuaWidget::onCancel()
{
  if (!fullscreen) {
    ButtonBase::onCancel();
    return;
  }

  LuaScriptManager::onCancelEvent();
}

void LuaWidget::foreground()
{
  if (closeFS) {
    closeFS = false;
    setFullscreen(false);
  }

  // Check widget is at least partially visible
  if (isOnScreen()) {
    if (useLvglLayout()) {
      auto save = luaScriptManager;
      PROTECT_LUA() {
        luaScriptManager = this;
        refresh(nullptr);
        if (!errorMessage) {
          if (!callRefs(lsWidgets)) {
            setErrorMessage("foreground calRefs error");
          }
        }
        refreshInstructionsPercent = instructionsPercent;
      } else {
        setErrorMessage("foreground protect Lua error");
      }
      luaScriptManager = save;
      UNPROTECT_LUA();
    } else {
      // Force call to redraw_cb()
      invalidate();
    }
  }

#if defined(DEBUG_WINDOWS)
  TRACE_WINDOWS("# refresh: %s", getWindowDebugString().c_str());
#endif
}

void LuaWidget::updateWithoutRefresh()
{
  Widget::updateWithoutRefresh();

  if (lsWidgets == 0 || errorMessage || luaFactory()->updateFunction == LUA_REFNIL) return;

  luaSetInstructionsLimit(lsWidgets, MAX_INSTRUCTIONS);
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, luaFactory()->updateFunction);
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, luaScriptContextRef);

  auto widgetData = getPersistentData();

  // Get options table and update values
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, optionsDataRef);
  int i = 0;
  for (const WidgetOption* option = getOptionDefinitions(); option->name; option++, i++) {
    switch (option->type) {
      case WidgetOption::String:
      case WidgetOption::File:
        lua_pushstring(lsWidgets, widgetData->getString(i).c_str());
        break;
      case WidgetOption::Integer:
      case WidgetOption::Switch:
        lua_pushinteger(lsWidgets, widgetData->getSignedValue(i));
        break;
      default:
        lua_pushinteger(lsWidgets, widgetData->getUnsignedValue(i));
        break;
    }
    lua_setfield(lsWidgets, -2, option->name);
  }

  auto save = luaScriptManager;
  luaScriptManager = this;

  if (lua_pcall(lsWidgets, 2, 0, 0) != 0)
    setErrorMessage("update()");

  luaScriptManager = save;
}

void LuaWidget::update()
{
  updateWithoutRefresh();

  Widget::update();

  auto save = luaScriptManager;
  luaScriptManager = this;

  if (useLvglLayout()) {
    if (!lv_obj_has_flag(lvobj, LV_OBJ_FLAG_HIDDEN)) {
      lv_area_t a;
      lv_obj_get_coords(lvobj, &a);
      // Check widget is at least partially visible
      if (a.x2 >= 0 && a.x1 < LCD_W) {
        PROTECT_LUA() {
          if (!callRefs(lsWidgets)) {
            setErrorMessage("update callRefs error");
          }
        } else {
          setErrorMessage("update protect Lua error");
        }
        UNPROTECT_LUA();
      }
    }
  }

  luaScriptManager = save;
}

// Update table on top of Lua stack - set entry with name 'idx' to value 'val'
// Return true if value has changed
bool LuaWidget::updateTable(const char* idx, int val)
{
  bool update = false;

  // Check existing value (or invalid value)
  lua_getfield(lsWidgets, -1, idx);
  if (lua_isnumber(lsWidgets, -1)) {
    int v = lua_tointeger(lsWidgets, -1);
    update = (v != val);
  } else {
    // Force table update
    update = true;
  }
  lua_pop(lsWidgets, 1);

  if (update) {
    lua_pushinteger(lsWidgets, val);
    lua_setfield(lsWidgets, -2, idx);
  }

  return update;
}

void LuaWidget::updateZoneRect(rect_t rect, bool updateUI)
{
  if (lsWidgets) {
    // Update widget zone with current size and position

    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, zoneRectDataRef);

    bool changed = false;

    if (updateTable("w", rect.w)) changed = true;
    if (updateTable("h", rect.h)) changed = true;
    if (updateTable("xabs", rect.x)) changed = true;
    if (updateTable("yabs", rect.y)) changed = true;

    lua_pop(lsWidgets, 1);

    if (changed && updateUI)
      updateWithoutRefresh();
  }
}

void LuaWidget::onFullscreen(bool enable)
{
  if (enable) {
    setupHandler(this);
  } else {
    removeHandler(this);
    luaEmptyEventBuffer();
  }
}

void LuaWidget::setErrorMessage(const char* funcName)
{
  constexpr int err_len = 255;

  const char* lua_err = lua_tostring(lsWidgets, -1);
  TRACE("Error in widget %s %s function: %s", factory->getName(), funcName,
        lua_err);
  TRACE("Widget disabled");

  if (!errorMessage)
    errorMessage = (char*)malloc(err_len + 1);

  if (errorMessage) {
    snprintf(errorMessage, err_len, "ERROR in %s: %s", funcName, lua_err);
    errorMessage[err_len] = '\0';
  }
}

void LuaWidget::luaShowError()
{
  setErrorMessage("widget");
}

const char* LuaWidget::getErrorMessage() const { return errorMessage; }

void LuaWidget::refresh(BitmapBuffer* dc)
{
  if (lsWidgets == 0 || luaFactory()->refreshFunction == LUA_REFNIL) return;

  if (errorMessage) {
    if (dc) {
      dc->drawTextLines(0, 0, fullscreen ? LCD_W : rect.w,
                        fullscreen ? LCD_H : rect.h, errorMessage,
                        FONT(XS) | COLOR_THEME_WARNING);
    } else {
      if (errorLabel == nullptr) {
        errorLabel = etx_label_create(lvobj, FONT_XS_INDEX);
        lv_obj_set_pos(errorLabel, 0, 0);
        lv_obj_set_size(errorLabel, width(), height());
        lv_label_set_long_mode(errorLabel, LV_LABEL_LONG_WRAP);
        etx_txt_color(errorLabel, COLOR_THEME_WARNING_INDEX);
        etx_bg_color(errorLabel, COLOR_THEME_SECONDARY3_INDEX);
        etx_obj_add_style(errorLabel, styles->bg_opacity_75, LV_PART_MAIN);
      }
      lv_label_set_text(errorLabel, errorMessage);
    }
    return;
  }

  luaSetInstructionsLimit(lsWidgets, MAX_INSTRUCTIONS);
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, luaFactory()->refreshFunction);
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, luaScriptContextRef);

  // Pass key event to fullscreen Lua widget
  LuaEventData evt;
  luaNextEvent(&evt);

  if (fullscreen) {
    lua_pushinteger(lsWidgets, evt.event);
  } else
    lua_pushnil(lsWidgets);

#if defined(HARDWARE_TOUCH)
  if (fullscreen && IS_TOUCH_EVENT(evt.event)) {
    luaPushTouchEventTable(lsWidgets, &evt);
  } else
#endif
    lua_pushnil(lsWidgets);

  // Enable drawing into the current LCD buffer
  luaLcdBuffer = dc;

  // This little hack is needed to not interfere with the LCD usage of preempted
  // scripts
  bool lla = luaLcdAllowed;
  luaLcdAllowed = true;

  if (lua_pcall(lsWidgets, 3, 0, 0) != 0) {
    setErrorMessage("refresh()");
  }
  // Remove LCD
  luaLcdAllowed = lla;
  luaLcdBuffer = nullptr;
}

void LuaWidget::background()
{
  if (lsWidgets == 0 || errorMessage) return;

  if (luaFactory()->backgroundFunction != LUA_REFNIL) {
    luaSetInstructionsLimit(lsWidgets, MAX_INSTRUCTIONS);
    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, luaFactory()->backgroundFunction);
    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, luaScriptContextRef);
    auto save = luaScriptManager;
    luaScriptManager = this;
    if (lua_pcall(lsWidgets, 1, 0, 0) != 0) {
      setErrorMessage("background()");
    }
    luaScriptManager = save;
  }
}

void LuaWidget::onEvent(event_t event)
{
  if (fullscreen) {
    LuaScriptManager::onLuaEvent(event);
  }
  Widget::onEvent(event);
}

void LuaWidget::clear()
{
  clearRefs(lsWidgets);
  Widget::clear();
}

bool LuaWidget::useLvglLayout() const { return luaFactory()->useLvglLayout(); }

bool LuaWidget::isAppMode() const
{
  return ((WidgetsContainer*)parent)->isAppMode();
}

void LuaScriptManager::saveLvglObjectRef(int ref)
{
  if (tempParent)
    tempParent->saveLvglObjectRef(ref);
  else
    lvglObjectRefs.push_back(ref);
}

void LuaScriptManager::clearRefs(lua_State *L)
{
  for (size_t i = 0; i < lvglObjectRefs.size(); i += 1) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, lvglObjectRefs[i]);
    auto p = LvglWidgetObjectBase::checkLvgl(L, -1);
    lua_pop(L, 1);
    if (p) p->clearRefs(L);
  }
  lvglObjectRefs.clear();
}

bool LuaScriptManager::callRefs(lua_State *L)
{
  for (size_t i = 0; i < lvglObjectRefs.size(); i += 1) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, lvglObjectRefs[i]);
    auto p = LvglWidgetObjectBase::checkLvgl(L, -1);
    lua_pop(L, 1);
    if (p) if (!p->callRefs(L)) return false;
  }
  return true;
}

void LuaScriptManager::createTelemetryQueue()
{
  if (luaInputTelemetryFifo == nullptr) {
    luaInputTelemetryFifo = new TelemetryQueue();
    registerTelemetryQueue(luaInputTelemetryFifo);
  }
}

LuaScriptManager::~LuaScriptManager()
{
  luaL_unref(lsWidgets, LUA_REGISTRYINDEX, luaScriptContextRef);
  if (luaInputTelemetryFifo != nullptr) {
    deregisterTelemetryQueue(luaInputTelemetryFifo);
    delete luaInputTelemetryFifo;
    luaInputTelemetryFifo = nullptr;
  }
}
