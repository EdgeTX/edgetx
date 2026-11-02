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

#include "gtests.h"

#if defined(LUA) && defined(COLORLCD)

#include "edgetx.h"
#include "lua/api_colorlcd.h"
#include "lua/lua_widget.h"
#include "mainwindow.h"
#include "view_main.h"
#include "numberedit.h"

extern void lv_stb_init();
extern int custom_lua_atpanic(lua_State* L);

namespace
{

class ErrorScriptManager : public LuaScriptManager
{
 public:
  Window* root = nullptr;
  lua_State* L = nullptr;

  Window* getCurrentParent() const override
  {
    return (tempParent && tempParent->getWindow()) ? tempParent->getWindow()
                                                    : root;
  }
  void clear() override
  {
    if (L) clearRefs(L);
    root->clear();
  }
  bool useLvglLayout() const override { return true; }
  bool isAppMode() const override { return false; }
  bool isWidget() override { return true; }
  bool isFullscreen() override { return true; }
  void luaShowError() override { errors += 1; }
  int errors = 0;
};

class CountingBox : public LvglWidgetBox
{
 public:
  explicit CountingBox(unsigned& destructions) : destructions(destructions) {}
  ~CountingBox() override { ++destructions; }
  using LvglWidgetBox::pcallGetStringVal;

 private:
  unsigned& destructions;
};

class LuaLvglObjectErrors : public ::testing::Test
{
 protected:
  lua_State* L = nullptr;
  LuaScriptManager* previousManager = nullptr;
  our_longjmp* previousPanicChain = nullptr;
  ErrorScriptManager manager;
  unsigned boxDestructions = 0;

  static void SetUpTestSuite() { lv_stb_init(); }

  void SetUp() override
  {
    previousPanicChain = global_lj;
    L = luaL_newstate();
    ASSERT_NE(L, nullptr);
    lua_atpanic(L, &custom_lua_atpanic);
    luaRegisterLibraries(L);
    manager.root = new Window(nullptr, {0, 0, 200, 200});
    manager.L = L;
    previousManager = luaScriptManager;
    luaScriptManager = &manager;
  }

  void TearDown() override
  {
    EXPECT_EQ(global_lj, previousPanicChain);
    EXPECT_EQ(manager.errors, 0);
    manager.clearRefs(L);
    manager.root->closeWindow();
    MainWindow::instance()->run();
    lua_close(L);
    luaScriptManager = previousManager;
  }

  ::testing::AssertionResult exec(const std::string& code)
  {
    if (luaL_dostring(L, code.c_str()))
      return ::testing::AssertionFailure() << lua_tostring(L, -1);
    return ::testing::AssertionSuccess();
  }

  CountingBox* createCountedBox()
  {
    auto object = new CountingBox(boxDestructions);
    lua_newtable(L);
    object->create(L, -1);
    lua_pop(L, 1);
    object->push(L);
    lua_setglobal(L, "box");
    return object;
  }
};

TEST_F(LuaLvglObjectErrors, FailedConstructorRestoresTheParent)
{
  ASSERT_TRUE(exec("box=lvgl.box({x=0,y=0,w=100,h=100});"
                   "assert(not pcall(lvgl.label,box,{bogus=1}));"
                   "top=lvgl.box({x=0,y=0,w=10,h=10})"));
  lua_getglobal(L, "box");
  auto boxWindow = LvglWidgetObjectBase::checkLvgl(L, -1, true)->getWindow();
  lua_pop(L, 1);
  lua_getglobal(L, "top");
  auto topWindow = LvglWidgetObjectBase::checkLvgl(L, -1, true)->getWindow();
  lua_pop(L, 1);
  EXPECT_NE(topWindow->getParent(), boxWindow);
  EXPECT_EQ(topWindow->getParent(), manager.root);
}

TEST_F(LuaLvglObjectErrors, FailedChildrenLeaveNoWindowBehind)
{
  ASSERT_TRUE(exec("box=lvgl.box({x=0,y=0,w=100,h=100});"
                   "assert(not pcall(lvgl.box,box,{x=0,y=0,w=10,h=10,"
                   "children={{type='bogus'}}}));"
                   "other=lvgl.box({x=0,y=0,w=10,h=10})"));
  lua_getglobal(L, "box");
  auto boxWindow = LvglWidgetObjectBase::checkLvgl(L, -1, true)->getWindow();
  lua_pop(L, 1);
  MainWindow::instance()->run();
  EXPECT_EQ(boxWindow->getChildren().size(), 0u);
  lua_getglobal(L, "other");
  auto other = LvglWidgetObjectBase::checkLvgl(L, -1, true)->getWindow();
  lua_pop(L, 1);
  EXPECT_EQ(other->getParent(), manager.root);
}

TEST_F(LuaLvglObjectErrors, FailedLineCallbacksLeaveNoLvglObjects)
{
  ASSERT_TRUE(exec("assert(not pcall(lvgl.line,{"
                   "pts={{0,0},{10,10}},pos=function() return {},0 end}))"));
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 0u);
}

