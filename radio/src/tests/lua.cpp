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

#include <math.h>
#include "gtests.h"

#if defined(LUA)

#include "edgetx.h"
#include "lua/lua_states.h"

#include <filesystem>

#define MIXSRC_THR     (MIXSRC_FIRST_STICK + inputMappingGetThrottle())
#define MIXSRC_TRIMTHR (MIXSRC_FIRST_TRIM + inputMappingGetThrottle())

::testing::AssertionResult __luaExecStr(const char * str)
{
  extern lua_State * lsScripts;
  if (!lsScripts) { luaInitMainState(); luaInit(); }
  if (!lsScripts) return ::testing::AssertionFailure() << "No Lua state!";
  if (luaL_dostring(lsScripts, str)) {
    return ::testing::AssertionFailure() << "lua error: " << lua_tostring(lsScripts, -1);
  }
  return ::testing::AssertionSuccess();
}

#define luaExecStr(test)  EXPECT_TRUE(__luaExecStr(test))

TEST(Lua, testSetModelInfo)
{
  luaExecStr("info = model.getInfo()");
  // luaExecStr("print('model name: '..info.name..' id: '..info.id)");
  luaExecStr("info.name = 'modelA'");
  luaExecStr("model.setInfo(info)");
  // luaExecStr("print('model name: '..info.name..' id: '..info.id)");
  EXPECT_STRNEQ("modelA", g_model.header.name);

  luaExecStr("info.name = 'Model 1'");
  luaExecStr("model.setInfo(info)");
  // luaExecStr("print('model name: '..info.name..' id: '..info.id)");
  EXPECT_STRNEQ("Model 1", g_model.header.name);
}

TEST(Lua, testPanicProtection)
{
  bool passed = false;
  PROTECT_LUA() {
    PROTECT_LUA() {
      // simulate panic
      longjmp(global_lj->b, 1);
    }
    else {
      // we should come here
      passed = true;
    }
    UNPROTECT_LUA();
  }
  else {
    // and not here
    // TRACE("testLuaProtection: test 1 FAILED");
    FAIL() << "Failed test 1";
  }
  UNPROTECT_LUA()

  EXPECT_EQ(passed, true);

  passed = false;

  PROTECT_LUA() {
    PROTECT_LUA() {
      int a = 5;
      UNUSED(a);
    }
    else {
      // we should not come here
      // TRACE("testLuaProtection: test 2 FAILED");
      FAIL() << "Failed test 2";
    }
    UNPROTECT_LUA()
    // simulate panic
    longjmp(global_lj->b, 1);
  }
  else {
    // we should come here
    passed = true;
  }
  UNPROTECT_LUA()

  EXPECT_EQ(passed, true);
}

TEST(Lua, testModelInputs)
{
  MODEL_RESET();
  luaExecStr("noInputs = model.getInputsCount(0)");
  luaExecStr("if noInputs > 0 then error('getInputsCount()') end");

  // add one line on Input4
#if defined(SURFACE_RADIO)
  luaExecStr("model.insertInput(3, 0, {name='test1', source=MIXSRC_TH, weight=56, offset=3, switch=2})");
#else
  luaExecStr("model.insertInput(3, 0, {name='test1', source=MIXSRC_Thr, weight=56, offset=3, switch=2})");
#endif
  EXPECT_EQ(3u, g_model.expoData[0].chn);
  EXPECT_STRNEQ("test1", g_model.expoData[0].name);
  EXPECT_EQ(MIXSRC_THR, g_model.expoData[0].srcRaw);
  EXPECT_EQ(56u, g_model.expoData[0].weight);
  EXPECT_EQ(3u, g_model.expoData[0].offset);
  EXPECT_EQ(2, g_model.expoData[0].swtch);

  // add another one before existing line on Input4
#if defined(SURFACE_RADIO)
  luaExecStr("model.insertInput(3, 0, {name='test2', source=MIXSRC_ST, weight=-56})");
#else
  luaExecStr("model.insertInput(3, 0, {name='test2', source=MIXSRC_Rud, weight=-56})");
#endif
  EXPECT_EQ(3u, g_model.expoData[0].chn);
  EXPECT_STRNEQ("test2", g_model.expoData[0].name);
  EXPECT_EQ((short int)MIXSRC_FIRST_STICK, g_model.expoData[0].srcRaw);
  SourceNumVal v;
  v.rawValue = g_model.expoData[0].weight;
  EXPECT_EQ(-56, v.value);
  EXPECT_EQ(0u, g_model.expoData[0].offset);
  EXPECT_EQ(0, g_model.expoData[0].swtch);

  EXPECT_EQ(3u, g_model.expoData[1].chn);
  EXPECT_STRNEQ("test1", g_model.expoData[1].name);
  EXPECT_EQ(MIXSRC_THR, g_model.expoData[1].srcRaw);
  EXPECT_EQ(56u, g_model.expoData[1].weight);
  EXPECT_EQ(3u, g_model.expoData[1].offset);
  EXPECT_EQ(2, g_model.expoData[1].swtch);


  // add another line after existing lines on Input4
#if defined(SURFACE_RADIO)
  luaExecStr("model.insertInput(3, model.getInputsCount(3), {name='test3', source=MIXSRC_TH, weight=100})");
#else
  luaExecStr("model.insertInput(3, model.getInputsCount(3), {name='test3', source=MIXSRC_Ail, weight=100})");
#endif
  EXPECT_EQ(3u, g_model.expoData[0].chn);
  EXPECT_STRNEQ("test2", g_model.expoData[0].name);
  EXPECT_EQ(MIXSRC_FIRST_STICK, g_model.expoData[0].srcRaw);
  v.rawValue = g_model.expoData[0].weight;
  EXPECT_EQ(-56, v.value);
  EXPECT_EQ(0u, g_model.expoData[0].offset);
  EXPECT_EQ(0, g_model.expoData[0].swtch);

  EXPECT_EQ(3u, g_model.expoData[1].chn);
  EXPECT_STRNEQ("test1", g_model.expoData[1].name);
  EXPECT_EQ(MIXSRC_THR, g_model.expoData[1].srcRaw);
  EXPECT_EQ(56u, g_model.expoData[1].weight);
  EXPECT_EQ(3u, g_model.expoData[1].offset);
  EXPECT_EQ(2, g_model.expoData[1].swtch);

  EXPECT_EQ(3u, g_model.expoData[2].chn);
  EXPECT_STRNEQ("test3", g_model.expoData[2].name);
#if defined(SURFACE_RADIO)
  EXPECT_EQ(MIXSRC_THR, g_model.expoData[2].srcRaw);
#else
  EXPECT_EQ(MIXSRC_LAST_STICK, g_model.expoData[2].srcRaw);
#endif
  EXPECT_EQ(100u, g_model.expoData[2].weight);
  EXPECT_EQ(0u, g_model.expoData[2].offset);
  EXPECT_EQ(0, g_model.expoData[2].swtch);

  // verify number of lines for Input4
  luaExecStr("noInputs = model.getInputsCount(3)");
  luaExecStr("if noInputs ~= 3 then error('getInputsCount()') end");
}

