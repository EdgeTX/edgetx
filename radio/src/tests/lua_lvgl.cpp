/*
 * Copyright (C) EdgeTX
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

#include "gtests.h"

#if defined(LUA) && defined(COLORLCD)

#include "lua/lua_widget.h"
#include "mainwindow.h"
#include "view_main.h"

class LuaLvglTestManager : public LuaScriptManager
{
 public:
  LuaLvglTestManager() :
      parent(new Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H}))
  {
  }
  ~LuaLvglTestManager() { parent->closeWindow(); }

  Window* getCurrentParent() const override { return parent; }
  void clear() override
  {
    clearRefs(lsWidgets);
    parent->clear();
  }
  bool useLvglLayout() const override { return true; }
  bool isAppMode() const override { return false; }
  bool isWidget() override { return true; }
  bool isFullscreen() override { return true; }
  void luaShowError() override { FAIL() << lua_tostring(lsWidgets, -1); }

 private:
  Window* parent;
};

class LuaLvglDialogTest : public testing::TestWithParam<bool>
{
 protected:
  void SetUp() override
  {
    ViewMain::instance();
    MainWindow::instance()->run();
    savedState = lsWidgets;
    savedManager = luaScriptManager;
    lsWidgets = luaL_newstate();
    ASSERT_NE(lsWidgets, nullptr);
    luaL_openlibs(lsWidgets);
    manager = new LuaLvglTestManager();
    luaScriptManager = manager;
  }

  void TearDown() override
  {
    if (manager) {
      manager->clear();
      delete manager;
    }
    if (lsWidgets)
      lua_close(lsWidgets);
    lsWidgets = savedState;
    luaScriptManager = savedManager;
    MainWindow::instance()->run();
  }

  lua_State* savedState = nullptr;
  LuaScriptManager* savedManager = nullptr;
  LuaLvglTestManager* manager = nullptr;
};

TEST_P(LuaLvglDialogTest, skipsClosedDialogsAfterNativeDestruction)
{
  ASSERT_EQ(luaL_dostring(lsWidgets,
      "closed = 0; dialogUpdates = 0; liveUpdates = 0\n"
      "live = lvgl.label({text = function()\n"
      "  liveUpdates = liveUpdates + 1; return 'live'\n"
      "end})"), LUA_OK);

  for (int cycle = 0; cycle < 3; ++cycle) {
    ASSERT_EQ(luaL_dostring(lsWidgets,
        "dialog = lvgl.dialog({title = 'Test',\n"
        "  close = function() closed = closed + 1 end,\n"
        "  children = {{type = lvgl.LABEL, text = function()\n"
        "    dialogUpdates = dialogUpdates + 1; return 'child'\n"
        "  end}}\n"
        "})"), LUA_OK);
    ASSERT_TRUE(manager->callRefs(lsWidgets));

    lua_getglobal(lsWidgets, "dialog");
    auto dialog = LvglWidgetObjectBase::checkLvgl(lsWidgets, -1);
    lua_pop(lsWidgets, 1);
    ASSERT_NE(dialog, nullptr);
    ASSERT_NE(dialog->getWindow(), nullptr);

    if (GetParam()) {
      ASSERT_EQ(luaL_dostring(lsWidgets, "lvgl.close(dialog)"), LUA_OK);
    } else {
      auto nativeDialog = dialog->getWindow()->getParent()->getParent();
      nativeDialog->onCancel();
    }

    EXPECT_EQ(dialog->getWindow(), nullptr);
    MainWindow::instance()->run();

    ASSERT_EQ(luaL_dostring(lsWidgets,
        "updatesAtClose = dialogUpdates; liveUpdatesAtClose = liveUpdates"),
        LUA_OK);
    ASSERT_TRUE(manager->callRefs(lsWidgets));
    ASSERT_TRUE(manager->callRefs(lsWidgets));
    ASSERT_EQ(luaL_dostring(lsWidgets,
        "assert(dialogUpdates == updatesAtClose)\n"
        "assert(liveUpdates > liveUpdatesAtClose)\n"
        "lvgl.close(dialog)"), LUA_OK);
    lua_getglobal(lsWidgets, "closed");
    EXPECT_EQ(lua_tointeger(lsWidgets, -1), cycle + 1);
    lua_pop(lsWidgets, 1);
  }

  manager->clear();
  EXPECT_TRUE(manager->getLvglObjectRefs().empty());
}

INSTANTIATE_TEST_SUITE_P(LuaLvgl, LuaLvglDialogTest, testing::Bool());

#endif
