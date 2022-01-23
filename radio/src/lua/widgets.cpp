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

#include <ctype.h>
#include <stdio.h>
#include "opentx.h"
#include "bin_allocator.h"
#include "lua_api.h"
#include "widget.h"
#include "libopenui_file.h"
#include "api_colorlcd.h"
#include "view_main.h"

#define WIDGET_SCRIPTS_MAX_INSTRUCTIONS    (10000/100)
#define MANUAL_SCRIPTS_MAX_INSTRUCTIONS    (20000/100)
#define LUA_WARNING_INFO_LEN               64

#if defined(HARDWARE_TOUCH)
#include "touch.h"
#define EVT_TOUCH_SWIPE_LOCK     4
#define EVT_TOUCH_SWIPE_SPEED   60
#define EVT_TOUCH_SWIPE_TIMEOUT 50
#endif

constexpr int LUA_WIDGET_REFRESH = 1000 / 10; // 10 Hz

lua_State * lsWidgets = NULL;

extern int custom_lua_atpanic(lua_State *L);

#define LUA_WIDGET_FILENAME                "/main.lua"
#define LUA_FULLPATH_MAXLEN                (LEN_FILE_PATH_MAX + LEN_SCRIPT_FILENAME + LEN_FILE_EXTENSION_MAX)  // max length (example: /SCRIPTS/THEMES/mytheme.lua)

static void luaHook(lua_State * L, lua_Debug *ar)
{
  if (ar->event == LUA_HOOKCOUNT) {
    instructionsPercent++;
#if defined(DEBUG)
  // Disable Lua script instructions limit in DEBUG mode,
  // just report max value reached
  static uint16_t max = 0;
  if (instructionsPercent > 100) {
    if (max + 10 < instructionsPercent) {
      max = instructionsPercent;
      TRACE("LUA instructionsPercent %u%%", (uint32_t)max);
    }
  }
  else if (instructionsPercent < 10) {
    max = 0;
  }
#else
    if (instructionsPercent > 100) {
      // From now on, as soon as a line is executed, error
      // keep erroring until you're script reaches the top
      lua_sethook(L, luaHook, LUA_MASKLINE, 0);
      luaL_error(L, "CPU limit");
    }
#endif
  }
#if defined(LUA_ALLOCATOR_TRACER)
  else if (ar->event == LUA_HOOKLINE) {
    lua_getinfo(L, "nSl", ar);
    LuaMemTracer * tracer = GET_TRACER(L);
    if (tracer->alloc || tracer->free) {
      TRACE("LT: [+%u,-%u] %s:%d", tracer->alloc, tracer->free, tracer->script, tracer->lineno);
    }
    tracer->script = ar->source;
    tracer->lineno = ar->currentline;
    tracer->alloc = 0;
    tracer->free = 0;
  }
#endif // #if defined(LUA_ALLOCATOR_TRACER)
}

void luaSetInstructionsLimit(lua_State * L, int count)
{
  instructionsPercent = 0;
#if defined(LUA_ALLOCATOR_TRACER)
  lua_sethook(L, luaHook, LUA_MASKCOUNT|LUA_MASKLINE, count);
#else
  lua_sethook(L, luaHook, LUA_MASKCOUNT, count);
#endif
}

