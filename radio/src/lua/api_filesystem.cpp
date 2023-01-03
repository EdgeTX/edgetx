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

#define LUA_LIB

#include <cstdio>

#include "lua_api.h"
#include "api_filesystem.h"

// garbage collector for luaDir
static int dir_gc(lua_State* L)
{
  DIR* dir = (DIR*)lua_touserdata(L, 1);
  if (dir) f_closedir(dir);
  return 0;
}

static int dir_iter(lua_State* L)
{
  DIR* dir = (DIR*)lua_touserdata(L, lua_upvalueindex(1));

  FILINFO info;
  FRESULT res = f_readdir(dir, &info);
  if (res != FR_OK || info.fname[0] == 0) { /* Break on error or end of dir */
    return 0;
  }
 
  lua_pushstring(L, info.fname);
  return 1;
}

/*luadoc
@function dir(directory)

 Return an iterator listing all the files and directories name in a directory

@param directory (string) Working directory

@status current Introduced in 2.5.0

### Example

```lua
  for fname in dir(".") do
    print(fname)
  end

*/
int luaDir(lua_State* L)
{
  const char* path = luaL_optstring(L, 1, nullptr);
  DIR* dir = (DIR*)lua_newuserdata(L, sizeof(DIR));

  luaL_getmetatable(L, DIR_METATABLE);
  lua_setmetatable(L, -2);

  FRESULT res = f_opendir(dir, path);
  if (res != FR_OK) {
    printf("luaDir cannot open %s\n", path);
  }
  
  lua_pushcclosure(L, dir_iter, 1);
  return 1;
}

/*luadoc
@function fstat(path)

 Checks the existence of file or directory.
 If not exist, return nil.
 If exist, return the object information.

@param path (string) path to the object

@retval table object info, table elements:
* 'size' (number) file size
* 'attrib' (number) file attribute flags
* 'time' (table) table with last time modified date and times, table elements:
  * `year` (number) year
  * `mon` (number) month
  * `day` (number) day of month
  * `hour` (number) hours
  * `hour12` (number) hours in US format
  * `min` (number) minutes
  * `sec` (number) seconds
  * `suffix` (text) am or pm

### Example

```lua
  info = fstat("radio")
  if info ~= nil then
    if (info.attrib == AM_DIR) then
      print("is a directory")
    end

    size = info.size
    time = info.time
  end

@status current Introduced in 2.5.0

*/
int luaFstat(lua_State* L)
{
  const char * path = luaL_optstring(L, 1, nullptr);

  FRESULT res;
  FILINFO info;

  res = f_stat(path, &info);
  if (res != FR_OK) {
    printf("luaFstat cannot open %s\n", path);
    return 0;
  }

  lua_newtable(L);
  lua_pushtableinteger(L, "size", info.fsize);
  lua_pushtableinteger(L, "attrib", info.fattrib);

  uint32_t year = (info.fdate >> 9) + 1980;
  uint32_t month = info.fdate >> 5 & 15;
  uint32_t day = info.fdate & 31;
  uint32_t hour = info.ftime >> 11;
  uint32_t mn = info.ftime >> 5 & 63;
  uint32_t sec = (info.ftime & 31) * 2;

  lua_pushstring(L, "time");
  luaPushDateTime(L, year, month, day, hour, mn, sec);
  lua_settable(L, -3);

  return 1;
}


LROT_BEGIN(dir_handle, NULL, LROT_MASK_GC)
  LROT_FUNCENTRY( __gc, dir_gc )
LROT_END(dir_handle, NULL, LROT_MASK_GC)

LROT_BEGIN(etxdir, NULL, 0)
  LROT_FUNCENTRY( dir, luaDir )
  LROT_FUNCENTRY( fstat, luaFstat )
LROT_END(etxdir, NULL, 0)

extern "C" {
  LUAMOD_API int luaopen_etxdir(lua_State* L) {
    luaL_rometatable( L, DIR_METATABLE,  LROT_TABLEREF(dir_handle));
    return 0;
  }
}