TEST_F(LuaLvglObjectErrors, FailedLabelCallbacksLeaveNoLvglObjects)
{
  ASSERT_TRUE(exec("assert(not pcall(lvgl.label,{"
                   "text=function() return {} end}))"));
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 0u);
}

TEST_F(LuaLvglObjectErrors, FailedPageLeavesNoPageBehind)
{
  ASSERT_TRUE(exec("ran=0; assert(not pcall(lvgl.page,{title='t',"
                   "back=function() ran=ran+1 end,"
                   "children={{type='bogus'}}}))"));
  EXPECT_EQ(manager.root->getChildren().size(), 0u);
  EXPECT_TRUE(exec("assert(ran == 0)"));
}

TEST_F(LuaLvglObjectErrors, FailedDialogLeavesNoDialogBehind)
{
  auto top = Window::topWindow();
  ASSERT_TRUE(exec("ran=0; assert(not pcall(lvgl.dialog,{title='t',"
                   "close=function() ran=ran+1 end,"
                   "children={{type='bogus'}}}))"));
  EXPECT_EQ(Window::topWindow(), top);
  EXPECT_TRUE(exec("assert(ran == 0)"));
}

TEST_F(LuaLvglObjectErrors, PageParamsComeFromTheFirstArgument)
{
  ASSERT_TRUE(exec("pg=lvgl.page({title='t',"
                   "children={{type='label',text='x'}}}, 5);"
                   "assert(pg ~= nil)"));
  lua_getglobal(L, "pg");
  auto body = LvglWidgetObjectBase::checkLvgl(L, -1, true)->getWindow();
  lua_pop(L, 1);
  EXPECT_EQ(lv_obj_get_child_cnt(body->getLvObj()), 1u);
}

TEST_F(LuaLvglObjectErrors, FailedBuildRestoresTheParent)
{
  ASSERT_TRUE(exec("box=lvgl.box({x=0,y=0,w=100,h=100});"
                   "assert(not pcall(lvgl.build,box,{{type='bogus'}}));"
                   "top=lvgl.box({x=0,y=0,w=10,h=10})"));
  lua_getglobal(L, "box");
  auto boxWindow = LvglWidgetObjectBase::checkLvgl(L, -1, true)->getWindow();
  lua_pop(L, 1);
  lua_getglobal(L, "top");
  auto topWindow = LvglWidgetObjectBase::checkLvgl(L, -1, true)->getWindow();
  lua_pop(L, 1);
  EXPECT_EQ(topWindow->getParent(), manager.root);
  EXPECT_NE(topWindow->getParent(), boxWindow);
}