ZoneOption * createOptionsArray(int reference, uint8_t maxOptions)
{
  if (reference == 0) {
    // TRACE("createOptionsArray() no options");
    return NULL;
  }

  int count = 0;
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, reference);
  for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2); lua_pop(lsWidgets, 1)) {
    count++;
  }

  // TRACE("we have %d options", count);
  if (count > maxOptions) {
    count = maxOptions;
    // TRACE("limited to %d options", count);
  }

  ZoneOption * options = (ZoneOption *)malloc(sizeof(ZoneOption) * (count+1));
  if (!options) {
    return NULL;
  }

  PROTECT_LUA() {
    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, reference);
    ZoneOption * option = options;
    for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2), count-- > 0; lua_pop(lsWidgets, 1)) {
      // TRACE("parsing option %d", count);
      luaL_checktype(lsWidgets, -2, LUA_TNUMBER); // key is number
      luaL_checktype(lsWidgets, -1, LUA_TTABLE); // value is table
      uint8_t field = 0;
      for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2) && field<5; lua_pop(lsWidgets, 1), field++) {
        switch (field) {
          case 0:
            luaL_checktype(lsWidgets, -2, LUA_TNUMBER); // key is number
            luaL_checktype(lsWidgets, -1, LUA_TSTRING); // value is string
            option->name = lua_tostring(lsWidgets, -1);
            // TRACE("name = %s", option->name);
            break;
          case 1:
            luaL_checktype(lsWidgets, -2, LUA_TNUMBER); // key is number
            luaL_checktype(lsWidgets, -1, LUA_TNUMBER); // value is number
            option->type = (ZoneOption::Type)lua_tointeger(lsWidgets, -1);
            // TRACE("type = %d", option->type);
            if (option->type > ZoneOption::Color) {
              // wrong type
              option->type = ZoneOption::Integer;
            }
            if (option->type == ZoneOption::Integer) {
              // set some sensible defaults (only Integer actually uses them)
              option->deflt.signedValue = 0;
              option->min.signedValue = -100;
              option->max.signedValue = 100;
            }
            break;
          case 2:
            luaL_checktype(lsWidgets, -2, LUA_TNUMBER); // key is number
            if (option->type == ZoneOption::Integer) {
              luaL_checktype(lsWidgets, -1, LUA_TNUMBER); // value is number
              option->deflt.signedValue = lua_tointeger(lsWidgets, -1);
              // TRACE("default signed = %d", option->deflt.signedValue);
            }
            else if (option->type == ZoneOption::Source ||
                     option->type == ZoneOption::TextSize) {
              luaL_checktype(lsWidgets, -1, LUA_TNUMBER); // value is number
              option->deflt.unsignedValue = lua_tounsigned(lsWidgets, -1);
              // TRACE("default unsigned = %u", option->deflt.unsignedValue);
            }
            else if (option->type == ZoneOption::Color) {
              luaL_checktype(lsWidgets, -1, LUA_TNUMBER); // value is number
              option->deflt.unsignedValue = COLOR_VAL(flagsRGB(lua_tounsigned(lsWidgets, -1)));
              // TRACE("default unsigned = %u", option->deflt.unsignedValue);
            }
            else if (option->type == ZoneOption::Bool) {
              luaL_checktype(lsWidgets, -1, LUA_TNUMBER); // value is number
              option->deflt.boolValue = (lua_tounsigned(lsWidgets, -1) != 0);
              // TRACE("default bool = %d", (int)(option->deflt.boolValue));
            }
            else if (option->type == ZoneOption::String) {
              strncpy(option->deflt.stringValue, lua_tostring(lsWidgets, -1), LEN_ZONE_OPTION_STRING);
              // TRACE("default string = %s", lua_tostring(lsWidgets, -1));
            }
            break;
          case 3:
            if (option->type == ZoneOption::Integer) {
              luaL_checktype(lsWidgets, -2, LUA_TNUMBER); // key is number
              luaL_checktype(lsWidgets, -1, LUA_TNUMBER); // value is number
              option->min.signedValue = lua_tointeger(lsWidgets, -1);
            }
            break;
          case 4:
            if (option->type == ZoneOption::Integer) {
              luaL_checktype(lsWidgets, -2, LUA_TNUMBER); // key is number
              luaL_checktype(lsWidgets, -1, LUA_TNUMBER); // value is number
              option->max.signedValue = lua_tointeger(lsWidgets, -1);
            }
            break;
        }
      }
      option++;
    }
    option->name = NULL; // sentinel
  }
  else {
    TRACE("error in theme/widget options");
    free(options);
    return NULL;
  }
  UNPROTECT_LUA();
  return options;
}

struct eventData {
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
};

class LuaWidget: public Widget
{
  friend class LuaWidgetFactory;
  
  public:
    LuaWidget(const WidgetFactory * factory, FormGroup * parent, const rect_t & rect, WidgetPersistentData * persistentData, int luaWidgetDataRef):
      Widget(factory, parent, rect, persistentData),
      luaWidgetDataRef(luaWidgetDataRef),
      errorMessage(nullptr)
    {
    }

