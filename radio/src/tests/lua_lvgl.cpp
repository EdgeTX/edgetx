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

// Tests for the Lua lvgl.* API (api_colorlcd_lvgl.cpp).
// These tests exercise the Lua-facing functions (lvgl.label, lvgl.build,
// lvgl.isAppMode, etc.) and verify correct nil-safety guards as well as
// widget creation when a script manager is active.

#if defined(LUA) && defined(COLORLCD)

#include "gtests.h"
#include "edgetx.h"
#include "lua/lua_states.h"
#include "lua/lua_widget.h"
#include "mainwindow.h"

// ── helpers ─────────────────────────────────────────────────────────────────

// Re-use the same luaExecStr helper as lua.cpp.
static ::testing::AssertionResult __luaLvglExecStr(const char* str)
{
  extern lua_State* lsScripts;
  if (!lsScripts) { luaInitMainState(); luaInit(); }
  if (!lsScripts)
    return ::testing::AssertionFailure() << "No Lua state!";
  if (luaL_dostring(lsScripts, str)) {
    return ::testing::AssertionFailure()
           << "lua error: " << lua_tostring(lsScripts, -1);
  }
  return ::testing::AssertionSuccess();
}
#define lvglExecStr(s) EXPECT_TRUE(__luaLvglExecStr(s))

// ── minimal LuaScriptManager mock ───────────────────────────────────────────
//
// TestLuaScriptManager satisfies the pure-virtual interface using
// MainWindow::instance() as the LVGL parent.  isFullscreen() returns true so
// that fullscreen-only controls (button, slider, …) are also created.

class TestLuaScriptManager : public LuaScriptManager
{
 public:
  TestLuaScriptManager()
  {
    // A child window on the MainWindow provides a real lv_obj_t parent.
    _window = new Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H});
  }

  ~TestLuaScriptManager()
  {
    // Window cleans up its own LVGL object on destruction.
    delete _window;
  }

  Window* getCurrentParent() const override { return _window; }
  void    clear()            override       {}
  bool    useLvglLayout()    const override { return true; }
  bool    isAppMode()        const override { return false; }
  bool    isWidget()         override       { return false; }
  bool    isFullscreen()     override       { return true; }
  void    luaShowError()     override       {}

 private:
  Window* _window;
};

// ── fixture ──────────────────────────────────────────────────────────────────

class LuaLvglTest : public EdgeTxTest
{
 protected:
  void SetUp() override
  {
    EdgeTxTest::SetUp();
    // Ensure Lua state is initialised.
    extern lua_State* lsScripts;
    if (!lsScripts) { luaInitMainState(); luaInit(); }
    _mgr = new TestLuaScriptManager();
    luaScriptManager = _mgr;
  }

  void TearDown() override
  {
    luaScriptManager = nullptr;
    delete _mgr;
    _mgr = nullptr;
    EdgeTxTest::TearDown();
  }

 private:
  TestLuaScriptManager* _mgr = nullptr;
};

// ── nil-safety tests (no manager) ────────────────────────────────────────────

// Without a script manager, lvgl.isAppMode() must return false.
TEST(LuaLvgl, isAppModeReturnsFalseWithoutManager)
{
  extern lua_State* lsScripts;
  if (!lsScripts) { luaInitMainState(); luaInit(); }

  luaScriptManager = nullptr;
  lvglExecStr("assert(lvgl.isAppMode() == false, 'expected false')");
}

// Without a script manager, lvgl.isFullScreen() must return false.
TEST(LuaLvgl, isFullscreenReturnsFalseWithoutManager)
{
  luaScriptManager = nullptr;
  lvglExecStr("assert(lvgl.isFullScreen() == false, 'expected false')");
}

// Without a script manager, lvgl.getContext() must return nil.
TEST(LuaLvgl, getContextReturnsNilWithoutManager)
{
  luaScriptManager = nullptr;
  lvglExecStr("assert(lvgl.getContext() == nil, 'expected nil')");
}

// Without a script manager, widget constructors must return nil rather than
// crashing — this verifies all nil-guard paths in luaLvglObjEx.
TEST(LuaLvgl, widgetCreationReturnsNilWithoutManager)
{
  luaScriptManager = nullptr;
  lvglExecStr("assert(lvgl.label({text='x'})     == nil, 'label')");
  lvglExecStr("assert(lvgl.rectangle({})          == nil, 'rectangle')");
  lvglExecStr("assert(lvgl.circle({})             == nil, 'circle')");
  lvglExecStr("assert(lvgl.box({})                == nil, 'box')");
}

