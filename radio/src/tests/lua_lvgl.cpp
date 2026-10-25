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

extern void lv_stb_init();

namespace
{

class ImageScriptManager : public LuaScriptManager
{
 public:
  Window* root = nullptr;
  Window* getCurrentParent() const override
  {
    return tempParent ? tempParent->getWindow() : root;
  }
  void clear() override { root->clear(); }
  bool useLvglLayout() const override { return true; }
  bool isAppMode() const override { return false; }
  bool isWidget() override { return true; }
  bool isFullscreen() override { return true; }
  void luaShowError() override { ADD_FAILURE() << "Lua LVGL callback failed"; }
};

class LuaBitmapImage : public ::testing::Test
{
 protected:
  lua_State* L = nullptr;
  LuaScriptManager* previousManager = nullptr;
  ImageScriptManager manager;
  uint32_t initialUsage = 0;

  static void SetUpTestSuite() { lv_stb_init(); }

  void SetUp() override
  {
    initialUsage = luaExtraMemoryUsage;
    L = luaL_newstate();
    ASSERT_NE(L, nullptr);
    luaRegisterLibraries(L);
    manager.root = new Window(nullptr, {0, 0, 100, 100});
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
    EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
  }

  ::testing::AssertionResult exec(const char* code)
  {
    if (luaL_dostring(L, code))
      return ::testing::AssertionFailure() << lua_tostring(L, -1);
    return ::testing::AssertionSuccess();
  }

  BitmapBuffer* addBitmap(const char* name, uint8_t format = BMP_RGB565,
                          uint16_t w = 2, uint16_t h = 1)
  {
    auto slot = (BitmapBuffer**)lua_newuserdata(L, sizeof(BitmapBuffer*));
    auto bitmap = *slot = new BitmapBuffer(format, w, h);
    bitmap->getData()[0] = format == BMP_RGB565 ? RGB(255, 0, 0) : 0x8f00;
    bitmap->getData()[1] = format == BMP_RGB565 ? RGB(0, 255, 0) : 0xf0f0;
    luaExtraMemoryUsage += bitmap->getDataSize();
    luaL_getmetatable(L, "BITMAP*");
    lua_setmetatable(L, -2);
    lua_setglobal(L, name);
    return bitmap;
  }

  LvglWidgetObjectBase* image()
  {
    lua_getglobal(L, "image");
    auto obj = LvglWidgetObjectBase::checkLvgl(L, -1, true);
    lua_pop(L, 1);
    return obj;
  }

