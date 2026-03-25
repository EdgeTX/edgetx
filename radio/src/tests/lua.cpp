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

// model.insertMix / getMixesCount / getMix / deleteMix round-trip.
TEST(Lua, modelMixes)
{
  MODEL_RESET();

  // Channel 0 should start empty.
  luaExecStr("assert(model.getMixesCount(0) == 0, 'expected 0 mixes')");

  // Insert one mix on channel 0: source MAX, weight 75.
  luaExecStr("model.insertMix(0, 0, {source=MIXSRC_MAX, weight=75, name='m1'})");
  luaExecStr("assert(model.getMixesCount(0) == 1, 'expected 1 mix after insert')");

  // Read it back and verify the key fields (use a global so it persists
  // across separate luaExecStr chunks).
  luaExecStr("mix0 = model.getMix(0, 0)");
  luaExecStr("assert(mix0 ~= nil, 'getMix returned nil')");
  luaExecStr("assert(mix0.source == MIXSRC_MAX, 'source mismatch')");
  luaExecStr("assert(mix0.weight == 75, 'weight mismatch: '..tostring(mix0.weight))");

  // Delete and confirm it is gone.
  luaExecStr("model.deleteMix(0, 0)");
  luaExecStr("assert(model.getMixesCount(0) == 0, 'expected 0 mixes after delete')");
}

// model.getOutput / setOutput — name, min and max limits round-trip.
TEST(Lua, modelOutputLimits)
{
  MODEL_RESET();

  // Set output 0: name "CH1", min -80%, max +80%.
  luaExecStr("model.setOutput(0, {name='CH1', min=-800, max=800})");

  luaExecStr("out0 = model.getOutput(0)");
  luaExecStr("assert(out0 ~= nil, 'getOutput returned nil')");
  luaExecStr("assert(out0.name == 'CH1', 'name mismatch')");
  luaExecStr("assert(out0.min == -800, 'min mismatch: '..tostring(out0.min))");
  luaExecStr("assert(out0.max ==  800, 'max mismatch: '..tostring(out0.max))");
}

#if defined(GVARS)
// model.getGlobalVariable / setGlobalVariable — write then read back a value.
TEST(Lua, modelGlobalVariables)
{
  MODEL_RESET();

  // Set GV1 (index 0) in flight mode 0 to 42, then read it back.
  luaExecStr("model.setGlobalVariable(0, 0, 42)");
  luaExecStr("gv0 = model.getGlobalVariable(0, 0)");
  luaExecStr("assert(gv0 == 42, 'GVar mismatch: '..tostring(gv0))");

  // Negative value round-trip.
  luaExecStr("model.setGlobalVariable(0, 0, -17)");
  luaExecStr("gv1 = model.getGlobalVariable(0, 0)");
  luaExecStr("assert(gv1 == -17, 'GVar negative mismatch: '..tostring(gv1))");

  // Out-of-range value must be clamped / rejected — API silently ignores it,
  // so the stored value must not change.
  luaExecStr("model.setGlobalVariable(0, 0, 5)");
  luaExecStr("model.setGlobalVariable(0, 0, 9999)");  // out of range
  luaExecStr("gv2 = model.getGlobalVariable(0, 0)");
  luaExecStr("assert(gv2 == 5, 'out-of-range write should be ignored: '..tostring(gv2))");
}
#endif  // defined(GVARS)

// model.getLogicalSwitch / setLogicalSwitch — write fields then read back.
TEST(Lua, modelLogicalSwitch)
{
  MODEL_RESET();

  // Configure LS1 (index 0): func=LS_FUNC_VPOS (2), v1=MIXSRC_FIRST_STICK (1),
  // v2=0 (threshold), delay=5 (0.5 s).
  luaExecStr("model.setLogicalSwitch(0, {func=2, v1=1, v2=0, delay=5})");

  luaExecStr("ls0 = model.getLogicalSwitch(0)");
  luaExecStr("assert(ls0 ~= nil, 'getLogicalSwitch returned nil')");
  luaExecStr("assert(ls0.func == 2, 'func mismatch: '..tostring(ls0.func))");
  luaExecStr("assert(ls0.v1 == 1, 'v1 mismatch: '..tostring(ls0.v1))");
  luaExecStr("assert(ls0.v2 == 0, 'v2 mismatch: '..tostring(ls0.v2))");
  luaExecStr("assert(ls0.delay == 5, 'delay mismatch: '..tostring(ls0.delay))");

  // Out-of-range index must return nil.
  luaExecStr("assert(model.getLogicalSwitch(999) == nil, 'expected nil for OOB index')");
}

