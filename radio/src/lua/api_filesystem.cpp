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


#include <cstdio>
#include "VirtualFS.h"
#include "lua_api.h"

#include "api_filesystem.h"

// garbage collector for luaDir
static int dir_gc(lua_State* L)
{
  VfsDir* dir = (VfsDir*)lua_touserdata(L, 1);
  if (dir) dir->close();
  return 0;
}

void registerDirIter(lua_State* L)
{
  luaL_newmetatable(L, DIR_METATABLE);
  
  /* set the garbage colector field */
  lua_pushstring(L, "__gc");
  lua_pushcfunction(L, dir_gc);
  lua_settable(L, -3);

  lua_pop(L, 1);
}

static int dir_iter(lua_State* L)
{
  VfsDir* dir = (VfsDir*)lua_touserdata(L, lua_upvalueindex(1));

  VfsFileInfo info;
  VfsError res = dir->read(info);
  if (res != VfsError::OK || info.getName().length() == 0) { /* Break on error or end of dir */
    return 0;
  }
 
  lua_pushstring(L, info.getName().c_str());
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
  VfsDir* dir = (VfsDir*)lua_newuserdata(L, sizeof(VfsDir));

  luaL_getmetatable(L, DIR_METATABLE);
  lua_setmetatable(L, -2);

  VfsError res = VirtualFS::instance().openDirectory(*dir, path);
  if (res != VfsError::OK) {
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

  VirtualFS& vfs = VirtualFS::instance();
  VfsError res;
  VfsFileInfo info;

  res = vfs.fstat(path, info);
  if (res != VfsError::OK) {
    printf("luaFstat cannot open %s\n", path);
    return 0;
  }

  lua_newtable(L);
  lua_pushtableinteger(L, "size", info.getSize());
  lua_pushtableinteger(L, "attrib", (int)info.getAttrib());

  int date = info.getDate();
  int time = info.getTime();
  uint32_t year = (date >> 9) + 1980;
  uint32_t month = date >> 5 & 15;
  uint32_t day = date & 31;
  uint32_t hour = time >> 11;
  uint32_t mn = time >> 5 & 63;
  uint32_t sec = (time & 31) * 2;

  lua_pushstring(L, "time");
  luaPushDateTime(L, year, month, day, hour, mn, sec);
  lua_settable(L, -3);

  return 1;
}