    ~LuaWidget() override
    {
      luaL_unref(lsWidgets, LUA_REGISTRYINDEX, luaWidgetDataRef);
      free(errorMessage);
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "LuaWidget";
    }
#endif
  
    // Widget interface
    const char * getErrorMessage() const override;
    void update() override;
    void background() override;
    
#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif
    
#if defined(HARDWARE_TOUCH)
    bool onTouchStart(coord_t x, coord_t y) override;
    bool onTouchEnd(coord_t x, coord_t y) override;
    bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY) override;
#endif

    // Calls LUA widget 'refresh' method
    void refresh(BitmapBuffer* dc) override;

  protected:
    int    luaWidgetDataRef;
    char * errorMessage;
    uint32_t lastRefresh = 0;
    bool     refreshed = false;

    static eventData events[EVENT_BUFFER_SIZE];
#if defined(HARDWARE_TOUCH)
    static bool fingerDown;
    static tmr10ms_t swipeTimeOut;
#endif  

    void checkEvents() override;
    void setErrorMessage(const char * funcName);
  
  private:
    eventData* findOpenEventSlot(event_t event = 0);
};

eventData LuaWidget::events[EVENT_BUFFER_SIZE] = { 0 };

#if defined(HARDWARE_TOUCH)
  bool LuaWidget::fingerDown = false;
  tmr10ms_t LuaWidget::swipeTimeOut = 0;
#endif  

static void l_pushtableint(const char * key, int value)
{
  lua_pushstring(lsWidgets, key);
  lua_pushinteger(lsWidgets, value);
  lua_settable(lsWidgets, -3);
}

static void l_pushtablebool(const char * key, bool value)
{
  lua_pushstring(lsWidgets, key);
  lua_pushboolean(lsWidgets, value);
  lua_settable(lsWidgets, -3);
}

class LuaWidgetFactory: public WidgetFactory
{
  friend void luaLoadWidgetCallback();
  friend class LuaWidget;

  public:
    LuaWidgetFactory(const char * name, ZoneOption * widgetOptions, int createFunction):
      WidgetFactory(name, widgetOptions),
      createFunction(createFunction),
      updateFunction(0),
      refreshFunction(0),
      backgroundFunction(0)
    {
    }

    ~LuaWidgetFactory()
    {
      unregisterWidget(this);
    }

    Widget * create(FormGroup * parent, const rect_t & rect, Widget::PersistentData * persistentData, bool init=true) const override
    {
      if (lsWidgets == 0) return 0;
      initPersistentData(persistentData, init);

      luaSetInstructionsLimit(lsWidgets, WIDGET_SCRIPTS_MAX_INSTRUCTIONS);
      lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, createFunction);

      lua_newtable(lsWidgets);
      l_pushtableint("x", 0);
      l_pushtableint("y", 0);
      l_pushtableint("w", rect.w);
      l_pushtableint("h", rect.h);
      l_pushtableint("xabs", rect.x);
      l_pushtableint("yabs", rect.y);

      lua_newtable(lsWidgets);
      int i = 0;
      for (const ZoneOption * option = options; option->name; option++, i++) {
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

      bool err = lua_pcall(lsWidgets, 2, 1, 0);
      int widgetData = err ? LUA_NOREF : luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      LuaWidget* lw = new LuaWidget(this, parent, rect, persistentData, widgetData);
      if (err)
        lw->setErrorMessage("create()");
      return lw;
    }

  protected:
    int createFunction;
    int updateFunction;
    int refreshFunction;
    int backgroundFunction;
};

// Look for a slot in the event buffer that is either unused (zero) or matches event
eventData* LuaWidget::findOpenEventSlot(event_t event)
{
  for (int i = 0; i < EVENT_BUFFER_SIZE; i++) {
    if (events[i].event == event || events[i].event == 0)
      return &events[i];
  }

  return NULL;
}

