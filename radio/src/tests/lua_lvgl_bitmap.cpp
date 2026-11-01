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
#include "static.h"
#include "view_main.h"

extern void lv_stb_init();

namespace
{

class BitmapScriptManager : public LuaScriptManager
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
  void luaShowError() override { ADD_FAILURE() << "Lua LVGL callback failed"; }
};

const char* const kFileA = "images/color/bitmap_320x240.png";
const char* const kFileB = "images/color/bitmap_480x272.png";

class LuaLvglBitmap : public ::testing::Test
{
 protected:
  lua_State* L = nullptr;
  LuaScriptManager* previousManager = nullptr;
  BitmapScriptManager manager;

  static void SetUpTestSuite() { lv_stb_init(); }

  void SetUp() override
  {
    L = luaL_newstate();
    ASSERT_NE(L, nullptr);
    luaRegisterLibraries(L);
    manager.root = new Window(nullptr, {0, 0, 200, 200});
    manager.L = L;
    previousManager = luaScriptManager;
    luaScriptManager = &manager;
  }

  void TearDown() override
  {
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

  StaticBitmap* bitmap(const char* name = "bmp")
  {
    lua_getglobal(L, name);
    auto obj = LvglWidgetObjectBase::checkLvgl(L, -1, true);
    lua_pop(L, 1);
    return obj ? (StaticBitmap*)obj->getWindow() : nullptr;
  }

  lv_obj_t* canvas(const char* name = "bmp")
  {
    auto b = bitmap(name);
    return b ? lv_obj_get_child(b->getLvObj(), 0) : nullptr;
  }

  const lv_img_dsc_t* pixels(const char* name = "bmp")
  {
    auto c = canvas(name);
    return c ? &((lv_canvas_t*)c)->dsc : nullptr;
  }

  // The decoded buffer is owned by the object. Marking a pixel shows whether
  // the buffer was replaced, without depending on allocator behaviour.
  void markPixels()
  {
    auto dsc = pixels();
    ASSERT_NE(dsc, nullptr);
    ((uint8_t*)dsc->data)[0] = 0xAB;
    ((uint8_t*)dsc->data)[1] = 0xCD;
  }

  bool pixelsMarked()
  {
    auto dsc = pixels();
    return dsc && dsc->data[0] == 0xAB && dsc->data[1] == 0xCD;
  }
};

TEST_F(LuaLvglBitmap, LoadsFileScaledToFrame)
{
  ASSERT_TRUE(exec(std::string("bmp=lvgl.bitmap({file='") + kFileA +
                   "',w=80,h=60})"));
  ASSERT_TRUE(bitmap()->hasImage());
  auto dsc = pixels();
  ASSERT_NE(dsc, nullptr);
  EXPECT_EQ(dsc->header.w, 80);
  EXPECT_EQ(dsc->header.h, 60);
}

TEST_F(LuaLvglBitmap, KeepsAspectRatioInsideFrame)
{
  ASSERT_TRUE(exec(std::string("bmp=lvgl.bitmap({file='") + kFileA +
                   "',w=100,h=40})"));
  auto dsc = pixels();
  ASSERT_NE(dsc, nullptr);
  EXPECT_EQ(dsc->header.h, 40);
  EXPECT_EQ(dsc->header.w, 53);
}

TEST_F(LuaLvglBitmap, EmptyOrMissingFileLeavesNoImage)
{
  ASSERT_TRUE(exec("bmp=lvgl.bitmap({w=80,h=60})"));
  EXPECT_FALSE(bitmap()->hasImage());
  ASSERT_TRUE(exec("bmp:set({file='images/color/missing.png'})"));
  EXPECT_FALSE(bitmap()->hasImage());
}

TEST_F(LuaLvglBitmap, OpaqueImagesKeepFullColorWithoutAlpha)
{
  ASSERT_TRUE(exec("bmp=lvgl.bitmap({file='images/color/"
                   "bitmap_gradient_64x64.jpg',w=64,h=64})"));
  ASSERT_TRUE(bitmap()->hasImage());
  EXPECT_EQ(pixels()->header.cf, LV_IMG_CF_TRUE_COLOR);
  EXPECT_EQ(pixels()->header.w, 64);
  // A 1:1 frame needs no scaled copy: pixels are exactly the decoded image
  auto decoded = BitmapBuffer::loadBitmap("images/color/bitmap_gradient_64x64.jpg");
  ASSERT_NE(decoded, nullptr);
  EXPECT_EQ(memcmp(pixels()->data, decoded->getData(), 64 * 64 * 2), 0);
  delete decoded;
}

TEST_F(LuaLvglBitmap, OpaqueImagesScaleToFitFrame)
{
  ASSERT_TRUE(exec("bmp=lvgl.bitmap({file='images/color/"
                   "bitmap_gradient_64x64.jpg',w=40,h=20})"));
  EXPECT_EQ(pixels()->header.cf, LV_IMG_CF_TRUE_COLOR);
  EXPECT_EQ(pixels()->header.w, 20);
  EXPECT_EQ(pixels()->header.h, 20);
  ASSERT_TRUE(exec("bmp:set({w=128,h=128})"));
  EXPECT_EQ(pixels()->header.w, 128);
  EXPECT_EQ(pixels()->header.h, 128);
}

TEST_F(LuaLvglBitmap, TransparentImagesKeepAlpha)
{
  ASSERT_TRUE(exec("bmp=lvgl.bitmap({file='images/color/edgetx.png',"
                   "w=50,h=50})"));
  ASSERT_TRUE(bitmap()->hasImage());
  EXPECT_EQ(pixels()->header.cf, LV_IMG_CF_TRUE_COLOR_ALPHA);
  EXPECT_EQ(pixels()->header.w, 50);
}

TEST_F(LuaLvglBitmap, SetFileReplacesAndClears)
{
  ASSERT_TRUE(exec(std::string("bmp=lvgl.bitmap({file='") + kFileA +
                   "',w=80,h=60})"));
  ASSERT_TRUE(exec(std::string("bmp:set({file='") + kFileB + "'})"));
  ASSERT_TRUE(bitmap()->hasImage());
  EXPECT_EQ(pixels()->header.w, 80);
  EXPECT_EQ(pixels()->header.h, 45);
  ASSERT_TRUE(exec("bmp:set({file=''})"));
  EXPECT_FALSE(bitmap()->hasImage());
}

TEST_F(LuaLvglBitmap, SameFileIsNotDecodedAgain)
{
  ASSERT_TRUE(exec(std::string("bmp=lvgl.bitmap({file='") + kFileA +
                   "',w=80,h=60})"));
  markPixels();
  ASSERT_TRUE(exec(std::string("bmp:set({file='") + kFileA + "',x=5})"));
  EXPECT_TRUE(pixelsMarked());
  ASSERT_TRUE(exec(std::string("bmp:set({file='") + kFileB + "'})"));
  EXPECT_FALSE(pixelsMarked());
}

TEST_F(LuaLvglBitmap, PixelsSurviveHidingAndMoving)
{
  ASSERT_TRUE(exec(std::string("bmp=lvgl.bitmap({file='") + kFileA +
                   "',w=80,h=60})"));
  markPixels();
  ASSERT_TRUE(exec("lvgl.hide(bmp); bmp:set({x=40,y=40}); lvgl.show(bmp)"));
  EXPECT_TRUE(pixelsMarked());
}

TEST_F(LuaLvglBitmap, ResizingReloadsAtNewSize)
{
  ASSERT_TRUE(exec(std::string("bmp=lvgl.bitmap({file='") + kFileA +
                   "',w=80,h=60})"));
  markPixels();
  ASSERT_TRUE(exec("bmp:set({w=40,h=30})"));
  EXPECT_FALSE(pixelsMarked());
  EXPECT_EQ(pixels()->header.w, 40);
  EXPECT_EQ(pixels()->header.h, 30);
}

TEST_F(LuaLvglBitmap, FunctionSourceLoadsOnlyWhenItChanges)
{
  ASSERT_TRUE(exec(std::string("calls=0; name='") + kFileA +
                   "'; bmp=lvgl.bitmap({w=80,h=60,file=function() "
                   "calls=calls+1; return name end})"));
  ASSERT_TRUE(bitmap()->hasImage());
  markPixels();
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_TRUE(exec("assert(calls == 3)"));
  EXPECT_TRUE(pixelsMarked());
  ASSERT_TRUE(exec(std::string("name='") + kFileB + "'"));
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_FALSE(pixelsMarked());
  EXPECT_EQ(pixels()->header.h, 45);
}

TEST_F(LuaLvglBitmap, FunctionSourceIsNotCalledWhileHidden)
{
  ASSERT_TRUE(exec(std::string("calls=0; name='") + kFileA +
                   "'; bmp=lvgl.bitmap({w=80,h=60,file=function() "
                   "calls=calls+1; return name end})"));
  ASSERT_TRUE(exec("lvgl.hide(bmp); calls=0; name='" + std::string(kFileB) +
                   "'"));
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_TRUE(exec("assert(calls == 0)"));
  EXPECT_EQ(pixels()->header.h, 60);
  ASSERT_TRUE(exec("lvgl.show(bmp)"));
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_TRUE(exec("assert(calls == 1)"));
  EXPECT_EQ(pixels()->header.h, 45);
}

TEST_F(LuaLvglBitmap, StringFileReplacesFunctionSource)
{
  ASSERT_TRUE(exec(std::string("calls=0; bmp=lvgl.bitmap({w=80,h=60,file="
                               "function() calls=calls+1; return '") +
                   kFileA + "' end})"));
  ASSERT_TRUE(exec(std::string("bmp:set({file='") + kFileB +
                   "'}); calls=0"));
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_TRUE(exec("assert(calls == 0)"));
  EXPECT_EQ(pixels()->header.h, 45);
}

TEST_F(LuaLvglBitmap, ObjectsKeepIndependentPixels)
{
  ASSERT_TRUE(exec(std::string("bmp=lvgl.bitmap({file='") + kFileA +
                   "',w=80,h=60}); other=lvgl.bitmap({file='" + kFileA +
                   "',w=80,h=60})"));
  markPixels();
  ASSERT_TRUE(exec(std::string("other:set({file='") + kFileB + "'})"));
  EXPECT_TRUE(pixelsMarked());
  EXPECT_NE(pixels("bmp")->data, pixels("other")->data);
}

TEST_F(LuaLvglBitmap, BuildAndParentedCreation)
{
  ASSERT_TRUE(exec(std::string("local t=lvgl.build({{type=lvgl.BITMAP,"
                               "name='b',file='") +
                   kFileA +
                   "',w=80,h=60}}); bmp=t.b;"
                   "local box=lvgl.box({x=0,y=0,w=100,h=100});"
                   "child=box:bitmap({file='" + kFileA + "',w=20,h=20})"));
  ASSERT_TRUE(bitmap()->hasImage());
  EXPECT_TRUE(bitmap("child")->hasImage());
}

TEST_F(LuaLvglBitmap, RejectsImageOnlyAndUnknownProperties)
{
  ASSERT_TRUE(exec("bmp=lvgl.bitmap({w=80,h=60})"));
  ASSERT_TRUE(exec("assert(not pcall(lvgl.bitmap,{w=80,h=60,fill=true}))"));
  ASSERT_TRUE(exec("assert(not pcall(bmp.set,bmp,{bogus=1}))"));
  ASSERT_TRUE(exec("assert(not pcall(bmp.set,bmp,{file={}}))"));
}

TEST_F(LuaLvglBitmap, ClearReleasesPixels)
{
  ASSERT_TRUE(exec(std::string("bmp=lvgl.bitmap({file='") + kFileA +
                   "',w=80,h=60}); lvgl.clear()"));
  MainWindow::instance()->run();
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 0);
}

}  // namespace

#endif
