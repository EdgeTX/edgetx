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

#include "edgetx.h"
#include "lua_api.h"

#include "widget.h"
#include "lib_file.h"
#include "view_main.h"

#include "lua_widget.h"
#include "lua_widget_factory.h"

#include "lua_states.h"

#define MAX_INSTRUCTIONS       (20000/100)
#define LUA_WARNING_INFO_LEN    64

lua_State * lsWidgets = NULL;

extern int custom_lua_atpanic(lua_State *L);

#define LUA_WIDGET_FILENAME "/main.lua"
#define LUA_FULLPATH_MAXLEN                  \
  (LEN_FILE_PATH_MAX + LEN_SCRIPT_FILENAME + \
   LEN_FILE_EXTENSION_MAX)

static void luaHook(lua_State *L, lua_Debug *ar)
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
    } else if (instructionsPercent < 10) {
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
    LuaMemTracer *tracer = GET_TRACER(L);
    if (tracer->alloc || tracer->free) {
      TRACE("LT: [+%u,-%u] %s:%d", tracer->alloc, tracer->free, tracer->script,
            tracer->lineno);
    }
    tracer->script = ar->source;
    tracer->lineno = ar->currentline;
    tracer->alloc = 0;
    tracer->free = 0;
  }
#endif  // #if defined(LUA_ALLOCATOR_TRACER)
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

ZoneOption *createOptionsArray(int reference, uint8_t maxOptions)
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

  ZoneOption *options = new ZoneOption[count + 1];
  if (!options) {
    return NULL;
  }

  PROTECT_LUA()
  {
    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, reference);
    ZoneOption *option = options;
    for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2), count-- > 0;
         lua_pop(lsWidgets, 1)) {
      // TRACE("parsing option %d", count);
      luaL_checktype(lsWidgets, -2, LUA_TNUMBER);  // key is number
      luaL_checktype(lsWidgets, -1, LUA_TTABLE);   // value is table
      uint8_t field = 0;
      for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2) && field < 5;
           lua_pop(lsWidgets, 1), field++) {
        luaL_checktype(lsWidgets, -2, LUA_TNUMBER);  // key is number
        switch (field) {
          case 0:
            option->name = luaL_checkstring(lsWidgets, -1);
            option->displayName = nullptr;
            // TRACE("name = %s", option->name);
            break;
          case 1:
            option->type = (ZoneOption::Type)luaL_checkinteger(lsWidgets, -1);
            option->deflt.unsignedValue = 0;
            // set some sensible defaults
            if (option->type == ZoneOption::Integer) {
              option->min.signedValue = -100;
              option->max.signedValue = 100;
            } else if (option->type == ZoneOption::Switch) {
              option->min.signedValue = SWSRC_FIRST;
              option->max.signedValue = SWSRC_LAST;
            } else if (option->type == ZoneOption::Timer) {
              option->min.unsignedValue = 0;
              option->max.unsignedValue = MAX_TIMERS - 1;
            } else if (option->type == ZoneOption::TextSize) {
              option->min.unsignedValue = FONT_STD_INDEX;
              option->max.unsignedValue = FONTS_COUNT - 1;
            } else if (option->type == ZoneOption::String || option->type == ZoneOption::File) {
              option->deflt.stringValue[0] = 0;
            } else if (option->type == ZoneOption::Slider) {
              option->min.unsignedValue = 0;
              option->max.unsignedValue = 9;
            }
            break;
          case 2:
            if (option->type == ZoneOption::Integer || option->type == ZoneOption::Switch) {
              option->deflt.signedValue = luaL_checkinteger(lsWidgets, -1);
            } else if (option->type == ZoneOption::Bool) {
              option->deflt.boolValue = (luaL_checkunsigned(lsWidgets, -1) != 0);
            } else if (option->type == ZoneOption::String || option->type == ZoneOption::File) {
              strncpy(option->deflt.stringValue, luaL_checkstring(lsWidgets, -1),
                      LEN_ZONE_OPTION_STRING);
            } else {
              option->deflt.unsignedValue = luaL_checkunsigned(lsWidgets, -1);
            }
            break;
          case 3:
            if (option->type == ZoneOption::Integer || option->type == ZoneOption::Switch || option->type == ZoneOption::Slider) {
              option->min.signedValue = luaL_checkinteger(lsWidgets, -1);
            } else if (option->type == ZoneOption::Choice) {
              luaL_checktype(lsWidgets, -1, LUA_TTABLE); // value is a table
              for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2); lua_pop(lsWidgets, 1)) {
                option->choiceValues.push_back(luaL_checkstring(lsWidgets, -1));
              }
            } else if (option->type == ZoneOption::File) {
              option->fileSelectPath = luaL_checkstring(lsWidgets, -1);
            }
            break;
          case 4:
            if (option->type == ZoneOption::Integer || option->type == ZoneOption::Switch || option->type == ZoneOption::Slider) {
              option->max.signedValue = luaL_checkinteger(lsWidgets, -1);
            }
            break;
        }
      }
      option++;
    }
    option->name = NULL;  // sentinel
  }
  else
  {
    TRACE("error in theme/widget options");
    delete[] options;
    return NULL;
  }
  UNPROTECT_LUA();
  return options;
}

void luaLoadWidgetCallback()
{
  TRACE("luaLoadWidgetCallback()");
  const char * name=NULL;

  int widgetOptions = 0, createFunction = 0, updateFunction = 0, settingsFunction = 0,
      refreshFunction = 0, backgroundFunction = 0, translateFunction = 0;
  bool lvglLayout = false;

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
    else if (!strcmp(key, "translate")) {
      translateFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "settings")) {
      settingsFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcasecmp(key, "useLvgl")) {
      lvglLayout = lua_toboolean(lsWidgets, -1);
    }
  }

  if (name && createFunction) {
    ZoneOption * options = createOptionsArray(widgetOptions, MAX_WIDGET_OPTIONS);
    if (options) {
      LuaWidgetFactory * factory = new LuaWidgetFactory(name, options, createFunction);
      factory->updateFunction = updateFunction;
      factory->refreshFunction = refreshFunction;
      factory->backgroundFunction = backgroundFunction;
      factory->translateFunction = translateFunction;
      factory->settingsFunction = settingsFunction;
      factory->translateOptions(options);
      factory->lvglLayout = lvglLayout;
      TRACE("Loaded Lua widget %s", name);
    }
  }
}

void luaLoadFile(const char * filename, void (*callback)())
{
  if (lsWidgets == NULL || callback == NULL)
    return;

  TRACE("luaLoadFile(%s)", filename);

  luaSetInstructionsLimit(lsWidgets, MAX_INSTRUCTIONS);

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
  lsWidgets = luaL_newstate();   //we use Lua default allocator
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
    luaLoadFiles(WIDGETS_PATH, luaLoadWidgetCallback);
    luaDoGc(lsWidgets, true);
  }
}

void luaUnregisterWidgets()
{
  std::list<const WidgetFactory *> regWidgets(WidgetFactory::getRegisteredWidgets());
  for (auto w : regWidgets) {
    if (w->isLuaWidgetFactory()) {
      delete w;
    }
  }
}
