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

#include <ctype.h>
#include <stdio.h>
#include "edgetx.h"
#include "lua_api.h"
#include "../timers.h"
#include "model_init.h"
#include "gvars.h"
#include "mixes.h"

#include <storage/sdcard_yaml.h>

#if defined(MULTIMODULE)
#include "pulses/multi.h"
#endif

/*luadoc
@function model.getInfo()

Get current Model information

@retval table model information:
 * `name` (string) model name
 * `extendedLimits` (boolean) extended limits enabled
 * `jitterFilter` (number) model level ADC filter
 * `bitmap` (string) bitmap name (not present on X7)
 * `filename` (string) model filename

@status current Introduced in 2.0.6, changed in 2.2.0, filename added in 2.6.0, extendedLimits, jitterFilter, and labels added in 2.8.0
*/
static int luaModelGetInfo(lua_State *L)
{
  lua_newtable(L);
  lua_pushtablenstring(L, "name", g_model.header.name);
  lua_pushtableboolean(L, "extendedLimits", g_model.extendedLimits);
  lua_pushtableinteger(L, "jitterFilter", g_model.jitterFilter);
#if LCD_DEPTH > 1
  lua_pushtablenstring(L, "bitmap", g_model.header.bitmap);
#endif

#if defined(STORAGE_MODELSLIST)
  lua_pushtablenstring(L, "labels", g_model.header.labels);
  lua_pushtablenstring(L, "filename", g_eeGeneral.currModelFilename);
#else
  lua_pushtablenstring(L, "labels", "");
  char fname[MODELIDX_STRLEN + sizeof(YAML_EXT)];
  getModelNumberStr(g_eeGeneral.currModel, fname);
  strcat(fname, YAML_EXT);
  lua_pushtablenstring(L, "filename", fname);
#endif

  return 1;
}

/*luadoc
@function model.setInfo(value)

Set the current Model information

@param value model information data, see model.getInfo()

@notice If a parameter is missing from the value, then
that parameter remains unchanged.

@status current Introduced in 2.0.6, extendedLimits and jitterFilter added in 2.8.0
*/
static int luaModelSetInfo(lua_State *L)
{
  luaL_checktype(L, -1, LUA_TTABLE);
  for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
    luaL_checktype(L, -2, LUA_TSTRING); // key is string
    const char * key = luaL_checkstring(L, -2);
    if (!strcmp(key, "name")) {
      const char * name = luaL_checkstring(L, -1);
      strncpy(g_model.header.name, name, sizeof(g_model.header.name));
    }
    else if (!strcmp(key, "extendedLimits")) {
      g_model.extendedLimits = lua_toboolean(L, -1);
    }
    else if (!strcmp(key, "jitterFilter")) {
      auto j = lua_tounsigned(L, -1);
      if (j > OVERRIDE_ON) j = OVERRIDE_ON;
      g_model.jitterFilter = j;
    }
#if LCD_DEPTH > 1
    else if (!strcmp(key, "bitmap")) {
      const char * name = luaL_checkstring(L, -1);
      strncpy(g_model.header.bitmap, name, LEN_BITMAP_NAME);
    }
#endif
  }
  storageDirty(EE_MODEL);
  return 0;
}

/*luadoc
@function model.getModule(index)

Get RF module parameters

`Type` values:
  * 0 NONE
  * 1 PPM
  * 2 XJT_PXX1
  * 3 ISRM_PXX2
  * 4 DSM2
  * 5 CROSSFIRE
  * 6 MULTIMODULE
  * 7 R9M_PXX1
  * 8 R9M_PXX2
  * 9 R9M_LITE_PXX1
  * 10 R9M_LITE_PXX2
  * 11 R9M_LITE_PRO_PXX1
  * 12 R9M_LITE_PRO_PXX2
  * 13 SBUS
  * 14 XJT_LITE_PXX2

`subType` values for XJT_PXX1:
 * -1 OFF
 * 0 D16
 * 1 D8
 * 2 LR12

@param index (number) module index (0 for internal, 1 for external)

@retval nil requested module does not exist

@retval table module parameters:
 * `subType` (number) protocol index
 * `modelId` (number) receiver number
 * `firstChannel` (number) start channel (0 is CH1)
 * `channelsCount` (number) number of channels sent to module
 * `Type` (number) module type
 * if the module type is Multi additional information are available
 * `protocol` (number) protocol number (Multi only)
 * `subProtocol` (number) sub-protocol number (Multi only)
 * `channelsOrder` (number) first 4 channels expected order (Multi only)

@status current Introduced in 2.2.0
*/
static int luaModelGetModule(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < NUM_MODULES) {
    ModuleData & module = g_model.moduleData[idx];
    lua_newtable(L);
    lua_pushtableinteger(L, "subType", module.subType);
    lua_pushtableinteger(L, "modelId", g_model.header.modelId[idx]);
    lua_pushtableinteger(L, "firstChannel", module.channelsStart);
    lua_pushtableinteger(L, "channelsCount", module.getChannelsCount());
    lua_pushtableinteger(L, "Type", module.type);
#if defined(MULTIMODULE)
    if (module.type == MODULE_TYPE_MULTIMODULE) {
      int protocol = g_model.moduleData[idx].multi.rfProtocol + 1;
      int subprotocol = g_model.moduleData[idx].subType;
      lua_pushtableinteger(L, "protocol", protocol);
      lua_pushtableinteger(L, "subProtocol", subprotocol);
      if (getMultiModuleStatus(idx).isValid()) {
        if (getMultiModuleStatus(idx).ch_order == 0xFF)
          lua_pushtableinteger(L, "channelsOrder", -1);
        else
          lua_pushtableinteger(L, "channelsOrder", getMultiModuleStatus(idx).ch_order);
      }
      else {
        lua_pushtableinteger(L, "channelsOrder", -1);
      }
    }
#endif
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function model.setModule(index, value)

Set RF module parameters

@param index (number) module index (0 for internal, 1 for external)

@param value module parameters, see model.getModule()

@notice If a parameter is missing from the value, then
that parameter remains unchanged.

@status current Introduced in 2.2.0, modified in 2.3.12 (proto/subproto)
*/
static int luaModelSetModule(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);

  if (idx < NUM_MODULES) {
    int protocol = -1;
    int subprotocol = -1;

    ModuleData & module = g_model.moduleData[idx];
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      luaL_checktype(L, -2, LUA_TSTRING); // key is string
      const char * key = luaL_checkstring(L, -2);
      if (!strcmp(key, "Type")) {
        uint8_t newtype = luaL_checkinteger(L, -1);
        if (newtype != module.type) {
          setModuleType(idx, newtype);
        }
      }
      else if (!strcmp(key, "subType")) {
        module.subType = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "modelId")) {
        g_model.header.modelId[idx] = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "firstChannel")) {
        module.channelsStart = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "channelsCount")) {
        module.channelsCount = luaL_checkinteger(L, -1) - 8;
      }
