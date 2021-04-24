/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "libopenui.h"
#include "lua_api.h"

class LuaTheme: public Theme
{
  friend void luaLoadThemeCallback();

  public:
    LuaTheme(const char * name, ZoneOption * options):
      Theme(name, options),
      loadFunction(0),
      drawBackgroundFunction(0),
      drawTopbarBackgroundFunction(0),
      drawAlertBoxFunction(0)
    {
    }

    virtual void load() const
    {
      luaLcdAllowed = true;
      exec(loadFunction);
    }

    virtual void drawBackground() const
    {
      exec(drawBackgroundFunction);
    }

    void drawTopbarBackground(uint8_t icon) const override
    {
      exec(drawTopbarBackgroundFunction);
    }

#if 0
    virtual void drawAlertBox(const char * title, const char * text, const char * action) const
    {
      exec(drawAlertBoxFunction);
    }
#endif

  protected:
    int loadFunction;
    int drawBackgroundFunction;
    int drawTopbarBackgroundFunction;
    int drawAlertBoxFunction;
};

void luaLoadThemeCallback()
{
  TRACE("luaLoadThemeCallback()");
  const char * name=NULL;
  int themeOptions=0, loadFunction=0, drawBackgroundFunction=0, drawTopbarBackgroundFunction=0;

  luaL_checktype(lsWidgets, -1, LUA_TTABLE);

  for (lua_pushnil(lsWidgets); lua_next(lsWidgets, -2); lua_pop(lsWidgets, 1)) {
    const char * key = lua_tostring(lsWidgets, -2);
    if (!strcmp(key, "name")) {
      name = luaL_checkstring(lsWidgets, -1);
    }
    else if (!strcmp(key, "options")) {
      themeOptions = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "load")) {
      loadFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "drawBackground")) {
      drawBackgroundFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
    else if (!strcmp(key, "drawTopbarBackground")) {
      drawTopbarBackgroundFunction = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
      lua_pushnil(lsWidgets);
    }
  }

  if (name) {
    ZoneOption * options = NULL;
    if (themeOptions) {
      options = createOptionsArray(themeOptions, MAX_THEME_OPTIONS);
      if (!options)
        return;
    }
    LuaTheme * theme = new LuaTheme(name, options);
    theme->loadFunction = loadFunction;
    theme->drawBackgroundFunction = drawBackgroundFunction;
    theme->drawTopbarBackgroundFunction = drawTopbarBackgroundFunction;   // NOSONAR
    TRACE("Loaded Lua theme %s", name);
  }
}