TEST(Lua, Switches)
{
  luaExecStr("if MIXSRC_SA == nil then error('failed') end");
  luaExecStr("if MIXSRC_SB == nil then error('failed') end");
#if defined(SURFACE_RADIO)
  luaExecStr("if getSwitchIndex('St-') == nil then error('failed') end");
#else
  luaExecStr("if getSwitchIndex('Rud-') == nil then error('failed') end");
#endif
}

// Defines rtMix() / rtInput(): insert a line on channel / input 0 with the
// given fields, read it back, delete it, and check the stored values.
static void luaDefineRoundTrip()
{
  luaExecStr(
      "local function check(l, expect)\n"
      "  if l == nil then error('line not inserted') end\n"
      "  for k, v in pairs(expect) do\n"
      "    if l[k] ~= v then\n"
      "      error(k .. ': expected ' .. tostring(v) .. ' got ' .. tostring(l[k]))\n"
      "    end\n"
      "  end\n"
      "end\n"
      "function rtMix(fields, expect)\n"
      "  model.insertMix(0, 0, fields)\n"
      "  local l = model.getMix(0, 0)\n"
      "  model.deleteMix(0, 0)\n"
      "  check(l, expect or fields)\n"
      "end\n"
      "function rtInput(fields, expect)\n"
      "  model.insertInput(0, 0, fields)\n"
      "  local l = model.getInput(0, 0)\n"
      "  model.deleteInput(0, 0)\n"
      "  check(l, expect or fields)\n"
      "end\n");
}

static std::string luaCurveRoundTrips(const char* rt)
{
  std::string fn(rt);
  std::string s;
  // DIFF and EXPO: -100..100, or a source
  s += "for t = 0, 1 do\n"
       "  for v = -100, 100 do " + fn + "({curveType = t, curveValue = v}) end\n"
       "  " + fn + "({curveType = t, curveValue = 1025})\n"
       "  " + fn + "({curveType = t, curveValue = -1025})\n"
       "end\n";
  // FUNC: 0..CURVE_BASE-1
  s += "for v = 0, " + std::to_string(CURVE_BASE - 1) + " do " + fn +
       "({curveType = 2, curveValue = v}) end\n";
  // CUSTOM: -MAX_CURVES..MAX_CURVES
  s += "for v = -" + std::to_string(MAX_CURVES) + ", " +
       std::to_string(MAX_CURVES) + " do " + fn +
       "({curveType = 3, curveValue = v}) end\n";
  return s;
}

static std::string luaSwitchRoundTrips(const char* rt)
{
  return "for v = " + std::to_string(SWSRC_FIRST) + ", " +
         std::to_string(SWSRC_LAST) + " do " + rt + "({switch = v}) end\n";
}

static std::string luaInvalidRoundTrips(const char* rt)
{
  std::string fn(rt);
  std::string last = std::to_string(SWSRC_LAST);
  std::string s;
  // out of range switches are stored as no switch
  for (auto sw : {last + " + 1", "-(" + last + " + 1)", std::string("100000"),
                  std::string("-100000")}) {
    s += fn + "({switch = " + sw + "}, {switch = 0})\n";
  }
  // invalid curve function values are reset to none
  for (auto v : {std::to_string(CURVE_BASE), std::string("-1"),
                 std::string("511"), std::string("1025")}) {
    s += fn + "({curveType = 2, curveValue = " + v +
         "}, {curveType = 2, curveValue = 0})\n";
  }
  // invalid custom curve values are reset to none
  for (auto v : {std::to_string(MAX_CURVES + 1),
                 "-" + std::to_string(MAX_CURVES + 1), std::string("100"),
                 std::string("1025")}) {
    s += fn + "({curveType = 3, curveValue = " + v +
         "}, {curveType = 3, curveValue = 0})\n";
  }
  // unknown curve types reset the curve
  for (auto t : {"4", "31"}) {
    s += fn + "({curveType = " + std::string(t) +
         ", curveValue = 5}, {curveType = 0, curveValue = 0})\n";
  }
  return s;
}

TEST(Lua, insertMixValidValues)
{
  MODEL_RESET();
  luaDefineRoundTrip();

  luaExecStr("for v = 0, 2 do rtMix({multiplex = v}) end");
  luaExecStr(luaSwitchRoundTrips("rtMix").c_str());
  luaExecStr(luaCurveRoundTrips("rtMix").c_str());
  luaExecStr("if model.getMixesCount(0) ~= 0 then error('lines left') end");
}

TEST(Lua, insertMixInvalidValues)
{
  MODEL_RESET();
  luaDefineRoundTrip();

  // out of range multiplex values are stored as ADD
  for (auto v : {"3", "-1", "5", "100"}) {
    luaExecStr((std::string("rtMix({multiplex = ") + v +
                "}, {multiplex = 0})").c_str());
  }
  luaExecStr(luaInvalidRoundTrips("rtMix").c_str());
  luaExecStr("if model.getMixesCount(0) ~= 0 then error('lines left') end");
}

TEST(Lua, insertInputValidValues)
{
  MODEL_RESET();
  luaDefineRoundTrip();

  luaExecStr(luaSwitchRoundTrips("rtInput").c_str());
  luaExecStr(luaCurveRoundTrips("rtInput").c_str());
  luaExecStr("if model.getInputsCount(0) ~= 0 then error('lines left') end");
}

TEST(Lua, insertInputInvalidValues)
{
  MODEL_RESET();
  luaDefineRoundTrip();

  luaExecStr(luaInvalidRoundTrips("rtInput").c_str());
  luaExecStr("if model.getInputsCount(0) ~= 0 then error('lines left') end");
}

TEST(Lua, getSwitchInfo)
{
  RADIO_RESET();
  MODEL_RESET();
  char name[32];
  char lua[320];

  // Unconfigured switches still return a table, with type SWITCH_NONE
  int unconfigured = -1;
  for (int i = 0; i < switchGetMaxAllSwitches(); i++) {
    if (!switchIsCustomSwitch(i)) {
      g_eeGeneral.switchSetType(i, SWITCH_NONE);
      unconfigured = i;
      break;
    }
  }

  for (int i = 0; i < switchGetMaxAllSwitches(); i++) {
    getSwitchName(name, i);
    snprintf(lua, sizeof(lua),
             "local info = getSwitchInfo(%d)\n"
             "if info == nil then error('nil') end\n"
             "if info.name ~= '%s' then error('name ' .. info.name) end\n"
             "if info.type ~= %d then error('type ' .. info.type) end\n"
             "if info.isCustomisableSwitch ~= %s then error('custom') end",
             MIXSRC_FIRST_SWITCH + i, name, g_model.getSwitchType(i),
             switchIsCustomSwitch(i) ? "true" : "false");
    EXPECT_TRUE(__luaExecStr(lua)) << "switch " << i << " (" << name << ")";
  }

  if (unconfigured >= 0) {
    EXPECT_EQ(SWITCH_NONE, g_model.getSwitchType(unconfigured));
  }

  RADIO_RESET();
}