void LuaWidget::checkEvents()
{
  Widget::checkEvents();

  // paint has not been called
  if (!refreshed) {
    background();
    refreshed = true;
  }
  
  uint32_t now = RTOS_GET_MS();
  if (now - lastRefresh >= LUA_WIDGET_REFRESH) {
    lastRefresh = now;
    refreshed = false;
    invalidate();

#if defined(DEBUG_WINDOWS)
    TRACE_WINDOWS("# refresh: %s", getWindowDebugString().c_str());
#endif
  }
}

void LuaWidget::update()
{
  Widget::update();
  
  if (lsWidgets == 0 || errorMessage) return;

  luaSetInstructionsLimit(lsWidgets, WIDGET_SCRIPTS_MAX_INSTRUCTIONS);
  LuaWidgetFactory * factory = (LuaWidgetFactory *)this->factory;
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, factory->updateFunction);
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

void LuaWidget::setErrorMessage(const char * funcName)
{
  TRACE("Error in widget %s %s function: %s", factory->getName(), funcName, lua_tostring(lsWidgets, -1));
  TRACE("Widget disabled");
  size_t needed = snprintf(NULL, 0, "ERROR in %s: %s", funcName, lua_tostring(lsWidgets, -1)) + 1;
  errorMessage = (char *)malloc(needed + 1);
  if (errorMessage) {
    snprintf(errorMessage, needed, "ERROR in %s: %s", funcName, lua_tostring(lsWidgets, -1));
    errorMessage[needed] = '\0';
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
    drawTextLines(dc, 0, 0, fullscreen ? LCD_W : rect.w, fullscreen ? LCD_H : rect.h, errorMessage, FONT(XS) | COLOR_THEME_WARNING);
    return;
  }

  luaSetInstructionsLimit(lsWidgets, WIDGET_SCRIPTS_MAX_INSTRUCTIONS);
  LuaWidgetFactory * factory = (LuaWidgetFactory *)this->factory;
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, factory->refreshFunction);
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, luaWidgetDataRef);
  
  // Pass key event to fullscreen Lua widget
  eventData* es = &events[0];
  if (fullscreen)
    lua_pushinteger(lsWidgets, es->event);
  else
    lua_pushnil(lsWidgets);

#if defined(HARDWARE_TOUCH)
  if (fullscreen && IS_TOUCH_EVENT(es->event)) {
    lua_newtable(lsWidgets);
    l_pushtableint("x", es->touchX);
    l_pushtableint("y", es->touchY);
    l_pushtableint("tapCount", es->tapCount);
    
    if (es->event == EVT_TOUCH_SLIDE) {
      l_pushtableint("startX", es->startX);
      l_pushtableint("startY", es->startY);
      l_pushtableint("slideX", es->slideX);
      l_pushtableint("slideY", es->slideY);

      // Do we have a swipe? Only one at a time!
      if (get_tmr10ms() > swipeTimeOut) {
        coord_t absX = (es->slideX < 0) ? -(es->slideX) : es->slideX;
        coord_t absY = (es->slideY < 0) ? -(es->slideY) : es->slideY;
        bool swiped = false;
  
        if (absX > EVT_TOUCH_SWIPE_LOCK * absY) {
          if ((swiped = (es->slideX > EVT_TOUCH_SWIPE_SPEED)))
            l_pushtablebool("swipeRight", true);
          else if ((swiped = (es->slideX < -EVT_TOUCH_SWIPE_SPEED)))
            l_pushtablebool("swipeLeft", true);
        }
        else if (absY > EVT_TOUCH_SWIPE_LOCK * absX) {
          if ((swiped = (es->slideY > EVT_TOUCH_SWIPE_SPEED)))
            l_pushtablebool("swipeDown", true);
          else if ((swiped = (es->slideY < -EVT_TOUCH_SWIPE_SPEED)))
            l_pushtablebool("swipeUp", true);
        }
        
        if (swiped)
          swipeTimeOut = get_tmr10ms() + EVT_TOUCH_SWIPE_TIMEOUT;
      }
    }
  } else