// model.getCurve / setCurve — standard 5-point curve round-trip.
TEST(Lua, modelCurve)
{
  MODEL_RESET();
  // loadCurves() must be called after MODEL_RESET() to initialise curveEnd[]
  // pointers; moveCurve() (called by setCurve) dereferences them.
  loadCurves();

  // Set curve 0: standard type, 5 points, y={-100,-50,0,50,100}.
  luaExecStr("rc = model.setCurve(0, {type=0, smooth=false, y={-100,-50,0,50,100}})");
  luaExecStr("assert(rc == 0, 'setCurve failed: '..tostring(rc))");

  luaExecStr("cv = model.getCurve(0)");
  luaExecStr("assert(cv ~= nil, 'getCurve returned nil')");
  luaExecStr("assert(cv.points == 5, 'points mismatch: '..tostring(cv.points))");
  luaExecStr("assert(cv.type == 0, 'type mismatch: '..tostring(cv.type))");
  luaExecStr("assert(cv.y[1] == -100, 'y[1] mismatch: '..tostring(cv.y[1]))");
  luaExecStr("assert(cv.y[3] ==    0, 'y[3] mismatch: '..tostring(cv.y[3]))");
  luaExecStr("assert(cv.y[5] ==  100, 'y[5] mismatch: '..tostring(cv.y[5]))");

  // Out-of-range curve index must return nil.
  luaExecStr("assert(model.getCurve(999) == nil, 'expected nil for OOB index')");
}

// model.getCurve / setCurve — 4-point custom curve with explicit x values.
TEST(Lua, modelCurveCustom)
{
  MODEL_RESET();
  loadCurves();

  // 4-point custom curve with monotonic x values.
  luaExecStr("rc = model.setCurve(1, {type=1, smooth=true, x={-100,-34,77,100}, y={-70,20,-89,-100}})");
  luaExecStr("assert(rc == 0, 'setCurve custom failed: '..tostring(rc))");

  luaExecStr("cv2 = model.getCurve(1)");
  luaExecStr("assert(cv2 ~= nil, 'getCurve custom returned nil')");
  luaExecStr("assert(cv2.type == 1, 'custom type mismatch')");
  luaExecStr("assert(cv2.smooth == true, 'smooth mismatch')");
  luaExecStr("assert(cv2.points == 4, 'custom points mismatch: '..tostring(cv2.points))");
  // x table must be present for custom curves and first/last fixed at -100/100.
  luaExecStr("assert(cv2.x ~= nil, 'x table missing for custom curve')");
  luaExecStr("assert(cv2.x[1] == -100, 'x[1] mismatch: '..tostring(cv2.x[1]))");
  luaExecStr("assert(cv2.x[4] == 100, 'x[4] mismatch: '..tostring(cv2.x[4]))");
}

// getVersion() — returns 6 values; osname must be "EdgeTX".
TEST(Lua, getVersion)
{
  luaExecStr("ver, radio, maj, minor, rev, osname = getVersion()");
  luaExecStr("assert(type(ver) == 'string', 'ver should be string')");
  luaExecStr("assert(type(radio) == 'string', 'radio should be string')");
  luaExecStr("assert(type(maj) == 'number', 'maj should be number')");
  luaExecStr("assert(type(minor) == 'number', 'minor should be number')");
  luaExecStr("assert(type(rev) == 'number', 'rev should be number')");
  luaExecStr("assert(osname == 'EdgeTX', 'osname mismatch: '..tostring(osname))");
}

