/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "gtests.h"

#if defined(LUA) && defined(COLORLCD)

#include <filesystem>

#include "standalone_lua.h"
#include "view_main.h"

extern lua_State* lsStandalone;

class LuaLvgl : public ::testing::Test
{
 protected:
  void SetUp() override
  {
    ViewMain::instance();
    MainWindow::instance()->run();
    luaExecStandalone("/scripts/lvgl_literal_replacement.lua");
    ASSERT_NE(nullptr, StandaloneLuaWindow::instance());
    ASSERT_NE(nullptr, lsStandalone);
    MainWindow::instance()->run();  // init
  }

  void TearDown() override
  {
    if (auto window = StandaloneLuaWindow::instance()) {
      window->clear();
      window->closeWindow();
    }
    MainWindow::instance()->run();
    std::filesystem::remove(
        simuFatfsGetRealPath("/scripts/lvgl_literal_replacement.luac"));
  }

  lv_obj_t* label()
  {
    return lv_obj_get_child(
        StandaloneLuaWindow::instance()->getLvObj(), 0);
  }

  ::testing::AssertionResult exec(const char* source)
  {
    if (luaL_dostring(lsStandalone, source) != LUA_OK)
      return ::testing::AssertionFailure() << lua_tostring(lsStandalone, -1);
    return ::testing::AssertionSuccess();
  }
};

TEST_F(LuaLvgl, LiteralTextStopsCallback)
{
  ASSERT_EQ(1u, lv_obj_get_child_cnt(
                    StandaloneLuaWindow::instance()->getLvObj()));
  EXPECT_STREQ("LITERAL", lv_label_get_text(label()));
  MainWindow::instance()->run();  // run returns 0, then dynamic refresh
  EXPECT_STREQ("LITERAL", lv_label_get_text(label()));
  MainWindow::instance()->run();  // run asserts the count and returns 1
  EXPECT_EQ(nullptr, StandaloneLuaWindow::instance());
}

class LuaLvglLiteralOpacity : public LuaLvgl,
                             public ::testing::WithParamInterface<bool>
{};

TEST_P(LuaLvglLiteralOpacity, StopsCallback)
{
  ASSERT_TRUE(exec(
      "opacityCalls = 0\n"
      "opacityObject = lvgl.rectangle({w = 20, h = 20, filled = true,\n"
      "opacity = function()\n"
      "  opacityCalls = opacityCalls + 1\n"
      "  return 255\n"
      "end})\n"));
  ASSERT_TRUE(exec(GetParam() ? "opacityObject:set({opacity = true})"
                             : "opacityObject:set({opacity = 96})"));
  ASSERT_TRUE(exec("opacityCallsAfterReplacement = opacityCalls"));
  auto opacityObject = lv_obj_get_child(
      StandaloneLuaWindow::instance()->getLvObj(), 1);
  EXPECT_EQ(GetParam() ? 1 : 96,
            lv_obj_get_style_bg_opa(opacityObject, LV_PART_MAIN));

  MainWindow::instance()->run();
  EXPECT_TRUE(exec("assert(opacityCalls == opacityCallsAfterReplacement,\n"
                   "       'opacity callback still active')"));
  EXPECT_EQ(GetParam() ? 1 : 96,
            lv_obj_get_style_bg_opa(opacityObject, LV_PART_MAIN));
}

INSTANTIATE_TEST_SUITE_P(Literals, LuaLvglLiteralOpacity,
                         ::testing::Bool());

#endif