#endif
    lua_pushnil(lsWidgets);
  
  // Move the event buffer forward
  for (int i = 1; i < EVENT_BUFFER_SIZE; i++)
    events[i - 1] = events[i];
  memclear(&events[EVENT_BUFFER_SIZE - 1], sizeof(eventData));

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
  luaSetInstructionsLimit(lsWidgets, WIDGET_SCRIPTS_MAX_INSTRUCTIONS);
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

#if defined(HARDWARE_KEYS)
void LuaWidget::onEvent(event_t event)
{
  if (fullscreen) {
    if (EVT_KEY_LONG(KEY_EXIT) == event) {
      // Clear event buffer on full screen exit
      memclear(&events, EVENT_BUFFER_SIZE * sizeof(eventData));
    } else {
      eventData* es = findOpenEventSlot();

      if (es)
        es->event = event;
    }
  }
  Widget::onEvent(event);
}
#endif

#if defined(HARDWARE_TOUCH)
bool LuaWidget::onTouchStart(coord_t x, coord_t y)
{
  TRACE_WINDOWS("LuaWidget received touch start (%d) x=%d;y=%d", hasFocus(), x, y);

  // Only one EVT_TOUCH_FIRST at a time
  if (fullscreen) {
    if (!fingerDown) {
      eventData* es = findOpenEventSlot();

      if (es) {
        es->event = EVT_TOUCH_FIRST;
        es->touchX = x;
        es->touchY = y;
      }
      
      fingerDown = true;
    }

    return true;
  }

  return Widget::onTouchStart(x, y);
}

bool LuaWidget::onTouchEnd(coord_t x, coord_t y)
{
  TRACE_WINDOWS("LuaWidget received touch end (%d) x=%d;y=%d", hasFocus(), x, y);

  if (fullscreen) {
    eventData* es = findOpenEventSlot();

    if (es) {
      if (touchState.tapCount > 0) {
        es->event = EVT_TOUCH_TAP;
        es->tapCount = touchState.tapCount;
      } else
        es->event = EVT_TOUCH_BREAK;
      es->touchX = x;
      es->touchY = y;
    }

    fingerDown = false;
    return true;
  }

  return Widget::onTouchEnd(x, y);
}

bool LuaWidget::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  TRACE_WINDOWS("LuaWidget touch slide");
  if (fullscreen) {
    // If we already have a SLIDE going, then accumulate slide values instead of allocating an empty slot
    eventData* es = findOpenEventSlot(EVT_TOUCH_SLIDE);

    if (es) {
      ViewMain* vm = ViewMain::instance();
      es->event = EVT_TOUCH_SLIDE;
      es->touchX = x + vm->getScrollPositionX();
      es->touchY = y + vm->getScrollPositionY();
      es->startX = startX;
      es->startY = startY;
      es->slideX += slideX;
      es->slideY += slideY;
    }
    
    fingerDown = false;    
    return true;
  }
  
  return Widget::onTouchSlide(x, y, startX, startY, slideX, slideY);
}
#endif

void luaLoadWidgetCallback()
{
  TRACE("luaLoadWidgetCallback()");
  const char * name=NULL;
  int widgetOptions=0, createFunction=0, updateFunction=0, refreshFunction=0, backgroundFunction=0;

  luaL_checktype(lsWidgets, -1, LUA_TTABLE);

  for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2); lua_pop(lsWidgets, 1)) {
    const char * key = lua_tostring(lsWidgets, -2);
    if (!strcmp(key, "name")) {
      name = luaL_checkstring(lsWidgets, -1);
    }
    else if (!strcmp(key, "options")) {
      widgetOptions = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "create")) {
      createFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "update")) {
      updateFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "refresh")) {
      refreshFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "background")) {
      backgroundFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
  }

  if (name && createFunction) {
    ZoneOption * options = createOptionsArray(widgetOptions, MAX_WIDGET_OPTIONS);
    if (options) {
      LuaWidgetFactory * factory = new LuaWidgetFactory(name, options, createFunction);
      factory->updateFunction = updateFunction;
      factory->refreshFunction = refreshFunction;
      factory->backgroundFunction = backgroundFunction;   // NOSONAR
      TRACE("Loaded Lua widget %s", name);
    }
  }
}