#if defined(MULTIMODULE)
      if (!strcmp(key, "protocol")) {
        protocol = luaL_checkinteger(L, -1);
      }
      if (!strcmp(key, "subProtocol")) {
        subprotocol = luaL_checkinteger(L, -1);
      }
#endif
    }
#if defined(MULTIMODULE)
    if (protocol > 0 && subprotocol >= 0) {
      g_model.moduleData[idx].multi.rfProtocol = protocol - 1;
      g_model.moduleData[idx].subType = subprotocol;
    }
#endif
    storageDirty(EE_MODEL);
  }
  return 0;
}

/*luadoc
@function model.getTimer(timer)

Get model timer parameters

@param timer (number) timer index (0 for Timer 1)

@retval nil requested timer does not exist

@retval table timer parameters:
 * `mode` (number) timer trigger source: off, abs, stk,  stk%, sw/!sw, !m_sw/!m_sw
 * `start` (number) start value [seconds], 0 for up timer, 0> down timer
 * `value` (number) current value [seconds]
 * `countdownBeep` (number) countdown beep (0­ = silent, 1 =­ beeps, 2­ = voice)
 * `minuteBeep` (boolean) minute beep
 * `persistent` (number) persistent timer
 * `name` (string) timer name
 * `showElapsed` (boolean) show elapsed

@status current Introduced in 2.0.0, name added in 2.3.6, showElapsed added in 2.8.0
*/
static int luaModelGetTimer(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_TIMERS) {
    TimerData & timer = g_model.timers[idx];
    lua_newtable(L);
    lua_pushtableinteger(L, "mode", timer.mode);
    lua_pushtableinteger(L, "start", timer.start);
    lua_pushtableinteger(L, "value", timersStates[idx].val);
    lua_pushtableinteger(L, "countdownBeep", timer.countdownBeep);
    lua_pushtableboolean(L, "minuteBeep", timer.minuteBeep);
    lua_pushtableinteger(L, "persistent", timer.persistent);
    lua_pushtablenstring(L, "name", timer.name);
    lua_pushtableboolean(L, "showElapsed", timer.showElapsed);
    lua_pushtableinteger(L, "switch", timer.swtch);
    lua_pushtableinteger(L, "countdownStart", timer.countdownStart);
    lua_pushtableinteger(L, "extraHaptic", timer.extraHaptic);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function model.setTimer(timer, value)

Set model timer parameters

@param timer (number) timer index (0 for Timer 1)

@param value timer parameters, see model.getTimer()

@notice If a parameter is missing from the value, then
that parameter remains unchanged.

@status current Introduced in 2.0.0, name added in 2.3.6, showElapsed added in 2.8.0
*/
static int luaModelSetTimer(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);

  if (idx < MAX_TIMERS) {
    TimerData & timer = g_model.timers[idx];
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      luaL_checktype(L, -2, LUA_TSTRING); // key is string
      const char * key = luaL_checkstring(L, -2);
      if (!strcmp(key, "mode")) {
        timer.mode = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "start")) {
        timer.start = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "value")) {
        timersStates[idx].val = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "countdownBeep")) {
        timer.countdownBeep = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "minuteBeep")) {
        timer.minuteBeep = lua_toboolean(L, -1);
      }
      else if (!strcmp(key, "persistent")) {
        timer.persistent = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "name")) {
        const char * name = luaL_checkstring(L, -1);
        strncpy(timer.name, name, sizeof(timer.name));
      }
      else if (!strcmp(key, "showElapsed")) {
        timer.showElapsed = lua_toboolean(L, -1);
      } 
      else if (!strcmp(key, "switch")) {
        timer.swtch = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "countdownStart")) {
        timer.countdownStart = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "extraHaptic")) {
        timer.extraHaptic = lua_tointeger(L, -1);
      }
    }
    storageDirty(EE_MODEL);
  }
  return 0;
}

/*luadoc
@function model.resetTimer(timer)

Reset model timer to a startup value

@param timer (number) timer index (0 for Timer 1)

@status current Introduced in TODO
*/
static int luaModelResetTimer(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_TIMERS) {
    timerReset(idx);
  }
  return 0;
}

static unsigned int getFirstInput(unsigned int chn)
{
  for (unsigned int i=0; i<MAX_EXPOS; i++) {
    ExpoData * expo = expoAddress(i);
    if (!expo->srcRaw || expo->chn >= chn) {
      return i;
    }
  }
  return 0;
}

static unsigned int getInputsCountFromFirst(unsigned int chn, unsigned int first)
{
  unsigned int count = 0;
  for (unsigned int i=first; i<MAX_EXPOS; i++) {
    ExpoData * expo = expoAddress(i);
    if (!expo->srcRaw || expo->chn!=chn) break;
    count++;
  }
  return count;
}

static unsigned int getInputsCount(unsigned int chn)
{
  return getInputsCountFromFirst(chn, getFirstInput(chn));
}

/*luadoc
@function model.getInputsCount(input)

Return number of lines for given input

@param input (unsigned number) input number (use 0 for Input1)

@retval number number of configured lines for given input

@status current Introduced in 2.0.0
*/
static int luaModelGetInputsCount(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  int count = getInputsCount(chn);
  lua_pushinteger(L, count);
  return 1;
}

/*luadoc
@function model.deleteFlightModes()

Clear all flightModes

@status current Introduced in 2.3.10
*/
static int luaModelDeleteFlightModes(lua_State *L)
{
  memset(g_model.flightModeData, 0, sizeof(g_model.flightModeData));
  return 0;
}

