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
#include "lua/lua_widget_factory.h"
#include "mainwindow.h"
#include "view_main.h"

class LuaWidgetTest : public ::testing::Test
{
 protected:
  void SetUp() override
  {
    ViewMain::instance();
    MainWindow::instance()->run();
    savedLuaState = luaState;
    luaInitThemesAndWidgets();
    ASSERT_NE(nullptr, lsWidgets);

    ASSERT_EQ(LUA_OK, luaL_dostring(lsWidgets,
        "refreshCalls = 0\n"
        "updateCalls = 0\n"
        "labelCalls = 0\n"
        "function create()\n"
        "  lvgl.label({text = function()\n"
        "    labelCalls = labelCalls + 1\n"
        "    return 'Shutdown test'\n"
        "  end})\n"
        "  return {}\n"
        "end\n"
        "function refresh()\n"
        "  if refreshError then error('refresh failed') end\n"
        "  refreshCalls = refreshCalls + 1\n"
        "end\n"
        "function update() updateCalls = updateCalls + 1 end\n"));

    lua_newtable(lsWidgets);
    int optionsRef = luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
    factory = new LuaWidgetFactory(
        new char('\0'), options, optionsRef, functionRef("create"),
        functionRef("update"), functionRef("refresh"), LUA_REFNIL, LUA_REFNIL,
        true, "/WIDGETS/Shutdown/main.lua");
    widget = static_cast<LuaWidget*>(factory->createNew(
        MainWindow::instance(), {0, 0, 100, 100}, 0, 0));
    ASSERT_NE(nullptr, widget);
    ASSERT_EQ(nullptr, widget->getErrorMessage());
    ASSERT_FALSE(widget->getLvglObjectRefs().empty());
    lv_obj_update_layout(widget->getLvObj());
    ASSERT_TRUE(widget->isOnScreen());
  }

  void TearDown() override
  {
    if (widget) {
      widget->clear();
      EXPECT_TRUE(widget->getLvglObjectRefs().empty());
      widget->closeWindow();
      MainWindow::instance()->run();
    }
    delete factory;
    luaClose(&lsWidgets);
    luaState = savedLuaState;
  }

  int functionRef(const char* name)
  {
    lua_getglobal(lsWidgets, name);
    return luaL_ref(lsWidgets, LUA_REGISTRYINDEX);
  }

  int calls(const char* name)
  {
    lua_getglobal(lsWidgets, name);
    int count = lua_tointeger(lsWidgets, -1);
    lua_pop(lsWidgets, 1);
    return count;
  }

  WidgetOption options[1] = {};
  LuaWidgetFactory* factory = nullptr;
  LuaWidget* widget = nullptr;
  uint8_t savedLuaState = 0;
};

TEST_F(LuaWidgetTest, callbacksRunWithLiveState)
{
  int refreshCalls = calls("refreshCalls");
  int updateCalls = calls("updateCalls");
  int labelCalls = calls("labelCalls");

  widget->foreground();
  EXPECT_EQ(refreshCalls + 1, calls("refreshCalls"));
  EXPECT_EQ(labelCalls + 1, calls("labelCalls"));

  widget->update();
  EXPECT_EQ(updateCalls + 1, calls("updateCalls"));
  EXPECT_EQ(labelCalls + 2, calls("labelCalls"));
  EXPECT_EQ(nullptr, widget->getErrorMessage());
}

TEST_F(LuaWidgetTest, foregroundAfterLuaClose)
{
  luaClose();
  ASSERT_EQ(nullptr, lsWidgets);

  widget->foreground();
  EXPECT_EQ(nullptr, widget->getErrorMessage());
}

TEST_F(LuaWidgetTest, refreshErrorsStillReported)
{
  ASSERT_EQ(LUA_OK, luaL_dostring(lsWidgets, "refreshError = true"));

  widget->foreground();

  ASSERT_NE(nullptr, widget->getErrorMessage());
  EXPECT_NE(nullptr, strstr(widget->getErrorMessage(), "refresh failed"));
}

TEST_F(LuaWidgetTest, updateAfterLuaClose)
{
  luaClose();
  ASSERT_EQ(nullptr, lsWidgets);

  widget->update();
  EXPECT_EQ(nullptr, widget->getErrorMessage());

  factory->parseOptionDefaults();
  EXPECT_EQ(nullptr, factory->createNew(
      MainWindow::instance(), {0, 0, 100, 100}, 0, 0));
}

#if LUA_MEM_MAX > 0
TEST_F(LuaWidgetTest, callbacksAfterMemoryLimitShutdown)
{
  // Dismiss the blocking "Lua disabled!" warning in the nested UI loop.
  ASSERT_EQ(LV_RES_OK, lv_async_call([](void*) {
    auto popup = Window::topWindow();
    ASSERT_NE(nullptr, popup);
    ASSERT_NE(ViewMain::instance(), popup);
    popup->closeWindow();
  }, nullptr));

  uint32_t savedExtraMemoryUsage = luaExtraMemoryUsage;
  luaExtraMemoryUsage = LUA_MEM_MAX + 1;
  checkLuaMemoryUsage();
  luaExtraMemoryUsage = savedExtraMemoryUsage;

  ASSERT_EQ(nullptr, lsWidgets);
  ASSERT_EQ(nullptr, lsScripts);
  EXPECT_EQ(INTERPRETER_PANIC, luaState);

  widget->foreground();
  widget->update();
  widget->background();
  EXPECT_EQ(nullptr, widget->getErrorMessage());
}
#endif

#endif