TEST(Lua, getFieldInfoSwitches)
{
  RADIO_RESET();
  MODEL_RESET();
  char lower[8];
  char lua[512];

  for (int i = 0; i < switchGetMaxAllSwitches(); i++) {
    const char* name = switchGetDefaultName(i);
    size_t len = strlen(name);
    for (size_t n = 0; n <= len && n < sizeof(lower); n++)
      lower[n] = tolower(name[n]);

    // The lower case name can belong to another field first, e.g. the 'sl'
    // slider on T22, but must never find a different switch
    snprintf(lua, sizeof(lua),
             "local id = %d\n"
             "local first, last = %d, %d\n"
             "local info = getFieldInfo('%s')\n"
             "if info == nil or info.id ~= id then error('upper') end\n"
             "info = getFieldInfo('%s')\n"
             "if info == nil then error('lower') end\n"
             "if info.id ~= id and info.id >= first and info.id <= last then\n"
             "  error('lower finds switch ' .. info.id)\n"
             "end\n"
             "info = getFieldInfo(id)\n"
             "if info == nil or info.name ~= '%s' then error('by id') end",
             MIXSRC_FIRST_SWITCH + i, MIXSRC_FIRST_SWITCH,
             MIXSRC_FIRST_SWITCH + switchGetMaxAllSwitches() - 1, name, lower,
             name);
    EXPECT_TRUE(__luaExecStr(lua)) << "switch " << i << " (" << name << ")";
  }
}

TEST(Lua, getFieldInfoSensorBeforeSwitch)
{
  RADIO_RESET();
  MODEL_RESET();
  char lower[TELEM_LABEL_LEN + 1] = {};
  char lua[256];

  // A switch only found by its lower case default name, e.g. 'sw1'
  for (int i = 0; i < switchGetMaxAllSwitches(); i++) {
    const char* name = switchGetDefaultName(i);
    size_t len = strlen(name);
    if (len > 2 && len <= TELEM_LABEL_LEN) {
      for (size_t n = 0; n < len; n++) lower[n] = tolower(name[n]);
      break;
    }
  }
  if (!lower[0]) return;  // no such switch on this target

  // A sensor with the same name is still found first
  strncpy(g_model.telemetrySensors[0].label, lower, TELEM_LABEL_LEN);
  snprintf(lua, sizeof(lua),
           "local info = getFieldInfo('%s')\n"
           "if info == nil or info.id ~= %d then error('not sensor') end",
           lower, MIXSRC_FIRST_TELEM);
  EXPECT_TRUE(__luaExecStr(lua)) << lower;
  MODEL_RESET();
}

TEST(Lua, getChargingState)
{
#if defined(USB_CHARGER)
  luaExecStr("assert(type(getChargingState) == 'function')");
  luaExecStr("assert(select('#', getChargingState()) == 1)");
  luaExecStr("assert(type(getChargingState()) == 'boolean')");
#else
  luaExecStr("assert(getChargingState == nil)");
#endif
}

#if defined(MODULE_BATTERY_SENSOR)
// Fixed names, so scripts work whatever language the source names are in
TEST(Lua, batterySensorFields)
{
  char lua[256];
  for (auto f : {std::make_pair("tx-current", (int)MIXSRC_TX_BAT_CURRENT),
                 std::make_pair("tx-power", (int)MIXSRC_TX_BAT_POWER)}) {
    snprintf(lua, sizeof(lua),
             "local info = getFieldInfo('%s')\n"
             "if info == nil or info.id ~= %d then error('id') end\n"
             "if math.type(getValue('%s')) ~= 'float' then error('float') end",
             f.first, f.second, f.first);
    EXPECT_TRUE(__luaExecStr(lua)) << f.first;
  }
}
#endif

#if defined(VOICE_CONTROL_SENSOR)
TEST(Lua, voiceControlFields)
{
  char lua[128];
  for (auto f : {std::make_pair("vgr", (int)MIXSRC_VGR),
                 std::make_pair("vfl", (int)MIXSRC_VFL)}) {
    snprintf(lua, sizeof(lua),
             "local info = getFieldInfo('%s')\n"
             "if info == nil or info.id ~= %d then error('id') end",
             f.first, f.second);
    EXPECT_TRUE(__luaExecStr(lua)) << f.first;
  }
}
#endif

TEST(Lua, getSwitchInfoOutOfRange)
{
  RADIO_RESET();
  MODEL_RESET();
  char lua[128];

  for (int src : {MIXSRC_FIRST_SWITCH - 1, MIXSRC_FIRST_SWITCH - 1000,
                  MIXSRC_FIRST_SWITCH + MAX_SWITCHES,
                  MIXSRC_FIRST_SWITCH + SWSRC_COUNT - 1, -100000, 100000}) {
    snprintf(lua, sizeof(lua),
             "if getSwitchInfo(%d) ~= nil then error('not nil') end", src);
    EXPECT_TRUE(__luaExecStr(lua)) << "source " << src;
  }
}

TEST(Lua, testFloatIntegerEquality)
{
  // 0.5 is not an integer, so it must not equal 0 (regression #7587)
  // both directions asserted explicitly so the intent is obvious at a glance
  luaExecStr("if 0.5 == 0 then error('0.5 == 0') end");
  luaExecStr("if not (0.5 ~= 0) then error('0.5 ~= 0') end");
  luaExecStr("if 0.50 == 0 then error('0.50 == 0') end");
  luaExecStr("if not (0.50 ~= 0) then error('0.50 ~= 0') end");
  luaExecStr("if 0.50 == 0.0 then error('0.50 == 0.0') end");
  // ... even when the value comes from a variable, as in the reported issue
  luaExecStr("local v = 0.50; if v == 0 then error('v == 0') end");
  luaExecStr("local v = 0.50; if not (v ~= 0) then error('v ~= 0') end");
  // negative non-integral floats must not equal integers either
  luaExecStr("if -0.5 == 0 then error('-0.5 == 0') end");
  luaExecStr("if not (-0.5 ~= 0) then error('-0.5 ~= 0') end");
  luaExecStr("if not (-0.5 < 0) then error('-0.5 < 0') end");
  luaExecStr("if -0.5 > 0 then error('-0.5 > 0') end");
  // integral floats still compare equal to their integer counterpart
  luaExecStr("if 1.0 ~= 1 then error('1.0 ~= 1') end");
  luaExecStr("if 0.0 ~= 0 then error('0.0 ~= 0') end");
  luaExecStr("if -1.0 ~= -1 then error('-1.0 ~= -1') end");
  // order comparisons on the same values must remain consistent
  luaExecStr("if not (0.5 >= 0) then error('0.5 >= 0') end");
  luaExecStr("if 0.5 <= 0 then error('0.5 <= 0') end");
  luaExecStr("if not (0.5 > 0) then error('0.5 > 0') end");
  luaExecStr("if 0.5 < 0 then error('0.5 < 0') end");
  luaExecStr("if not (0.50 >= 0) then error('0.50 >= 0') end");
  luaExecStr("if 0.50 <= 0 then error('0.50 <= 0') end");
  luaExecStr("if not (1.0 >= 1) then error('1.0 >= 1') end");
  luaExecStr("if not (1.0 <= 1) then error('1.0 <= 1') end");
  luaExecStr("if not (0.0 >= 0) then error('0.0 >= 0') end");
  luaExecStr("if not (0.0 <= 0) then error('0.0 <= 0') end");
  // mixed arithmetic must still yield floats; the fix only affects equality
  luaExecStr("if math.type(0.5 + 0) ~= 'float' then error('0.5 + 0') end");
  luaExecStr("if math.type(1.0 + 1) ~= 'float' then error('1.0 + 1') end");
  luaExecStr("if math.type(1 / 2) ~= 'float' then error('1 / 2') end");
  luaExecStr("if math.type(7.5 % 2) ~= 'float' then error('7.5 % 2') end");
  luaExecStr("if math.type(0.5 * 2) ~= 'float' then error('0.5 * 2') end");
}

