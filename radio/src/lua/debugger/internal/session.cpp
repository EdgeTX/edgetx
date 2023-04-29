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
#include <result.hpp>
#include <string>
#include <system_error>
#include <vector>

#include "../eldb.hpp"
#include "encode_decode.h"

std::string eldbScriptToRun;

struct Breakpoint {
  unsigned int line;
  bool enabled;
};

bool inSession = false;
bool hasHitBreakpoint = false;
std::vector<Breakpoint> breakpoints;

static void pauseLuaExecution();
static void resumeLuaExecution();
static void executeCommand(edgetx_eldp_ExecuteCommand &cmd);
static void setBreakpoint(edgetx_eldp_SetBreakpoint &cmd);

auto eldbStartSession(std::string &targetName,
                      edgetx_eldp_StartDebug_Target targetType)
    -> cpp::result<void, edgetx_eldp_Error_Type>
{
  if (inSession) {
    return cpp::fail(edgetx_eldp_Error_Type_ALREADY_STARTED);
  }

  // TODO: Handle target type
  // FIXME: Replace this abomination with std::format when C++
  // standart becomes C++20
  eldbScriptToRun.clear();
  eldbScriptToRun += "/SCRIPTS/TOOLS/";
  eldbScriptToRun += targetName;

  if (isFileAvailable(eldbScriptToRun.c_str())) {
    RTOS_GIVE_NOTIFICATION(menusTaskId);
  } else {
    return cpp::fail(edgetx_eldp_Error_Type_FILE_DOES_NOT_EXIST);
  }

  inSession = true;
  return {};
}

void eldbLuaDebugHook(lua_State *L, lua_Debug *ar)
{
  switch (ar->event) {
    // case LUA_HOOKCALL: cliSerialPrintf("ELDB: LUA_HOOKCALL\n"); break;
    // case LUA_HOOKRET: cliSerialPrintf("ELDB: LUA_HOOKRET\n"); break;
    // case LUA_HOOKTAILCALL: cliSerialPrintf("ELDB: LUA_HOOKTAILCALL\n");
    // break;
    case LUA_HOOKLINE:
      if (ar->currentline < 0) return;
      if (std::any_of(breakpoints.cbegin(), breakpoints.cend(),
                      [ar](const Breakpoint &arg) {
                        return arg.line == (unsigned int)ar->currentline;
                      })) {
        pauseLuaExecution();
      }
      break;
    default:
      break;
  }
}

bool eldbIsInSession() { return inSession; }
bool eldbHasHitBreakpoint() { return hasHitBreakpoint; }

auto eldbForwardToRunningSession(edgetx_eldp_Request &request)
    -> cpp::result<void, edgetx_eldp_Error_Type>
{
  if (request.has_setBreakpoint) {
    setBreakpoint(request.setBreakpoint);
  } else if (request.has_executeCommand) {
    executeCommand(request.executeCommand);
  } else {
    return cpp::fail(edgetx_eldp_Error_Type_UNKNOWN_REQUEST);
  }
  return {};
}

static void executeCommand(edgetx_eldp_ExecuteCommand &cmd)
{
  // TODO: Handle all other commands
  // breakpoints.clear();
  resumeLuaExecution();
  cliSerialPrintf("Resumed\n");
}

static void setBreakpoint(edgetx_eldp_SetBreakpoint &cmd)
{
  auto breakpointIndex = std::find_if(
      breakpoints.cbegin(), breakpoints.cend(),
      [cmd](const Breakpoint &arg) { return arg.line == cmd.breakpoint.line; });
  bool enabled = cmd.state == edgetx_eldp_SetBreakpoint_State_ENABLED;

  switch (cmd.state) {
    case edgetx_eldp_SetBreakpoint_State_DISABLED:
    case edgetx_eldp_SetBreakpoint_State_ENABLED:
      if (breakpointIndex != breakpoints.cend()) {  // breakpoint exists
        breakpoints[breakpointIndex - breakpoints.cbegin()].enabled = enabled;
      } else {
        breakpoints.push_back(
            Breakpoint{.line = cmd.breakpoint.line, .enabled = enabled});
      }
      break;
    case edgetx_eldp_SetBreakpoint_State_NONE:
      if (breakpointIndex != breakpoints.cend()) {
        breakpoints.erase(breakpointIndex);
      }
    default:
      break;
  }
  cliSerialPrintf("Set breakpoint\n");
}

static void pauseLuaExecution()
{
  hasHitBreakpoint = true;
  luaPauseExecution();
}

static void resumeLuaExecution()
{
  hasHitBreakpoint = false;
  luaResumeExecution();
}
