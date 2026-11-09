/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "gtests.h"

#if defined(LUA) && defined(COLORLCD)

#include "lua/lua_widget.h"
#include "lua/lua_states.h"
#include "mainwindow.h"
#include "location.h"
#include <algorithm>
#include <fstream>
#include <iterator>

class LuaLvglRollback : public testing::Test, public LuaScriptManager
{
 protected:
  lua_State* L = nullptr;
  Window* root = nullptr;
  LuaScriptManager* previous = nullptr;
  std::list<Window*> originalWindows;

  void SetUp() override
  {
    L = luaL_newstate();
    luaRegisterLibraries(L);
    originalWindows = MainWindow::instance()->getChildren();
    root = new Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H});
    previous = luaScriptManager;
    luaScriptManager = this;
  }

  void TearDown() override
  {
    setTempParent(nullptr);
    clear();
    root->closeWindow();
    auto windows = MainWindow::instance()->getChildren();
    for (auto window : windows) {
      if (std::find(originalWindows.begin(), originalWindows.end(), window) ==
          originalWindows.end())
        window->closeWindow();
    }
    lua_gc(L, LUA_GCCOLLECT, 0);
    lua_close(L);
    luaScriptManager = previous;
    MainWindow::instance()->run();
  }

 public:
  Window* getCurrentParent() const override
  {
    return tempParent ? tempParent->getWindow() : root;
  }
  void clear() override { clearRefs(L); root->clear(); }
  bool useLvglLayout() const override { return true; }
  bool isAppMode() const override { return false; }
  bool isWidget() override { return false; }
  bool isFullscreen() override { return true; }
  void luaShowError() override { ADD_FAILURE() << "Unexpected callback error"; }
};

TEST_F(LuaLvglRollback, StandaloneReproducer)
{
  std::ifstream file(TESTS_PATH "/lua/lvgl_container_rollback.lua");
  ASSERT_TRUE(file.is_open());
  std::string script((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
  ASSERT_EQ(LUA_OK, luaL_dostring(L, script.c_str()))
      << lua_tostring(L, -1);
  lua_getfield(L, -1, "init");
  ASSERT_EQ(LUA_OK, lua_pcall(L, 0, 0, 0)) << lua_tostring(L, -1);
  EXPECT_EQ(1u, lv_obj_get_child_cnt(root->getLvObj()));
  EXPECT_TRUE(root->getChildren().empty());
}

TEST_F(LuaLvglRollback, ReleasesSubtreeCallbacks)
{
  ASSERT_EQ(LUA_OK, luaL_dostring(L, R"lua(
    weak = setmetatable({}, {__mode = "v"})
    do
      local callback = function() return "child" end
      weak[1] = callback
      local ok, err = pcall(function()
        lvgl.box({children = {
          {type = "box", children = {{type = "label", text = callback}}},
          {type = "not-a-widget"}
        }})
      end)
      assert(not ok and string.find(err, "Invalid type 'not%-a%-widget'"))
    end
    collectgarbage("collect")
    assert(weak[1] == nil, "failed subtree retained callback")
  )lua")) << lua_tostring(L, -1);
  EXPECT_EQ(0u, lv_obj_get_child_cnt(root->getLvObj()));
  EXPECT_TRUE(root->getChildren().empty());
}

TEST_F(LuaLvglRollback, BuildKeepsCompletedTopLevelSiblings)
{
  ASSERT_EQ(LUA_OK, luaL_dostring(L, R"lua(
    assert(not pcall(function()
      lvgl.build({
        {type = "label", text = "keep"},
        {type = "box", children = {
          {type = "label", text = "discard"},
          {type = "not-a-widget"}
        }}
      })
    end))
  )lua")) << lua_tostring(L, -1);
  EXPECT_EQ(1u, lv_obj_get_child_cnt(root->getLvObj()));
  EXPECT_TRUE(root->getChildren().empty());
}

TEST_F(LuaLvglRollback, PageClosesOuterWindow)
{
  auto count = lv_obj_get_child_cnt(root->getLvObj());
  ASSERT_EQ(LUA_OK, luaL_dostring(L, R"lua(
    assert(not pcall(function()
      lvgl.page({title = "Failed page", children = {
        {type = "label", text = "discard"},
        {type = "not-a-widget"}
      }})
    end))
  )lua")) << lua_tostring(L, -1);
  EXPECT_EQ(count, lv_obj_get_child_cnt(root->getLvObj()));
  EXPECT_TRUE(root->getChildren().empty());
}

TEST_F(LuaLvglRollback, DialogClosesOuterWindow)
{
  auto count = lv_obj_get_child_cnt(MainWindow::instance()->getLvObj());
  ASSERT_EQ(LUA_OK, luaL_dostring(L, R"lua(
    assert(not pcall(function()
      lvgl.dialog({title = "Failed dialog", children = {
        {type = "label", text = "discard"},
        {type = "not-a-widget"}
      }})
    end))
  )lua")) << lua_tostring(L, -1);
  EXPECT_EQ(count, lv_obj_get_child_cnt(MainWindow::instance()->getLvObj()));
}

TEST_F(LuaLvglRollback, SuccessfulNestedBuildAndTeardown)
{
  ASSERT_EQ(LUA_OK, luaL_dostring(L, R"lua(
    local refs = lvgl.build({
      {type = "box", name = "outer", children = {
        {type = "box", name = "inner", children = {
          {type = "label", name = "text", text = "built"}
        }}
      }}
    })
    assert(refs.outer and refs.inner and refs.text)
    refs.text:set({text = "updated"})
    assert(lvgl.box(refs.inner, {children = {{type = "label", text = "direct"}}}))
  )lua")) << lua_tostring(L, -1);
  EXPECT_EQ(1u, lv_obj_get_child_cnt(root->getLvObj()));
  ASSERT_EQ(1u, root->getChildren().size());
  auto outer = root->getChildren().front();
  ASSERT_EQ(1u, outer->getChildren().size());
  auto inner = outer->getChildren().front();
  EXPECT_EQ(2u, lv_obj_get_child_cnt(inner->getLvObj()));
  clear();
  EXPECT_EQ(0u, lv_obj_get_child_cnt(root->getLvObj()));
  EXPECT_TRUE(root->getChildren().empty());
  EXPECT_TRUE(getLvglObjectRefs().empty());
}

TEST_F(LuaLvglRollback, RepeatedFailureReleasesRefsBeforeNextConstruction)
{
  ASSERT_EQ(LUA_OK, luaL_dostring(L, R"lua(
    for i = 1, 20 do
      assert(not pcall(function()
        lvgl.box({children = {
          {type = "label", text = function() return "discard" end},
          {type = "not-a-widget"}
        }})
      end))
      local valid = lvgl.label({text = "keep"})
      valid:set({text = "still valid"})
      lvgl.clear()
      collectgarbage("collect")
    end
  )lua")) << lua_tostring(L, -1);
  EXPECT_EQ(0u, lv_obj_get_child_cnt(root->getLvObj()));
  EXPECT_TRUE(root->getChildren().empty());
  EXPECT_TRUE(getLvglObjectRefs().empty());
}

#endif
