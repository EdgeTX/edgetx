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

// model.setLogicalSwitch / getLogicalSwitch — LS_FUNC_VEQUAL (func=1).
// Every rc-soar model uses VEQUAL for flight-mode detection, e.g.
// "L01: I5 == -100" to detect the flight-mode source sitting at -100%.
TEST(Lua, modelLogicalSwitchVEqual)
{
  MODEL_RESET();

  // func=1 (LS_FUNC_VEQUAL): true when v1 == v2.
  luaExecStr("model.setLogicalSwitch(0, {func=1, v1=2, v2=-100, delay=0, duration=0})");

  luaExecStr("ls = model.getLogicalSwitch(0)");
  luaExecStr("assert(ls ~= nil, 'getLogicalSwitch returned nil')");
  luaExecStr("assert(ls.func == 1, 'func mismatch: '..tostring(ls.func))");
  luaExecStr("assert(ls.v1 == 2,  'v1 mismatch: '..tostring(ls.v1))");
  luaExecStr("assert(ls.v2 == -100, 'v2 mismatch: '..tostring(ls.v2))");
  luaExecStr("assert(ls.delay == 0, 'delay mismatch: '..tostring(ls.delay))");
}

// model.setLogicalSwitch / getLogicalSwitch — LS_FUNC_VNEG (func=4).
// rc-soar models use VNEG for launch-detect (e.g. "I5 < -50").
TEST(Lua, modelLogicalSwitchVNeg)
{
  MODEL_RESET();

  // func=4 (LS_FUNC_VNEG): true when v1 < v2.
  luaExecStr("model.setLogicalSwitch(1, {func=4, v1=3, v2=-500, delay=2})");

  luaExecStr("ls = model.getLogicalSwitch(1)");
  luaExecStr("assert(ls ~= nil, 'getLogicalSwitch returned nil')");
  luaExecStr("assert(ls.func == 4,    'func mismatch: '..tostring(ls.func))");
  luaExecStr("assert(ls.v1 == 3,      'v1 mismatch: '..tostring(ls.v1))");
  luaExecStr("assert(ls.v2 == -500,   'v2 mismatch: '..tostring(ls.v2))");
  luaExecStr("assert(ls.delay == 2,   'delay mismatch: '..tostring(ls.delay))");
}

// model.setLogicalSwitch / getLogicalSwitch — LS_FUNC_STICKY (func=17).
// rc-soar models use STICKY for state-latching (e.g. launch-detected latch
// that stays set until the pilot resets it via a different switch).
TEST(Lua, modelLogicalSwitchSticky)
{
  MODEL_RESET();

  // func=17 (LS_FUNC_STICKY): set by v1 (switch), cleared by v2 (switch).
  // Use small switch indices that are valid across all target configs.
  luaExecStr("model.setLogicalSwitch(2, {func=17, v1=1, v2=2})");

  luaExecStr("ls = model.getLogicalSwitch(2)");
  luaExecStr("assert(ls ~= nil, 'getLogicalSwitch returned nil')");
  luaExecStr("assert(ls.func == 17, 'func mismatch: '..tostring(ls.func))");
  luaExecStr("assert(ls.v1 == 1,   'v1 (set-switch) mismatch: '..tostring(ls.v1))");
  luaExecStr("assert(ls.v2 == 2,   'v2 (clr-switch) mismatch: '..tostring(ls.v2))");
}

