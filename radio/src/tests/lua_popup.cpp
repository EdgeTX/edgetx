/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "gtests.h"

#if defined(LUA) && defined(COLORLCD)

#include "dialog.h"
#include "lua/lua_widget.h"
#include "mainwindow.h"
#include "standalone_lua.h"
#include "view_main.h"

extern lua_State* lsStandalone;

class LuaPopup : public ::testing::Test
{
 protected:
  void SetUp() override
  {
    ViewMain::instance();
    MainWindow::instance()->run();
    ASSERT_EQ(nullptr, lsStandalone);
    lsStandalone = luaL_newstate();
    ASSERT_NE(nullptr, lsStandalone);
    luaRegisterLibraries(lsStandalone);
  }

  void TearDown() override
  {
    if (StandaloneLuaWindow::instance()) {
      StandaloneLuaWindow::instance()->clear();
      StandaloneLuaWindow::instance()->closeWindow();
    } else {
      luaClose(&lsStandalone);
    }
    MainWindow::instance()->run();
  }

  ::testing::AssertionResult exec(const char* script)
  {
    if (luaL_dostring(lsStandalone, script) != LUA_OK)
      return ::testing::AssertionFailure() << lua_tostring(lsStandalone, -1);
    return ::testing::AssertionSuccess();
  }

  void update()
  {
    static_cast<Window*>(StandaloneLuaWindow::instance())->checkEvents();
    MainWindow::instance()->run();
  }

  void start()
  {
    ASSERT_TRUE(exec("return function() return 0 end"));
    int run = luaL_ref(lsStandalone, LUA_REGISTRYINDEX);
    StandaloneLuaWindow::setup(true, LUA_REFNIL, run);
  }
};

TEST_F(LuaPopup, CancelReleasesCallbackAndCapturedData)
{
  start();
  ASSERT_TRUE(exec(R"lua(
    weak = setmetatable({}, {__mode = 'v'})
    closed = false
    do
      local data = {string.rep('x', 4096)}
      local callback = function()
        assert(#data[1] == 4096)
        closed = true
      end
      weak[1], weak[2] = callback, data
      lvgl.confirm({title = 'Reproduction', message = 'Cancel this popup',
                    confirm = callback, cancel = callback})
    end
    collectgarbage('collect')
    assert(weak[1] ~= nil and weak[2] ~= nil)
  )lua"));

  auto popup = dynamic_cast<ConfirmDialog*>(Window::topWindow());
  ASSERT_NE(nullptr, popup);
  static_cast<Window*>(popup)->onCancel();
  ASSERT_TRUE(exec("assert(closed)"));
  // An opaque native overlay delays the script update, but not native deletion.
  auto overlay = new MessageDialog("Native overlay", "Pause the script");
  MainWindow::instance()->run();
  ASSERT_TRUE(exec("collectgarbage('collect'); assert(weak[1] ~= nil)"));
  overlay->closeWindow();
  update();
  ASSERT_TRUE(exec("collectgarbage('collect')"));
  EXPECT_TRUE(exec("assert(weak[1] == nil, 'dismissed popup leaked callback memory')"));
  EXPECT_TRUE(exec("assert(weak[2] == nil, 'dismissed popup leaked captured data')"));
}

TEST_F(LuaPopup, ClearClosesLivePopupsAndReleasesCallbacks)
{
  start();
  ASSERT_TRUE(exec(R"lua(
    weak = setmetatable({}, {__mode = 'v'})
    calls = 0
    do
      local callback = function() calls = calls + 1; return 1 end
      weak[1] = callback
      lvgl.confirm({title = 'Confirm', cancel = callback, confirm = callback})
      lvgl.message({title = 'Message', visible = callback})
      lvgl.menu({title = 'Menu', values = {'One'}, get = callback, set = callback})
    end
    collectgarbage('collect')
    assert(weak[1] ~= nil)
    beforeClear = calls
    lvgl.clear()
    collectgarbage('collect')
    assert(weak[1] == nil, 'clearing live popups leaked callbacks')
    assert(calls == beforeClear, 'teardown invoked a popup callback')
  )lua"));
  EXPECT_EQ(StandaloneLuaWindow::instance(), Window::topWindow());
  update();
}

TEST_F(LuaPopup, MessageAndMenuDismissalReleasesCallbacks)
{
  start();
  for (const char* constructor : {"message", "menu"}) {
    std::string script = R"lua(
      weak = setmetatable({}, {__mode = 'v'})
      do
        local callback = function() return 1 end
        weak[1] = callback
        lvgl.)lua";
    script += constructor;
    script += R"lua(({title = 'Popup', visible = callback})
      end
      collectgarbage('collect')
      assert(weak[1] ~= nil)
    )lua";
    ASSERT_TRUE(exec(script.c_str()));
    ASSERT_NE(StandaloneLuaWindow::instance(), Window::topWindow());
    Window::topWindow()->onCancel();
    MainWindow::instance()->run();
    update();
    ASSERT_TRUE(exec("collectgarbage('collect'); assert(weak[1] == nil)"));
  }
}

TEST_F(LuaPopup, CancelCallbackCanOpenAnotherPopup)
{
  start();
  ASSERT_TRUE(exec(R"lua(
    weak = setmetatable({}, {__mode = 'v'})
    do
      local callback = function()
        lvgl.message({title = 'Opened from cancel'})
      end
      weak[1] = callback
      lvgl.confirm({title = 'Confirm', cancel = callback})
    end
  )lua"));
  auto popup = dynamic_cast<ConfirmDialog*>(Window::topWindow());
  ASSERT_NE(nullptr, popup);
  static_cast<Window*>(popup)->onCancel();
  EXPECT_NE(nullptr, dynamic_cast<MessageDialog*>(Window::topWindow()));
  update();
  ASSERT_TRUE(exec("collectgarbage('collect'); assert(weak[1] == nil)"));
  Window::topWindow()->onCancel();
  MainWindow::instance()->run();
}

TEST_F(LuaPopup, ScriptExitAfterClearingLivePopup)
{
  start();
  ASSERT_TRUE(exec(R"lua(
    lvgl.confirm({title = 'Exit', cancel = function() error('must not run') end})
    lvgl.clear()
  )lua"));
  StandaloneLuaWindow::instance()->closeWindow();
  EXPECT_EQ(nullptr, lsStandalone);
  EXPECT_NE(nullptr, Window::topWindow());
  EXPECT_EQ(ViewMain::instance(), Window::topWindow());
}

TEST_F(LuaPopup, StandaloneReproduction)
{
  ASSERT_EQ(LUA_OK, luaL_dofile(lsStandalone, "/lua/popup_callback_gc.lua"))
      << lua_tostring(lsStandalone, -1);
  lua_getfield(lsStandalone, -1, "init");
  int init = luaL_ref(lsStandalone, LUA_REGISTRYINDEX);
  lua_getfield(lsStandalone, -1, "run");
  int run = luaL_ref(lsStandalone, LUA_REGISTRYINDEX);
  lua_pop(lsStandalone, 1);
  StandaloneLuaWindow::setup(true, init, run);
  MainWindow::instance()->run();

  auto popup = dynamic_cast<ConfirmDialog*>(Window::topWindow());
  ASSERT_NE(nullptr, popup);
  static_cast<Window*>(popup)->onCancel();
  for (int i = 0; i < 3; ++i)
    MainWindow::instance()->run();

  EXPECT_EQ(nullptr, StandaloneLuaWindow::instance())
      << "the standalone script must pass its GC assertions and exit";
}

#endif