TEST_F(LuaLvglObjectErrors, ConstructorErrorsKeepTheScriptLocation)
{
  ASSERT_TRUE(exec("local ok, msg = pcall(function() lvgl.label({bogus=1}) end)\n"
                   "assert(not ok)\n"
                   "assert(string.find(msg, 'Invalid property', 1, true), msg)\n"
                   "assert(string.find(msg, '^%[string.-%]:1: '), msg)"));
}

TEST_F(LuaLvglObjectErrors, SuccessfulCreationStillReturnsTheObject)
{
  ASSERT_TRUE(exec("box=lvgl.box({x=0,y=0,w=100,h=100});"
                   "lbl=box:label({x=1,y=1,text='x'}); assert(lbl ~= nil);"
                   "lbl:set({text='y'});"
                   "local t=lvgl.build(box,{{type=lvgl.BOX,name='b',w=10,h=10,"
                   "children={{type=lvgl.LABEL,name='c',text='z'}}}});"
                   "assert(t.b ~= nil)"));
}

TEST_F(LuaLvglObjectErrors, MetadataErrorsNeverChangeTheParent)
{
  ASSERT_TRUE(exec("box=lvgl.box({w=100,h=100});"
                   "assert(not pcall(lvgl.label,box,7));"
                   "assert(not pcall(lvgl.label,box,{name={}}));"
                   "assert(not pcall(lvgl.label,box,{type='bogus'}));"
                   "assert(not pcall(lvgl.label,box,{[true]=1}));"
                   "top=lvgl.box({w=10,h=10})"));
  lua_getglobal(L, "top");
  auto top = LvglWidgetObjectBase::checkLvgl(L, -1, true)->getWindow();
  lua_pop(L, 1);
  EXPECT_EQ(top->getParent(), manager.root);
}

TEST_F(LuaLvglObjectErrors, NestedBuildErrorsReleaseCallbacksAndWindows)
{
  ASSERT_TRUE(exec("weak=setmetatable({},{__mode='v'});"
                   "local callback=function() return true end; weak[1]=callback;"
                   "local params={{type=lvgl.BOX,w=100,h=100,children={"
                   "{type=lvgl.LINE,pts={{0,0},{10,10}},visible=callback},"
                   "{type=lvgl.LINE,pts={{0,0},{10,10}},visible=callback,"
                   "pos=function() return {},0 end}}}};"
                   "assert(not pcall(lvgl.build,params))"));
  EXPECT_EQ(manager.root->getChildren().size(), 0u);
  EXPECT_TRUE(manager.getLvglObjectRefs().empty());
  lua_gc(L, LUA_GCCOLLECT, 0);
  ASSERT_TRUE(exec("assert(weak[1] == nil)"));
}

TEST_F(LuaLvglObjectErrors, DeepBuildReturnsAllNamedDescendants)
{
  ASSERT_TRUE(exec("local t=lvgl.build({{type=lvgl.BOX,name='a',w=100,h=100,"
                   "children={{type=lvgl.BOX,name='b',w=80,h=80,"
                   "children={{type=lvgl.BOX,name='c',w=40,h=30}}}}}});"
                   "assert(t.a ~= nil and t.b ~= nil and t.c ~= nil);"
                   "t.c:set({x=2})"));
}

TEST_F(LuaLvglObjectErrors, ClearFromACallbackIsRejectedWithoutDeletingObjects)
{
  ASSERT_TRUE(exec("box=lvgl.box({w=100,h=100});"
                   "assert(not pcall(lvgl.line,{pts={{0,0},{10,10}},"
                   "pos=function() lvgl.clear(); return 0,0 end}));"
                   "box:set({x=2})"));
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 1u);
  EXPECT_FALSE(manager.inCallback());
}