// model.getCurve / setCurve — 3-point standard curve.
// rc-soar aileron/elevator differential curves use 3-point standard curves
// (YAML: points: -2, which stores numPoints-5 = -2 → numPoints = 3).
TEST(Lua, modelCurve3Point)
{
  MODEL_RESET();
  loadCurves();  // initialise curveEnd[] before any setCurve call

  // Symmetric V-curve: y = {-50, 0, 50} on evenly-spaced x = {-100, 0, 100}.
  luaExecStr("rc = model.setCurve(0, {type=0, smooth=false, y={-50, 0, 50}})");
  luaExecStr("assert(rc == 0, 'setCurve 3-pt failed: '..tostring(rc))");

  luaExecStr("cv = model.getCurve(0)");
  luaExecStr("assert(cv ~= nil, 'getCurve returned nil')");
  luaExecStr("assert(cv.points == 3, 'expected 3 points, got: '..tostring(cv.points))");
  luaExecStr("assert(cv.type == 0,   'type mismatch: '..tostring(cv.type))");
  luaExecStr("assert(cv.y[1] == -50, 'y[1] mismatch: '..tostring(cv.y[1]))");
  luaExecStr("assert(cv.y[2] ==   0, 'y[2] mismatch: '..tostring(cv.y[2]))");
  luaExecStr("assert(cv.y[3] ==  50, 'y[3] mismatch: '..tostring(cv.y[3]))");
  // Standard curve: no x table.
  luaExecStr("assert(cv.x == nil, 'standard curve should have no x table')");
}

// model.getOutput / setOutput — curve reference field.
// All rc-soar limitData entries carry a curve index (e.g. curve: 1, curve: 2).
// The API stores it as (curve+1) internally and returns (curve-1); the field
// is absent when the output has no curve assigned.
TEST(Lua, modelOutputCurveRef)
{
  MODEL_RESET();

  // Assign curve 0 (Curve1) to output 0.
  luaExecStr("model.setOutput(0, {name='AIL', min=-1000, max=1000, curve=0})");
  luaExecStr("out = model.getOutput(0)");
  luaExecStr("assert(out ~= nil, 'getOutput returned nil')");
  luaExecStr("assert(out.curve == 0, 'curve field mismatch: '..tostring(out.curve))");

  // Assign a different curve (index 2) to output 1.
  luaExecStr("model.setOutput(1, {name='ELE', min=-1000, max=1000, curve=2})");
  luaExecStr("out1 = model.getOutput(1)");
  luaExecStr("assert(out1.curve == 2, 'curve 2 mismatch: '..tostring(out1.curve))");

  // Output with no curve assigned must not have the curve field.
  luaExecStr("model.setOutput(2, {name='RUD', min=-1000, max=1000})");
  luaExecStr("out2 = model.getOutput(2)");
  luaExecStr("assert(out2.curve == nil, 'no-curve output should have nil curve')");
}

#if defined(GVARS)
// model.setCustomFunction / getCustomFunction — FUNC_ADJUST_GVAR (func=5).
// Every rc-soar model uses ADJUST_GVAR special functions to change GVar values
// based on switch position (e.g. set GV9 from channel 10 via a logical switch).
TEST(Lua, modelCustomFunctionAdjustGVar)
{
  MODEL_RESET();

  // func=5 (FUNC_ADJUST_GVAR), switch=1, value=8 (GV9), mode=1 (Source),
  // param=0, active=1.
  luaExecStr("model.setCustomFunction(0, {switch=1, func=5, value=8, mode=1, param=0, active=1})");

  luaExecStr("cf = model.getCustomFunction(0)");
  luaExecStr("assert(cf ~= nil, 'getCustomFunction returned nil')");
  luaExecStr("assert(cf.func   == 5, 'func mismatch: '..tostring(cf.func))");
  luaExecStr("assert(cf.switch == 1, 'switch mismatch: '..tostring(cf.switch))");
  luaExecStr("assert(cf.value  == 8, 'value (GVar index) mismatch: '..tostring(cf.value))");
  luaExecStr("assert(cf.mode   == 1, 'mode mismatch: '..tostring(cf.mode))");
  luaExecStr("assert(cf.active == 1, 'active mismatch: '..tostring(cf.active))");

  // Out-of-range index must return nil.
  luaExecStr("assert(model.getCustomFunction(999) == nil, 'expected nil for OOB index')");
}
#endif  // defined(GVARS)

