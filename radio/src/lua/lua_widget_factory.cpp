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
#include "lua_states.h"

#define MAX_INSTRUCTIONS       (20000/100)

LuaWidgetFactory::LuaWidgetFactory(const char* name, ZoneOption* widgetOptions,
                                   int createFunction) :
    WidgetFactory(name, widgetOptions),
    createFunction(createFunction),
    updateFunction(0),
    refreshFunction(0),
    backgroundFunction(0),
    translateFunction(0)
{
}

LuaWidgetFactory::~LuaWidgetFactory() {
  unregisterWidget(this);

  if (displayName) {
    delete displayName;
  }

  auto option = getOptions();
  while (option && option->name != nullptr) {
    if (option->displayName) {
      delete option->displayName;
    }
    option++;
  }
}

Widget* LuaWidgetFactory::create(Window* parent, const rect_t& rect,
                                 Widget::PersistentData* persistentData,
                                 bool init) const
{
  if (lsWidgets == 0) return 0;
  initPersistentData(persistentData, init);

  luaSetInstructionsLimit(lsWidgets, MAX_INSTRUCTIONS);
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, createFunction);

  // Make 'zone' table for 'create' call
  lua_newtable(lsWidgets);
  l_pushtableint(lsWidgets, "x", 0);
  l_pushtableint(lsWidgets, "y", 0);
  l_pushtableint(lsWidgets, "w", rect.w);
  l_pushtableint(lsWidgets, "h", rect.h);
  l_pushtableint(lsWidgets, "xabs", rect.x);
  l_pushtableint(lsWidgets, "yabs", rect.y);

  // Store the zone data in registry for later updates
  int zoneRectDataRef = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
  // Push stored zone for 'create' call
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, zoneRectDataRef);

  // Create options table
  lua_newtable(lsWidgets);
  int i = 0;
  for (const ZoneOption* option = options; option->name; option++, i++) {
    if (option->type == ZoneOption::String || option->type == ZoneOption::File) {
      lua_pushstring(lsWidgets, option->name);
      // Zero-terminated string for Lua
      char str[LEN_ZONE_OPTION_STRING + 1] = {0};
      strncpy(str, persistentData->options[i].value.stringValue,
              LEN_ZONE_OPTION_STRING);
      lua_pushstring(lsWidgets, &str[0]);
      lua_settable(lsWidgets, -3);
    } else if (option->type == ZoneOption::Integer || option->type == ZoneOption::Switch) {
      l_pushtableint(lsWidgets, option->name, persistentData->options[i].value.signedValue);
    } else {
      l_pushtableint(lsWidgets, option->name, persistentData->options[i].value.unsignedValue);
    }
  }

  // Store the options data in registry for later updates
  int optionsDataRef = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
  // Push stored options for 'create' call
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, optionsDataRef);

  bool err = lua_pcall(lsWidgets, 2, 1, 0);
  int widgetData = err ? LUA_NOREF : luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
  LuaWidget* lw = new LuaWidget(this, parent, rect, persistentData, widgetData, zoneRectDataRef, optionsDataRef);
  if (err) lw->setErrorMessage("create()");
  return lw;
}


void LuaWidgetFactory::translateOptions(ZoneOption * options)
{
  if (lsWidgets == 0) return;

  // No translations provided
  if (!translateFunction) return;

  auto lang = TRANSLATIONS;

  auto option = options;
  while (option && option->name != nullptr) {
    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, translateFunction);
    lua_pushstring(lsWidgets, option->name);
    lua_pushstring(lsWidgets, lang);
    bool err = lua_pcall(lsWidgets, 2, 1, 0);
    if (!err) {
      auto dn = lua_tostring(lsWidgets, -1);
      if (dn) option->displayName = strdup(dn);
    }
    lua_pop(lsWidgets, 1);

    option++;
  }

  // Widget display name
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, translateFunction);
  lua_pushstring(lsWidgets, name);
  lua_pushstring(lsWidgets, lang);
  bool err = lua_pcall(lsWidgets, 2, 1, 0);
  if (!err) {
    auto dn = lua_tostring(lsWidgets, -1);
    if (dn) displayName = strdup(dn);
  }
  lua_pop(lsWidgets, 1);
}