/*luadoc
@function model.getFlightMode(index)

@param index (unsigned number) flight mode number (use 0 for FM0)
 
Return input data for given input and line number

@retval nil requested input or line does not exist

@retval table input data:
 * `name` (string) input line name
 * `switch` (number) input switch index
 * `fadeIn` (number) fade in value (in 0.1s)
 * `fadeOut` (number) fade out value (in 0.1s)
 * `trimsValues` (table) table of trim values:
   * `key` is trim number (zero based)
   * `value` is trim value
 * `trimsModes` (table) table of trim mode:
   * `key` is trim number (zero based)
   * `value` is trim mode

@status current Introduced in 2.3.10
*/
static int luaModelGetFlightMode(lua_State * L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_FLIGHT_MODES) {
    FlightModeData * fm = flightModeAddress(idx);
    lua_newtable(L);
    lua_pushtablenstring(L, "name", fm->name);
    lua_pushtableinteger(L, "switch", fm->swtch);
    lua_pushtableinteger(L, "fadeIn", fm->fadeIn);
    lua_pushtableinteger(L, "fadeOut", fm->fadeOut);
    lua_pushstring(L, "trimsValues");
    lua_newtable(L);
    for (uint8_t i = 0; i < keysGetMaxTrims(); i++) {
      lua_pushinteger(L, i + 1);
      lua_pushinteger(L, fm->trim[i].value);
      lua_settable(L, -3);
    }
    lua_settable(L, -3);
    lua_pushstring(L, "trimsModes");
    lua_newtable(L);
    for (uint8_t i = 0; i < keysGetMaxTrims(); i++) {
      lua_pushinteger(L, i + 1);
      lua_pushinteger(L, fm->trim[i].mode);
      lua_settable(L, -3);
    }
    lua_settable(L, -3);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function model.setFlightMode(index, params)

Set Flight mode parameters

@param index (unsigned number) flight mode number (use 0 for FM0)

@param params see model.getFlightMode return format for table format.

@status current Introduced in 2.3.10
*/
static int luaModelSetFlightMode(lua_State * L)
{
  unsigned int flightMode = luaL_checkunsigned(L, 1);

  if (flightMode >= MAX_FLIGHT_MODES) {
    lua_pushinteger(L, 2);
    return 1;
  }
  FlightModeData * fm = flightModeAddress(flightMode);
  luaL_checktype(L, -1, LUA_TTABLE);
  auto max_trims = keysGetMaxTrims();
  for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
    luaL_checktype(L, -2, LUA_TSTRING); // key is string
    const char * key = luaL_checkstring(L, -2);
    if (!strcmp(key, "name")) {
      const char * name = luaL_checkstring(L, -1);
      strncpy(fm->name, name, sizeof(fm->name));
    }
    else if (!strcmp(key, "switch")) {
      fm->swtch = luaL_checkinteger(L, -1);
    }
    else if (!strcmp(key, "fadeIn")) {
      fm->fadeIn = luaL_checkinteger(L, -1);
    }
    else if (!strcmp(key, "fadeOut")) {
      fm->fadeOut = luaL_checkinteger(L, -1);
    }
    else if (!strcmp(key, "trimsValues")) {
      luaL_checktype(L, -1, LUA_TTABLE);
      for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
        int idx = luaL_checkinteger(L, -2) - 1; // key is integer
        if (idx < 0 || idx >= max_trims) continue;
        int16_t val = luaL_checkinteger(L, -1);
        if (g_model.extendedTrims)
          val = limit<int16_t>(TRIM_EXTENDED_MIN, val, TRIM_EXTENDED_MAX);
        else
          val = limit<int16_t>(TRIM_MIN, val, TRIM_MAX);
        if (idx < max_trims)
          fm->trim[idx].value = val;
      }
    }
    else if (!strcmp(key, "trimsModes")) {
      luaL_checktype(L, -1, LUA_TTABLE);
      for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
        int idx = luaL_checkinteger(L, -2) - 1; // key is integer
        if (idx < 0 || idx >= max_trims) continue;
        uint16_t val = luaL_checkinteger(L, -1);
        if (idx < max_trims)
          fm->trim[idx].mode = (val & 0x1F);
      }
    }
  }
  storageDirty(EE_MODEL);
  lua_pushinteger(L, 0);
  return 1;
}

/*luadoc
@function model.getInput(input, line)

Return input data for given input and line number

@param input (unsigned number) input number (use 0 for Input1)

@param line  (unsigned number) input line (use 0 for first line)

@retval nil requested input or line does not exist

@retval table input data:
 * `name` (string) input line name
 * `inputName` (string) input input name
 * `source` (number) input source index
 * `scale` (number)  input scaling (for telemetry)
 * `weight` (number) input weight
 * `offset` (number) input offset
 * `switch` (number) input switch index
 * `curveType` (number) curve type (function, expo, custom curve)
 * `curveValue` (number) curve index
 * `carryTrim` deprecated, please use trimSource instead. WARNING: carryTrim was getting negative values (carryTrim = - trimSource)
 * 'trimSource' (number) a positive number representing trim source
 * 'flightModes' (number) bit-mask of active flight modes

@status current Introduced in 2.0.0, curveType/curveValue/carryTrim added in 2.3, inputName added 2.3.10, flighmode reworked in 2.3.11, broken carryTrim replaced by trimSource in 2.8.1, scale added in 2.10
*/
static int luaModelGetInput(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  unsigned int idx = luaL_checkunsigned(L, 2);
  unsigned int first = getFirstInput(chn);
  unsigned int count = getInputsCountFromFirst(chn, first);
  if (idx < count) {
    ExpoData * expo = expoAddress(first+idx);
    lua_newtable(L);
    lua_pushtablenstring(L, "name", expo->name);
    lua_pushtablenstring(L, "inputName", g_model.inputNames[chn]);
    lua_pushtableinteger(L, "source", expo->srcRaw);
    lua_pushtableinteger(L, "scale", expo->scale);
    lua_pushtableinteger(L, "weight", expo->weight);
    lua_pushtableinteger(L, "offset", expo->offset);
    lua_pushtableinteger(L, "switch", expo->swtch);
    lua_pushtableinteger(L, "curveType", expo->curve.type);
    lua_pushtableinteger(L, "curveValue", expo->curve.value);
    lua_pushtableinteger(L, "trimSource", - expo->trimSource);
    lua_pushtableinteger(L, "flightModes", expo->flightModes);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function model.insertInput(input, line, value)

Insert an Input at specified line

@param input (unsigned number) input number (use 0 for Input1)

@param line  (unsigned number) input line (use 0 for first line)

@param value (table) input data, see model.getInput()

@status current Introduced in 2.0.0, curveType/curveValue/carryTrim added in 2.3, inputName added 2.3.10, broken carryTrim replaced by trimSource in EdgeTX 2.8.1, scale added in 2.10
*/
static int luaModelInsertInput(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  unsigned int idx = luaL_checkunsigned(L, 2);

  unsigned int first = getFirstInput(chn);
  unsigned int count = getInputsCountFromFirst(chn, first);

  if (chn<MAX_INPUTS && getExposCount()<MAX_EXPOS && idx<=count) {
    idx = first + idx;
    s_currCh = chn + 1;
#if defined(COLORLCD)
    insertExpo(idx, chn);
#else
    insertExpo(idx);
#endif
    ExpoData * expo = expoAddress(idx);
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      luaL_checktype(L, -2, LUA_TSTRING); // key is string
      const char * key = luaL_checkstring(L, -2);
      if (!strcmp(key, "name")) {
        const char * name = luaL_checkstring(L, -1);
        strncpy(expo->name, name, sizeof(expo->name));
      }
      else if (!strcmp(key, "inputName")) {
        const char * name = luaL_checkstring(L, -1);
        strncpy(g_model.inputNames[chn], name, LEN_INPUT_NAME);
      }
      else if (!strcmp(key, "source")) {
        expo->srcRaw = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "scale")) {
        expo->scale = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "weight")) {
        int val = luaL_checkinteger(L, -1);
        SourceNumVal v;
        v.isSource = (abs(val) >= 1024);
        v.value = val;
        expo->weight = v.rawValue;
      }
      else if (!strcmp(key, "offset")) {
        int val = luaL_checkinteger(L, -1);
        SourceNumVal v;
        v.isSource = (abs(val) >= 1024);
        v.value = val;
        expo->offset = v.rawValue;
      }
      else if (!strcmp(key, "switch")) {
        expo->swtch = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "curveType")) {
        expo->curve.type = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "curveValue")) {
        int val = luaL_checkinteger(L, -1);
        SourceNumVal v;
        v.isSource = (abs(val) >= 1024);
        v.value = val;
        expo->curve.value = v.rawValue;
      }
      else if (!strcmp(key, "trimSource")) {
        expo->trimSource = - luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "flightModes")) {
        expo->flightModes = luaL_checkinteger(L, -1);
      }
    }
  }

  return 0;
}