TEST_F(LuaLvglObjectErrors, CallbackReplacementsReleasePreviousReferences)
{
  ASSERT_TRUE(exec("weak=setmetatable({},{__mode='v'});"
                   "box=lvgl.box({w=80,h=60});"
                   "for i=1,24 do local fn=function() return 80,60 end;"
                   "weak[i]=fn; box:set({size=fn}) end"));
  lua_gc(L, LUA_GCCOLLECT, 0);
  ASSERT_TRUE(exec("for i=1,23 do assert(weak[i] == nil) end;"
                   "assert(weak[24] ~= nil); lvgl.clear(); box=nil"));
  lua_gc(L, LUA_GCCOLLECT, 0);
  ASSERT_TRUE(exec("assert(weak[24] == nil)"));
}

TEST_F(LuaLvglObjectErrors, LiteralValuesReplaceDynamicCallbacks)
{
  ASSERT_TRUE(exec("local fn=function() return 'dynamic' end;"
                   "weak=setmetatable({fn},{__mode='v'});"
                   "label=lvgl.label({text=fn}); label:set({text='literal'})"));
  lua_gc(L, LUA_GCCOLLECT, 0);
  ASSERT_TRUE(exec("assert(weak[1] == nil)"));
  ASSERT_TRUE(manager.callRefs(L));
}

TEST_F(LuaLvglObjectErrors, InvalidPointUpdatesKeepTheLiveLineBuffer)
{
  ASSERT_TRUE(exec("line=lvgl.line({pts={{0,0},{10,10}}});"
                   "assert(not pcall(line.set,line,{pts={{0,0},{5,5},{'bad',9}}}));"
                   "line:set({color=123})"));
  MainWindow::instance()->run();
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 1u);
}

TEST_F(LuaLvglObjectErrors, LazyLineHandlesExpireWithTheirParent)
{
  ASSERT_TRUE(exec("box=lvgl.box({w=100,h=100}); line=lvgl.line(box,{})"));
  lua_getglobal(L, "box");
  auto box = LvglWidgetObjectBase::checkLvgl(L, -1, true)->getWindow();
  lua_pop(L, 1);
  box->closeWindow();
  MainWindow::instance()->run();
  ASSERT_TRUE(exec("assert(not pcall(line.set,line,{pts={{0,0},{10,10}}}))"));
}

TEST_F(LuaLvglObjectErrors, InvalidNestedKeysAndValuesRaiseLuaErrors)
{
  ASSERT_TRUE(exec("assert(not pcall(lvgl.box,{borderPad={[false]=1}}));"
                   "assert(not pcall(lvgl.page,{prevButton={[false]=1}}));"
                   "assert(not pcall(lvgl.page,{nextButton={[false]=1}}));"
                   "assert(not pcall(lvgl.choice,{values={false}}));"
                   "assert(not pcall(lvgl.choice,{values={{}}}))"));
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 0u);
  EXPECT_TRUE(manager.getLvglObjectRefs().empty());
}

TEST_F(LuaLvglObjectErrors, NonStringCallbackErrorsAreReportedSafely)
{
  ASSERT_TRUE(exec("local ok,msg=pcall(lvgl.label,{"
                   "text=function() error(42) end});"
                   "assert(not ok and type(msg)=='string');"
                   "assert(string.find(msg,'42',1,true),msg);"
                   "ok,msg=pcall(lvgl.label,{"
                   "text=function() error({}) end});"
                   "assert(not ok and type(msg)=='string');"
                   "assert(string.find(msg,'table',1,true),msg)"));
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 0u);
  EXPECT_FALSE(manager.inCallback());
}

TEST_F(LuaLvglObjectErrors, ImageCallbackErrorsAreReturnedWithoutThrowing)
{
  ASSERT_TRUE(exec("image=lvgl.image({w=80,h=60});"
                   "image:set({file=function() return {} end})"));
  int top = lua_gettop(L);
  EXPECT_FALSE(manager.callRefs(L));
  ASSERT_EQ(lua_gettop(L), top + 1);
  EXPECT_TRUE(lua_isstring(L, -1));
  lua_pop(L, 1);
  EXPECT_FALSE(manager.inCallback());
  EXPECT_EQ(luaScriptManager, &manager);
  EXPECT_EQ(global_lj, previousPanicChain);
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 1u);
  ASSERT_TRUE(exec("image:set({file=''})"));
  EXPECT_TRUE(manager.callRefs(L));
}

