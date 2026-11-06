/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "gtests.h"

#if defined(LUA) && defined(COLORLCD)

#include <filesystem>
#include <string>

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

  ::testing::AssertionResult exec(const std::string& source)
  {
    if (luaL_dostring(lsStandalone, source.c_str()) != LUA_OK)
      return ::testing::AssertionFailure() << lua_tostring(lsStandalone, -1);
    return ::testing::AssertionSuccess();
  }

  // Create an object with a function for 'key', rebind it several times via
  // set(), then check that only the current function is still reachable.
  // Superseded functions are tracked in a weak table, so they only survive a
  // full GC if something (e.g. a leaked registry ref) still holds them.
  void expectRebindReleases(const char* create, const char* key,
                            const char* body)
  {
    std::string fn = std::string("tracked(function() ") + body + " end)";
    ASSERT_TRUE(exec(
        "rebindTracked = setmetatable({}, {__mode = 'k'})\n"
        "function tracked(f) rebindTracked[f] = true return f end\n"
        "rebindObject = lvgl." + std::string(create) + "({" + key + " = " +
        fn + "})\n"
        "for i = 1, 5 do rebindObject:set({" + key + " = " + fn +
        "}) end\n"));
    ASSERT_TRUE(exec(
        "collectgarbage('collect') collectgarbage('collect')\n"
        "local alive = 0\n"
        "for _ in pairs(rebindTracked) do alive = alive + 1 end\n"
        "assert(alive == 1, alive .. ' functions alive after rebinding')\n"));
  }
};

TEST_F(LuaLvgl, RebindTextReleasesPrevious)
{
  expectRebindReleases("label", "text", "return 'DYNAMIC'");
}

TEST_F(LuaLvgl, RebindOpacityReleasesPrevious)
{
  expectRebindReleases("rectangle", "opacity", "return 255");
}

TEST_F(LuaLvgl, RebindVisibleReleasesPrevious)
{
  expectRebindReleases("label", "visible", "return true");
}

TEST_F(LuaLvgl, RebindPosReleasesPrevious)
{
  expectRebindReleases("label", "pos", "return 10, 10");
}

TEST_F(LuaLvgl, RebindPressReleasesPrevious)
{
  expectRebindReleases("button", "press", "return 0");
}

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