// model.setLogicalSwitch / getLogicalSwitch — LS_FUNC_EDGE (func=10).
// RC-soar DLG model uses EDGE to detect launch: switch held for a minimum
// duration triggers the launch-detected latch.  Tests v1 (switch), v2 (min
// time), v3 (max time), and duration fields together.
TEST(Lua, modelLogicalSwitchEdge)
{
  MODEL_RESET();

  // func=10 (LS_FUNC_EDGE): true while v1 switch held inside [v2,v2+v3].
  luaExecStr("model.setLogicalSwitch(3, {func=10, v1=1, v2=-100, v3=-50, duration=30})");

  luaExecStr("ls = model.getLogicalSwitch(3)");
  luaExecStr("assert(ls ~= nil, 'getLogicalSwitch returned nil')");
  luaExecStr("assert(ls.func == 10,   'func mismatch: '..tostring(ls.func))");
  luaExecStr("assert(ls.v1   ==  1,   'v1 mismatch: '..tostring(ls.v1))");
  luaExecStr("assert(ls.v2   == -100, 'v2 mismatch: '..tostring(ls.v2))");
  luaExecStr("assert(ls.v3   == -50,  'v3 mismatch: '..tostring(ls.v3))");
  luaExecStr("assert(ls.duration == 30, 'duration mismatch: '..tostring(ls.duration))");
}

// model.setLogicalSwitch / getLogicalSwitch — LS_FUNC_OR (func=8).
// RC-soar F3J, easygee, and F3F models use OR to combine two mode-trigger
// logical switches, e.g. "L01 OR L02".
TEST(Lua, modelLogicalSwitchOr)
{
  MODEL_RESET();

  // func=8 (LS_FUNC_OR): true when either v1 or v2 switch is active.
  luaExecStr("model.setLogicalSwitch(4, {func=8, v1=1, v2=2})");

  luaExecStr("ls = model.getLogicalSwitch(4)");
  luaExecStr("assert(ls ~= nil, 'getLogicalSwitch returned nil')");
  luaExecStr("assert(ls.func == 8, 'func mismatch: '..tostring(ls.func))");
  luaExecStr("assert(ls.v1 == 1,   'v1 mismatch: '..tostring(ls.v1))");
  luaExecStr("assert(ls.v2 == 2,   'v2 mismatch: '..tostring(ls.v2))");
}

// model.setLogicalSwitch / getLogicalSwitch — the "and" (AND-switch) field.
// "and" is a reserved Lua keyword so it must be accessed via bracket syntax:
// ls["and"]. Virtually every real-world LS uses a non-zero AND-switch, but
// this code path has never been tested.
TEST(Lua, modelLogicalSwitchAndField)
{
  MODEL_RESET();

  // Set func=1 (VEQUAL) with an AND-switch value of 5.
  luaExecStr("model.setLogicalSwitch(5, {func=1, v1=2, v2=-100, [\"and\"]=5})");

  luaExecStr("ls = model.getLogicalSwitch(5)");
  luaExecStr("assert(ls ~= nil, 'getLogicalSwitch returned nil')");
  luaExecStr("assert(ls.func == 1, 'func mismatch: '..tostring(ls.func))");
  // "and" is a Lua reserved word — must use bracket syntax to read it.
  luaExecStr("assert(ls[\"and\"] == 5, 'and field mismatch: '..tostring(ls[\"and\"]))");
}

// model.setCustomFunction / getCustomFunction — FUNC_PLAY_TRACK (func=11).
// All rc-soar models use PLAY_TRACK to announce flight-mode changes.
// This exercises the separate code path in getCustomFunction that returns a
// "name" string field instead of "value/mode/param".
TEST(Lua, modelCustomFunctionPlayTrack)
{
  MODEL_RESET();

  // func=11 (FUNC_PLAY_TRACK): stores a filename in cfn->play.name.
  luaExecStr("model.setCustomFunction(1, {switch=1, func=11, name='hello', active=1})");

  luaExecStr("cf = model.getCustomFunction(1)");
  luaExecStr("assert(cf ~= nil, 'getCustomFunction returned nil')");
  luaExecStr("assert(cf.func   == 11,      'func mismatch: '..tostring(cf.func))");
  luaExecStr("assert(cf.name   == 'hello', 'name mismatch: '..tostring(cf.name))");
  luaExecStr("assert(cf.active == 1,       'active mismatch: '..tostring(cf.active))");
  // PLAY_TRACK returns name, not value/mode/param.
  luaExecStr("assert(cf.value == nil, 'value field should be absent for PLAY_TRACK')");
}

