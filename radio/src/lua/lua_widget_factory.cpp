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

#include "lua_widget_factory.h"
#include "lua_widget.h"

#include "lua_api.h"
#include "api_colorlcd.h"

#define MAX_INSTRUCTIONS       (20000/100)

static void l_pushtableint(const char * key, int value)
{
  lua_pushstring(lsWidgets, key);
  lua_pushinteger(lsWidgets, value);
  lua_settable(lsWidgets, -3);
}

LuaWidgetFactory::LuaWidgetFactory(const char* name, ZoneOption* widgetOptions,
                                   int createFunction) :
    WidgetFactory(name, widgetOptions),
    createFunction(createFunction),
    updateFunction(0),
    refreshFunction(0),
    backgroundFunction(0)
{
}

LuaWidgetFactory::~LuaWidgetFactory() { unregisterWidget(this); }

Widget* LuaWidgetFactory::create(Window* parent, const rect_t& rect,
                                 Widget::PersistentData* persistentData,
                                 bool init) const
{
  if (lsWidgets == 0) return 0;
  initPersistentData(persistentData, init);

  luaSetInstructionsLimit(lsWidgets, MAX_INSTRUCTIONS);
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
  for (const ZoneOption* option = options; option->name; option++, i++) {
    if (option->type == ZoneOption::String) {
      lua_pushstring(lsWidgets, option->name);
      // Zero-terminated string for Lua
      char str[LEN_ZONE_OPTION_STRING + 1] = {0};
      strncpy(str, persistentData->options[i].value.stringValue,
              LEN_ZONE_OPTION_STRING);
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
  if (err) lw->setErrorMessage("create()");
  return lw;
}
