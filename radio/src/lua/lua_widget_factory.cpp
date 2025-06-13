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
#include "strhelpers.h"

#define MAX_INSTRUCTIONS       (20000/100)

LuaWidgetFactory::LuaWidgetFactory(const char* name, ZoneOption* widgetOptions, int optionDefinitionsReference,
                                   int createFunction, int updateFunction, int refreshFunction,
                                   int backgroundFunction, int translateFunction, bool lvglLayout,
                                   const char* filename) :
    WidgetFactory(name, widgetOptions),
    optionDefinitionsReference(optionDefinitionsReference),
    createFunction(createFunction),
    updateFunction(updateFunction),
    refreshFunction(refreshFunction),
    backgroundFunction(backgroundFunction),
    translateFunction(translateFunction),
    lvglLayout(lvglLayout),
    path(filename)
{
  path = path.substr(0, path.rfind("/") + 1);
  translateOptions(widgetOptions);
}

LuaWidgetFactory::~LuaWidgetFactory() {
  unregisterWidget(this);

  if (name) delete name;
  if (displayName) delete displayName;

  auto option = getDefaultOptions();
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

  return new LuaWidget(this, parent, rect, persistentData, zoneRectDataRef, optionsDataRef, createFunction, path);
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

static int switchValue()
{
  int v = SWSRC_INVERT;
  if (lua_istable(lsWidgets, -1)) {
    // Find first available
    int t = lua_gettop(lsWidgets);
    for (lua_pushnil(lsWidgets); v == SWSRC_INVERT && lua_next(lsWidgets, -2); lua_pop(lsWidgets, 1)) {
      v = getSwitchIndex(luaL_checkstring(lsWidgets, -1), false);
    }
    lua_settop(lsWidgets, t);
  } else if (lua_type(lsWidgets, -1) == LUA_TSTRING) {
    v = getSwitchIndex(lua_tostring(lsWidgets, -1), true);
  } else {
    v = luaL_checkinteger(lsWidgets, -1);
  }
  if (v == SWSRC_INVERT) v = SWSRC_NONE;
  return v;
}

static int sourceValue()
{
  int v = -1;
  if (lua_istable(lsWidgets, -1)) {
    // Find first available
    int t = lua_gettop(lsWidgets);
    for (lua_pushnil(lsWidgets); v < 0 && lua_next(lsWidgets, -2); lua_pop(lsWidgets, 1)) {
      v = getSourceIndex(luaL_checkstring(lsWidgets, -1), false);
    }
    lua_settop(lsWidgets, t);
  } else if (lua_type(lsWidgets, -1) == LUA_TSTRING) {
    v = getSourceIndex(lua_tostring(lsWidgets, -1), true);
  } else {
    v = luaL_checkunsigned(lsWidgets, -1);
  }
  if (v == -1) v = MIXSRC_NONE;
  return v;
}

// Parse the options table to get the default, min and max values
// Called when the widget settings dialog is opened to get values
// for current loaded model.
const void LuaWidgetFactory::parseOptionDefaults() const
{
  if (optionDefinitionsReference == LUA_REFNIL) {
    // TRACE("parseOptionDefaults() no options");
    return;
  }

  PROTECT_LUA()
  {
    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, optionDefinitionsReference);
    ZoneOption *option = (ZoneOption*)options;
    for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2), option->name;
         lua_pop(lsWidgets, 1)) {
      // TRACE("parsing option %d", count);
      luaL_checktype(lsWidgets, -2, LUA_TNUMBER);  // key is number
      luaL_checktype(lsWidgets, -1, LUA_TTABLE);   // value is table
      uint8_t field = 0;
      for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2) && field < 5;
           lua_pop(lsWidgets, 1), field++) {
        luaL_checktype(lsWidgets, -2, LUA_TNUMBER);  // key is number
        switch (field) {
          case 2:
            if (option->type == ZoneOption::Switch) {
              option->deflt.signedValue = switchValue();
            } else if (option->type == ZoneOption::Source) {
              option->deflt.unsignedValue = sourceValue();
            } else if (option->type == ZoneOption::Integer) {
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
            if (option->type == ZoneOption::Switch) {
              option->min.signedValue = switchValue();
            } else if (option->type == ZoneOption::Source) {
              option->min.unsignedValue = sourceValue();
            } else if (option->type == ZoneOption::Integer || option->type == ZoneOption::Slider) {
              option->min.signedValue = luaL_checkinteger(lsWidgets, -1);
            } else if (option->type == ZoneOption::Choice) {
              luaL_checktype(lsWidgets, -1, LUA_TTABLE); // value is a table
              option->choiceValues.clear();
              for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2); lua_pop(lsWidgets, 1)) {
                option->choiceValues.push_back(luaL_checkstring(lsWidgets, -1));
              }
            } else if (option->type == ZoneOption::File) {
              option->fileSelectPath = luaL_checkstring(lsWidgets, -1);
            }
            break;
          case 4:
            if (option->type == ZoneOption::Switch) {
              option->max.signedValue = switchValue();
            } else if (option->type == ZoneOption::Source) {
              option->max.unsignedValue = sourceValue();
            } else if (option->type == ZoneOption::Integer || option->type == ZoneOption::Slider) {
              option->max.signedValue = luaL_checkinteger(lsWidgets, -1);
            }
            break;
          default:
            break;
        }
      }
      option++;
    }
  }
  else
  {
    TRACE("error in theme/widget options");
  }
  UNPROTECT_LUA();
  return;
}

// Parse options table to get name and type values.
// Called on radio startup to build base data for all widgtes.
ZoneOption* LuaWidgetFactory::parseOptionDefinitions(int reference)
{
  if (reference == LUA_REFNIL) {
    // TRACE("parseOptionDefinitions() no options");
    return NULL;
  }

  int count = 0;
  lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, reference);
  for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2); lua_pop(lsWidgets, 1)) {
    count++;
  }
  lua_pop(lsWidgets, 1);

  // TRACE("we have %d options", count);
  if (count > MAX_WIDGET_OPTIONS) {
    count = MAX_WIDGET_OPTIONS;
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
          default:
            break;
        }
      }
      option++;
    }
    lua_pop(lsWidgets, 1);
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