TEST(Lua, TouchEnabled)
{
#if defined(HARDWARE_TOUCH)
  const bool savedBacklight = boardBacklightOn;
  const CustomFunctionsContext savedGlobal = globalFunctionsContext;
  const CustomFunctionsContext savedModel = modelFunctionsContext;
  const MASK_FUNC_TYPE disableTouch = (MASK_FUNC_TYPE)1 << FUNCTION_DISABLE_TOUCH;

  globalFunctionsContext.reset();
  modelFunctionsContext.reset();
  boardBacklightOn = true;
  luaExecStr("assert(getTouchEnabled() == true)");

  modelFunctionsContext.activeFunctions = disableTouch;
  luaExecStr("assert(getTouchEnabled() == false)");
  modelFunctionsContext.reset();

  modelFunctionsContext.activeUIFunctions = disableTouch;
  luaExecStr("assert(getTouchEnabled() == false)");
  modelFunctionsContext.reset();

  globalFunctionsContext.activeFunctions = disableTouch;
  luaExecStr("assert(getTouchEnabled() == false)");
  globalFunctionsContext.reset();

  globalFunctionsContext.activeUIFunctions = disableTouch;
  luaExecStr("assert(getTouchEnabled() == false)");
  globalFunctionsContext.reset();

  boardBacklightOn = false;
  luaExecStr("assert(getTouchEnabled() == false)");

  boardBacklightOn = savedBacklight;
  globalFunctionsContext = savedGlobal;
  modelFunctionsContext = savedModel;
#else
  luaExecStr("assert(getTouchEnabled() == nil)");
#endif
}

TEST(Lua, testLegacyNames)
{
  MODEL_RESET();
#if defined(SURFACE_RADIO)
  for (uint8_t i = 0; i < 2; i ++)
    anaSetFiltered(i, -1024);
  luaExecStr("value = getValue('thr')");
  luaExecStr("if value ~= -1024 then error('th not defined in Legacy') end");
  luaExecStr("value = getValue('ste')");
  luaExecStr("if value ~= -1024 then error('st not defined in Legacy') end");
#else
  for (uint8_t i = 0; i < 4; i ++)
    anaSetFiltered(i, -1024);
  luaExecStr("value = getValue('thr')");
  luaExecStr("if value ~= -1024 then error('thr not defined in Legacy') end");
  luaExecStr("value = getValue('ail')");
  luaExecStr("if value ~= -1024 then error('ail not defined in Legacy') end");
  luaExecStr("value = getValue('rud')");
  luaExecStr("if value ~= -1024 then error('rud not defined in Legacy') end");
  luaExecStr("value = getValue('ele')");
  luaExecStr("if value ~= -1024 then error('ele not defined in Legacy') end");
#endif
}

TEST(Lua, ioSeek)
{
  const char io_seek_tst[] =
      "local file_name = \"seek-test.txt\"\n"
      "local file = io.open(file_name, \"w\")\n"
      "io.write(file, \"abcd\")\n"
      "io.close(file)\n"
      "file = io.open(file_name, \"r\")\n"
      // the file should have 4 characters
      "assert(#io.read(file, 32) == 4)\n"
      // io.seek() should return 0 if it is successful
      "assert(io.seek(file, 2) == 0)\n"
      "local r = io.read(file, 32)\n"
      // if reading from position 2,
      // we should read 2 characters,
      "assert(#r == 2)\n";

  luaExecStr(io_seek_tst);
  std::filesystem::remove(simuFatfsGetRealPath("seek-test.txt"));
}

// Adapted from the sample script in PR #7562 (minus the LVGL UI part):
// populates the same app/key/value set and exercises the full Lua API
// through the real binding layer, not just the underlying C++ store.
TEST(Lua, testUserData)
{
  MODEL_RESET();

  const char userdata_tst[] =
      "model.setUserData('App', 'K1', 'String')\n"
      "model.setUserData('App', 'K2', 12345)\n"
      "model.setUserData('App', 'K3', 12.345)\n"

      "assert(model.getUserData('App', 'K1') == 'String')\n"
      "assert(model.getUserData('App', 'K2') == 12345)\n"
      "assert(math.abs(model.getUserData('App', 'K3') - 12.345) < 0.001)\n"
      "assert(model.getUserData('Other', 'K1') == nil)\n"

      "local ud = model.getAllUserData('App')\n"
      "local n = 0\n"
      "for k in pairs(ud) do n = n + 1 end\n"
      "assert(n == 3)\n"
      "assert(ud.K1 == 'String')\n"

      "local all = model.getAllUserData()\n"
      "assert(all['App|K1'] == 'String')\n"

      "model.deleteUserData('App', 'K1')\n"
      "assert(model.getUserData('App', 'K1') == nil)\n"
      "assert(model.getUserData('App', 'K2') == 12345)\n";

  luaExecStr(userdata_tst);
}

// string.char() builds the value at runtime, sidestepping the fact that
// luaL_loadstring() (used to load this test's own Lua source) is itself
// strlen()-based and couldn't carry a literal embedded NUL through.
TEST(Lua, testUserDataEmbeddedNul)
{
  MODEL_RESET();

  const char userdata_nul_tst[] =
      "local v = string.char(65, 0, 66)\n"
      "assert(#v == 3)\n"
      "model.setUserData('App', 'NulKey', v)\n"

      "local got = model.getUserData('App', 'NulKey')\n"
      "assert(#got == 3)\n"
      "assert(got == v)\n"
      "assert(string.byte(got, 2) == 0)\n"

      "local all = model.getAllUserData('App')\n"
      "assert(#all.NulKey == 3)\n"
      "assert(all.NulKey == v)\n";

  luaExecStr(userdata_nul_tst);
}

#if defined(LED_STRIP_LENGTH)
#include "boards/generic_stm32/rgb_leds.h"

#if !defined(BLING_LED_STRIP_LENGTH)
  #define BLING_LED_STRIP_START 0
  #define BLING_LED_STRIP_LENGTH 0
#endif
#if !defined(CFS_LED_STRIP_LENGTH)
  #define CFS_LED_STRIP_LENGTH 0
