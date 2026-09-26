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

TEST(Lua, getSwitchInfo)
{
  RADIO_RESET();
  MODEL_RESET();
  char name[32];
  char lua[256];

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

#endif   // #if defined(LUA)
