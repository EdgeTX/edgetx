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

LuaWidgetFactory::LuaWidgetFactory(const char* name, WidgetOption* widgetOptions, int optionDefinitionsReference,
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

  luaL_unref(lsWidgets, LUA_REGISTRYINDEX, optionDefinitionsReference);
  luaL_unref(lsWidgets, LUA_REGISTRYINDEX, createFunction);
  luaL_unref(lsWidgets, LUA_REGISTRYINDEX, updateFunction);
  luaL_unref(lsWidgets, LUA_REGISTRYINDEX, refreshFunction);
  luaL_unref(lsWidgets, LUA_REGISTRYINDEX, backgroundFunction);
  luaL_unref(lsWidgets, LUA_REGISTRYINDEX, translateFunction);

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

Widget* LuaWidgetFactory::createNew(Window* parent, const rect_t& rect,
                                    int screenNum, int zoneNum) const
{
  if (lsWidgets == 0) return 0;

  auto widgetData = g_model.getWidgetData(screenNum, zoneNum);

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
  for (const WidgetOption* option = options; option->name; option++, i++) {
    if (option->type == WidgetOption::String || option->type == WidgetOption::File) {
      lua_pushstring(lsWidgets, option->name);
      lua_pushstring(lsWidgets, widgetData->options[i].value.stringValue.c_str());
      lua_settable(lsWidgets, -3);
    } else if (option->type == WidgetOption::Integer || option->type == WidgetOption::Switch) {
      l_pushtableint(lsWidgets, option->name, widgetData->options[i].value.signedValue);
    } else {
      l_pushtableint(lsWidgets, option->name, widgetData->options[i].value.unsignedValue);
    }
  }

  // Store the options data in registry for later updates
  int optionsDataRef = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);

  return new LuaWidget(this, parent, rect, screenNum, zoneNum, zoneRectDataRef, optionsDataRef, createFunction, path);
}

void LuaWidgetFactory::translateOptions(WidgetOption * options)
{
  if (lsWidgets == 0) return;

  // No translations provided
  if (translateFunction == LUA_REFNIL) return;

#if defined(ALL_LANGS)
  char lang[3];
  lang[0] = toupper(g_eeGeneral.uiLanguage[0]);
  lang[1] = toupper(g_eeGeneral.uiLanguage[1]);
  lang[2] = 0;
#else
  const char* lang = TRANSLATIONS;
#endif

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
    WidgetOption *option = (WidgetOption*)options;
    for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2), option->name;
         lua_pop(lsWidgets, 1)) {
      // TRACE("parseOptionDefaults parsing option %s", option->name);
      luaL_checktype(lsWidgets, -2, LUA_TNUMBER);  // key is number
      luaL_checktype(lsWidgets, -1, LUA_TTABLE);   // value is table
      uint8_t field = 0;
      for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2) && field < 5;
           lua_pop(lsWidgets, 1), field++) {
        luaL_checktype(lsWidgets, -2, LUA_TNUMBER);  // key is number
        switch (field) {
          case 2:
            if (option->type == WidgetOption::Switch) {
              option->deflt.signedValue = switchValue();
            } else if (option->type == WidgetOption::Source) {
              option->deflt.unsignedValue = sourceValue();
            } else if (option->type == WidgetOption::Integer) {
              option->deflt.signedValue = luaL_checkinteger(lsWidgets, -1);
            } else if (option->type == WidgetOption::Bool) {
              option->deflt.boolValue = (luaL_checkunsigned(lsWidgets, -1) != 0);
            } else if (option->type == WidgetOption::String || option->type == WidgetOption::File) {
              option->deflt.stringValue = luaL_checkstring(lsWidgets, -1);
            } else {
              option->deflt.unsignedValue = luaL_checkunsigned(lsWidgets, -1);
            }
            break;
          case 3:
            if (option->type == WidgetOption::Switch) {
              option->min.signedValue = switchValue();
            } else if (option->type == WidgetOption::Source) {
              option->min.unsignedValue = sourceValue();
            } else if (option->type == WidgetOption::Integer || option->type == WidgetOption::Slider) {
              option->min.signedValue = luaL_checkinteger(lsWidgets, -1);
            } else if (option->type == WidgetOption::Choice) {
              luaL_checktype(lsWidgets, -1, LUA_TTABLE); // value is a table
              option->choiceValues.clear();
              for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2); lua_pop(lsWidgets, 1)) {
                option->choiceValues.push_back(luaL_checkstring(lsWidgets, -1));
              }
            } else if (option->type == WidgetOption::File) {
              option->fileSelectPath = luaL_checkstring(lsWidgets, -1);
            }
            break;
          case 4:
            if (option->type == WidgetOption::Switch) {
              option->max.signedValue = switchValue();
            } else if (option->type == WidgetOption::Source) {
              option->max.unsignedValue = sourceValue();
            } else if (option->type == WidgetOption::Integer || option->type == WidgetOption::Slider) {
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
WidgetOption* LuaWidgetFactory::parseOptionDefinitions(int reference)
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

  WidgetOption *options = new WidgetOption[count + 1];
  if (!options) {
    return NULL;
  }

  PROTECT_LUA()
  {
    lua_rawgeti(lsWidgets, LUA_REGISTRYINDEX, reference);
    WidgetOption *option = options;
    for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2), count-- > 0;
         lua_pop(lsWidgets, 1)) {
      // TRACE("parseOptionDefinitions parsing option %d", count);
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
            option->type = (WidgetOption::Type)luaL_checkinteger(lsWidgets, -1);
            option->deflt.unsignedValue = 0;
            // set some sensible defaults
            if (option->type == WidgetOption::Integer) {
              option->min.signedValue = -100;
              option->max.signedValue = 100;
            } else if (option->type == WidgetOption::Switch) {
              option->min.signedValue = SWSRC_FIRST;
              option->max.signedValue = SWSRC_LAST;
            } else if (option->type == WidgetOption::Timer) {
              option->min.unsignedValue = 0;
              option->max.unsignedValue = MAX_TIMERS - 1;
            } else if (option->type == WidgetOption::TextSize) {
              option->min.unsignedValue = FONT_STD_INDEX;
              option->max.unsignedValue = FONTS_COUNT - 1;
            } else if (option->type == WidgetOption::String || option->type == WidgetOption::File) {
              option->deflt.stringValue.clear();
            } else if (option->type == WidgetOption::Slider) {
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
