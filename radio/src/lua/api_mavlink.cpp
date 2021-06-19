/*
 * Copyright (C) EdgeTX
 *
 * (c) www.olliw.eu, OlliW, OlliW42
 *
 * Based on code named
 *   opentx - http://github.com/opentx/opentx
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

#include <ctype.h>
#include <stdio.h>
#include "opentx.h"
#include "lua_api.h"
#include "telemetry/mavlink/mavlink_telem.h"
#include "thirdparty/Mavlink/edgetx_lua_lib_constants.h"
#include "thirdparty/Mavlink/edgetx_lua_lib_messages.h"


static int luaMavlinkGetVersion(lua_State * L)
{
  lua_pushinteger(L, FASTMAVLINK_MAVLINK_VERSION); // this is the version reported also by the heartbeat
  return 1;
}

static int luaMavlinkGetChannelStatus(lua_State * L)
{
  lua_createtable(L, 0, 6);
  lua_pushtableinteger(L, "msg_rx_count", mavlinkTelem.msg_rx_count);
  lua_pushtableinteger(L, "msg_rx_per_sec", mavlinkTelem.msg_rx_persec);
  lua_pushtableinteger(L, "bytes_rx_per_sec", mavlinkTelem.bytes_rx_persec);
  lua_pushtableinteger(L, "msg_tx_count", mavlinkTelem.msg_tx_count);
  lua_pushtableinteger(L, "msg_tx_per_sec", mavlinkTelem.msg_tx_persec);
  lua_pushtableinteger(L, "bytes_tx_per_sec", mavlinkTelem.bytes_tx_persec);
  return 1;
}

//-- mavlink api --

static int luaMavlinkGetSystemId(lua_State *L)
{
  lua_pushinteger(L, mavlinkTelem.systemSysId());
  return 1;
}

static int luaMavlinkGetAutopilotIds(lua_State *L)
{
  lua_pushinteger(L, mavlinkTelem.systemSysId());
  lua_pushinteger(L, mavlinkTelem.autopilotCompId());
  return 1;
}

static int luaMavlinkGetCameraIds(lua_State *L)
{
  lua_pushinteger(L, mavlinkTelem.systemSysId());
  lua_pushinteger(L, mavlinkTelem.cameraCompId());
  return 1;
}

static int luaMavlinkGetGimbalIds(lua_State *L)
{
  lua_pushinteger(L, mavlinkTelem.systemSysId());
  lua_pushinteger(L, mavlinkTelem.gimbalCompId());
  return 1;
}

static int luaMavlinkGetGimbalManagerIds(lua_State *L)
{
  lua_pushinteger(L, mavlinkTelem.systemSysId());
  lua_pushinteger(L, mavlinkTelem.gimbalManagerCompId());
  return 1;
}

//-- mavlink api, messages --

static int luaMavlinkInEnable(lua_State *L)
{
  bool flag = (luaL_checkinteger(L, 1) > 0);
  mavlinkTelem.mavapiMsgInEnable(flag);
  return 0;
}

static int luaMavlinkInCount(lua_State *L)
{
  lua_pushinteger(L, mavlinkTelem.mavapiMsgInCount());
  return 1;
}

static int luaMavlinkGetMessage(lua_State *L)
{
  int msgid = luaL_checknumber(L, 1);

  MavlinkTelem::MavMsg* mavmsg = mavlinkTelem.mavapiMsgInGet(msgid);
  if (!mavmsg) {
    lua_pushnil(L);
  }
  else {
    luaMavlinkPushMavMsg(L, mavmsg);
    mavmsg->updated = false;
  }
  return 1;
}

static int luaMavlinkGetMessageLast(lua_State *L)
{
  MavlinkTelem::MavMsg* mavmsg = mavlinkTelem.mavapiMsgInGetLast();
  if (!mavmsg) {
    lua_pushnil(L);
  }
  else {
    luaMavlinkPushMavMsg(L, mavmsg);
  }
  return 1;
}

static int luaMavlinkOutEnable(lua_State *L)
{
  bool flag = (luaL_checkinteger(L, 1) > 0);
  mavlinkTelem.mavapiMsgOutEnable(flag);
  return 0;
}

static int luaMavlinkIsFree(lua_State *L)
{
  lua_pushboolean(L, (mavlinkTelem.mavapiMsgOutPtr() != NULL));
  return 1;
}

static int luaMavlinkSendMessage(lua_State *L)
{
  fmav_message_t* msg_out = mavlinkTelem.mavapiMsgOutPtr();

  if (!lua_istable(L, -1) || !msg_out) {
    lua_pushnil(L);
  }
  else if (luaMavlinkCheckMsgOut(L, msg_out)) {
    mavlinkTelem.mavapiMsgOutSet();
    lua_pushboolean(L, true);
  }
  else {
    lua_pushboolean(L, false);
  }
  return 1;
}


//------------------------------------------------------------
// mavlink luaL and luaR arrays
//------------------------------------------------------------

const luaL_Reg mavlinkLib[] = {
  { "getVersion", luaMavlinkGetVersion },
  { "getChannelStatus", luaMavlinkGetChannelStatus },

  { "getSystemId", luaMavlinkGetSystemId },
  { "getAutopilotIds", luaMavlinkGetAutopilotIds },
  { "getCameraIds", luaMavlinkGetCameraIds },
  { "getGimbalIds", luaMavlinkGetGimbalIds },
  { "getGimbalManagerIds", luaMavlinkGetGimbalManagerIds },

  { "enableIn", luaMavlinkInEnable },
  { "getInCount", luaMavlinkInCount },
  { "getMessage", luaMavlinkGetMessage },
  { "getMessageLast", luaMavlinkGetMessageLast },
  { "enableOut", luaMavlinkOutEnable },
  { "isFree", luaMavlinkIsFree },
  { "sendMessage", luaMavlinkSendMessage },

  { nullptr, nullptr }  /* sentinel */
};

const luaR_value_entry mavlinkConstants[] = {
  MAVLINK_LIB_CONSTANTS

  { nullptr, 0 }  /* sentinel */
};

