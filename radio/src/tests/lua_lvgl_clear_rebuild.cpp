/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "gtests.h"

#if defined(LUA) && defined(COLORLCD)

#include "lua/lua_widget.h"
#include "location.h"
#include "mainwindow.h"

#include <fstream>
#include <iterator>
#include <string>

namespace {

class ClearRebuildManager : public Window, public LuaScriptManager
{
 public:
  ClearRebuildManager() :
      Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H})
  {
  }

  Window* getCurrentParent() const override
  {
    auto p = getTempParent();
    return p && p->getWindow() ? p->getWindow() :
                                const_cast<ClearRebuildManager*>(this);
  }
  void clear() override
  {
    clearRefs(lsWidgets);
    Window::clear();
  }
  bool useLvglLayout() const override { return true; }
  bool isAppMode() const override { return false; }
  bool isWidget() override { return false; }
  bool isFullscreen() override { return true; }
  void luaShowError() override { ADD_FAILURE() << lua_tostring(lsWidgets, -1); }
};

class LuaLvglClearRebuild : public testing::Test
{
 protected:
  void SetUp() override
  {
    savedState = lsWidgets;
    savedManager = luaScriptManager;
    lsWidgets = luaL_newstate();
    ASSERT_NE(nullptr, lsWidgets);
    luaRegisterLibraries(lsWidgets);
    manager = new ClearRebuildManager();
    luaScriptManager = manager;
  }

  void TearDown() override
  {
    if (manager) {
      manager->clear();
      lua_gc(lsWidgets, LUA_GCCOLLECT, 0);
      manager->closeWindow();
      MainWindow::instance()->run();
    }
    if (lsWidgets) lua_close(lsWidgets);
    lsWidgets = savedState;
    luaScriptManager = savedManager;
  }

  testing::AssertionResult exec(const char* code)
  {
    if (luaL_dostring(lsWidgets, code) != LUA_OK)
      return testing::AssertionFailure() << lua_tostring(lsWidgets, -1);
    return testing::AssertionSuccess();
  }

  bool valid(const char* name)
  {
    lua_getglobal(lsWidgets, name);
    bool result = LvglWidgetObjectBase::checkLvgl(lsWidgets, -1) != nullptr;
    lua_pop(lsWidgets, 1);
    return result;
  }

  ClearRebuildManager* manager = nullptr;
  lua_State* savedState = nullptr;
  LuaScriptManager* savedManager = nullptr;
};

TEST_F(LuaLvglClearRebuild, StandaloneReproducer)
{
  std::ifstream file(std::string(TESTS_PATH) + "/lua/lvgl_clear_rebuild.lua");
  ASSERT_TRUE(file.is_open());
  std::string source((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
  ASSERT_EQ(LUA_OK, luaL_loadbuffer(lsWidgets, source.data(), source.size(),
                                  "lvgl_clear_rebuild.lua"));
  ASSERT_EQ(LUA_OK, lua_pcall(lsWidgets, 0, 1, 0));
  lua_setglobal(lsWidgets, "script");

  ASSERT_TRUE(exec("assert(script.useLvgl == true)"));
  ASSERT_TRUE(exec("script.init(); assert(script.run(0) == 0)"));
  ASSERT_TRUE(manager->callRefs(lsWidgets));
  ASSERT_TRUE(exec("assert(script.run(0) == 1)"));
}

TEST_F(LuaLvglClearRebuild, RepeatedClearBeforeRefresh)
{
  ASSERT_TRUE(exec(
      "parent = lvgl.box({w=200, h=100})\n"
      "lvgl.label(parent, {text='Old'})\n"
      "lvgl.clear(parent)\n"
      "lvgl.label(parent, {text='Intermediate'})\n"
      "lvgl.clear(parent)\n"
      "replacement = lvgl.label(parent, {text='Replacement'})"));
  ASSERT_TRUE(manager->callRefs(lsWidgets));
  ASSERT_TRUE(exec("replacement:set({text='Still works'})"));
  ASSERT_TRUE(manager->callRefs(lsWidgets));
  ASSERT_TRUE(exec("replacement:set({text='Still works again'})"));
}

TEST_F(LuaLvglClearRebuild, InvalidatesOnlyOldDescendantsImmediately)
{
  ASSERT_TRUE(exec(
      "parent = lvgl.box({w=200, h=100})\n"
      "old = lvgl.box(parent, {w=100, h=50})\n"
      "nested = lvgl.label(old, {text='Nested'})\n"
      "sibling = lvgl.label({text='Sibling'})\n"
      "lvgl.clear(parent)\n"
      "replacement = lvgl.label(parent, {text='Replacement'})"));
  EXPECT_FALSE(valid("old"));
  EXPECT_FALSE(valid("nested"));
  EXPECT_TRUE(valid("parent"));
  EXPECT_TRUE(valid("sibling"));
  EXPECT_TRUE(valid("replacement"));
  ASSERT_TRUE(manager->callRefs(lsWidgets));
  EXPECT_TRUE(valid("replacement"));
}

TEST_F(LuaLvglClearRebuild, ReplacementCallbacksContinueRefreshing)
{
  ASSERT_TRUE(exec(
      "parent = lvgl.box({w=200, h=100})\n"
      "lvgl.label(parent, {text='Old'})\n"
      "lvgl.clear(parent)\n"
      "calls = 0\n"
      "replacement = lvgl.label(parent, {text=function()\n"
      "  calls = calls + 1; return 'Replacement'\n"
      "end})\n"
      "calls = 0"));
  ASSERT_TRUE(manager->callRefs(lsWidgets));
  ASSERT_TRUE(manager->callRefs(lsWidgets));
  ASSERT_TRUE(exec("assert(calls > 0)"));
}

TEST_F(LuaLvglClearRebuild, EntireUiCanStillBeRebuilt)
{
  ASSERT_TRUE(exec(
      "old = lvgl.box({w=200, h=100})\n"
      "nested = lvgl.label(old, {text='Nested'})\n"
      "lvgl.clear()\n"
      "replacement = lvgl.label({text='Replacement'})"));
  EXPECT_FALSE(valid("old"));
  EXPECT_FALSE(valid("nested"));
  ASSERT_TRUE(manager->callRefs(lsWidgets));
  ASSERT_TRUE(exec("replacement:set({text='Still works'})"));
}

}  // namespace

#endif