// model.setCustomFunction / getCustomFunction — FUNC_RESET (func=3).
// DLG and easygee models use RESET to reset Timer1 on arm.
// This exercises the value/mode/param code path (different from PLAY_TRACK).
TEST(Lua, modelCustomFunctionReset)
{
  MODEL_RESET();

  // func=3 (FUNC_RESET), value=0 (Timer1 reset target), active=1.
  luaExecStr("model.setCustomFunction(2, {switch=1, func=3, value=0, active=1})");

  luaExecStr("cf = model.getCustomFunction(2)");
  luaExecStr("assert(cf ~= nil, 'getCustomFunction returned nil')");
  luaExecStr("assert(cf.func   == 3, 'func mismatch: '..tostring(cf.func))");
  luaExecStr("assert(cf.value  == 0, 'value (Timer1) mismatch: '..tostring(cf.value))");
  luaExecStr("assert(cf.active == 1, 'active mismatch: '..tostring(cf.active))");
  // FUNC_RESET uses value/mode/param, not name.
  luaExecStr("assert(cf.name == nil, 'name field should be absent for RESET')");
}

// model.insertMix / getMix — flightModes bitmask and multiplex.
// The rc-soar "all-except-FM1" pattern (flightModes=509, YAML "101111111")
// and multiplex REPL (2) are used in every calibration override mix.
TEST(Lua, modelInsertMixFlightModes)
{
  MODEL_RESET();

  // CH1 (index 0), line 0.  flightModes=509 = bits 0,2-8 set (FM1 active).
  luaExecStr("model.insertMix(0, 0, {source=1, weight=100, multiplex=2, flightModes=509})");

  luaExecStr("mx = model.getMix(0, 0)");
  luaExecStr("assert(mx ~= nil, 'getMix returned nil')");
  luaExecStr("assert(mx.flightModes == 509, 'flightModes mismatch: '..tostring(mx.flightModes))");
  luaExecStr("assert(mx.multiplex  == 2,   'multiplex (REPL) mismatch: '..tostring(mx.multiplex))");
}

// model.insertMix / getMix — MUL multiplex and non-zero offset.
// RC-soar speed-compensation mixes use MUL(1) with offset: 100 to bias the
// output before the multiplication stage.
TEST(Lua, modelInsertMixMulAndOffset)
{
  MODEL_RESET();

  // CH2 (index 1): MUL with offset=50.
  luaExecStr("model.insertMix(1, 0, {source=1, weight=100, multiplex=1, offset=50, flightModes=0})");

  luaExecStr("mx1 = model.getMix(1, 0)");
  luaExecStr("assert(mx1 ~= nil, 'getMix (CH2) returned nil')");
  luaExecStr("assert(mx1.multiplex == 1,  'MUL mismatch: '..tostring(mx1.multiplex))");
  luaExecStr("assert(mx1.offset    == 50, 'offset mismatch: '..tostring(mx1.offset))");
}