TEST_F(LuaLvglObjectErrors, CloseDuringCreationWaitsForTheCallbackToReturn)
{
  ASSERT_TRUE(exec("dialog=lvgl.dialog({title='t'});"
                   "child=lvgl.box(dialog,{w=40,h=30,"
                   "size=function() dialog:close(); return 40,30 end})"));
  lv_timer_handler();
  MainWindow::instance()->run();
  ASSERT_TRUE(exec("assert(not pcall(child.set,child,{x=1}))"));
  ASSERT_TRUE(manager.callRefs(L));
  EXPECT_TRUE(manager.getLvglObjectRefs().empty());
}

TEST_F(LuaLvglObjectErrors, CollectedCreationCoroutinesCannotInvalidateNativeCallbacks)
{
  auto thread = lua_newthread(L);
  int status = luaL_dostring(
      thread, "box=lvgl.box({w=100,h=100});"
              "child=lvgl.box(box,{w=40,h=30});"
              "number=lvgl.numberEdit(box,{w=80,h=30,"
              "get=function() return 11 end,set=function() end})");
  ASSERT_EQ(status, LUA_OK) << lua_tostring(thread, -1);
  lua_pop(L, 1);
  lua_gc(L, LUA_GCCOLLECT, 0);
  lua_getglobal(L, "number");
  auto number = static_cast<NumberEdit*>(
      LvglWidgetObjectBase::checkLvgl(L, -1, true)->getWindow());
  lua_pop(L, 1);
  EXPECT_EQ(number->getValue(), 11);
  ASSERT_TRUE(exec("lvgl.clear(box);"
                   "assert(not pcall(child.set,child,{x=1}));"
                   "assert(not pcall(number.set,number,{x=1}))"));
}

TEST_F(LuaLvglObjectErrors, DeepCleanupDoesNotGrowTheLuaStack)
{
  ASSERT_TRUE(exec("current=lvgl.box({w=100,h=100});"
                   "for i=1,96 do current=lvgl.box(current,{w=80,h=80}) end"));
  lua_gc(L, LUA_GCCOLLECT, 0);
  struct Allocator {
    lua_Alloc original;
    void* data;
    unsigned failures = 0;
  } allocator;
  allocator.original = lua_getallocf(L, &allocator.data);
  lua_setallocf(
      L, [](void* context, void* ptr, size_t oldSize, size_t newSize) -> void* {
        auto allocator = static_cast<Allocator*>(context);
        if (newSize > oldSize && (ptr || oldSize == 0)) {
          ++allocator->failures;
          return nullptr;
        }
        return allocator->original(allocator->data, ptr, oldSize, newSize);
      },
      &allocator);
  manager.clearRefs(L);
  lua_setallocf(L, allocator.original, allocator.data);
  EXPECT_EQ(allocator.failures, 0u);
  EXPECT_TRUE(manager.getLvglObjectRefs().empty());
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 0u);
  lua_gc(L, LUA_GCCOLLECT, 0);
  ASSERT_TRUE(exec("assert(not pcall(current.set,current,{x=1}))"));
}

TEST_F(LuaLvglObjectErrors, SetterErrorsKeepTheScriptLocation)
{
  ASSERT_TRUE(exec("box=lvgl.box({w=40,h=30})"));
  ASSERT_TRUE(exec("local ok,msg=pcall(function() box:set({bogus=1}) end)\n"
                   "assert(not ok)\n"
                   "assert(string.find(msg,'Invalid property',1,true),msg)\n"
                   "assert(string.find(msg,'^%[string.-%]:1: '),msg)"));
}