/*luadoc
@function model.deleteInput(input, line)

Delete line from specified input

@param input (unsigned number) input number (use 0 for Input1)

@param line  (unsigned number) input line (use 0 for first line)

@status current Introduced in 2.0.0
*/
static int luaModelDeleteInput(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  unsigned int idx = luaL_checkunsigned(L, 2);

  int first = getFirstInput(chn);
  unsigned int count = getInputsCountFromFirst(chn, first);

  if (idx < count) {
    deleteExpo(first+idx);
  }

  return 0;
}

/*luadoc
@function model.deleteInputs()

Delete all Inputs

@status current Introduced in 2.0.0
*/
static int luaModelDeleteInputs(lua_State *L)
{
  clearInputs();
  return 0;
}

/*luadoc
@function model.defaultInputs()

Set all inputs to defaults

@status current Introduced in 2.0.0
*/
static int luaModelDefaultInputs(lua_State *L)
{
  clearInputs();
  setDefaultInputs();
  return 0;
}

static unsigned int getFirstMix(unsigned int chn)
{
  for (unsigned int i=0; i<MAX_MIXERS; i++) {
    MixData * mix = mixAddress(i);
    if (!mix->srcRaw || mix->destCh>=chn) {
      return i;
    }
  }
  return 0;
}

static unsigned int getMixesCountFromFirst(unsigned int chn, unsigned int first)
{
  unsigned int count = 0;
  for (unsigned int i=first; i<MAX_MIXERS; i++) {
    MixData * mix = mixAddress(i);
    if (!mix->srcRaw || mix->destCh!=chn) break;
    count++;
  }
  return count;
}

static unsigned int getMixesCount(unsigned int chn)
{
  return getMixesCountFromFirst(chn, getFirstMix(chn));
}

/*luadoc
@function model.getMixesCount(channel)

Get the number of Mixer lines that the specified Channel has

@param channel (unsigned number) channel number (use 0 for CH1)

@retval number number of mixes for requested channel

@status current Introduced in 2.0.0
*/
static int luaModelGetMixesCount(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  unsigned int count = getMixesCount(chn);
  lua_pushinteger(L, count);
  return 1;
}

/*luadoc
@function model.getMix(channel, line)

Get configuration for specified Mix

@param channel (unsigned number) channel number (use 0 for CH1)

@param line (unsigned number) mix number (use 0 for first line(mix))

@retval nil requested channel or line does not exist

@retval table mix data:
 * `name` (string) mix line name
 * `source` (number) source index
 * `weight` (number) weight (1024 == 100%) value or GVAR1..9 = 4096..4011, -GVAR1..9 = 4095..4087
 * `offset` (number) offset value or GVAR1..9 = 4096..4011, -GVAR1..9 = 4095..4087
 * `switch` (number) switch index
 * `multiplex` (number) multiplex (0 = ADD, 1 = MULTIPLY, 2 = REPLACE)
 * `curveType` (number) curve type (function, expo, custom curve)
 * `curveValue` (number) curve index
 * `flightModes` (number) bit-mask of active flight modes
 * `carryTrim` (boolean) carry trim
 * `mixWarn` (number) warning (0 = off, 1 = 1 beep, .. 3 = 3 beeps)
 * `delayUp` (number) delay up (time in 1/10 s)
 * `delayDown` (number) delay down
 * `speedUp` (number) speed up
 * `speedDown` (number) speed down

@status current Introduced in 2.0.0, parameters below `multiplex` added in 2.0.13
*/
static int luaModelGetMix(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  unsigned int idx = luaL_checkunsigned(L, 2);
  unsigned int first = getFirstMix(chn);
  unsigned int count = getMixesCountFromFirst(chn, first);
  if (idx < count) {
    MixData * mix = mixAddress(first+idx);
    lua_newtable(L);
    lua_pushtablenstring(L, "name", mix->name);
    lua_pushtableinteger(L, "source", mix->srcRaw);
    lua_pushtableinteger(L, "weight", mix->weight);
    lua_pushtableinteger(L, "offset", mix->offset);
    lua_pushtableinteger(L, "switch", mix->swtch);
    lua_pushtableinteger(L, "curveType", mix->curve.type);
    lua_pushtableinteger(L, "curveValue", mix->curve.value);
    lua_pushtableinteger(L, "multiplex", mix->mltpx);
    lua_pushtableinteger(L, "flightModes", mix->flightModes);
    lua_pushtableboolean(L, "carryTrim", mix->carryTrim);
    lua_pushtableinteger(L, "mixWarn", mix->mixWarn);
    lua_pushtableinteger(L, "delayUp", mix->delayUp);
    lua_pushtableinteger(L, "delayDown", mix->delayDown);
    lua_pushtableinteger(L, "speedUp", mix->speedUp);
    lua_pushtableinteger(L, "speedDown", mix->speedDown);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function model.insertMix(channel, line, value)

Insert a mixer line into Channel

@param channel (unsigned number) channel number (use 0 for CH1)

@param line (unsigned number) mix number (use 0 for first line(mix))

@param value (table) see model.getMix() for table format

@status current Introduced in 2.0.0, parameters below `multiplex` added in 2.0.13
*/
static int luaModelInsertMix(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  unsigned int idx = luaL_checkunsigned(L, 2);

  unsigned int first = getFirstMix(chn);
  unsigned int count = getMixesCountFromFirst(chn, first);

  if (chn<MAX_OUTPUT_CHANNELS && getMixCount()<MAX_MIXERS && idx<=count) {
    idx += first;
    insertMix(idx, chn);
    MixData *mix = mixAddress(idx);
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      luaL_checktype(L, -2, LUA_TSTRING); // key is string
      const char * key = luaL_checkstring(L, -2);
      if (!strcmp(key, "name")) {
        const char * name = luaL_checkstring(L, -1);
        strncpy(mix->name, name, sizeof(mix->name));
      }
      else if (!strcmp(key, "source")) {
        mix->srcRaw = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "weight")) {
        int val = luaL_checkinteger(L, -1);
        SourceNumVal v;
        v.isSource = (abs(val) >= 1024);
        v.value = val;
        mix->weight = v.rawValue;
      }
      else if (!strcmp(key, "offset")) {
        int val = luaL_checkinteger(L, -1);
        SourceNumVal v;
        v.isSource = (abs(val) >= 1024);
        v.value = val;
        mix->offset = v.rawValue;
      }
      else if (!strcmp(key, "switch")) {
        mix->swtch = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "curveType")) {
        mix->curve.type = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "curveValue")) {
        int val = luaL_checkinteger(L, -1);
        SourceNumVal v;
        v.isSource = (abs(val) >= 1024);
        v.value = val;
        mix->curve.value = v.rawValue;
      }
      else if (!strcmp(key, "multiplex")) {
        mix->mltpx = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "flightModes")) {
        mix->flightModes = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "carryTrim")) {
        mix->carryTrim = lua_toboolean(L, -1);
      }
      else if (!strcmp(key, "mixWarn")) {
        mix->mixWarn = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "delayUp")) {
        mix->delayUp = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "delayDown")) {
        mix->delayDown = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "speedUp")) {
        mix->speedUp = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "speedDown")) {
        mix->speedDown = luaL_checkinteger(L, -1);
      }
    }
  }

  return 0;
}