#if defined(GVARS)
// model.setGlobalVariable / getGlobalVariable — per-FM GVar isolation.
// Each FM has its own GVar array.  f3j model: FM0 GV1=80 (cruise),
// FM2 GV1=0 (launch).  Verifies independence through the Lua API.
TEST(Lua, modelSetGetGlobalVariablePerFM)
{
  MODEL_RESET();

  // GVar index 0 = GV1.
  luaExecStr("model.setGlobalVariable(0, 0, 80)");  // FM0 GV1 = 80
  luaExecStr("model.setGlobalVariable(0, 2, 0)");   // FM2 GV1 = 0

  luaExecStr("assert(model.getGlobalVariable(0, 0) == 80, 'FM0 GV1 should be 80')");
  luaExecStr("assert(model.getGlobalVariable(0, 2) == 0,  'FM2 GV1 should be 0')");

  // Updating FM2 must not affect FM0.
  luaExecStr("model.setGlobalVariable(0, 2, 600)");
  luaExecStr("assert(model.getGlobalVariable(0, 0) == 80,  'FM0 GV1 unchanged')");
  luaExecStr("assert(model.getGlobalVariable(0, 2) == 600, 'FM2 GV1 updated')");

  // Out-of-range GVar index must return nil.
  luaExecStr("assert(model.getGlobalVariable(999, 0) == nil, 'OOB GVar index')");
}
#endif  // defined(GVARS)

// model.setFlightMode / getFlightMode — named FM1 (CAL mode).
// Every rc-soar model names FM1 "CAL" and assigns it to a switch position.
// Tests that name, switch, fadeIn, and fadeOut round-trip correctly.
TEST(Lua, modelGetFlightModeNamed)
{
  MODEL_RESET();

  luaExecStr("model.setFlightMode(1, {name='CAL', switch=2, fadeIn=5, fadeOut=5})");

  luaExecStr("fm = model.getFlightMode(1)");
  luaExecStr("assert(fm ~= nil, 'getFlightMode(1) returned nil')");
  luaExecStr("assert(fm.name   == 'CAL', 'name mismatch: '..tostring(fm.name))");
  luaExecStr("assert(fm.switch == 2,     'switch mismatch: '..tostring(fm.switch))");
  luaExecStr("assert(fm.fadeIn == 5,     'fadeIn mismatch: '..tostring(fm.fadeIn))");
  luaExecStr("assert(fm.fadeOut == 5,    'fadeOut mismatch: '..tostring(fm.fadeOut))");
}

// model.setOutput / getOutput — ppmCenter and revert fields.
// ahi_110 rc-soar model sets ppmCenter: 18 on output channels;
// revert: 1 is used to reverse servo travel direction.
TEST(Lua, modelOutputPpmCenterRevert)
{
  MODEL_RESET();

  luaExecStr("model.setOutput(0, {name='AIL', min=-1000, max=1000, ppmCenter=18, revert=1})");

  luaExecStr("out = model.getOutput(0)");
  luaExecStr("assert(out ~= nil, 'getOutput returned nil')");
  luaExecStr("assert(out.ppmCenter == 18, 'ppmCenter mismatch: '..tostring(out.ppmCenter))");
  luaExecStr("assert(out.revert    == 1,  'revert mismatch: '..tostring(out.revert))");

  // Default output (no revert, center=0).
  luaExecStr("model.setOutput(1, {name='ELE', min=-1000, max=1000})");
  luaExecStr("out1 = model.getOutput(1)");
  luaExecStr("assert(out1.ppmCenter == 0, 'default ppmCenter should be 0')");
  luaExecStr("assert(out1.revert    == 0, 'default revert should be 0')");
}

// model.insertInput / getInput — flightModes bitmask on expo lines.
// ahi_110 rc-soar model uses flightModes: 011111111 on aileron/elevator
// inputs so those expo lines are active only in FM0 (normal flight).
// YAML "011111111" → bits 1-8 set = decimal 510.
TEST(Lua, modelInsertInputFlightModes)
{
  MODEL_RESET();

  // Input 0, line 0; flightModes=510 means FM0 active, FM1-FM8 excluded.
  luaExecStr("model.insertInput(0, 0, {source=1, weight=100, flightModes=510})");

  luaExecStr("inp = model.getInput(0, 0)");
  luaExecStr("assert(inp ~= nil, 'getInput returned nil')");
  luaExecStr("assert(inp.flightModes == 510, 'flightModes mismatch: '..tostring(inp.flightModes))");
}

#endif   // #if defined(LUA)
