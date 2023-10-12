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

#define SWAP_DEFINED
#include "edgetx.h"

#define MIXSRC_THR     (MIXSRC_FIRST_STICK + inputMappingGetThrottle())
#define MIXSRC_TRIMTHR (MIXSRC_FIRST_TRIM + inputMappingGetThrottle())

::testing::AssertionResult __luaExecStr(const char * str)
{
  extern lua_State * lsScripts;
  if (!lsScripts) luaInit();
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
  EXPECT_EQ(3, (int)g_model.expoData[0].chn);
  EXPECT_STRNEQ("test1", g_model.expoData[0].name);
  EXPECT_EQ(MIXSRC_THR, g_model.expoData[0].srcRaw);
  EXPECT_EQ(56, g_model.expoData[0].weight);
  EXPECT_EQ(3, g_model.expoData[0].offset);
  EXPECT_EQ(2, g_model.expoData[0].swtch);

  // add another one before existing line on Input4
#if defined(SURFACE_RADIO)
  luaExecStr("model.insertInput(3, 0, {name='test2', source=MIXSRC_ST, weight=-56})");
#else
  luaExecStr("model.insertInput(3, 0, {name='test2', source=MIXSRC_Rud, weight=-56})");
#endif
  EXPECT_EQ(3, (int)g_model.expoData[0].chn);
  EXPECT_STRNEQ("test2", g_model.expoData[0].name);
  EXPECT_EQ(MIXSRC_FIRST_STICK, g_model.expoData[0].srcRaw);
  SourceNumVal v;
  v.rawValue = g_model.expoData[0].weight;
  EXPECT_EQ(-56, v.value);
  EXPECT_EQ(0, g_model.expoData[0].offset);
  EXPECT_EQ(0, g_model.expoData[0].swtch);

  EXPECT_EQ(3, (int)g_model.expoData[1].chn);
  EXPECT_STRNEQ("test1", g_model.expoData[1].name);
  EXPECT_EQ(MIXSRC_THR, g_model.expoData[1].srcRaw);
  EXPECT_EQ(56, g_model.expoData[1].weight);
  EXPECT_EQ(3, g_model.expoData[1].offset);
  EXPECT_EQ(2, g_model.expoData[1].swtch);


  // add another line after existing lines on Input4
#if defined(SURFACE_RADIO)
  luaExecStr("model.insertInput(3, model.getInputsCount(3), {name='test3', source=MIXSRC_TH, weight=100})");
#else
  luaExecStr("model.insertInput(3, model.getInputsCount(3), {name='test3', source=MIXSRC_Ail, weight=100})");
#endif
  EXPECT_EQ(3, (int)g_model.expoData[0].chn);
  EXPECT_STRNEQ("test2", g_model.expoData[0].name);
  EXPECT_EQ(MIXSRC_FIRST_STICK, g_model.expoData[0].srcRaw);
  v.rawValue = g_model.expoData[0].weight;
  EXPECT_EQ(-56, v.value);
  EXPECT_EQ(0, g_model.expoData[0].offset);
  EXPECT_EQ(0, g_model.expoData[0].swtch);

  EXPECT_EQ(3, (int)g_model.expoData[1].chn);
  EXPECT_STRNEQ("test1", g_model.expoData[1].name);
  EXPECT_EQ(MIXSRC_THR, g_model.expoData[1].srcRaw);
  EXPECT_EQ(56, g_model.expoData[1].weight);
  EXPECT_EQ(3, g_model.expoData[1].offset);
  EXPECT_EQ(2, g_model.expoData[1].swtch);

  EXPECT_EQ(3, (int)g_model.expoData[2].chn);
  EXPECT_STRNEQ("test3", g_model.expoData[2].name);
#if defined(SURFACE_RADIO)
  EXPECT_EQ(MIXSRC_THR, g_model.expoData[2].srcRaw);
#else
  EXPECT_EQ(MIXSRC_LAST_STICK, g_model.expoData[2].srcRaw);
#endif
  EXPECT_EQ(100, g_model.expoData[2].weight);
  EXPECT_EQ(0, g_model.expoData[2].offset);
  EXPECT_EQ(0, g_model.expoData[2].swtch);

  // verify number of lines for Input4
  luaExecStr("noInputs = model.getInputsCount(3)");
  luaExecStr("if noInputs ~= 3 then error('getInputsCount()') end");
}

TEST(Lua, Switches)
{
  luaExecStr("if MIXSRC_SA == nil then error('failed') end");
  luaExecStr("if MIXSRC_SB == nil then error('failed') end");
  luaExecStr("if getSwitchIndex('Rud-') == nil then error('failed') end");
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

#endif   // #if defined(LUA)