#endif

TEST(Lua, RGBLedIndexes)
{
  MODEL_RESET();

#if BLING_LED_STRIP_LENGTH > 0
  luaExecStr("if not setRGBLedColor(0, 4, 5, 6) then error('bling') end");
  EXPECT_EQ(0x040506u, rgbGetLedColor(BLING_LED_STRIP_START));
#endif

#if CFS_LED_STRIP_LENGTH > 0
  // Lua index of the first LED of the first custom switch
  const std::string cfs = std::to_string(BLING_LED_STRIP_LENGTH);
  uint8_t sw = switchGetSwitchFromCustomIdx(0);
  ASSERT_LT(sw, switchGetMaxSwitches());

  g_model.setSwitchType(sw, SWITCH_NONE);
  luaExecStr(("if not setRGBLedColor(" + cfs + ", 1, 2, 3) then error('unused cfs') end").c_str());
  EXPECT_EQ(0x010203u, fsGetLedRGB(0));

  g_model.setSwitchType(sw, SWITCH_TOGGLE);
  luaExecStr(("if setRGBLedColor(" + cfs + ", 7, 8, 9) then error('used cfs') end").c_str());
  EXPECT_EQ(0x010203u, fsGetLedRGB(0));
#endif

  luaExecStr("if setRGBLedColor(LED_STRIP_LENGTH, 1, 2, 3) then error('out of range') end");
}

TEST(Lua, RGBLedInfo)
{
  luaExecStr("if FUNC_RGB_LED == nil then error('FUNC_RGB_LED') end");
  luaExecStr("info = getRGBLedInfo()");
  luaExecStr("if info.length ~= LED_STRIP_LENGTH then error('length') end");
  luaExecStr("if info.bling ~= BLING_LED_STRIP_LENGTH then error('bling') end");
  luaExecStr(
      "for name, g in pairs(info.groups) do\n"
      "  if g.first < 0 or g.count <= 0 or g.first + g.count > info.bling then\n"
      "    error(name)\n"
      "  end\n"
      "end");
#if CFS_LED_STRIP_LENGTH > 0
  luaExecStr(
      "if info.cfs.first ~= info.bling or\n"
      "   info.cfs.first + info.cfs.count ~= info.length or\n"
      "   info.cfs.perSwitch < 1 then error('cfs') end");
#else
  luaExecStr("if info.cfs ~= nil then error('cfs') end");
#endif
}
#endif

TEST(Lua, setTelemetryValueUnits)
{
  MODEL_RESET();
  allowNewSensors = true;

  luaExecStr("assert(setTelemetryValue(0x5000, 0, 1, 42, UNIT_SECONDS, 0, 'Tim'))");
  EXPECT_EQ(g_model.telemetrySensors[0].unit, UNIT_SECONDS);
  EXPECT_STREQ(STR_VTELEMUNIT[UNIT_SECONDS], "s");
  EXPECT_STREQ(STR_VTELEMUNIT[UNIT_MINUTES], "min");
  EXPECT_STREQ(STR_VTELEMUNIT[UNIT_HOURS], "h");

  // units that cannot be stored in a sensor fall back to raw
  luaExecStr("assert(setTelemetryValue(0x5100, 0, 1, 7, 60, 0, 'Bad'))");
  EXPECT_EQ(g_model.telemetrySensors[1].unit, UNIT_RAW);

  allowNewSensors = false;
}

TEST(Lua, testOutputLimitsAsSources)
{
  // plain numbers
  luaExecStr("model.setOutput(0, {min=-800, max=1200, offset=-35})");
  LimitData* ld = limitAddress(0);
  LimitNumVal v;
  v.rawValue = ld->min;
  EXPECT_FALSE(v.isSource);
  EXPECT_EQ(v.value, 200);  // stored relative to -1000
  v.rawValue = ld->max;
  EXPECT_FALSE(v.isSource);
  EXPECT_EQ(v.value, 200);  // stored relative to +1000
  v.rawValue = ld->offset;
  EXPECT_EQ(v.value, -35);
  luaExecStr(
      "o = model.getOutput(0)"
      "if o.min ~= -800 or o.max ~= 1200 or o.offset ~= -35 then error('numbers') end");

  // sources: gvar, inverted gvar and a channel
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "model.setOutput(0, {min=%d, max=%d, offset=%d})",
           LIMIT_LUA_SRC_BASE + MIXSRC_FIRST_GVAR + 2,
           -(LIMIT_LUA_SRC_BASE + MIXSRC_FIRST_GVAR + 1),
           LIMIT_LUA_SRC_BASE + MIXSRC_FIRST_CH + 3);
  luaExecStr(cmd);
  ld = limitAddress(0);
  v.rawValue = ld->min;
  EXPECT_TRUE(v.isSource);
  EXPECT_EQ(v.value, MIXSRC_FIRST_GVAR + 2);
  v.rawValue = ld->max;
  EXPECT_TRUE(v.isSource);
  EXPECT_EQ(v.value, -(MIXSRC_FIRST_GVAR + 1));
  v.rawValue = ld->offset;
  EXPECT_TRUE(v.isSource);
  EXPECT_EQ(v.value, MIXSRC_FIRST_CH + 3);

  snprintf(cmd, sizeof(cmd),
           "o = model.getOutput(0)"
           "if o.min ~= %d or o.max ~= %d or o.offset ~= %d then error('sources') end",
           LIMIT_LUA_SRC_BASE + MIXSRC_FIRST_GVAR + 2,
           -(LIMIT_LUA_SRC_BASE + MIXSRC_FIRST_GVAR + 1),
           LIMIT_LUA_SRC_BASE + MIXSRC_FIRST_CH + 3);
  luaExecStr(cmd);

  // extended range numbers must not be read as sources
  luaExecStr("model.setOutput(0, {min=-1500, max=1500, offset=1000})");
  luaExecStr(
      "o = model.getOutput(0)"
      "if o.min ~= -1500 or o.max ~= 1500 or o.offset ~= 1000 then error('extended') end");
}

// F(bits): the float with these IEEE-754 bits, built without the parser
#define LUA_F "local function F(b) return string.unpack('<f', string.pack('<I4', b)) end "

