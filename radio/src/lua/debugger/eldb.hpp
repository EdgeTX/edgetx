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
#include <array>
#include <eldp.pb.h>
#include <result.hpp>

extern std::string eldbScriptToRun; // used by the UI thread for running a Lua script

namespace eldb {
    // This function is only called from cli.cpp and used to
    // relay data from CLI to ELDB
    template <size_t N>
    void receive(std::array<uint8_t, N> &rxBuf, size_t dataLen);
    void luaDebugHook(lua_State *L, lua_Debug *ar);
    bool hasHitBreakpoint();
    bool isInSession();

    auto startSession(std::string &targetName,
                        edgetx_eldp_StartDebug_Target targetType)
        -> cpp::result<void, edgetx_eldp_Error_Type>;
    auto forwardToRunningSession(edgetx_eldp_Request &request)
        -> cpp::result<void, edgetx_eldp_Error_Type>;
}

#include "eldb.tpp"