  const lv_img_dsc_t* source()
  {
    auto child = lv_obj_get_child(image()->getWindow()->getLvObj(), 0);
    return (const lv_img_dsc_t*)lv_img_get_src(child);
  }
};

TEST_F(LuaBitmapImage, BorrowsRgb565AndKeepsUserdataAlive)
{
  auto bitmap = addBitmap("bitmap");
  ASSERT_TRUE(
      exec("assert(lvgl.IMAGE_BITMAP_SUPPORTED == 1);"
           "image = lvgl.image({bitmap=bitmap, w=2, h=1});"
           "weak = setmetatable({bitmap}, {__mode='v'});"
           "bitmap = nil; collectgarbage(); assert(weak[1])"));
  EXPECT_EQ(source()->data, (const uint8_t*)bitmap->getData());
  EXPECT_EQ(source()->header.cf, LV_IMG_CF_TRUE_COLOR);
  EXPECT_EQ(source()->header.w, 2);
  EXPECT_EQ(source()->header.h, 1);
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage + 4);
  ASSERT_TRUE(
      exec("image:set({bitmap=false}); collectgarbage();"
           "assert(weak[1] == nil)"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
  EXPECT_EQ(lv_obj_get_child_cnt(image()->getWindow()->getLvObj()), 0);
}

TEST_F(LuaBitmapImage, ReplacesAndClearsImmediately)
{
  addBitmap("first");
  auto second = addBitmap("second");
  ASSERT_TRUE(
      exec("image = lvgl.image({bitmap=first, w=2, h=1});"
           "first = nil; image:set({bitmap=second}); collectgarbage()"));
  EXPECT_EQ(source()->data, (const uint8_t*)second->getData());
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage + 4);
  ASSERT_TRUE(exec("second = nil; image:set({bitmap=nil}); collectgarbage()"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage + 4);
  ASSERT_TRUE(exec("image:set({bitmap=false}); collectgarbage()"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
}

TEST_F(LuaBitmapImage, UsesBitmapOpenResultWithoutCopying)
{
  ASSERT_TRUE(
      exec("bitmap=Bitmap.open('images/color/bitmap_480x272.png');"
           "local w,h=Bitmap.getSize(bitmap); assert(w==480 and h==272);"
           "image=lvgl.image({bitmap=bitmap,w=480,h=272})"));
  lua_getglobal(L, "bitmap");
  auto bitmap = checkBitmap(L, -1);
  lua_pop(L, 1);
  ASSERT_NE(bitmap, nullptr);
  EXPECT_EQ(source()->data, (const uint8_t*)bitmap->getData());
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage + 480 * 272 * 2);
}

TEST_F(LuaBitmapImage, ConvertsArgb4444AndAccountsForPixels)
{
  auto bitmap = addBitmap("bitmap", BMP_ARGB4444);
  ASSERT_TRUE(exec("image = lvgl.image({bitmap=bitmap, w=2, h=1})"));
  const auto descriptor = source();
  EXPECT_EQ(descriptor->header.cf, LV_IMG_CF_TRUE_COLOR_ALPHA);
  EXPECT_EQ(descriptor->data_size, 6);
  const uint8_t expected[] = {0x00, 0xf8, 0x88, 0xe0, 0x07, 0xff};
  EXPECT_EQ(memcmp(descriptor->data, expected, sizeof(expected)), 0);
  EXPECT_EQ(bitmap->getData()[0], 0x8f00);
  EXPECT_EQ(bitmap->getData()[1], 0xf0f0);
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage + 10);
  ASSERT_TRUE(exec("image:set({bitmap=false})"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage + 4);
}

TEST_F(LuaBitmapImage, RejectsInvalidSourcesWithoutReplacingImage)
{
  auto bitmap = addBitmap("bitmap");
  ASSERT_TRUE(
      exec("image = lvgl.image({bitmap=bitmap, w=2, h=1});"
           "failed = Bitmap.open('/missing-image.png');"
           "local w,h = Bitmap.getSize(failed); assert(w==0 and h==0);"
           "assert(not pcall(image.set, image, {bitmap=failed}));"
           "assert(not pcall(image.set, image, {bitmap=function() return "
           "bitmap end}));"
           "assert(not pcall(image.set, image, {bitmap=true}));"
           "assert(not pcall(image.set, image, {file='', bitmap=bitmap}))"));
  EXPECT_EQ(source()->data, (const uint8_t*)bitmap->getData());
  addBitmap("oversized", BMP_RGB565, 2048, 1);
  ASSERT_TRUE(exec("assert(not pcall(image.set, image, {bitmap=oversized}))"));
  EXPECT_EQ(source()->data, (const uint8_t*)bitmap->getData());
}

TEST_F(LuaBitmapImage, ConversionCannotBypassLegacyBudget)
{
  auto bitmap = addBitmap("bitmap");
  addBitmap("alpha", BMP_ARGB4444);
  ASSERT_TRUE(exec("image = lvgl.image({bitmap=bitmap, w=2, h=1})"));
  auto actualUsage = luaExtraMemoryUsage;
  luaExtraMemoryUsage = LUA_MEM_EXTRA_MAX;
  EXPECT_TRUE(
      exec("local ok,err = pcall(image.set, image, {bitmap=alpha});"
           "assert(not ok and string.find(err, 'memory limit'))"));
  luaExtraMemoryUsage = actualUsage;
  EXPECT_EQ(source()->data, (const uint8_t*)bitmap->getData());
}

TEST_F(LuaBitmapImage, NativeDeletionReleasesStrongReference)
{
  addBitmap("bitmap");
  ASSERT_TRUE(
      exec("image = lvgl.image({bitmap=bitmap, w=2, h=1}); bitmap=nil"));
  image()->getWindow()->closeWindow();
  ASSERT_TRUE(
      exec("collectgarbage();"
           "assert(not pcall(image.set, image, {bitmap=false}))"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
  manager.clearRefs(L);
  ASSERT_TRUE(exec("image=nil; collectgarbage()"));
}

TEST_F(LuaBitmapImage, WidgetTeardownReleasesStrongReference)
{
  addBitmap("bitmap");
  ASSERT_TRUE(
      exec("image = lvgl.image({bitmap=bitmap, w=2, h=1}); bitmap=nil"));
  manager.clearRefs(L);
  ASSERT_TRUE(exec("collectgarbage()"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
}

TEST_F(LuaBitmapImage, HiddenImagesCanBeCleared)
{
  addBitmap("bitmap");
  ASSERT_TRUE(
      exec("image = lvgl.image({bitmap=bitmap, w=2, h=1});"
           "image:hide(); bitmap=nil; collectgarbage()"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage + 4);
  ASSERT_TRUE(exec("image:set({bitmap=false}); collectgarbage()"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
}

TEST_F(LuaBitmapImage, RepeatedVisibleSetsStayWithinExistingBudget)
{
  ASSERT_TRUE(
      exec("images={}; for i=1,9 do "
           "images[i]=lvgl.image({bitmap=false,w=256,h=256}) end"));
  for (int set = 0; set < 20; ++set) {
    for (int i = 1; i <= 9; ++i) {
      addBitmap("bitmap", BMP_RGB565, 256, 256);
      lua_pushinteger(L, i);
      lua_setglobal(L, "index");
      ASSERT_TRUE(exec("images[index]:set({bitmap=bitmap}); bitmap=nil"));
    }
    EXPECT_EQ(luaExtraMemoryUsage, initialUsage + 9 * 256 * 256 * 2);
    EXPECT_LE(luaExtraMemoryUsage, LUA_MEM_EXTRA_MAX);
    ASSERT_TRUE(
        exec("for i=1,9 do images[i]:set({bitmap=false}) end;"
             "collectgarbage()"));
    EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
  }
}

TEST_F(LuaBitmapImage, FileSourcesStillReplaceBitmapSources)
{
  addBitmap("bitmap");
  ASSERT_TRUE(
      exec("image = lvgl.image({bitmap=bitmap, w=2, h=1}); bitmap=nil;"
           "image:set({file='images/color/edgetx.png'}); collectgarbage()"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
  auto child = lv_obj_get_child(image()->getWindow()->getLvObj(), 0);
  ASSERT_NE(child, nullptr);
  EXPECT_STREQ((const char*)lv_img_get_src(child), "A/images/color/edgetx.png");
  ASSERT_TRUE(
      exec("image:set({file=function() return "
           "'images/color/bitmap_320x240.png' end})"));
  EXPECT_TRUE(manager.callRefs(L));
  child = lv_obj_get_child(image()->getWindow()->getLvObj(), 0);
  ASSERT_NE(child, nullptr);
  EXPECT_STREQ((const char*)lv_img_get_src(child),
               "A/images/color/bitmap_320x240.png");
  addBitmap("bitmap");
  ASSERT_TRUE(
      exec("image:set({bitmap=bitmap});"
           "image:set({file=function() return "
           "'images/color/bitmap_320x240.png' end})"));
  EXPECT_TRUE(manager.callRefs(L));
  child = lv_obj_get_child(image()->getWindow()->getLvObj(), 0);
  ASSERT_NE(child, nullptr);
  EXPECT_STREQ((const char*)lv_img_get_src(child),
               "A/images/color/bitmap_320x240.png");
}

TEST_F(LuaBitmapImage, FilenameSourcesCanBeReplacedAndCleared)
{
  ASSERT_TRUE(
      exec("image=lvgl.image({file='images/color/edgetx.png',w=2,h=1});"
           "image:set({file='images/color/bitmap_320x240.png'})"));
  auto child = lv_obj_get_child(image()->getWindow()->getLvObj(), 0);
  ASSERT_NE(child, nullptr);
  EXPECT_STREQ((const char*)lv_img_get_src(child),
               "A/images/color/bitmap_320x240.png");
  ASSERT_TRUE(exec("image:set({file=''})"));
  EXPECT_EQ(lv_obj_get_child_cnt(image()->getWindow()->getLvObj()), 0);
}

TEST_F(LuaBitmapImage, AlphaConversionIsReleasedOnNativeDeletion)
{
  addBitmap("bitmap", BMP_ARGB4444);
  ASSERT_TRUE(exec("image=lvgl.image({bitmap=bitmap,w=2,h=1}); bitmap=nil"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage + 10);
  image()->getWindow()->closeWindow();
  ASSERT_TRUE(exec("collectgarbage()"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
  manager.clearRefs(L);
}

}  // namespace

#endif