TEST(Lua, floatTonumber)
{
  // Compared with exact bit patterns, not with the parser's own output
  luaExecStr(LUA_F "assert(tonumber('3.14') == F(0x4048F5C3))");
  luaExecStr("assert(tonumber('0') == 0)");
  luaExecStr("assert(tonumber('-1.5') == -3 / 2)");

  // Scientific notation
  luaExecStr(LUA_F "assert(tonumber('1e10') == F(0x501502F9))");
  luaExecStr(LUA_F "assert(tonumber('1.5e-3') == F(0x3AC49BA6))");
  luaExecStr(LUA_F "assert(tonumber('-2.5e8') == F(0xCD6E6B28))");
  luaExecStr(LUA_F "assert(tonumber('0.0000001') == F(0x33D6BF95))");
  luaExecStr("assert(tonumber('9999999') == 9999999)");

  // Float boundaries
  luaExecStr(LUA_F "assert(tonumber('3.4028235e38') == F(0x7F7FFFFF))");  // FLT_MAX
  luaExecStr(LUA_F "assert(tonumber('1.17549435e-38') == F(0x00800000))");  // FLT_MIN

  // Invalid input
  luaExecStr("assert(tonumber('not_a_number') == nil)");
  luaExecStr("assert(tonumber('') == nil)");

  // Hex integer (parsed as integer, not float)
  luaExecStr("assert(tonumber('0xff') == 255)");

  // Hex float
  luaExecStr("assert(tonumber('0x1.8p0') == 3 / 2)");
  luaExecStr("assert(tonumber('0x1p10') == 2.0 ^ 10)");
  luaExecStr("assert(tonumber('0x1.0p-1') == 1 / 2)");
}

TEST(Lua, floatTostring)
{
  // Basic formatting (uses "%.7g")
  luaExecStr(LUA_F "assert(tostring(F(0x4048F5C3)) == '3.14')");
  luaExecStr("assert(tostring(-3 / 2) == '-1.5')");
  luaExecStr(LUA_F "assert(tostring(F(0x38D1B717)) == '0.0001')");
  luaExecStr("assert(tostring(100000.0) == '100000.0', 'got: ' .. tostring(100000.0))");

  // 7 significant digits
  luaExecStr("assert(tostring(1/3) == '0.3333333', 'got: ' .. tostring(1/3))");

  // Scientific notation threshold
  luaExecStr("assert(tostring(1e10) == '1e+10', 'got: ' .. tostring(1e10))");
  luaExecStr("assert(tostring(1000000.0) == '1000000.0', 'got: ' .. tostring(1000000.0))");

  // Special values
  luaExecStr("assert(tostring(1/0) == 'inf')");
  luaExecStr("assert(tostring(-1/0) == '-inf')");
  luaExecStr("assert(tostring(0/0) == '-nan' or tostring(0/0) == 'nan')");

  // Integer vs float distinction (Lua 5.3)
  luaExecStr("assert(tostring(0) == '0')");        // integer 0
  luaExecStr("assert(tostring(0.0) == '0.0')");    // float 0.0
  luaExecStr("assert(tostring(42) == '42')");       // integer
  luaExecStr("assert(tostring(42.0) == '42.0')");   // float
}

TEST(Lua, floatStringFormat)
{
  // Fixed decimal with precision
  luaExecStr(LUA_F "assert(string.format('%.3f', F(0x40490FD0)) == '3.142')");
  luaExecStr("assert(string.format('%.1f', 2.5) == '2.5')");
  luaExecStr("assert(string.format('%.0f', 3.75) == '4')");

  // Scientific notation
  luaExecStr("assert(string.format('%e', 12345.0) == '1.234500e+04')");

  // General format (%g switches at 1e6)
  luaExecStr("assert(string.format('%g', 100000.0) == '100000')");
  luaExecStr("assert(string.format('%g', 1000000.0) == '1e+06')");

  // More digits than float precision: the exact value of the float
  luaExecStr("assert(string.format('%.10g', 1/3) == '0.3333333433', 'got: ' .. string.format('%.10g', 1/3))");

  // Integer formatting
  luaExecStr("assert(string.format('%d', 42) == '42')");
  luaExecStr("assert(string.format('%d', -7) == '-7')");
}

TEST(Lua, floatHexFormat)
{
  // C layout, as standard Lua 5.3 prints on a PC
  luaExecStr("assert(string.format('%a', 1.5) == '0x1.8p+0', 'got: ' .. string.format('%a', 1.5))");
  luaExecStr(LUA_F "assert(string.format('%a', F(0x3DCCCCCD)) == '0x1.99999ap-4')");
  luaExecStr(LUA_F "assert(string.format('%a', F(1)) == '0x1p-149')");  // subnormal
  luaExecStr("assert(string.format('%a', 0.0) == '0x0p+0')");
  luaExecStr("assert(string.format('%a', -0.0) == '-0x0p+0')");
  luaExecStr("assert(string.format('%A', 1.5) == '0X1.8P+0')");

  // Precision rounds (ties to even); a carry can make the leading digit 2
  luaExecStr(LUA_F "assert(string.format('%.2a', F(0x3DCCCCCD)) == '0x1.9ap-4')");
  luaExecStr("assert(string.format('%.0a', 1.5) == '0x2p+0')");
  luaExecStr("assert(string.format('%.0a', 1.25) == '0x1p+0')");
  luaExecStr("assert(string.format('%.8a', 1.5) == '0x1.80000000p+0')");
  luaExecStr("assert(string.format('%#.0a', 1.0) == '0x1.p+0')");

  // Width and flags
  luaExecStr("assert(string.format('%12a', 1.5) == '    0x1.8p+0')");
  luaExecStr("assert(string.format('%-12a|', 1.5) == '0x1.8p+0    |')");
  luaExecStr("assert(string.format('%012a', -1.5) == '-0x0001.8p+0')");
  luaExecStr("assert(string.format('%+a', 1.5) == '+0x1.8p+0')");
  luaExecStr("assert(string.format('% a', 1.5) == ' 0x1.8p+0')");

  // inf and nan as C prints them
  luaExecStr("assert(string.format('%a', 1/0) == 'inf')");
  luaExecStr("assert(string.format('%+6A', -1/0) == '  -INF')");
  luaExecStr("assert(string.format('%06a', 1/0) == '   inf')");

  // %q of floats, and round trip
  luaExecStr("assert(string.format('%q', 1.5) == '0x1.8p+0')");
  luaExecStr(LUA_F "assert(tonumber(string.format('%a', F(0x3DCCCCCD))) == F(0x3DCCCCCD))");
}

TEST(Lua, floatArithmeticRoundTrip)
{
  // tostring keeps 7 digits, so these come back exactly
  luaExecStr(LUA_F "assert(tonumber(tostring(F(0x4048F5C3))) == F(0x4048F5C3))");
  luaExecStr(LUA_F "assert(tonumber(tostring(F(0x501502F9))) == F(0x501502F9))");
  luaExecStr(LUA_F "assert(tonumber(tostring(F(0x38D1B717))) == F(0x38D1B717))");
  luaExecStr(LUA_F "assert(tonumber(tostring(F(0xC479E000))) == F(0xC479E000))");

  // Arithmetic
  luaExecStr("assert(2.0 ^ 24 + 1 == 2.0 ^ 24)");  // float precision: 1 is lost
  luaExecStr("assert(math.floor(3.75) == 3)");
  luaExecStr("assert(math.ceil(3.25) == 4)");
  luaExecStr("assert(math.abs(-5.5) == 5.5)");
}

