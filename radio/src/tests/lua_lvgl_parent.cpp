/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "gtests.h"

#if defined(LUA) && defined(COLORLCD)

#include "lua/lua_widget.h"
#include "mainwindow.h"
#include "location.h"

#include <fstream>
#include <iterator>

namespace {

class ParentTestManager : public LuaScriptManager
{
 public:
  Window root{MainWindow::instance(), {0, 0, LCD_W, LCD_H}};
  bool fullscreen = true;

  Window* getCurrentParent() const override
  {
    return tempParent ? tempParent->getWindow() : const_cast<Window*>(&root);
  }
  void clear() override { root.clear(); }
  bool useLvglLayout() const override { return true; }
  bool isAppMode() const override { return false; }
  bool isWidget() override { return false; }
  bool isFullscreen() override { return fullscreen; }
  void luaShowError() override { ADD_FAILURE() << "Unexpected callback error"; }
};

class LuaLvglParent : public testing::Test
{
 protected:
  ParentTestManager manager;
  LuaScriptManager* previousManager = nullptr;
  lua_State* previousWidgets = nullptr;
  lua_State* L = nullptr;

  void SetUp() override
  {
    previousManager = luaScriptManager;
    previousWidgets = lsWidgets;
    L = luaL_newstate();
    ASSERT_NE(L, nullptr);
    lsWidgets = L;
    luaRegisterLibraries(L);
    luaScriptManager = &manager;
  }

  void TearDown() override
  {
    manager.setTempParent(nullptr);
    manager.clearRefs(L);
    manager.clear();
    luaScriptManager = previousManager;
    lua_close(L);
    lsWidgets = previousWidgets;
  }

  void run(const char* source)
  {
    ASSERT_EQ(luaL_loadbuffer(L, source, strlen(source), "@lvgl-parent.lua"), LUA_OK);
    ASSERT_EQ(lua_pcall(L, 0, 0, 0), LUA_OK) << lua_tostring(L, -1);
    EXPECT_EQ(lua_gettop(L), 0);
  }

