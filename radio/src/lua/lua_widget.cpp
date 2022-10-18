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
#include "lua_widget_factory.h"

#include "lua_api.h"
#include "lua_event.h"
#include "draw_functions.h"
#include "touch.h"

#define MAX_INSTRUCTIONS       (20000/100)

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

  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_KEY) {
    uint32_t key = *(uint32_t*)lv_event_get_param(e);
    if (key == LV_KEY_LEFT) {
      win->onEvent(EVT_ROTARY_LEFT);
    } else if (key == LV_KEY_RIGHT) {
      win->onEvent(EVT_ROTARY_RIGHT);
    }
  }
  else if (code == LV_EVENT_LONG_PRESSED) {
    lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());
    if(indev_type != LV_INDEV_TYPE_POINTER) {
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
      LuaEventData* es = luaGetEventSlot(EVT_TOUCH_SLIDE);

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
      LuaEventData* es = luaGetEventSlot();
      if (es) {
        es->event = EVT_TOUCH_FIRST;
        es->touchX = rel_pos.x;
        es->touchY = rel_pos.y;
        TRACE("EVT_TOUCH_FIRST [%d,%d]", rel_pos.x, rel_pos.y);
      }

      _startX = rel_pos.x;
      _startY = rel_pos.y;

      downTime = RTOS_GET_MS();
    }
  } else if (code == LV_EVENT_RELEASED) {
    // tap count handling
    uint32_t now = RTOS_GET_MS();
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
  }
#endif  
}

void LuaEventHandler::onClicked()
{
#if defined(HARDWARE_TOUCH)
  auto click_source = lv_indev_get_act();
  bool is_pointer = lv_indev_get_type(click_source) == LV_INDEV_TYPE_POINTER;
  if (is_pointer) {

    lv_point_t point_act;
    lv_indev_get_point(click_source, &point_act);

    LuaEventData* es = luaGetEventSlot();
    if (!es) return;

    if (tapCount > 0) {
      es->event = EVT_TOUCH_TAP;
      es->tapCount = tapCount;
    } else {
      es->event = EVT_TOUCH_BREAK;
    }

    es->touchX = point_act.x;
    es->touchY = point_act.y;

    _sliding = false;
    return;
  }
#endif

  luaPushEvent(EVT_KEY_BREAK(KEY_ENTER));
}

void LuaEventHandler::onCancel()
{
  luaPushEvent(EVT_KEY_BREAK(KEY_EXIT));
}

void LuaEventHandler::onEvent(event_t event)
{
  if (event == EVT_KEY_LONG(KEY_EXIT)) {
    killEvents(KEY_EXIT);
  }
#if !defined(KEYS_GPIO_REG_PGUP)
  else if (event == EVT_KEY_LONG(KEY_PAGEDN)) {
    killEvents(KEY_PAGEDN);
  }
#endif
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

LuaWidget::LuaWidget(const WidgetFactory* factory, Window* parent,
                     const rect_t& rect, WidgetPersistentData* persistentData,
                     int luaWidgetDataRef,int zoneRectDataRef) :
    Widget(factory, parent, rect, persistentData),
    luaWidgetDataRef(luaWidgetDataRef),
    zoneRectDataRef(zoneRectDataRef),
    errorMessage(nullptr)
{
}

LuaWidget::~LuaWidget()
{
  luaL_unref(lsWidgets, LUA_REGISTRYINDEX, luaWidgetDataRef);
  luaL_unref(lsWidgets, LUA_REGISTRYINDEX, zoneRectDataRef);
  free(errorMessage);
}

void LuaWidget::onClicked()
{
  if (!fullscreen) {
    Button::onClicked();
    return;
  }

  LuaEventHandler::onClicked();
}

void LuaWidget::onCancel()
{
  if (!fullscreen) {
    Button::onCancel();
    return;
  }

  LuaEventHandler::onCancel();
}

void LuaWidget::checkEvents()
{
  Widget::checkEvents();

  // paint has not been called
  if (!refreshed) {
    background();
    refreshed = true;
  }
  
  refreshed = false;
  invalidate();

#if defined(DEBUG_WINDOWS)
    TRACE_WINDOWS("# refresh: %s", getWindowDebugString().c_str());
#endif
}

static void l_pushtableint(const char * key, int value)
{
  lua_pushstring(lsWidgets, key);
  lua_pushinteger(lsWidgets, value);
  lua_settable(lsWidgets, -3);
}

void LuaWidget::update()
{
  Widget::update();
  
  if (lsWidgets == 0 || errorMessage) return;
  LuaWidgetFactory * lua_factory = (LuaWidgetFactory *)factory;

  luaSetInstructionsLimit(lsWidgets, MAX_INSTRUCTIONS);
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, lua_factory->updateFunction);
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, luaWidgetDataRef);

  lua_newtable(lsWidgets);
  int i = 0;
  for (const ZoneOption * option = getOptions(); option->name; option++, i++) {
    if (option->type == ZoneOption::String) {
      lua_pushstring(lsWidgets, option->name);
      char str[LEN_ZONE_OPTION_STRING + 1] = {0}; // Zero-terminated string for Lua
      strncpy(str, persistentData->options[i].value.stringValue, LEN_ZONE_OPTION_STRING);
      lua_pushstring(lsWidgets, &str[0]);
      lua_settable(lsWidgets, -3);
    } else if (option->type == ZoneOption::Color) {
      int32_t value = persistentData->options[i].value.signedValue;
      l_pushtableint(option->name, COLOR2FLAGS(value) | RGB_FLAG);
    } else {
      int32_t value = persistentData->options[i].value.signedValue;
      l_pushtableint(option->name, value);
    }
  }

  if (lua_pcall(lsWidgets, 2, 0, 0) != 0) {
    setErrorMessage("update()");
  }
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