TEST_F(LuaLvglObjectErrors, CoroutineConstructorErrorsUseTheirProtectedCaller)
{
  auto thread = lua_newthread(L);
  int status = luaL_dostring(
      thread, "assert(not pcall(lvgl.line,{pts={{0,0},{'bad',1}}}));"
              "assert(not pcall(lvgl.label,{"
              "text=function() return {} end}));"
              "assert(not pcall(lvgl.box,{borderPad={[false]=1}}))");
  ASSERT_EQ(status, LUA_OK) << lua_tostring(thread, -1);
  lua_pop(L, 1);
  lua_gc(L, LUA_GCCOLLECT, 0);
  EXPECT_TRUE(manager.getLvglObjectRefs().empty());
}

TEST_F(LuaLvglObjectErrors, ClosingANativeControlParentWaitsUntilTheEventReturns)
{
  ASSERT_TRUE(exec("dialog=lvgl.dialog({title='t'});"
                   "number=lvgl.numberEdit(dialog,{w=80,h=30,"
                   "get=function() return 1 end,"
                   "set=function() dialog:close() end})"));
  lua_getglobal(L, "number");
  auto number = static_cast<NumberEdit*>(
      LvglWidgetObjectBase::checkLvgl(L, -1, true)->getWindow());
  lua_pop(L, 1);
  number->setValue(2);
  ASSERT_TRUE(exec("number:set({x=1})"));
  lv_timer_handler();
  MainWindow::instance()->run();
  ASSERT_TRUE(exec("assert(not pcall(number.set,number,{x=2}))"));
}

TEST_F(LuaLvglObjectErrors, PendingClosesAreCancelledByClearAndGarbageCollection)
{
  auto top = Window::topWindow();
  ASSERT_TRUE(exec("dialog=lvgl.dialog({title='t'});"
                   "child=lvgl.box(dialog,{w=40,h=30,"
                   "size=function() dialog:close(); return 40,30 end});"
                   "lvgl.clear(); dialog=nil; child=nil"));
  lua_gc(L, LUA_GCCOLLECT, 0);
  lv_timer_handler();
  MainWindow::instance()->run();
  EXPECT_TRUE(manager.getLvglObjectRefs().empty());
  EXPECT_EQ(Window::topWindow(), top);
}

TEST_F(LuaLvglObjectErrors, MissingAndInvalidBuildTablesRaiseLuaErrors)
{
  ASSERT_TRUE(exec("assert(not pcall(lvgl.build));"
                   "assert(not pcall(lvgl.build,7));"
                   "assert(not pcall(lvgl.build,false));"
                   "assert(not pcall(lvgl.build,{} ,7))"));
  EXPECT_EQ(manager.getTempParent(), nullptr);
  EXPECT_TRUE(manager.getLvglObjectRefs().empty());
}

TEST_F(LuaLvglObjectErrors, PopupCallbacksAreReleasedAfterNativeClosure)
{
  auto top = Window::topWindow();
  ASSERT_TRUE(exec("local callback=function() end;"
                   "weak=setmetatable({callback},{__mode='v'});"
                   "lvgl.confirm({title='t',message='m',cancel=callback})"));
  ASSERT_NE(Window::topWindow(), top);
  EXPECT_EQ(manager.getLvglObjectRefs().size(), 1u);
  Window::topWindow()->onCancel();
  ASSERT_TRUE(manager.callRefs(L));
  EXPECT_TRUE(manager.getLvglObjectRefs().empty());
  MainWindow::instance()->run();
  lua_gc(L, LUA_GCCOLLECT, 0);
  ASSERT_TRUE(exec("assert(weak[1] == nil)"));
  EXPECT_EQ(Window::topWindow(), top);
}

TEST_F(LuaLvglObjectErrors, PopupParserFailuresLeaveNoRegisteredObjects)
{
  ASSERT_TRUE(exec("assert(not pcall(lvgl.confirm,{cancel=function() end,bogus=1}));"
                   "assert(not pcall(lvgl.message,{message={}}));"
                   "assert(not pcall(lvgl.menu,{values={false}}))"));
  EXPECT_TRUE(manager.getLvglObjectRefs().empty());
}