/*luadoc
@function model.deleteMix(channel, line)

Delete mixer line from specified Channel

@param channel (unsigned number) channel number (use 0 for CH1)

@param line (unsigned number) mix number (use 0 for first line(mix))

@status current Introduced in 2.0.0
*/
static int luaModelDeleteMix(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  unsigned int idx = luaL_checkunsigned(L, 2);

  unsigned int first = getFirstMix(chn);
  unsigned int count = getMixesCountFromFirst(chn, first);

  if (idx < count) {
    deleteMix(first+idx);
  }

  return 0;
}

/*luadoc
@function model.deleteMixes()

Remove all mixers

@status current Introduced in 2.0.0
*/
static int luaModelDeleteMixes(lua_State *L)
{
  memset(g_model.mixData, 0, sizeof(g_model.mixData));
  return 0;
}

/*luadoc
@function model.getLogicalSwitch(switch)

Get Logical Switch parameters

@param switch (unsigned number) logical switch number (use 0 for LS1)

@retval nil requested logical switch does not exist

@retval table logical switch data:
 * `func` (number) function index
 * `v1` (number) V1 value (index)
 * `v2` (number) V2 value (index or value)
 * `v3` (number) V3 value (index or value)
 * `and` (number) AND switch index
 * `delay` (number) delay (time in 1/10 s)
 * `duration` (number) duration (time in 1/10 s)

@status current Introduced in 2.0.0
*/
static int luaModelGetLogicalSwitch(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_LOGICAL_SWITCHES) {
    LogicalSwitchData * sw = lswAddress(idx);
    lua_newtable(L);
    lua_pushtableinteger(L, "func", sw->func);
    lua_pushtableinteger(L, "v1", sw->v1);
    lua_pushtableinteger(L, "v2", sw->v2);
    lua_pushtableinteger(L, "v3", sw->v3);
    lua_pushtableinteger(L, "and", sw->andsw);
    lua_pushtableinteger(L, "delay", sw->delay);
    lua_pushtableinteger(L, "duration", sw->duration);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function model.setLogicalSwitch(switch, value)

Set Logical Switch parameters

@param switch (unsigned number) logical switch number (use 0 for LS1)

@param value (table) see model.getLogicalSwitch() for table format

@notice If a parameter is missing from the value, then
that parameter remains unchanged.

@notice To set the `and` member (which is Lua keyword)
use the following syntax: `model.setLogicalSwitch(30, {func=4,v1=1,v2=-99, ["and"]=24})`

@status current Introduced in 2.0.0
*/
static int luaModelSetLogicalSwitch(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_LOGICAL_SWITCHES) {
    LogicalSwitchData * sw = lswAddress(idx);
    memclear(sw, sizeof(LogicalSwitchData));
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      luaL_checktype(L, -2, LUA_TSTRING); // key is string
      const char * key = luaL_checkstring(L, -2);
      if (!strcmp(key, "func")) {
        sw->func = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "v1")) {
        sw->v1 = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "v2")) {
        sw->v2 = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "v3")) {
        sw->v3 = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "and")) {
        sw->andsw = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "delay")) {
        sw->delay = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "duration")) {
        sw->duration = luaL_checkinteger(L, -1);
      }
    }
    storageDirty(EE_MODEL);
  }

  return 0;
}