// Without a script manager, lvgl.build() must return nil.
TEST(LuaLvgl, buildReturnsNilWithoutManager)
{
  luaScriptManager = nullptr;
  lvglExecStr("assert(lvgl.build({{type='label',text='hi'}}) == nil, 'expected nil')");
}

// ── widget-creation tests (with mock manager) ─────────────────────────────

// lvgl.isAppMode() returns false from our mock (isAppMode returns false).
TEST_F(LuaLvglTest, isAppModeReturnsFalseFromMock)
{
  lvglExecStr("assert(lvgl.isAppMode() == false, 'expected false from mock')");
}

// lvgl.isFullScreen() returns true from our mock (isFullscreen returns true).
TEST_F(LuaLvglTest, isFullscreenReturnsTrueFromMock)
{
  lvglExecStr("assert(lvgl.isFullScreen() == true, 'expected true from mock')");
}

// lvgl.label() must return a non-nil userdata object when a manager is active.
// Use a global (not local) so the variable persists across luaExecStr calls.
TEST_F(LuaLvglTest, labelCreationReturnsUserdata)
{
  lvglExecStr("wLabel = lvgl.label({text='hello'})");
  lvglExecStr("assert(wLabel ~= nil, 'label should not be nil')");
  lvglExecStr("assert(type(wLabel) == 'userdata', 'label should be userdata')");
  lvglExecStr("lvgl.clear(wLabel)");
}

// lvgl.rectangle() must return a non-nil widget object.
TEST_F(LuaLvglTest, rectangleCreationReturnsUserdata)
{
  lvglExecStr("wRect = lvgl.rectangle({x=0, y=0, w=50, h=20})");
  lvglExecStr("assert(wRect ~= nil, 'rectangle should not be nil')");
  lvglExecStr("lvgl.clear(wRect)");
}

// lvgl.button() is fullscreen-only; with our mock (isFullscreen=true) it must
// return a non-nil object.
TEST_F(LuaLvglTest, buttonCreationReturnsUserdataInFullscreen)
{
  lvglExecStr("wBtn = lvgl.button({x=0, y=0, w=80, h=30, text='OK'})");
  lvglExecStr("assert(wBtn ~= nil, 'button should not be nil in fullscreen mode')");
  lvglExecStr("lvgl.clear(wBtn)");
}

// lvgl.build() must return a Lua table (possibly empty, but never nil) when a
// manager is active.
TEST_F(LuaLvglTest, buildReturnsTable)
{
  lvglExecStr("tBuild = lvgl.build({{type='label', text='test'}})");
  lvglExecStr("assert(tBuild ~= nil, 'build should not return nil')");
  lvglExecStr("assert(type(tBuild) == 'table', 'build should return a table')");
}

// Named widgets in lvgl.build() must appear as fields in the returned table.
TEST_F(LuaLvglTest, buildReturnsNamedWidgetRefs)
{
  lvglExecStr("tNamed = lvgl.build({"
              "  {type='label', name='myLabel', text='hi'},"
              "  {type='rectangle', name='myRect', x=0, y=30, w=40, h=10}"
              "})");
  lvglExecStr("assert(tNamed ~= nil, 'build returned nil')");
  lvglExecStr("assert(tNamed.myLabel ~= nil, 'myLabel ref missing')");
  lvglExecStr("assert(tNamed.myRect  ~= nil, 'myRect ref missing')");
  lvglExecStr("assert(type(tNamed.myLabel) == 'userdata', 'myLabel not userdata')");
}

// lvgl.show() and lvgl.hide() must not crash on a valid widget.
TEST_F(LuaLvglTest, showHideDoNotCrash)
{
  lvglExecStr("wVis = lvgl.label({text='vis'})");
  lvglExecStr("assert(wVis ~= nil)");
  lvglExecStr("lvgl.hide(wVis)");
  lvglExecStr("lvgl.show(wVis)");
  lvglExecStr("lvgl.clear(wVis)");
}

// lvgl.set() on a label must not crash (even if visual state is not
// independently verified here).
TEST_F(LuaLvglTest, setDoesNotCrash)
{
  lvglExecStr("wSet = lvgl.label({text='before'})");
  lvglExecStr("assert(wSet ~= nil)");
  lvglExecStr("lvgl.set(wSet, {text='after'})");
  lvglExecStr("lvgl.clear(wSet)");
}

#endif  // defined(LUA) && defined(COLORLCD)
