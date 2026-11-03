#include "gtests.h"

#if defined(LUA) && defined(COLORLCD)

#include "standalone_lua.h"
#include "view_main.h"

TEST(LuaLvgl, propertyKeys)
{
  ViewMain::instance();
  MainWindow::instance()->run();

  luaExecStandalone("/lvgl_property_keys.lua");
  ASSERT_NE(nullptr, StandaloneLuaWindow::instance());

  // init checks the real LVGL bindings; run exits only after all checks pass.
  MainWindow::instance()->run();
  MainWindow::instance()->run();
  bool completed = StandaloneLuaWindow::instance() == nullptr;
  if (!completed) {
    StandaloneLuaWindow::instance()->closeWindow();
    MainWindow::instance()->run();
  }
  f_unlink("/lvgl_property_keys.luac");
  EXPECT_TRUE(completed);
}

#endif