/*luadoc
@function model.getCurve(curve)

Get Curve parameters

@param curve (unsigned number) curve number (use 0 for Curve1)

@retval nil requested curve does not exist

@retval table curve data:
 * `name` (string) name
 * `type` (number) type
 * `smooth` (boolean) smooth
 * `points` (number) number of points
 * `y` (table) table of Y values:
   * `key` is point number (zero based)
   * `value` is y value
 * `x` (table) **only included for custom curve type**:
   * `key` is point number (zero based)
   * `value` is x value

 Note that functions returns the tables starting with index 0 contrary to LUA's
 usual index starting with 1

@status current Introduced in 2.0.12
*/
static int luaModelGetCurve(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_CURVES) {
    CurveHeader & CurveHeader = g_model.curves[idx];
    lua_newtable(L);
    lua_pushtablenstring(L, "name", CurveHeader.name);
    lua_pushtableinteger(L, "type", CurveHeader.type);
    lua_pushtableboolean(L, "smooth", CurveHeader.smooth);
    lua_pushtableinteger(L, "points", CurveHeader.points + 5);
    lua_pushstring(L, "y");
    lua_newtable(L);
    int8_t * point = curveAddress(idx);
    for (int i=0; i < CurveHeader.points + 5; i++) {
      lua_pushinteger(L, i + 1);
      lua_pushinteger(L, *point++);
      lua_settable(L, -3);
    }
    lua_settable(L, -3);
    if (CurveHeader.type == CURVE_TYPE_CUSTOM) {
      lua_pushstring(L, "x");
      lua_newtable(L);
      lua_pushinteger(L, 1);
      lua_pushinteger(L, -100);
      lua_settable(L, -3);
      for (int i=0; i < CurveHeader.points + 3; i++) {
        lua_pushinteger(L, i + 2);
        lua_pushinteger(L, *point++);
        lua_settable(L, -3);
      }
      lua_pushinteger(L, CurveHeader.points + 5);
      lua_pushinteger(L, 100);
      lua_settable(L, -3);
      lua_settable(L, -3);
    }
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function model.setCurve(curve, params)

Set Curve parameters

@param curve (unsigned number) curve number (use 0 for Curve1)

@param params see model.getCurve return format for table format. setCurve uses standard
 lua array indexing and arrays start at index 1

The first and last x value must -100 and 100 and x values must be monotonically increasing

@retval  0 - Everything okay
         1 - Wrong number of points
         2 - Invalid Curve number
         3 - Cuve does not fit anymore
         4 - point of out of index
         5 - x value not monotonically increasing
         6 - y value not in range [-100;100]
         7 - extra values for y are set
         8 - extra values for x are set

@status current Introduced in 2.2.0

Example setting a 4-point custom curve:
```lua
  params = {}
  params["x"] =  {-100, -34, 77, 100}
  params["y"] = {-70, 20, -89, -100}
  params["smooth"] = true
  params["type"] = 1
  val =  model.setCurve(2, params)
 ```
setting a 6-point standard smoothed curve
 ```lua
 val = model.setCurve(3, {smooth=true, y={-100, -50, 0, 50, 100, 80}})
 ```

*/
static int luaModelSetCurve(lua_State *L)
{
  unsigned int curveIdx = luaL_checkunsigned(L, 1);

  if (curveIdx >= MAX_CURVES) {
    lua_pushinteger(L, 2);
    return 1;
  }
  int8_t xPoints[MAX_POINTS_PER_CURVE];
  int8_t yPoints[MAX_POINTS_PER_CURVE];

  // Init to invalid values
  memset(xPoints, -127, sizeof(xPoints));
  memset(yPoints, -127, sizeof(yPoints));


  CurveHeader &destCurveHeader = g_model.curves[curveIdx];
  CurveHeader newCurveHeader;
  memclear(&newCurveHeader, sizeof(CurveHeader));

  luaL_checktype(L, -1, LUA_TTABLE);
  for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
    luaL_checktype(L, -2, LUA_TSTRING); // key is string
    const char *key = luaL_checkstring(L, -2);
    if (!strcmp(key, "name")) {
      const char *name = luaL_checkstring(L, -1);
      strncpy(newCurveHeader.name, name, sizeof(newCurveHeader.name));
    }
    else if (!strcmp(key, "type")) {
      newCurveHeader.type = luaL_checkinteger(L, -1);
    }
    else if (!strcmp(key, "smooth")) {
      // Earlier version of this api expected a 0/1 integer instead of a boolean
      // Still accept a 0/1 here
      if (lua_isboolean(L,-1))
        newCurveHeader.smooth = lua_toboolean(L, -1);
      else
        newCurveHeader.smooth = luaL_checkinteger(L, -1);
    }
    else if (!strcmp(key, "x") || !strcmp(key, "y")) {
      luaL_checktype(L, -1, LUA_TTABLE);
      bool isX = !strcmp(key, "x");

      for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
        int idx = luaL_checkinteger(L, -2) - 1; // key is integer
        if (idx < 0 || idx > MAX_POINTS_PER_CURVE) {
          lua_pushinteger(L, 4);
          return 1;
        }
        int8_t val = luaL_checkinteger(L, -1);
        if (val < -100 || val > 100) {
          lua_pushinteger(L, 6);
          return 1;
        }
        if (isX)
          xPoints[idx] = val;
        else
          yPoints[idx] = val;
      }
    }
  }
  // Check how many points are set
  uint8_t numPoints=0;
  do {
    numPoints++;
  } while (yPoints[numPoints]!=-127 && numPoints < MAX_POINTS_PER_CURVE);
  newCurveHeader.points = numPoints - 5;

  if (numPoints < MIN_POINTS_PER_CURVE || numPoints > MAX_POINTS_PER_CURVE) {
    lua_pushinteger(L, 1);
    return 1;
  }

  if (newCurveHeader.type == CURVE_TYPE_CUSTOM) {

    // The rest of the points are checked by the monotonic condition
    for (unsigned int i=numPoints; i < sizeof(xPoints);i++)
    {
      if (xPoints[i] != -127)
      {
        lua_pushinteger(L, 8);
        return 1;
      }
    }

    // Check first and last point
    if (xPoints[0] != -100 || xPoints[newCurveHeader.points + 4] != 100) {
      lua_pushinteger(L, 5);
      return 1;
    }

    // Check that x values are increasing
    for (int i = 1; i < numPoints; i++) {
      if (xPoints[i - 1] > xPoints[i]) {
        lua_pushinteger(L, 5);
        return 1;
      }
    }
  }

  // Check that ypoints have the right number of points set
  for (int i=0; i <  5 + newCurveHeader.points;i++)
  {
    if (yPoints[i] == -127)
    {
      lua_pushinteger(L, 7);
      return 1;
    }
  }

  // Calculate size of curve we replace
  int oldCurveMemSize;
  if (destCurveHeader.type == CURVE_TYPE_STANDARD) {
    oldCurveMemSize = 5 + destCurveHeader.points;
  }
  else {
    oldCurveMemSize = 8 + 2 * destCurveHeader.points;
  }

  // Calculate own size
  int newCurveMemSize;
  if (newCurveHeader.type == CURVE_TYPE_STANDARD)
    newCurveMemSize = 5 + newCurveHeader.points;
  else
    newCurveMemSize = 8 + 2 * newCurveHeader.points;

  int shift = newCurveMemSize - oldCurveMemSize;

  // Also checks if new curve size would fit
  if (!moveCurve(curveIdx, shift)) {
    lua_pushinteger(L, 3);
    TRACE("curve shift is  %d", shift);
    return 1;
  }

  // Curve fits into mem, fill new curve
  destCurveHeader = newCurveHeader;

  int8_t *point = curveAddress(curveIdx);
  for (int i = 0; i < destCurveHeader.points + 5; i++) {
    *point++ = yPoints[i];
  }

  if (destCurveHeader.type == CURVE_TYPE_CUSTOM) {
    for (int i = 1; i < destCurveHeader.points + 4; i++) {
      *point++ = xPoints[i];
    }
  }
  storageDirty(EE_MODEL);

  lua_pushinteger(L, 0);
  return 1;
}

/*luadoc
@function model.getCustomFunction(function)

Get Custom Function parameters

@param function (unsigned number) custom function number (use 0 for CF1)

@retval nil requested custom function does not exist

@retval table custom function data:
 * `switch` (number) switch index
 * `func` (number) function index
 * `name` (string)  Name of track to play (only returned only returned if action is play track, sound or script)
 * `value` (number) value (only returned only returned if action is **not** play track, sound or script)
 * `mode` (number) mode (only returned only returned if action is **not** play track, sound or script)
 * `param` (number) parameter (only returned only returned if action is **not** play track, sound or script)
 * `active` (number) 0 = disabled, 1 = enabled
 * `repetition` (number) -1 to 60, range and meaning depend on function

@status current Introduced in 2.0.0, TODO rename function
*/
static int luaModelGetCustomFunction(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_SPECIAL_FUNCTIONS) {
    CustomFunctionData * cfn = &g_model.customFn[idx];
    lua_newtable(L);
    lua_pushtableinteger(L, "switch", CFN_SWITCH(cfn));
    lua_pushtableinteger(L, "func", CFN_FUNC(cfn));
    if (CFN_FUNC(cfn) == FUNC_PLAY_TRACK || CFN_FUNC(cfn) == FUNC_BACKGND_MUSIC || CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT || CFN_FUNC(cfn) == FUNC_RGB_LED) {
      lua_pushtablenstring(L, "name", cfn->play.name);
    }
    else {
      lua_pushtableinteger(L, "value", cfn->all.val);
      lua_pushtableinteger(L, "mode", cfn->all.mode);
      lua_pushtableinteger(L, "param", cfn->all.param);
    }
    lua_pushtableinteger(L, "active", CFN_ACTIVE(cfn));
    lua_pushtableinteger(L, "repetition", cfn->repeat);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function model.setCustomFunction(function, value)

Set Custom Function parameters

@param function (unsigned number) custom function number (use 0 for CF1)

@param value (table) custom function parameters, see model.getCustomFunction() for table format

@notice If a parameter is missing from the value, then
that parameter remains unchanged.

@status current Introduced in 2.0.0, TODO rename function
*/
static int luaModelSetCustomFunction(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_SPECIAL_FUNCTIONS) {
    CustomFunctionData * cfn = &g_model.customFn[idx];
    memclear(cfn, sizeof(CustomFunctionData));
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      luaL_checktype(L, -2, LUA_TSTRING); // key is string
      const char * key = luaL_checkstring(L, -2);
      if (!strcmp(key, "switch")) {
        CFN_SWITCH(cfn) = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "func")) {
        CFN_FUNC(cfn) = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "name")) {
        const char * name = luaL_checkstring(L, -1);
        strncpy(cfn->play.name, name, sizeof(cfn->play.name));
      }
      else if (!strcmp(key, "value")) {
        cfn->all.val = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "mode")) {
        cfn->all.mode = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "param")) {
        cfn->all.param = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "active")) {
        CFN_ACTIVE(cfn) = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "repetition")) {
        cfn->repeat = luaL_checkinteger(L, -1);
      }
    }
    storageDirty(EE_MODEL);
  }

  return 0;
}

