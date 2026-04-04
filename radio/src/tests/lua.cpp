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

TEST(Lua, floatTonumber)
{
  // Basic decimal parsing
  luaExecStr("assert(tonumber('3.14') == 3.14)");
  luaExecStr("assert(tonumber('0') == 0)");
  luaExecStr("assert(tonumber('-1.5') == -1.5)");

  // Scientific notation
  luaExecStr("assert(tonumber('1e10') == 1e10)");
  luaExecStr("assert(tonumber('1.5e-3') == 1.5e-3)");
  luaExecStr("assert(tonumber('-2.5e8') == -2.5e8)");

  // Edge cases: 7 significant digits (float precision boundary)
  luaExecStr("assert(tonumber('0.0000001') == 0.0000001)");
  luaExecStr("assert(tonumber('9999999') == 9999999)");

  // Float boundaries
  luaExecStr("assert(tonumber('3.4028235e38') == 3.4028235e38)");  // FLT_MAX
  luaExecStr("assert(tonumber('1.17549435e-38') == 1.17549435e-38)");  // FLT_MIN

  // Invalid input
  luaExecStr("assert(tonumber('not_a_number') == nil)");
  luaExecStr("assert(tonumber('') == nil)");

  // Hex integer (parsed as integer, not float)
  luaExecStr("assert(tonumber('0xff') == 255)");

  // Hex float
  luaExecStr("assert(tonumber('0x1.8p0') == 1.5)");
  luaExecStr("assert(tonumber('0x1p10') == 1024.0)");
  luaExecStr("assert(tonumber('0x1.0p-1') == 0.5)");
}

TEST(Lua, floatTostring)
{
  // Basic formatting (uses "%.7g")
  luaExecStr("assert(tostring(3.14) == '3.14')");
  luaExecStr("assert(tostring(-1.5) == '-1.5')");
  luaExecStr("assert(tostring(0.0001) == '0.0001')");
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
  luaExecStr("assert(string.format('%.3f', 3.14159) == '3.142')");
  luaExecStr("assert(string.format('%.1f', 2.5) == '2.5')");
  luaExecStr("assert(string.format('%.0f', 3.7) == '4')");

  // Scientific notation
  luaExecStr("assert(string.format('%e', 12345.0) == '1.234500e+04')");

  // General format (%g switches at 1e6)
  luaExecStr("assert(string.format('%g', 100000.0) == '100000')");
  luaExecStr("assert(string.format('%g', 1000000.0) == '1e+06')");

  // High precision (more digits than float can deliver — exposes float mantissa)
  luaExecStr("assert(string.format('%.10g', 1/3) == '0.3333333433', 'got: ' .. string.format('%.10g', 1/3))");

  // Hex float output (%a) - verify round-trip
  luaExecStr("local s = string.format('%a', 1.5); assert(string.find(s, '0x') == 1, 'got: ' .. s)");
  luaExecStr("assert(tonumber(string.format('%a', 1.5)) == 1.5)");
  luaExecStr("assert(tonumber(string.format('%a', 0.1)) == 0.1)");

  // Integer formatting of float
  luaExecStr("assert(string.format('%d', 42) == '42')");
  luaExecStr("assert(string.format('%d', -7) == '-7')");
}

TEST(Lua, floatArithmeticRoundTrip)
{
  // Values that round-trip through tostring/tonumber exactly
  luaExecStr("assert(tonumber(tostring(3.14)) == 3.14)");
  luaExecStr("assert(tonumber(tostring(1e10)) == 1e10)");
  luaExecStr("assert(tonumber(tostring(0.0001)) == 0.0001)");
  luaExecStr("assert(tonumber(tostring(-999.5)) == -999.5)");

  // Arithmetic consistency
  luaExecStr("assert(0.1 + 0.2 == 0.1 + 0.2)");  // same float result both sides
  luaExecStr("assert(1e10 + 1 == 1e10)");  // float precision: 1 is lost
  luaExecStr("assert(math.floor(3.7) == 3)");
  luaExecStr("assert(math.ceil(3.2) == 4)");
  luaExecStr("assert(math.abs(-5.5) == 5.5)");
}

TEST(Lua, floatMathRandom)
{
  // math.random() returns float in [0, 1)
  luaExecStr(
    "for i = 1, 100 do\n"
    "  local r = math.random()\n"
    "  assert(r >= 0.0 and r < 1.0, 'random() out of range: ' .. tostring(r))\n"
    "end"
  );

  // math.random(n) returns integer in [1, n]
  luaExecStr(
    "for i = 1, 100 do\n"
    "  local r = math.random(10)\n"
    "  assert(r >= 1 and r <= 10 and r == math.floor(r), 'random(10) bad: ' .. tostring(r))\n"
    "end"
  );

  // math.random(low, up) returns integer in [low, up]
  luaExecStr(
    "for i = 1, 100 do\n"
    "  local r = math.random(5, 15)\n"
    "  assert(r >= 5 and r <= 15 and r == math.floor(r), 'random(5,15) bad: ' .. tostring(r))\n"
    "end"
  );

  // Degenerate case
  luaExecStr("assert(math.random(1, 1) == 1)");
  luaExecStr("assert(math.random(42, 42) == 42)");
}

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

#endif   // #if defined(LUA)
