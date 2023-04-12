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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <lua.h>

extern char eldbScriptToRun[128]; // used by the UI thread for running a Lua script

// This function is only called from cli.cpp and used to
// relay data from CLI to ELDB
void eldbReceive(uint8_t *rxBuf, size_t rxBufLen, size_t dataLen);
void eldbLuaDebugHook(lua_State *L, lua_Debug *ar);