/*luadoc
@function model.getOutput(index)

Get servo parameters

@param index (unsigned number) output number (use 0 for CH1)

@retval nil requested output does not exist

@retval table output parameters:
 * `name` (string) name
 * `min` (number) Minimum % * 10
 * `max` (number) Maximum % * 10
 * `offset` (number) Subtrim * 10
 * `ppmCenter` (number) offset from PPM Center. 0 = 1500
 * `symetrical` (number) linear Subtrim 0 = Off, 1 = On
 * `revert` (number) irection 0 = ­­­---, 1 = INV
 * `curve`
   * (number) Curve number (0 for Curve1)
   * or `nil` if no curve set

@status current Introduced in 2.0.0
*/
static int luaModelGetOutput(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_OUTPUT_CHANNELS) {
    LimitData * limit = limitAddress(idx);
    lua_newtable(L);
    lua_pushtablenstring(L, "name", limit->name);
    lua_pushtableinteger(L, "min", limit->min-1000);
    lua_pushtableinteger(L, "max", limit->max+1000);
    lua_pushtableinteger(L, "offset", limit->offset);
    lua_pushtableinteger(L, "ppmCenter", limit->ppmCenter);
    lua_pushtableinteger(L, "symetrical", limit->symetrical);
    lua_pushtableinteger(L, "revert", limit->revert);
    if (limit->curve)
      lua_pushtableinteger(L, "curve", limit->curve-1);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function model.setOutput(index, value)

Set servo parameters

@param index (unsigned number) channel number (use 0 for CH1)

@param value (table) servo parameters, see model.getOutput() for table format

@notice If a parameter is missing from the value, then
that parameter remains unchanged.

@status current Introduced in 2.0.0
*/
static int luaModelSetOutput(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_OUTPUT_CHANNELS) {
    LimitData * limit = limitAddress(idx);
    memclear(limit, sizeof(LimitData));
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      luaL_checktype(L, -2, LUA_TSTRING); // key is string
      const char * key = luaL_checkstring(L, -2);
      if (!strcmp(key, "name")) {
        const char * name = luaL_checkstring(L, -1);
        strncpy(limit->name, name, sizeof(limit->name));
      }
      else if (!strcmp(key, "min")) {
        limit->min = luaL_checkinteger(L, -1)+1000;
      }
      else if (!strcmp(key, "max")) {
        limit->max = luaL_checkinteger(L, -1)-1000;
      }
      else if (!strcmp(key, "offset")) {
        limit->offset = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "ppmCenter")) {
        limit->ppmCenter = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "symetrical")) {
        limit->symetrical = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "revert")) {
        limit->revert = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "curve")) {
        limit->curve = luaL_checkinteger(L, -1) + 1;
      }
    }
    storageDirty(EE_MODEL);
  }

  return 0;
}

#if defined(GVARS)
/*luadoc
@function model.getGlobalVariable(index, flight_mode)

Return current global variable value

@notice a simple warning or notice

@param index  zero based global variable index, use 0 for GV1, 8 for GV9

@param flight_mode  Flight mode number (0 = FM0, 8 = FM8)

@retval nil   requested global variable does not exist

@retval number current value of global variable

Example:

```lua
  -- get GV3 (index = 2) from Flight mode 0 (FM0)
  val = model.getGlobalVariable(2, 0)
```
*/
static int luaModelGetGlobalVariable(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  unsigned int phase = luaL_checkunsigned(L, 2);
  if (phase < MAX_FLIGHT_MODES && idx < MAX_GVARS)
    lua_pushinteger(L, getGVarValue(idx, phase));
  else
    lua_pushnil(L);
  return 1;
}

/*luadoc
@function model.setGlobalVariable(index, flight_mode, value)

Sets current global variable value. See also model.getGlobalVariable()

@param index  zero based global variable index, use 0 for GV1, 8 for GV9

@param flight_mode  Flight mode number (0 = FM0, 8 = FM8)

@param value  new value for global variable. Permitted range is
from -1024 to 1024.

@notice Global variable can only store integer values,
any floating point value is converted into integer value
by truncating everything behind a floating point.
*/
static int luaModelSetGlobalVariable(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  unsigned int phase = luaL_checkunsigned(L, 2);
  int value = luaL_checkinteger(L, 3);
  if (phase < MAX_FLIGHT_MODES && idx < MAX_GVARS && value >= -GVAR_MAX && value <= GVAR_MAX) {
    SET_GVAR(idx, value, phase);
    storageDirty(EE_MODEL);
  }
  return 0;
}
#endif

/*luadoc
@function model.getSensor(sensor)

Get Telemetry Sensor parameters

@param sensor (unsigned number) sensor number (use 0 for sensor 1)

@retval nil requested sensor does not exist

@retval table with sensor data:
 * `type` (number) 0 = custom, 1 = calculated
 * `name` (string) Name
 * `unit` (number) See list of units in the appendix of the OpenTX Lua Reference Guide
 * `prec` (number) Number of decimals
 * `id`   (number) Only custom sensors
 * `instance` (number) Only custom sensors
 * `formula` (number) Only calculated sensors. 0 = Add etc. see list of formula choices in Companion popup

@status current Introduced in 2.3.0
*/
static int luaModelGetSensor(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_TELEMETRY_SENSORS) {
    TelemetrySensor & sensor = g_model.telemetrySensors[idx];
    lua_newtable(L);
    lua_pushtableinteger(L, "type", sensor.type);
    lua_pushtablenstring(L, "name", sensor.label);
    lua_pushtableinteger(L, "unit", sensor.unit);
    lua_pushtableinteger(L, "prec", sensor.prec);
    if (sensor.type == TELEM_TYPE_CUSTOM) {
      lua_pushtableinteger(L, "id", sensor.id);
      lua_pushtableinteger(L, "instance", sensor.instance);
    }
    else {
      lua_pushtableinteger(L, "formula", sensor.formula);
    }
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function model.resetSensor(sensor)

Reset Telemetry Sensor parameters

@param sensor (unsigned number) sensor number (use 0 for sensor 1)

@retval nil

@status current Introduced in 2.3.11
*/
static int luaModelResetSensor(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_TELEMETRY_SENSORS) {
    telemetryItems[idx].clear();
  }

  lua_pushnil(L);
  return 1;
}