class RegistryFailureAllocator
{
 public:
  lua_Alloc original = nullptr;
  void* originalData = nullptr;
  bool failGrowth = false;
  bool failTables = false;
  unsigned failures = 0;

  static void* allocate(void* context, void* ptr, size_t oldSize,
                         size_t newSize)
  {
    auto self = static_cast<RegistryFailureAllocator*>(context);
    bool reject = self->failTables
                      ? !ptr && oldSize == LUA_TTABLE && newSize != 0
                      : newSize > oldSize && (ptr || oldSize == 0);
    if (self->failGrowth && reject) {
      ++self->failures;
      return nullptr;
    }
    return self->original(self->originalData, ptr, oldSize, newSize);
  }

  static int arm(lua_State* L)
  {
    auto self = static_cast<RegistryFailureAllocator*>(
        lua_touserdata(L, lua_upvalueindex(1)));
    auto count = lua_rawlen(L, LUA_REGISTRYINDEX);
    for (lua_Integer i = count + 1; i <= 1024; ++i) {
      lua_pushboolean(L, true);
      lua_rawseti(L, LUA_REGISTRYINDEX, i);
    }
    lua_pushinteger(L, 0);
    lua_rawseti(L, LUA_REGISTRYINDEX, 0);
    lua_pushinteger(L, 80);
    lua_pushinteger(L, 60);
    self->failGrowth = true;
    return 2;
  }

  static int armNextAllocation(lua_State* L)
  {
    auto self = static_cast<RegistryFailureAllocator*>(
        lua_touserdata(L, lua_upvalueindex(1)));
    self->failGrowth = true;
    return 0;
  }
};

TEST_F(LuaLvglObjectErrors, RegistryAllocationFailureLeavesNoDanglingFinalizer)
{
  RegistryFailureAllocator allocator;
  allocator.original = lua_getallocf(L, &allocator.originalData);
  lua_pushlightuserdata(L, &allocator);
  lua_pushcclosure(L, RegistryFailureAllocator::arm, 1);
  lua_setglobal(L, "arm");
  lua_setallocf(L, RegistryFailureAllocator::allocate, &allocator);
  auto result = exec("ok,msg=pcall(lvgl.box,{size=arm})");
  allocator.failGrowth = false;
  lua_setallocf(L, allocator.original, allocator.originalData);
  ASSERT_TRUE(result);
  ASSERT_GT(allocator.failures, 0u);
  ASSERT_TRUE(exec("assert(not ok, msg)"));
  lua_getglobal(L, "msg");
  EXPECT_NE(strstr(lua_tostring(L, -1), "memory"), nullptr);
  lua_pop(L, 1);
  EXPECT_TRUE(manager.getLvglObjectRefs().empty());
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 0u);
  lua_gc(L, LUA_GCCOLLECT, 0);
}

TEST_F(LuaLvglObjectErrors, BuildResultAllocationFailureRestoresTheParent)
{
  RegistryFailureAllocator allocator;
  allocator.original = lua_getallocf(L, &allocator.originalData);
  allocator.failTables = true;
  lua_pushlightuserdata(L, &allocator);
  lua_pushcclosure(L, RegistryFailureAllocator::armNextAllocation, 1);
  lua_setglobal(L, "arm");
  lua_setallocf(L, RegistryFailureAllocator::allocate, &allocator);
  auto result = exec("parent=lvgl.box({w=100,h=100});"
                     "params={}; arm(); ok,msg=pcall(lvgl.build,parent,params)");
  allocator.failGrowth = false;
  lua_setallocf(L, allocator.original, allocator.originalData);
  ASSERT_TRUE(result);
  EXPECT_GT(allocator.failures, 0u);
  EXPECT_EQ(manager.getTempParent(), nullptr);
  ASSERT_TRUE(exec("assert(not ok); parent:set({x=1});"
                   "box=lvgl.box({w=40,h=30})"));
  lua_getglobal(L, "box");
  auto window = LvglWidgetObjectBase::checkLvgl(L, -1, true)->getWindow();
  lua_pop(L, 1);
  EXPECT_EQ(window->getParent(), manager.root);
}