TEST(Lua, floatMathRandom)
{
  // math.random() returns float in [0, 1), using all 24 bits
  luaExecStr(
    "local low = false\n"
    "for i = 1, 1000 do\n"
    "  local r = math.random()\n"
    "  assert(r >= 0.0 and r < 1.0, 'random() out of range: ' .. tostring(r))\n"
    "  local s = r * 2^23\n"
    "  if s ~= math.floor(s) then low = true end\n"
    "end\n"
    "assert(low, 'random() never uses its 24th bit')"
  );

  // math.random(n) returns integer in [1, n]
  luaExecStr(
    "for i = 1, 100 do\n"
    "  local r = math.random(10)\n"
    "  assert(r >= 1 and r <= 10 and math.type(r) == 'integer', 'random(10) bad: ' .. tostring(r))\n"
    "end"
  );

  // math.random(low, up) returns integer in [low, up], all values reached
  luaExecStr(
    "local seen = {}\n"
    "for i = 1, 1000 do\n"
    "  local r = math.random(5, 15)\n"
    "  assert(r >= 5 and r <= 15, 'random(5,15) bad: ' .. tostring(r))\n"
    "  seen[r] = true\n"
    "end\n"
    "for v = 5, 15 do assert(seen[v], 'never got ' .. v) end"
  );

  // Large ranges return (they used to loop forever)
  luaExecStr(
    "for i = 1, 100 do\n"
    "  local r = math.random(0, 0x7fffffff)\n"
    "  assert(r >= 0)\n"
    "  r = math.random(-0x7fffffff, 0)\n"
    "  assert(r <= 0)\n"
    "end"
  );

  // Floats are floored (LUA_FLOORN2I)
  luaExecStr("assert(math.random(1.5) == 1)");

  // Degenerate case
  luaExecStr("assert(math.random(1, 1) == 1)");
  luaExecStr("assert(math.random(42, 42) == 42)");

  // Same seed, same sequence
  luaExecStr(
    "math.randomseed(42)\n"
    "local a, b = math.random(1, 1000000), math.random()\n"
    "math.randomseed(42)\n"
    "assert(math.random(1, 1000000) == a and math.random() == b)\n"
    "math.randomseed(43)\n"
    "assert(math.random(1, 1000000) ~= a or math.random() ~= b, 'seed ignored')"
  );
}

#if defined(INTERNAL_GPS)
TEST(Lua, txGpsDegrees)
{
  // Nearest float to micro-degrees / 1e6 (bits computed with exact
  // arithmetic). The first three were 1 ulp off with float multiplication.
  const struct {
    int32_t lat, lon;
    const char* check;
  } cases[] = {
    {125589001, 73534732, "g.lat == F(0x42FB2D92) and g.lon == F(0x429311C8)"},
    {169821315, -180000000, "g.lat == F(0x4329D242) and g.lon == F(0xC3340000)"},
    {45123457, -122419416, "g.lat == F(0x42347E6C) and g.lon == F(0xC2F4D6BE)"},
    {90000000, 1, "g.lat == F(0x42B40000) and g.lon == F(0x358637BD)"},
    {-127845, 0, "g.lat == F(0xBE02E9CD) and g.lon == 0"},
  };
  gpsdata_t saved = gpsData;
  for (const auto& c : cases) {
    gpsData.latitude = c.lat;
    gpsData.longitude = c.lon;
    std::string code = std::string(LUA_F "local g = getTxGPS() assert(") +
                       c.check + ", string.format('%a %a', g.lat, g.lon))";
    luaExecStr(code.c_str());
  }
  gpsData = saved;
}
#endif

TEST(Lua, floatStringPack)
{
  // Pack/unpack float ('f' = 4-byte IEEE 754 single)
  luaExecStr("assert(string.unpack('f', string.pack('f', 3.14)) == 3.14)");
  luaExecStr("assert(string.unpack('f', string.pack('f', -1.5)) == -1.5)");
  luaExecStr("assert(string.unpack('f', string.pack('f', 0.0)) == 0.0)");

  // Pack as 'n' (lua_Number = float in LUA_32BITS)
  luaExecStr("assert(string.unpack('n', string.pack('n', 3.14)) == 3.14)");

  // Pack as 'd' (8-byte double) - should still work, precision may differ
  luaExecStr(
    "local packed = string.pack('d', 3.14)\n"
    "assert(#packed == 8, 'double pack should be 8 bytes')\n"
    "local unpacked = string.unpack('d', packed)\n"
    "assert(unpacked == 3.14, 'double round-trip failed')\n"
  );
}

TEST(Lua, testModelSensors)
{
  MODEL_RESET();
  allowNewSensors = false;

  // A new custom sensor in an empty slot, without discovery
  luaExecStr(
      "assert(model.getSensor(0).name == '')\n"
      "assert(model.setSensor(0, {name = 'Tmp1', id = 0x1311, subId = 2,"
      " instance = 5, unit = UNIT_CELSIUS, prec = 1}))\n"
      "local s = model.getSensor(0)\n"
      "assert(s.type == 0 and s.name == 'Tmp1' and s.id == 0x1311)\n"
      "assert(s.subId == 2 and s.instance == 5)\n"
      "assert(s.unit == UNIT_CELSIUS and s.prec == 1)\n");
  EXPECT_TRUE(g_model.telemetrySensors[0].logs);

  // Missing parameters remain unchanged; an empty name keeps the old one
  luaExecStr(
      "assert(model.setSensor(0, {instance = 6, name = ''}))\n"
      "local s = model.getSensor(0)\n"
      "assert(s.name == 'Tmp1' and s.id == 0x1311 and s.instance == 6)\n");

  // A new sensor without a name is named after its ID
  luaExecStr(
      "assert(model.setSensor(1, {id = 0x1310, instance = 3}))\n"
      "assert(model.getSensor(1).name == '1310')\n");

  // Out of range, a calculated sensor: not set
  g_model.telemetrySensors[2].type = TELEM_TYPE_CALCULATED;
  memcpy(g_model.telemetrySensors[2].label, "Calc", TELEM_LABEL_LEN);
  luaExecStr(
      "assert(not model.setSensor(60000, {id = 1}))\n"
      "assert(not model.setSensor(2, {id = 1}))\n");

  luaExecStr(
      "model.deleteSensor(0)\n"
      "assert(model.getSensor(0).name == '')\n"
      "assert(model.getSensor(1).name == '1310')\n");
}

#if defined(PXX2) && defined(HARDWARE_INTERNAL_MODULE) && defined(HARDWARE_EXTERNAL_MODULE)
#include "pulses/pxx2.h"
#include "pulses/pxx2_transport.h"

extern tmr10ms_t luaCycleStart;

class LuaAccessFrame : public testing::Test
{
 protected:
  void SetUp() override
  {
    memclear(g_model.moduleData, sizeof(g_model.moduleData));
    g_model.moduleData[EXTERNAL_MODULE].type = MODULE_TYPE_R9M_PXX2;
    for (auto& state : moduleState) state.mode = MODULE_MODE_NORMAL;
    luaCycleStart = g_tmr10ms;
  }

  void TearDown() override
  {
    // leave the output slot free for the next test
    advanceTime(100);
    EXPECT_TRUE(pxx2LuaFrameAvailable());
    memclear(g_model.moduleData, sizeof(g_model.moduleData));
  }