#if defined(HELI)
/*luadoc
@function model.getSwashRing()

Get heli swash parameters

@retval table with heli swash parameters:
* `type` (number) 0=---, 1=120, 2=120X, 3=140, 4=90
* `value` (number) swash ring value (normally 0)
* 'collectiveSource' (number) source index
* 'aileronSource' (number) source index
* 'elevatorSource' (number) source index
* 'collectiveWeight'(value) -100 to 100
* 'aileronWeight' (value) -100 to 100
* 'elevatorWeight' (value) -100 to 100

 @status current Introduced in 2.8.0
*/
static int luaModelGetSwashRing(lua_State *L)
{
  lua_newtable(L);
  lua_pushtableinteger(L, "type", g_model.swashR.type);
  lua_pushtableinteger(L, "value", g_model.swashR.value);
  lua_pushtableinteger(L, "collectiveSource", g_model.swashR.collectiveSource);
  lua_pushtableinteger(L, "aileronSource", g_model.swashR.aileronSource);
  lua_pushtableinteger(L, "elevatorSource", g_model.swashR.elevatorSource);
  lua_pushtableinteger(L, "collectiveWeight", g_model.swashR.collectiveWeight);
  lua_pushtableinteger(L, "aileronWeight", g_model.swashR.aileronWeight);
  lua_pushtableinteger(L, "elevatorWeight", g_model.swashR.elevatorWeight);

  return 1;
}

/*luadoc
@function model.setSwashRing(params)

Set heli swash parameters

@param value (table) swash ring parameters, see model.getSwashRing() for table format

@notice If a parameter is missing, then that parameter remains unchanged.

@status current Introduced in 2.8.0
*/
static int luaModelSetSwashRing(lua_State *L)
{
  luaL_checktype(L, -1, LUA_TTABLE);
  for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
    luaL_checktype(L, -2, LUA_TSTRING); // key is string
    const char * key = luaL_checkstring(L, -2);
    if (!strcmp(key, "type")) {
      g_model.swashR.type = luaL_checkinteger(L, -1);
    }
    else if (!strcmp(key, "value")) {
      g_model.swashR.value = luaL_checkinteger(L, -1);
    }
    else if (!strcmp(key, "collectiveSource")) {
      g_model.swashR.collectiveSource = luaL_checkinteger(L, -1);
    }
    else if (!strcmp(key, "aileronSource")) {
      g_model.swashR.aileronSource = luaL_checkinteger(L, -1);
    }
    else if (!strcmp(key, "elevatorSource")) {
      g_model.swashR.elevatorSource = luaL_checkinteger(L, -1);
    }
    else if (!strcmp(key, "collectiveWeight")) {
      g_model.swashR.collectiveWeight = luaL_checkinteger(L, -1);
    }
    else if (!strcmp(key, "aileronWeight")) {
      g_model.swashR.aileronWeight = luaL_checkinteger(L, -1);
    }
    else if (!strcmp(key, "elevatorWeight")) {
      g_model.swashR.elevatorWeight = luaL_checkinteger(L, -1);
    }
  }

  storageDirty(EE_MODEL);
  return 0;
}
#endif // HELI

LROT_BEGIN(modellib, NULL, 0)
  LROT_FUNCENTRY( getInfo, luaModelGetInfo )
  LROT_FUNCENTRY( setInfo, luaModelSetInfo )
  LROT_FUNCENTRY( getModule, luaModelGetModule )
  LROT_FUNCENTRY( setModule, luaModelSetModule )
  LROT_FUNCENTRY( getTimer, luaModelGetTimer )
  LROT_FUNCENTRY( setTimer, luaModelSetTimer )
  LROT_FUNCENTRY( resetTimer, luaModelResetTimer )
  LROT_FUNCENTRY( deleteFlightModes, luaModelDeleteFlightModes )
  LROT_FUNCENTRY( getFlightMode, luaModelGetFlightMode )
  LROT_FUNCENTRY( setFlightMode, luaModelSetFlightMode )
  LROT_FUNCENTRY( getInputsCount, luaModelGetInputsCount )
  LROT_FUNCENTRY( getInput, luaModelGetInput )
  LROT_FUNCENTRY( insertInput, luaModelInsertInput )
  LROT_FUNCENTRY( deleteInput, luaModelDeleteInput )
  LROT_FUNCENTRY( deleteInputs, luaModelDeleteInputs )
  LROT_FUNCENTRY( defaultInputs, luaModelDefaultInputs )
  LROT_FUNCENTRY( getMixesCount, luaModelGetMixesCount )
  LROT_FUNCENTRY( getMix, luaModelGetMix )
  LROT_FUNCENTRY( insertMix, luaModelInsertMix )
  LROT_FUNCENTRY( deleteMix, luaModelDeleteMix )
  LROT_FUNCENTRY( deleteMixes, luaModelDeleteMixes )
  LROT_FUNCENTRY( getLogicalSwitch, luaModelGetLogicalSwitch )
  LROT_FUNCENTRY( setLogicalSwitch, luaModelSetLogicalSwitch )
  LROT_FUNCENTRY( getCustomFunction, luaModelGetCustomFunction )
  LROT_FUNCENTRY( setCustomFunction, luaModelSetCustomFunction )
  LROT_FUNCENTRY( getCurve, luaModelGetCurve )
  LROT_FUNCENTRY( setCurve, luaModelSetCurve )
  LROT_FUNCENTRY( getOutput, luaModelGetOutput )
  LROT_FUNCENTRY( setOutput, luaModelSetOutput )
#if defined (GVARS)
  LROT_FUNCENTRY( getGlobalVariable, luaModelGetGlobalVariable )
  LROT_FUNCENTRY( setGlobalVariable, luaModelSetGlobalVariable )
#endif
  LROT_FUNCENTRY( getSensor, luaModelGetSensor )
  LROT_FUNCENTRY( resetSensor, luaModelResetSensor )
#if defined(HELI)
  LROT_FUNCENTRY( getSwashRing, luaModelGetSwashRing )
  LROT_FUNCENTRY( setSwashRing, luaModelSetSwashRing )
#endif
LROT_END(modellib, NULL, 0)
