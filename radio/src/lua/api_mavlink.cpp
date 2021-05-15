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
/*
 *  (c) www.olliw.eu, OlliW, OlliW42
 */

#include <ctype.h>
#include <stdio.h>
#include "opentx.h"
#include "lua_api.h"


//-- some statistics --

extern int _end;
extern int _heap_end;
extern unsigned char *heap;

static int luaMavlinkGetMemUsed(lua_State * L)
{
  uint32_t s = luaGetMemUsed(lsScripts);
#if defined(COLORLCD)
  uint32_t w = luaGetMemUsed(lsWidgets);
  uint32_t e = luaExtraMemoryUsage;
#else
  uint32_t w = 0;
  uint32_t e = 0;
#endif
  lua_createtable(L, 0, 6);
  lua_pushtableinteger(L, "scripts", s);
  lua_pushtableinteger(L, "widgets", w);
  lua_pushtableinteger(L, "extra", e);
  lua_pushtableinteger(L, "total", s+w+e);
  lua_pushtableinteger(L, "heap_used", (int)(heap - (unsigned char *)&_end));
  lua_pushtableinteger(L, "heap_free", (int)((unsigned char *)&_heap_end - heap));
  return 1;
}

static int luaMavlinkGetStackUsed(lua_State * L)
{
  lua_createtable(L, 0, 10);
  lua_pushtableinteger(L, "main_available", stackAvailable()*4);
  lua_pushtableinteger(L, "main_size", stackSize()*4);
  lua_pushtableinteger(L, "menus_available", menusStack.available()*4);
  lua_pushtableinteger(L, "menus_size", menusStack.size());
  lua_pushtableinteger(L, "mixer_available", mixerStack.available()*4);
  lua_pushtableinteger(L, "mixer_size", mixerStack.size());
  lua_pushtableinteger(L, "audio_available", audioStack.available()*4);
  lua_pushtableinteger(L, "audio_size", audioStack.size());
  lua_pushtableinteger(L, "mavlink_available", mavlinkStack.available()*4);
  lua_pushtableinteger(L, "mavlink_size", mavlinkStack.size());
  return 1;
}

static int luaMavlinkGetTaskStats(lua_State *L)
{
  lua_newtable(L);
  lua_pushtableinteger(L, "time", mavlinkTaskRunTime());
  lua_pushtableinteger(L, "max", mavlinkTaskRunTimeMax());
  lua_pushtableinteger(L, "loop", mavlinkTaskLoop());
  return 1;
}

//-- mavlink api --


//------------------------------------------------------------
// mavlink luaL and luaR arrays
//------------------------------------------------------------

const luaL_Reg mavlinkLib[] = {
  { "getMemUsed", luaMavlinkGetMemUsed },
  { "getStackUsed", luaMavlinkGetStackUsed },
  { "getTaskStats", luaMavlinkGetTaskStats },

  { nullptr, nullptr }  /* sentinel */
};

const luaR_value_entry mavlinkConstants[] = {

  { nullptr, 0 }  /* sentinel */
};