  // the Lua hook yields scripts running for longer than a Lua task period
  void advanceTime(tmr10ms_t ticks)
  {
    g_tmr10ms += ticks;
    luaCycleStart = g_tmr10ms;
  }

  // returns TYPE_ID of the frame the mixer sends next
  uint8_t sendFrame(uint8_t module, uint8_t* buffer, uint32_t* size = nullptr)
  {
    int16_t channels[MAX_OUTPUT_CHANNELS] = {};
    Pxx2Pulses pxx2(buffer);
    pxx2.setupFrame(module, channels, 16);
    if (size) *size = pxx2.getSize();
    return buffer[3];
  }
};

TEST_F(LuaAccessFrame, push)
{
  luaExecStr("assert(accessFramePush() == true)");
  luaExecStr("assert(accessFramePush(1, 0x01, 0x05, {0x40, 0x12, 0x00, 0x01}) == true)");

  // one frame at a time
  luaExecStr("assert(accessFramePush() == false)");
  luaExecStr("assert(accessFramePush(1, 0x01, 0x05, {}) == false)");

  uint8_t buffer[PXX2_FRAME_MAXLENGTH];
  uint32_t size = 0;
  sendFrame(EXTERNAL_MODULE, buffer, &size);

  // HEAD, LEN, TYPE_C, TYPE_ID, payload, CRC
  const uint8_t expected[] = {0x7E, 0x06, 0x01, 0x05, 0x40,
                              0x12, 0x00, 0x01, 0xFF, 0xA6};
  ASSERT_EQ(sizeof(expected), size);
  EXPECT_EQ(0, memcmp(expected, buffer, size));

  luaExecStr("assert(accessFramePush() == true)");

  // back to channels
  EXPECT_EQ(PXX2_TYPE_ID_CHANNELS, sendFrame(EXTERNAL_MODULE, buffer));
}

TEST_F(LuaAccessFrame, pushInvalid)
{
  // not a PXX2 module, or no module at all
  luaExecStr("assert(accessFramePush(0, 0x01, 0x05, {0}) == nil)");
  luaExecStr("assert(accessFramePush(2, 0x01, 0x05, {0}) == nil)");
  luaExecStr("assert(accessFramePush(-1, 0x01, 0x05, {0}) == nil)");

  // payload too long
  char cmd[128];
  snprintf(cmd, sizeof(cmd),
           "local p = {} for i = 1, %d do p[i] = i end "
           "assert(accessFramePush(1, 0x01, 0x05, p) == false)",
           PXX2_LUA_FRAME_MAX_PAYLOAD + 1);
  luaExecStr(cmd);
  luaExecStr("assert(accessFramePush() == true)");
}

TEST_F(LuaAccessFrame, otherModule)
{
  g_model.moduleData[INTERNAL_MODULE].type = MODULE_TYPE_ISRM_PXX2;
  luaExecStr("assert(accessFramePush(1, 0x01, 0x06, {0x00}) == true)");

  uint8_t buffer[PXX2_FRAME_MAXLENGTH];
  EXPECT_EQ(PXX2_TYPE_ID_CHANNELS, sendFrame(INTERNAL_MODULE, buffer));
  luaExecStr("assert(accessFramePush() == false)");

  EXPECT_EQ(PXX2_TYPE_ID_HW_INFO, sendFrame(EXTERNAL_MODULE, buffer));
  luaExecStr("assert(accessFramePush() == true)");
}

TEST_F(LuaAccessFrame, waitForNormalMode)
{
  luaExecStr("assert(accessFramePush(1, 0x01, 0x06, {0x00}) == true)");

  uint8_t buffer[PXX2_FRAME_MAXLENGTH];
  moduleState[EXTERNAL_MODULE].mode = MODULE_MODE_REGISTER;
  EXPECT_EQ(PXX2_TYPE_ID_REGISTER, sendFrame(EXTERNAL_MODULE, buffer));

  moduleState[EXTERNAL_MODULE].mode = MODULE_MODE_NORMAL;
  EXPECT_EQ(PXX2_TYPE_ID_HW_INFO, sendFrame(EXTERNAL_MODULE, buffer));
}

TEST_F(LuaAccessFrame, timeout)
{
  luaExecStr("assert(accessFramePush(1, 0x01, 0x06, {0x00}) == true)");

  advanceTime(99);
  luaExecStr("assert(accessFramePush() == false)");

  advanceTime(1);
  luaExecStr("assert(accessFramePush() == true)");

  // dropped
  uint8_t buffer[PXX2_FRAME_MAXLENGTH];
  EXPECT_EQ(PXX2_TYPE_ID_CHANNELS, sendFrame(EXTERNAL_MODULE, buffer));
}

TEST_F(LuaAccessFrame, pop)
{
  // the first call creates the queue
  luaExecStr("while accessFramePop() do end");
  luaExecStr("assert(accessFramePop() == nil)");

  // LEN, TYPE_C, TYPE_ID, payload: receiver settings
  const uint8_t settings[] = {0x06, 0x01, 0x05, 0x00, 0x12, 0x00, 0x01};
  processPXX2Frame(EXTERNAL_MODULE, settings, nullptr, nullptr);

  // telemetry is not queued
  const uint8_t telemetry[] = {0x0B, 0x01, 0xFE, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  processPXX2Frame(EXTERNAL_MODULE, telemetry, nullptr, nullptr);

  // spectrum analyser: another TYPE_C
  const uint8_t spectrum[] = {0x03, 0x02, 0x02, 0x7F};
  processPXX2Frame(INTERNAL_MODULE, spectrum, nullptr, nullptr);

  luaExecStr(
      "local m, c, t, p = accessFramePop() "
      "assert(m == 1 and c == 0x01 and t == 0x05) "
      "assert(#p == 4 and p[1] == 0x00 and p[2] == 0x12 and p[3] == 0x00 and p[4] == 0x01)");
  luaExecStr(
      "local m, c, t, p = accessFramePop() "
      "assert(m == 0 and c == 0x02 and t == 0x02 and #p == 1 and p[1] == 0x7F)");
  luaExecStr("assert(accessFramePop() == nil)");
}

TEST_F(LuaAccessFrame, popFull)
{
  luaExecStr("while accessFramePop() do end");

  // the queue keeps whole frames only
  const uint8_t settings[] = {0x06, 0x01, 0x05, 0x00, 0x12, 0x00, 0x01};
  for (int i = 0; i < 100; i++) {
    processPXX2Frame(EXTERNAL_MODULE, settings, nullptr, nullptr);
  }
  // [length] [module] + frame without its LEN byte
  const int pushed =
      (LUA_TELEMETRY_INPUT_FIFO_SIZE - 1) / (sizeof(settings) + 1);

  char cmd[160];
  snprintf(cmd, sizeof(cmd),
           "local n = 0 "
           "while true do local m, c, t, p = accessFramePop() "
           "if not m then break end assert(#p == 4) n = n + 1 end "
           "assert(n == %d, n)",
           pushed);
  luaExecStr(cmd);
}
#endif

#endif   // #if defined(LUA)