void LuaWidget::updateZoneRect(rect_t rect)
{
  if (lsWidgets)
  {
    // Update widget zone with current size and position

    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, zoneRectDataRef);

    bool changed = false;

    if (updateTable("w", rect.w)) changed = true;
    if (updateTable("h", rect.h)) changed = true;
    if (updateTable("xabs", rect.x)) changed = true;
    if (updateTable("yabs", rect.y)) changed = true;

    lua_pop(lsWidgets, 1);

    if (changed)
      update();
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

void LuaWidget::setErrorMessage(const char * funcName)
{
  const char* lua_err = lua_tostring(lsWidgets, -1);
  TRACE("Error in widget %s %s function: %s", factory->getName(), funcName, lua_err);
  TRACE("Widget disabled");

  size_t err_len = snprintf(NULL, 0, "ERROR in %s: %s", funcName, lua_err);
  errorMessage = (char*)malloc(err_len + 1);

  if (errorMessage) {
    snprintf(errorMessage, err_len, "ERROR in %s: %s", funcName, lua_err);
    errorMessage[err_len] = '\0';
  }
}

const char * LuaWidget::getErrorMessage() const
{
  return errorMessage;
}

void LuaWidget::refresh(BitmapBuffer* dc)
{
  if (lsWidgets == 0) return;

  if (errorMessage) {
    drawTextLines(dc, 0, 0, fullscreen ? LCD_W : rect.w,
                  fullscreen ? LCD_H : rect.h, errorMessage,
                  FONT(XS) | COLOR_THEME_WARNING);
    return;
  }

  luaSetInstructionsLimit(lsWidgets, MAX_INSTRUCTIONS);
  LuaWidgetFactory * factory = (LuaWidgetFactory *)this->factory;
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, factory->refreshFunction);
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, luaWidgetDataRef);
  
  // Pass key event to fullscreen Lua widget
  LuaEventData evt;
  luaNextEvent(&evt);

  if (fullscreen) {
    lua_pushinteger(lsWidgets, evt.event);
  }
  else
    lua_pushnil(lsWidgets);

#if defined(HARDWARE_TOUCH)
  if (fullscreen && IS_TOUCH_EVENT(evt.event)) {
    luaPushTouchEventTable(lsWidgets, &evt);
  } else
#endif
    lua_pushnil(lsWidgets);
  
  // Enable drawing into the current LCD buffer
  luaLcdBuffer = dc;

  // This little hack is needed to not interfere with the LCD usage of preempted scripts
  bool lla = luaLcdAllowed;
  luaLcdAllowed = true;
  runningFS = this;

  if (lua_pcall(lsWidgets, 3, 0, 0) != 0) {
    setErrorMessage("refresh()");
  }
  runningFS = nullptr;
  // Remove LCD
  luaLcdAllowed = lla;
  luaLcdBuffer = nullptr;

  // mark as refreshed
  refreshed = true;
}

void LuaWidget::background()
{
  if (lsWidgets == 0 || errorMessage) return;

  // TRACE("LuaWidget::background()");
  luaSetInstructionsLimit(lsWidgets, MAX_INSTRUCTIONS);
  LuaWidgetFactory * factory = (LuaWidgetFactory *)this->factory;
  if (factory->backgroundFunction) {
    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, factory->backgroundFunction);
    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, luaWidgetDataRef);
    runningFS = this;
    if (lua_pcall(lsWidgets, 1, 0, 0) != 0) {
      setErrorMessage("background()");
    }
    runningFS = nullptr;
  }
}

void LuaWidget::onEvent(event_t event)
{
  if (fullscreen) {
    LuaEventHandler::onEvent(event);
  }
  Widget::onEvent(event);
}
