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

#include "definitions.h"

EXTERN_C_START
  #include "lua.h"
  #include "lauxlib.h"
  #include "lualib.h"
  #include "lgc.h"
EXTERN_C_END

// #define LUA_INIT_THEMES_AND_WIDGETS()  luaInitThemesAndWidgets()
// #define lua_registernumber(L, n, i)    (lua_pushnumber(L, (i)), lua_setglobal(L, (n)))
// #define lua_registerint(L, n, i)       (lua_pushinteger(L, (i)), lua_setglobal(L, (n)))

#define lua_pushtableboolean(L, k, v) \
  (lua_pushstring(L, (k)), lua_pushboolean(L, (v)), lua_settable(L, -3))

#define lua_pushtableinteger(L, k, v)                                   \
  (lua_pushstring(L, (k)), lua_pushinteger(L, (v)), lua_settable(L, -3))

#define lua_pushtablenumber(L, k, v) \
  (lua_pushstring(L, (k)), lua_pushnumber(L, (v)), lua_settable(L, -3))

// size based string (possibly no null-termination)
#define __lua_strncpy(s)              \
  char tmp[sizeof(s) + 1];            \
  strncpy(tmp, (s), sizeof(tmp) - 1); \
  tmp[sizeof(s)] = '\0';

// size based string (possibly no null-termination)
#define lua_pushnstring(L, s) \
  {                           \
    __lua_strncpy(s);         \
    lua_pushstring(L, tmp);   \
  }
#define lua_pushtablenstring(L, k, v) \
  {                                   \
    __lua_strncpy(v);                 \
    lua_pushstring(L, (k));           \
    lua_pushstring(L, tmp);           \
    lua_settable(L, -3);              \
  }

// null-terminated string
#define lua_pushtablestring(L, k, v) \
  (lua_pushstring(L, (k)), lua_pushstring(L, (v)), lua_settable(L, -3))

// #define lua_registerlib(L, name, tab)  (luaL_newmetatable(L, name), luaL_setfuncs(L, tab, 0), lua_setglobal(L, name))

extern lua_State* lsScripts;
extern lua_State* lsWidgets;

void luaSetInstructionsLimit(lua_State* L, int count);
int luaLoadScriptFileToState(lua_State * L, const char * filename, const char * mode);
void luaRegisterLibraries(lua_State * L);
void luaClose(lua_State ** L);
void luaDoGc(lua_State * L, bool full);
uint32_t luaGetMemUsed(lua_State * L);
