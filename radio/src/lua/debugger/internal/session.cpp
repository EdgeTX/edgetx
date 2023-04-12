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

#include "session.h"
#include "../eldb.h"

#include <cstdio>
#include <cstring>
#include <tasks.h>
#include <lua/lua_api.h>
#include <eldp.pb.h>
#include <sdcard.h>
#include <cli.h>

bool isRunning = false;

bool eldbStartSession(const char *targetName, edgetx_eldp_Error_Type *err) {
    if (isRunning) {
        *err = edgetx_eldp_Error_Type_ALREADY_STARTED;
        return false;
    }

    // TODO: Handle target type

    snprintf(eldbScriptToRun, sizeof(eldbScriptToRun), "/SCRIPTS/TOOLS/%s", targetName);

    if (isFileAvailable(eldbScriptToRun)) {
        RTOS_GIVE_NOTIFICATION(menusTaskId);
    } else {
        *err = edgetx_eldp_Error_Type_FILE_DOES_NOT_EXIST;
        return false;
    }

    isRunning = true;
    return true;
}

void eldbLuaDebugHook(lua_State *L, lua_Debug *ar) {
    luaL_error(L, "interrupted!");
    cliSerialPrintf("debug hook called\n");
}

bool eldbIsRunning() {
    return isRunning;
}