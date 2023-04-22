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

#include <cli.h>
#include <eldp.pb.h>
#include <lua/lua_api.h>
#include <sdcard.h>
#include <tasks.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

#include "../eldb.h"
#include "encode_decode.h"

bool inSession = false;

typedef struct {
  unsigned int line;
  // TODO: Fill required data
} Breakpoint_t;

std::vector<Breakpoint_t> breakpoints;

bool eldbStartSession(const char *targetName, edgetx_eldp_Error_Type *err)
{
  if (inSession) {
    *err = edgetx_eldp_Error_Type_ALREADY_STARTED;
    return false;
  }

  // TODO: Handle target type

  snprintf(eldbScriptToRun, sizeof(eldbScriptToRun), "/SCRIPTS/TOOLS/%s",
           targetName);

  if (isFileAvailable(eldbScriptToRun)) {
    RTOS_GIVE_NOTIFICATION(menusTaskId);
  } else {
    *err = edgetx_eldp_Error_Type_FILE_DOES_NOT_EXIST;
    return false;
  }

  inSession = true;
  return true;
}

void eldbLuaDebugHook(lua_State *L, lua_Debug *ar)
{
  switch (ar->event) {
    // case LUA_HOOKCALL: cliSerialPrintf("ELDB: LUA_HOOKCALL\n"); break;
    // case LUA_HOOKRET: cliSerialPrintf("ELDB: LUA_HOOKRET\n"); break;
    // case LUA_HOOKTAILCALL: cliSerialPrintf("ELDB: LUA_HOOKTAILCALL\n");
    // break;
    case LUA_HOOKLINE:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
      if (std::any_of(breakpoints.cbegin(), breakpoints.cend(),
                      [ar](const Breakpoint_t &arg) {
                        return arg.line == ar->currentline;
                      })) {
        cliSerialPrintf("ELDB: Can stop at line\n");
        luaPauseExecution();
      } else {
        // cliSerialPrintf("ELDB: %d\n", ar->currentline);
      }
#pragma GCC diagnostic pop
      break;
    default:
      break;
  }
}

bool eldbIsInSession() { return inSession; }

bool eldbForwardToRunningSession(const edgetx_eldp_Request *request,
                                 edgetx_eldp_Error_Type *err, std::string *msg)
{
  if (request->has_setBreakpoint) {
    cliSerialPrintf("ELDBSB: %d", request->setBreakpoint.state);
    switch (request->setBreakpoint.state) {
      case edgetx_eldp_SetBreakpoint_State_ENABLED:
        breakpoints.push_back(
            Breakpoint_t{.line = request->setBreakpoint.breakpoint.line});
        break;
      default:
        break;
    }
  } else if (request->has_executeDebuggerCommand) {
    breakpoints.clear();
    luaResumeExecution();
    cliSerialPrintf("Resumed");
  } else {
    *err = edgetx_eldp_Error_Type_SESSION;
    *msg = "Request cannot be handled; unknown request";
    return false;
  }
  return true;
}