void luaLoadFile(const char * filename, void (*callback)())
{
  if (lsWidgets == NULL || callback == NULL)
    return;

  TRACE("luaLoadFile(%s)", filename);

  luaSetInstructionsLimit(lsWidgets, MANUAL_SCRIPTS_MAX_INSTRUCTIONS);

  PROTECT_LUA() {
    if (luaLoadScriptFileToState(lsWidgets, filename, LUA_SCRIPT_LOAD_MODE) == SCRIPT_OK) {
      if (lua_pcall(lsWidgets, 0, 1, 0) == LUA_OK && lua_istable(lsWidgets, -1)) {
        (*callback)();
      }
      else {
        TRACE("luaLoadFile(%s): Error parsing script: %s", filename, lua_tostring(lsWidgets, -1));
      }
    }
  }
  else {
    // error while loading Lua widget/theme,
    // do not disable whole Lua state, just ingnore bad widget/theme
    return;
  }
  UNPROTECT_LUA();
}

void luaLoadFiles(const char * directory, void (*callback)())
{
  char path[LUA_FULLPATH_MAXLEN+1];
  FILINFO fno;
  DIR dir;

  strcpy(path, directory);
  TRACE("luaLoadFiles() %s", path);

  FRESULT res = f_opendir(&dir, path);        /* Open the directory */

  if (res == FR_OK) {
    int pathlen = strlen(path);
    path[pathlen++] = '/';
    for (;;) {
      res = f_readdir(&dir, &fno);                   /* Read a directory item */
      if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
      uint8_t len = strlen(fno.fname);
      if (len > 0 && (unsigned int)(len + pathlen + sizeof(LUA_WIDGET_FILENAME)) <= sizeof(path) &&
          fno.fname[0]!='.' && (fno.fattrib & AM_DIR)) {
        strcpy(&path[pathlen], fno.fname);
        strcat(&path[pathlen], LUA_WIDGET_FILENAME);
        if (isFileAvailable(path)) {
          luaLoadFile(path, callback);
        }
      }
    }
  }
  else {
    TRACE("f_opendir(%s) failed, code=%d", path, res);
  }

  f_closedir(&dir);
}

#if defined(LUA_ALLOCATOR_TRACER)
LuaMemTracer lsWidgetsTrace;
#endif

void luaInitThemesAndWidgets()
{
  TRACE("luaInitThemesAndWidgets");

#if defined(USE_BIN_ALLOCATOR)
  lsWidgets = lua_newstate(bin_l_alloc, NULL);   //we use our own allocator!
#elif defined(LUA_ALLOCATOR_TRACER)
  memclear(&lsWidgetsTrace, sizeof(lsWidgetsTrace));
  lsWidgetsTrace.script = "lua_newstate(widgets)";
  lsWidgets = lua_newstate(tracer_alloc, &lsWidgetsTrace);   //we use tracer allocator
#else
  lsWidgets = lua_newstate(l_alloc, NULL);   //we use Lua default allocator
#endif
  if (lsWidgets) {
    // install our panic handler
    lua_atpanic(lsWidgets, &custom_lua_atpanic);

#if defined(LUA_ALLOCATOR_TRACER)
    lua_sethook(lsWidgets, luaHook, LUA_MASKLINE, 0);
#endif

    // protect libs and constants registration
    PROTECT_LUA() {
      luaRegisterLibraries(lsWidgets);
    }
    else {
      // if we got panic during registration
      // we disable Lua for this session
      // luaDisable();
      luaClose(&lsWidgets);
      lsWidgets = 0;
    }
    UNPROTECT_LUA();
    TRACE("lsWidgets %p", lsWidgets);
    //luaLoadFiles(THEMES_PATH, luaLoadThemeCallback);
    luaLoadFiles(WIDGETS_PATH, luaLoadWidgetCallback);
    luaDoGc(lsWidgets, true);
  }
}

void luaUnregisterWidgets()
{
  std::list<const WidgetFactory *> regWidgets(getRegisteredWidgets());
  for (auto w : regWidgets) {
    if (dynamic_cast<const LuaWidgetFactory*>(w)) {
      delete w;
    }
  }
}