TEST_F(LuaLvglObjectErrors, HiddenAncestorCannotKeepClearedDescendantsValid)
{
  ASSERT_TRUE(exec("outer=lvgl.box({w=100,h=100});"
                   "inner=lvgl.box(outer,{w=100,h=100});"
                   "child=lvgl.box(inner,{w=80,h=60});"
                   "lvgl.hide(outer); lvgl.clear(inner)"));
  MainWindow::instance()->run();
  ASSERT_TRUE(exec("assert(not pcall(child.set,child,{x=1}));"
                   "assert(not pcall(lvgl.show,child));"
                   "assert(not pcall(lvgl.clear,child))"));
  ASSERT_TRUE(manager.callRefs(L));
}

TEST_F(LuaLvglObjectErrors, ClearRebuildKeepsOnlyTheNewChildHandles)
{
  ASSERT_TRUE(exec("parent=lvgl.box({w=100,h=100});"
                   "old=lvgl.box(parent,{w=80,h=60});"
                   "lvgl.hide(parent); lvgl.clear(parent);"
                   "new=lvgl.box(parent,{w=40,h=30});"
                   "assert(not pcall(old.set,old,{x=1}));"
                   "new:set({x=2}); lvgl.show(parent)"));
  ASSERT_TRUE(manager.callRefs(L));
  MainWindow::instance()->run();
  ASSERT_TRUE(exec("new:set({x=3})"));
}

TEST_F(LuaLvglObjectErrors, ClearedHandlesRetainTheirWrapperOnlyUntilGc)
{
  ASSERT_NE(createCountedBox(), nullptr);
  ASSERT_TRUE(exec("lvgl.clear()"));
  MainWindow::instance()->run();
  lua_gc(L, LUA_GCCOLLECT, 0);
  EXPECT_EQ(boxDestructions, 0u);
  ASSERT_TRUE(exec("assert(not pcall(box.set,box,{x=1})); box=nil"));
  lua_gc(L, LUA_GCCOLLECT, 0);
  EXPECT_EQ(boxDestructions, 1u);
}

TEST_F(LuaLvglObjectErrors, ExternalWindowClosureInvalidatesRetainedHandles)
{
  auto box = createCountedBox();
  ASSERT_NE(box, nullptr);
  box->getWindow()->closeWindow();
  MainWindow::instance()->run();
  ASSERT_TRUE(exec("assert(not pcall(box.set,box,{x=1}))"));
  ASSERT_TRUE(manager.callRefs(L));
  EXPECT_TRUE(manager.getLvglObjectRefs().empty());
  ASSERT_TRUE(exec("box=nil"));
  lua_gc(L, LUA_GCCOLLECT, 0);
  EXPECT_EQ(boxDestructions, 1u);
}

TEST_F(LuaLvglObjectErrors, StringGetterResultSurvivesLuaGarbageCollection)
{
  auto box = createCountedBox();
  ASSERT_NE(box, nullptr);
  ASSERT_TRUE(exec("getter=function() return string.rep('x',2000) end"));
  lua_getglobal(L, "getter");
  int ref = luaL_ref(L, LUA_REGISTRYINDEX);
  const char* result = box->pcallGetStringVal(L, ref);
  luaL_unref(L, LUA_REGISTRYINDEX, ref);
  ASSERT_TRUE(exec("getter=nil"));
  lua_gc(L, LUA_GCCOLLECT, 0);
  EXPECT_EQ(strlen(result), 2000u);
  for (unsigned i = 0; i < 2000; ++i) EXPECT_EQ(result[i], 'x');
}

}  // namespace

#endif