  lv_obj_t* rootObject() { return manager.root.getLvObj(); }
};

TEST_F(LuaLvglParent, ConstructorErrorRestoresRoot)
{
  run("parent = lvgl.box({w=200,h=120})\n"
      "ok, err = pcall(function() lvgl.label(parent, {bogus=1}) end)\n"
      "assert(not ok and err == \"lvgl-parent.lua:2: Invalid property 'bogus'\", err)\n"
      "lvgl.label({text='root'})\n");
  EXPECT_EQ(manager.getTempParent(), nullptr);
  ASSERT_EQ(lv_obj_get_child_cnt(rootObject()), 2u);
  EXPECT_EQ(lv_obj_get_child_cnt(lv_obj_get_child(rootObject(), 0)), 0u);
  EXPECT_EQ(lv_obj_get_parent(lv_obj_get_child(rootObject(), 1)), rootObject());
}

TEST_F(LuaLvglParent, BuildErrorRestoresRoot)
{
  run("parent = lvgl.box({w=200,h=120})\n"
      "ok, err = pcall(function() lvgl.build(parent, {{type='bogus'}}) end)\n"
      "assert(not ok and err == \"lvgl-parent.lua:2: Invalid type 'bogus'\", err)\n"
      "lvgl.label({text='root'})\n");
  EXPECT_EQ(manager.getTempParent(), nullptr);
  EXPECT_EQ(lv_obj_get_child_cnt(rootObject()), 2u);
}

TEST_F(LuaLvglParent, ConstructorChildrenErrorRestoresRoot)
{
  run("ok, err = pcall(function()\n"
      "  lvgl.box({children={{type='box', children={{type='bogus'}}}}})\n"
      "end)\n"
      "assert(not ok and err == \"lvgl-parent.lua:2: Invalid type 'bogus'\", err)\n"
      "lvgl.label({text='root'})\n");
  EXPECT_EQ(manager.getTempParent(), nullptr);
  EXPECT_EQ(lv_obj_get_child_cnt(rootObject()), 2u);
}

TEST_F(LuaLvglParent, BuildChildrenErrorRestoresRoot)
{
  run("ok, err = pcall(function()\n"
      "  lvgl.build({{type='box', children={{type='box', children={{type='bogus'}}}}}})\n"
      "end)\n"
      "assert(not ok and err == \"lvgl-parent.lua:2: Invalid type 'bogus'\", err)\n"
      "lvgl.label({text='root'})\n");
  EXPECT_EQ(manager.getTempParent(), nullptr);
  EXPECT_EQ(lv_obj_get_child_cnt(rootObject()), 2u);
}

TEST_F(LuaLvglParent, PageChildrenErrorRestoresRoot)
{
  run("ok, err = pcall(function()\n"
      "  lvgl.page({children={{type='bogus'}}})\n"
      "end)\n"
      "assert(not ok and err == \"lvgl-parent.lua:2: Invalid type 'bogus'\", err)\n"
      "lvgl.label({text='root'})\n");
  EXPECT_EQ(manager.getTempParent(), nullptr);
  EXPECT_EQ(lv_obj_get_child_cnt(rootObject()), 2u);
}

TEST_F(LuaLvglParent, SuccessfulConstructionPreservesReturnsAndParent)
{
  run("parent = lvgl.box({w=200,h=120})\n"
      "assert(type(lvgl.label(parent, {text='child'})) == 'userdata')\n"
      "refs = lvgl.build(parent, {{type='box', name='box', children={\n"
      "  {type='label', name='child', text='nested'}\n"
      "}}})\n"
      "assert(type(refs) == 'table' and refs.box and refs.child)\n"
      "lvgl.label({text='root'})\n");
  EXPECT_EQ(manager.getTempParent(), nullptr);
  ASSERT_EQ(lv_obj_get_child_cnt(rootObject()), 2u);
  auto parent = lv_obj_get_child(rootObject(), 0);
  ASSERT_EQ(lv_obj_get_child_cnt(parent), 2u);
  EXPECT_EQ(lv_obj_get_child_cnt(lv_obj_get_child(parent, 1)), 1u);
}

TEST_F(LuaLvglParent, ErrorRestoresNonRootParent)
{
  run("outer = lvgl.box({})\n"
      "parent = lvgl.box({})\n");
  lua_getglobal(L, "outer");
  auto outer = LvglWidgetObjectBase::checkLvgl(L, -1, true);
  lua_pop(L, 1);
  manager.setTempParent(outer);
  run("assert(not pcall(function() lvgl.label(parent, {bogus=1}) end))\n"
      "lvgl.label({text='outer child'})\n");
  EXPECT_EQ(manager.getTempParent(), outer);
  EXPECT_EQ(lv_obj_get_child_cnt(outer->getWindow()->getLvObj()), 1u);
}

TEST_F(LuaLvglParent, BuildErrorRestoresNonRootParent)
{
  run("outer = lvgl.box({})\n"
      "parent = lvgl.box({})\n");
  lua_getglobal(L, "outer");
  auto outer = LvglWidgetObjectBase::checkLvgl(L, -1, true);
  lua_pop(L, 1);
  manager.setTempParent(outer);
  run("assert(not pcall(function()\n"
      "  lvgl.build(parent, {{type='box', children={{type='bogus'}}}})\n"
      "end))\n"
      "lvgl.label({text='outer child'})\n");
  EXPECT_EQ(manager.getTempParent(), outer);
  EXPECT_EQ(lv_obj_get_child_cnt(outer->getWindow()->getLvObj()), 1u);
}

TEST_F(LuaLvglParent, DisabledConstructorsKeepNilReturn)
{
  manager.fullscreen = false;
  run("assert(lvgl.button({}) == nil)\n"
      "assert(lvgl.page({}) == nil)\n");
  EXPECT_EQ(manager.getTempParent(), nullptr);
  EXPECT_EQ(lv_obj_get_child_cnt(rootObject()), 0u);
}

TEST_F(LuaLvglParent, MissingManagerKeepsNilReturn)
{
  luaScriptManager = nullptr;
  run("assert(lvgl.label({}) == nil)\n"
      "assert(lvgl.page({}) == nil)\n"
      "assert(lvgl.build({}) == nil)\n");
  luaScriptManager = &manager;
  EXPECT_EQ(lv_obj_get_child_cnt(rootObject()), 0u);
}

static void raiseConstructionErrorObject(lua_State* L, lua_Debug* frame)
{
  lua_getinfo(L, "Su", frame);
  if (!strcmp(frame->what, "C") && frame->nups == 1) {
    // Interrupt the protected invocation, not the public entry point.
    lua_sethook(L, nullptr, 0, 0);
    lua_getglobal(L, "constructionError");
    lua_error(L);
  }
}

TEST_F(LuaLvglParent, NonStringErrorKeepsIdentity)
{
  run("constructionError = {}\n");
  lua_sethook(L, raiseConstructionErrorObject, LUA_MASKCALL, 0);
  run("local ok, err = pcall(function() lvgl.label({text='unused'}) end)\n"
      "assert(not ok and rawequal(err, constructionError))\n");
  lua_sethook(L, nullptr, 0, 0);
  EXPECT_EQ(manager.getTempParent(), nullptr);
  EXPECT_EQ(lv_obj_get_child_cnt(rootObject()), 0u);
}

TEST_F(LuaLvglParent, StandaloneReproducer)
{
  std::ifstream file(TESTS_PATH "/lua_lvgl_parent.lua");
  ASSERT_TRUE(file);
  std::string source{std::istreambuf_iterator<char>(file), {}};
  ASSERT_EQ(luaL_loadbuffer(L, source.data(), source.size(),
                           "@lua_lvgl_parent.lua"), LUA_OK);
  ASSERT_EQ(lua_pcall(L, 0, 1, 0), LUA_OK) << lua_tostring(L, -1);
  lua_getfield(L, -1, "init");
  ASSERT_EQ(lua_pcall(L, 0, 0, 0), LUA_OK) << lua_tostring(L, -1);
  lua_pop(L, 1);
  EXPECT_EQ(manager.getTempParent(), nullptr);
  ASSERT_EQ(lv_obj_get_child_cnt(rootObject()), 3u);
  EXPECT_STREQ(lv_label_get_text(lv_obj_get_child(rootObject(), 0)),
               "This label should remain visible");
  EXPECT_TRUE(lv_obj_has_flag(lv_obj_get_child(rootObject(), 1),
                             LV_OBJ_FLAG_HIDDEN));
  EXPECT_STREQ(lv_label_get_text(lv_obj_get_child(rootObject(), 2)),
               "This label should ALSO remain visible");
}

}  // namespace

#endif