// getFieldInfo() — look up a known source by name and by id.
TEST(Lua, getFieldInfo)
{
  MODEL_RESET();

  // Look up throttle by its legacy name.
#if defined(SURFACE_RADIO)
  luaExecStr("fi = getFieldInfo('thr')");
#else
  luaExecStr("fi = getFieldInfo('thr')");
#endif
  luaExecStr("assert(fi ~= nil, 'getFieldInfo(thr) returned nil')");
  luaExecStr("assert(type(fi.id) == 'number', 'id should be number')");
  luaExecStr("assert(type(fi.name) == 'string', 'name should be string')");
  luaExecStr("assert(type(fi.desc) == 'string', 'desc should be string')");

  // Look up by numeric id — must return the same name.
  luaExecStr("fi2 = getFieldInfo(fi.id)");
  luaExecStr("assert(fi2 ~= nil, 'getFieldInfo(id) returned nil')");
  luaExecStr("assert(fi2.name == fi.name, 'name mismatch by id: '..tostring(fi2.name))");

  // Unknown name must return nil (getFieldInfo returns 0 values on failure → nil in Lua).
  luaExecStr("fi3 = getFieldInfo('__no_such_field__')");
  luaExecStr("assert(fi3 == nil, 'expected nil for unknown field')");
}

// getFlightMode() — returns current mode number and name.
TEST(Lua, getFlightMode)
{
  MODEL_RESET();

  // Set flight mode 0 name and verify getFlightMode() returns it.
  strncpy(g_model.flightModeData[0].name, "FM0", LEN_FLIGHT_MODE_NAME);

  luaExecStr("fmnum, fmname = getFlightMode()");
  luaExecStr("assert(type(fmnum) == 'number', 'fmnum should be number')");
  luaExecStr("assert(type(fmname) == 'string', 'fmname should be string')");
  // Current mode in a fresh model should be 0.
  luaExecStr("assert(fmnum == 0, 'expected FM0, got: '..tostring(fmnum))");
  luaExecStr("assert(fmname == 'FM0', 'name mismatch: '..tostring(fmname))");

  // Explicit index must return the same data.
  luaExecStr("fmnum2, fmname2 = getFlightMode(0)");
  luaExecStr("assert(fmnum2 == 0, 'explicit FM0 num mismatch')");
  luaExecStr("assert(fmname2 == 'FM0', 'explicit FM0 name mismatch')");
}

// bit32.extract / replace / btest — basic correctness.
TEST(Lua, bit32Operations)
{
  // extract: pull bits 4-5 (width 2) from 0x30 = 0011 0000b → value 3.
  luaExecStr("assert(bit32.extract(0x30, 4, 2) == 3, 'extract failed')");

  // replace: insert value 0xF into bits 0-3 of 0x00 → 0x0F.
  luaExecStr("assert(bit32.replace(0x00, 0xF, 0, 4) == 0x0F, 'replace failed')");

  // replace: overwrite bits 4-7 of 0xFF with 0x0 → 0x0F.
  luaExecStr("assert(bit32.replace(0xFF, 0x0, 4, 4) == 0x0F, 'replace clear failed')");

  // btest: at least one of bit 0 and bit 1 set in 0x03 → true.
  luaExecStr("assert(bit32.btest(0x03, 0x01) == true, 'btest true failed')");

  // btest: bit 1 not set in 0x01 → false.
  luaExecStr("assert(bit32.btest(0x01, 0x02) == false, 'btest false failed')");

  // round-trip: encode two nibbles and decode them back.
  luaExecStr("packed = bit32.replace(bit32.replace(0, 7, 0, 4), 12, 4, 4)");
  luaExecStr("assert(bit32.extract(packed, 0, 4) == 7, 'round-trip low nibble')");
  luaExecStr("assert(bit32.extract(packed, 4, 4) == 12, 'round-trip high nibble')");
}

#endif   // #if defined(LUA)
