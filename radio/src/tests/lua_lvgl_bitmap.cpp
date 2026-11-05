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

#include <memory>
#include <limits>
#include <vector>

#include "edgetx.h"
#include "lua/api_colorlcd.h"
#include "lua/lua_widget.h"
#include "mainwindow.h"
#include "static.h"
#include "view_main.h"

extern void lv_stb_init();
extern int custom_lua_atpanic(lua_State* L);

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
  void luaShowError() override { errors += 1; }
  int errors = 0;
};

const char* const kFileA = "images/color/bitmap_320x240.png";
const char* const kFileB = "images/color/bitmap_480x272.png";

class CountingBitmap : public LvglWidgetBitmap
{
 public:
  unsigned loads = 0;
  bool failAfterFileStage = false;

  bool hasPendingFile() const
  {
    return fileStaged || stagedFunction != LUA_REFNIL;
  }

 protected:
  void parseParam(lua_State* L, const char* key) override
  {
    LvglWidgetBitmap::parseParam(L, key);
    if (failAfterFileStage && !strcmp(key, "file"))
      luaL_error(L, "simulated parameter error after staging file");
  }

  void load() override
  {
    ++loads;
    LvglWidgetBitmap::load();
  }
};

class LuaLvglBitmap : public ::testing::Test
{
 protected:
  lua_State* L = nullptr;
  LuaScriptManager* previousManager = nullptr;
  BitmapScriptManager manager;
  int expectedErrors = 0;
  std::unique_ptr<CountingBitmap> countedBitmap;

  static void SetUpTestSuite() { lv_stb_init(); }

  void SetUp() override
  {
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
    EXPECT_EQ(manager.errors, expectedErrors);
    manager.clearRefs(L);
    manager.root->closeWindow();
    MainWindow::instance()->run();
    lua_close(L);
    countedBitmap.reset();
    luaScriptManager = previousManager;
  }

  ::testing::AssertionResult exec(const std::string& code)
  {
    if (luaL_dostring(L, code.c_str()))
      return ::testing::AssertionFailure() << lua_tostring(L, -1);
    return ::testing::AssertionSuccess();
  }

  void createCountedBitmap(const std::string& params)
  {
    ASSERT_FALSE(countedBitmap);
    ASSERT_TRUE(exec("params=" + params));
    auto object = std::make_unique<CountingBitmap>();
    lua_getglobal(L, "params");
    object->create(L, -1);
    lua_pop(L, 1);
    object->push(L);
    countedBitmap = std::move(object);
    lua_setglobal(L, "bmp");
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

TEST_F(LuaLvglBitmap, OpaqueScalingPaintsEveryDestinationPixel)
{
  std::unique_ptr<BitmapBuffer> decoded(
      BitmapBuffer::loadBitmap(kFileA, BMP_RGB565));
  ASSERT_NE(decoded, nullptr);

  struct Frame {
    coord_t w, h, scaledw, scaledh;
  };
  const Frame frames[] = {
      {100, 40, 53, 40}, {40, 100, 40, 30}, {79, 41, 54, 41},
      {53, 53, 53, 39}, {13, 7, 9, 7},     {1, 1, 1, 1},
      {320, 240, 320, 240}, {641, 481, 641, 480}};
  for (const auto& frame : frames) {
    auto b = new StaticBitmap(manager.root, {0, 0, frame.w, frame.h},
                              kFileA, BMP_RGB565);
    ASSERT_TRUE(b->hasImage());
    auto c = (lv_canvas_t*)lv_obj_get_child(b->getLvObj(), 0);
    ASSERT_NE(c, nullptr);
    ASSERT_EQ(c->dsc.header.cf, LV_IMG_CF_TRUE_COLOR);
    ASSERT_EQ(c->dsc.header.w, frame.scaledw);
    ASSERT_EQ(c->dsc.header.h, frame.scaledh);
    auto data = (const pixel_t*)c->dsc.data;
    float scale = min(float(frame.w) / decoded->width(),
                      float(frame.h) / decoded->height());
    for (coord_t y = 0; y < frame.scaledh; ++y) {
      for (coord_t x = 0; x < frame.scaledw; ++x) {
        auto expected = *decoded->getPixelPtrAbs(
            min((coord_t)(x / scale), (coord_t)decoded->width() - 1),
            min((coord_t)(y / scale), (coord_t)decoded->height() - 1));
        ASSERT_EQ(data[y * frame.scaledw + x], expected)
            << "frame " << frame.w << "x" << frame.h << ", pixel "
            << x << "," << y;
      }
    }
    b->closeWindow();
  }
}

TEST_F(LuaLvglBitmap, OneToOneOpaqueConversionKeepsItsBuffer)
{
  std::unique_ptr<BitmapBuffer> decoded(
      BitmapBuffer::loadBitmap(kFileA, BMP_RGB565));
  ASSERT_NE(decoded, nullptr);
  auto data = decoded->getData();
  ASSERT_TRUE(decoded->resizeToLVGL(320, 240));
  EXPECT_EQ(decoded->getData(), data);
}

TEST_F(LuaLvglBitmap, OversizedConversionLeavesItsBufferUnchanged)
{
  for (auto format : {BMP_RGB565, BMP_ARGB4444}) {
    BitmapBuffer buffer(format, 1, 1);
    auto data = buffer.getData();
    ASSERT_NE(data, nullptr);
    ASSERT_FALSE(buffer.resizeToLVGL(65536, 65536));
    EXPECT_EQ(buffer.getData(), data);
    EXPECT_EQ(buffer.width(), 1);
    EXPECT_EQ(buffer.height(), 1);
  }
}

TEST_F(LuaLvglBitmap, ResizingBorrowedPixelsDoesNotFreeTheirStorage)
{
  pixel_t pixels[] = {0x1234};
  {
    BitmapBuffer buffer(BMP_RGB565, 1, 1, pixels);
    ASSERT_TRUE(buffer.resizeToLVGL(2, 2));
    EXPECT_NE(buffer.getData(), pixels);
    for (int i = 0; i < 4; ++i)
      EXPECT_EQ(buffer.getData()[i], pixels[0]);
  }
  EXPECT_EQ(pixels[0], 0x1234);
}

TEST_F(LuaLvglBitmap, ForcedAlphaScalingKeepsTheExistingPixelConversion)
{
  const char* file = "images/color/edgetx.png";
  std::unique_ptr<BitmapBuffer> decoded(
      BitmapBuffer::loadBitmap(file, BMP_ARGB4444));
  ASSERT_NE(decoded, nullptr);
  auto b = new StaticBitmap(manager.root, {0, 0, 50, 50},
                            file, BMP_ARGB4444);
  ASSERT_TRUE(b->hasImage());
  auto c = (lv_canvas_t*)lv_obj_get_child(b->getLvObj(), 0);
  ASSERT_NE(c, nullptr);
  ASSERT_EQ(c->dsc.header.cf, LV_IMG_CF_TRUE_COLOR_ALPHA);
  float scale = min(50.0f / decoded->width(), 50.0f / decoded->height());
  for (coord_t y = 0; y < c->dsc.header.h; ++y) {
    for (coord_t x = 0; x < c->dsc.header.w; ++x) {
      auto pixel = *decoded->getPixelPtrAbs(
          (coord_t)(x / scale), (coord_t)(y / scale));
      ARGB_SPLIT(pixel, a, r, g, blue);
      auto expected = RGB_JOIN((r * 31 + 7) / 15,
                               (g * 63 + 7) / 15,
                               (blue * 31 + 7) / 15);
      auto data = &c->dsc.data[(y * c->dsc.header.w + x) * 3];
      ASSERT_EQ(data[0], expected & 0xFF);
      ASSERT_EQ(data[1], expected >> 8);
      ASSERT_EQ(data[2], a * 17);
    }
  }
}

TEST_F(LuaLvglBitmap, TransparentImagesKeepAlpha)
{
  ASSERT_TRUE(exec("bmp=lvgl.bitmap({file='images/color/edgetx.png',"
                   "w=50,h=50})"));
  ASSERT_TRUE(bitmap()->hasImage());
  EXPECT_EQ(pixels()->header.cf, LV_IMG_CF_TRUE_COLOR_ALPHA);
  EXPECT_EQ(pixels()->header.w, 50);
}

TEST_F(LuaLvglBitmap, FrameMustHaveARealSizeAndALimitedArea)
{
  std::string file = std::string("file='") + kFileA + "'";
  ASSERT_TRUE(exec("assert(not pcall(lvgl.bitmap,{" + file + "}))"));
  ASSERT_TRUE(exec("assert(not pcall(lvgl.bitmap,{" + file + ",w=80}))"));
  ASSERT_TRUE(exec("assert(not pcall(lvgl.bitmap,{" + file +
                   ",w=lvgl.PERCENT_SIZE,h=lvgl.PERCENT_SIZE}))"));
  ASSERT_TRUE(exec("assert(not pcall(lvgl.bitmap,{" + file + ",w=0,h=60}))"));
  ASSERT_TRUE(exec("assert(not pcall(lvgl.bitmap,{" + file + ",w=4000,h=4000}))"));
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 0u);
}

TEST_F(LuaLvglBitmap, FramesLargerThanTheDisplayAreAllowed)
{
  ASSERT_TRUE(exec(std::string("bmp=lvgl.bitmap({file='") + kFileA + "',w=" +
                   std::to_string(LCD_W * 2) + ",h=" +
                   std::to_string(LCD_H) + "})"));
  ASSERT_TRUE(bitmap()->hasImage());
  EXPECT_EQ(pixels()->header.h, LCD_H);
}

TEST_F(LuaLvglBitmap, OverflowingOrNarrowedFramesAreRejected)
{
  ASSERT_TRUE(exec("assert(not pcall(lvgl.bitmap,{w=65536,h=65536}))"));
  ASSERT_TRUE(exec("assert(not pcall(lvgl.bitmap,{w=4294967376,h=60}))"));
  ASSERT_TRUE(exec("assert(not pcall(lvgl.bitmap,{w=80,h=4294967356}))"));
  ASSERT_TRUE(exec("assert(not pcall(lvgl.bitmap,{w=-4294967216,h=60}))"));
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 0u);
}

TEST_F(LuaLvglBitmap, FrameAreaBoundaryIsExact)
{
  auto w = std::to_string(LCD_W * 2);
  auto h = std::to_string(LCD_H * 2);
  auto oversizedW = std::to_string(LCD_W * 2 + 1);
  ASSERT_TRUE(exec("assert(pcall(lvgl.bitmap,{w=" + w + ",h=" + h + "}))"));
  ASSERT_TRUE(exec("assert(not pcall(lvgl.bitmap,{w=" + oversizedW +
                   ",h=" + h + "}))"));
}

TEST_F(LuaLvglBitmap, SizeFunctionProvidesTheFrame)
{
  ASSERT_TRUE(exec(std::string("bmp=lvgl.bitmap({file='") + kFileA +
                   "',size=function() return 80,60 end})"));
  ASSERT_TRUE(bitmap()->hasImage());
  EXPECT_EQ(pixels()->header.w, 80);
  EXPECT_EQ(pixels()->header.h, 60);
}

TEST_F(LuaLvglBitmap, SizeMustBeAFunction)
{
  ASSERT_TRUE(exec("for _,v in ipairs({7,false,'size',{}}) do "
                   "assert(not pcall(lvgl.bitmap,{size=v})) end"));
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 0u);
  ASSERT_TRUE(exec(std::string("calls=0; bmp=lvgl.bitmap({file='") + kFileA +
                   "',size=function() calls=calls+1; return 80,60 end});"
                   "assert(not pcall(bmp.set,bmp,{size=7})); calls=0"));
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_TRUE(exec("assert(calls == 1)"));
  EXPECT_TRUE(bitmap()->hasImage());
}

TEST_F(LuaLvglBitmap, InvalidFileValueKeepsThePreviousSource)
{
  ASSERT_TRUE(exec(std::string("calls=0; bmp=lvgl.bitmap({w=80,h=60,file="
                               "function() calls=calls+1; return '") +
                   kFileA + "' end})"));
  ASSERT_TRUE(exec("assert(not pcall(bmp.set,bmp,{file={}}))"));
  ASSERT_TRUE(exec("calls=0"));
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_TRUE(exec("assert(calls == 1)"));
  EXPECT_TRUE(bitmap()->hasImage());
}

TEST_F(LuaLvglBitmap, FileCallbackErrorsPreserveThePreviousImage)
{
  ASSERT_TRUE(exec(std::string("bmp=lvgl.bitmap({w=80,h=60,file='") + kFileA +
                   "'}); bmp:set({file=function() error('file callback') end})"));
  EXPECT_FALSE(manager.callRefs(L));
  EXPECT_TRUE(bitmap()->hasImage());
}

TEST_F(LuaLvglBitmap, ClearOnTheObjectKeepsItsImage)
{
  ASSERT_TRUE(exec(std::string("bmp=lvgl.bitmap({file='") + kFileA +
                   "',w=80,h=60}); lvgl.clear(bmp)"));
  MainWindow::instance()->run();
  ASSERT_NE(canvas(), nullptr);
  ASSERT_TRUE(bitmap()->hasImage());
  EXPECT_EQ(lv_obj_get_child_cnt(bitmap()->getLvObj()), 1u);
  ASSERT_TRUE(exec(std::string("bmp:set({file='") + kFileB + "'})"));
  EXPECT_EQ(lv_obj_get_child_cnt(bitmap()->getLvObj()), 1u);
  EXPECT_EQ(pixels()->header.h, 45);
}

TEST_F(LuaLvglBitmap, ClearedChildHandlesBecomeInvalid)
{
  ASSERT_TRUE(exec(std::string("box=lvgl.box({x=0,y=0,w=100,h=100});"
                               "bmp=box:bitmap({file='") +
                   kFileA + "',w=80,h=60}); box:clear()"));
  EXPECT_TRUE(manager.callRefs(L));
  MainWindow::instance()->run();
  ASSERT_TRUE(exec("assert(not pcall(bmp.set,bmp,{x=1}))"));
  EXPECT_EQ(lv_obj_get_child_cnt(
                LvglWidgetObjectBase::checkLvgl((lua_getglobal(L, "box"), L), -1)
                    ->getWindow()->getLvObj()),
            0u);
}

TEST_F(LuaLvglBitmap, RejectedUpdateLeavesTheSourceAlone)
{
  ASSERT_TRUE(exec(std::string("calls=0; bmp=lvgl.bitmap({w=80,h=60,file="
                               "function() calls=calls+1; return '") +
                   kFileA + "' end})"));
  markPixels();
  // Vary the table layout so that the invalid key is met before and after file
  ASSERT_TRUE(exec(std::string("for i=1,24 do local p={file='") + kFileB +
                   "'}; for j=1,i do p['k'..j]=true end; p.bogus=1;"
                   "assert(not pcall(bmp.set,bmp,p)) end"));
  ASSERT_TRUE(exec("calls=0"));
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_TRUE(exec("assert(calls == 1)"));
  EXPECT_TRUE(pixelsMarked());
  // and a later successful update does not apply what the failed ones staged
  ASSERT_TRUE(exec("bmp:set({x=3})"));
  EXPECT_TRUE(pixelsMarked());
  EXPECT_EQ(pixels()->header.h, 60);
}

TEST_F(LuaLvglBitmap, RejectedUpdateWithFunctionLeavesTheStringSource)
{
  ASSERT_TRUE(exec(std::string("bmp=lvgl.bitmap({w=80,h=60,file='") + kFileA +
                   "'})"));
  markPixels();
  ASSERT_TRUE(exec("for i=1,24 do local p={file=function() return 'x' end};"
                   "for j=1,i do p['k'..j]=true end; p.bogus=1;"
                   "assert(not pcall(bmp.set,bmp,p)) end"));
  ASSERT_TRUE(exec("bmp:set({x=3})"));
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_TRUE(bitmap()->hasImage());
  EXPECT_TRUE(pixelsMarked());
}

TEST_F(LuaLvglBitmap, ChangingFileAndSizeInOneUpdateDecodesOnce)
{
  createCountedBitmap(std::string("{w=80,h=60,file='") + kFileA + "'}");
  ASSERT_NE(countedBitmap, nullptr);
  countedBitmap->loads = 0;
  markPixels();
  ASSERT_TRUE(exec(std::string("bmp:set({w=40,h=30,file='") + kFileB + "'})"));
  EXPECT_EQ(lv_obj_get_child_cnt(bitmap()->getLvObj()), 1u);
  EXPECT_EQ(pixels()->header.w, 40);
  EXPECT_EQ(pixels()->header.h, 22);
  EXPECT_FALSE(pixelsMarked());
  EXPECT_EQ(countedBitmap->loads, 1u);
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_EQ(countedBitmap->loads, 1u);
}

TEST_F(LuaLvglBitmap, FileFunctionAndSizeChangeDecodeOnce)
{
  createCountedBitmap(std::string("{w=80,h=60,file='") + kFileA + "'}");
  ASSERT_NE(countedBitmap, nullptr);
  countedBitmap->loads = 0;
  markPixels();
  ASSERT_TRUE(exec(std::string("bmp:set({w=40,h=30,file=function() return '") +
                   kFileB + "' end})"));
  EXPECT_EQ(countedBitmap->loads, 0u);
  EXPECT_TRUE(pixelsMarked());
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_EQ(countedBitmap->loads, 1u);
  EXPECT_EQ(pixels()->header.w, 40);
  EXPECT_EQ(pixels()->header.h, 22);
}

TEST_F(LuaLvglBitmap, SizeFunctionAndFileChangeDecodeOnce)
{
  createCountedBitmap(std::string("{w=80,h=60,file='") + kFileA + "'}");
  ASSERT_NE(countedBitmap, nullptr);
  countedBitmap->loads = 0;
  ASSERT_TRUE(exec(std::string("bmp:set({file='") + kFileB +
                   "',size=function() return 40,30 end})"));
  EXPECT_EQ(countedBitmap->loads, 0u);
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_EQ(countedBitmap->loads, 1u);
  EXPECT_EQ(pixels()->header.w, 40);
  EXPECT_EQ(pixels()->header.h, 22);
}

TEST_F(LuaLvglBitmap, DynamicFileAndSizeChangeDecodeOnce)
{
  ASSERT_TRUE(exec(std::string("name='") + kFileA + "'; fw=80; fh=60"));
  createCountedBitmap("{file=function() return name end,"
                      "size=function() return fw,fh end}");
  ASSERT_NE(countedBitmap, nullptr);
  EXPECT_EQ(countedBitmap->loads, 1u);
  countedBitmap->loads = 0;
  ASSERT_TRUE(exec(std::string("name='") + kFileB + "'; fw=40; fh=30"));
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_EQ(countedBitmap->loads, 1u);
  EXPECT_EQ(pixels()->header.w, 40);
  EXPECT_EQ(pixels()->header.h, 22);
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_EQ(countedBitmap->loads, 1u);
}

TEST_F(LuaLvglBitmap, ConstructionUsesTheFinalDynamicFrame)
{
  createCountedBitmap(std::string("{w=80,h=60,file='") + kFileB +
                      "',size=function() return 40,30 end}");
  ASSERT_NE(countedBitmap, nullptr);
  EXPECT_EQ(countedBitmap->loads, 1u);
  EXPECT_EQ(pixels()->header.w, 40);
  EXPECT_EQ(pixels()->header.h, 22);
}

TEST_F(LuaLvglBitmap, HiddenDynamicUpdatesWaitAndEmptyFileClearsImmediately)
{
  createCountedBitmap(std::string("{w=80,h=60,file='") + kFileA +
                      "',size=function() return 80,60 end}");
  ASSERT_NE(countedBitmap, nullptr);
  countedBitmap->loads = 0;
  markPixels();
  ASSERT_TRUE(exec(std::string("lvgl.hide(bmp); bmp:set({file='") + kFileB +
                   "',size=function() return 40,30 end})"));
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_EQ(countedBitmap->loads, 0u);
  EXPECT_TRUE(pixelsMarked());
  ASSERT_TRUE(exec("lvgl.show(bmp)"));
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_EQ(countedBitmap->loads, 1u);
  EXPECT_EQ(pixels()->header.w, 40);
  EXPECT_EQ(pixels()->header.h, 22);
  ASSERT_TRUE(exec("lvgl.hide(bmp); bmp:set({file=''})"));
  EXPECT_FALSE(bitmap()->hasImage());
}

TEST_F(LuaLvglBitmap, SuccessfulConstructionStillReturnsTheObject)
{
  ASSERT_TRUE(exec(std::string("bmp=lvgl.bitmap({w=40,h=30,file='") + kFileA +
                   "'}); assert(bmp ~= nil); bmp:set({x=2});"
                   "local t=lvgl.build({{type=lvgl.BOX,name='b',w=10,h=10,"
                   "children={{type=lvgl.BITMAP,name='c',w=10,h=10}}}});"
                   "assert(t.b ~= nil)"));
  EXPECT_TRUE(bitmap()->hasImage());
}

TEST_F(LuaLvglBitmap, ImageStaysBehindChildrenThatClearRemoves)
{
  ASSERT_TRUE(exec("bmp=lvgl.bitmap({w=80,h=60,"
                   "children={{type='label',text='x'}}});"
                   "lbl=lvgl.label(bmp,{text='y'});"
                   "box=lvgl.box(bmp,{w=10,h=10})"));
  auto obj = bitmap()->getLvObj();
  for (auto file : {kFileA, kFileB}) {
    ASSERT_TRUE(exec(std::string("bmp:set({file='") + file + "'})"));
    ASSERT_EQ(lv_obj_get_child_cnt(obj), 4u);
    EXPECT_TRUE(lv_obj_check_type(lv_obj_get_child(obj, 0), &lv_canvas_class));
    for (uint32_t i = 1; i < 4; i += 1)
      EXPECT_FALSE(
          lv_obj_check_type(lv_obj_get_child(obj, i), &lv_canvas_class));
  }
  markPixels();

  ASSERT_TRUE(exec("lvgl.clear(bmp)"));
  EXPECT_TRUE(manager.callRefs(L));
  MainWindow::instance()->run();
  ASSERT_EQ(lv_obj_get_child_cnt(obj), 1u);
  EXPECT_TRUE(lv_obj_check_type(lv_obj_get_child(obj, 0), &lv_canvas_class));
  EXPECT_TRUE(pixelsMarked());
  ASSERT_TRUE(exec("assert(not pcall(lbl.set,lbl,{text='z'}));"
                   "assert(not pcall(box.set,box,{x=1}))"));
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

TEST_F(LuaLvglBitmap, ClearThenDynamicUpdateUsesTheFinalFrameOnce)
{
  createCountedBitmap(std::string("{w=80,h=60,file='") + kFileA + "'}");
  ASSERT_NE(countedBitmap, nullptr);
  countedBitmap->loads = 0;
  ASSERT_TRUE(exec(std::string("lvgl.clear(bmp); bmp:set({file='") + kFileB +
                   "',size=function() return 40,30 end})"));
  ASSERT_TRUE(manager.callRefs(L));
  EXPECT_EQ(pixels()->header.w, 40);
  EXPECT_EQ(pixels()->header.h, 22);
  EXPECT_EQ(countedBitmap->loads, 1u);
  ASSERT_TRUE(manager.callRefs(L));
  EXPECT_EQ(countedBitmap->loads, 1u);
}

TEST_F(LuaLvglBitmap, ScaledDimensionsMustFitTheImageDescriptor)
{
  const char* file = "images/color/bitmap_dimension_limits.bmp";
  uint8_t bytes[54 + 64 * 3] = {'B', 'M', 246};
  bytes[10] = 54;
  bytes[14] = 40;
  bytes[18] = 64;
  bytes[22] = 1;
  bytes[26] = 1;
  bytes[28] = 24;
  bytes[34] = 192;
  FIL handle;
  ASSERT_EQ(f_open(&handle, file, FA_CREATE_NEW | FA_WRITE), FR_OK);
  UINT written = 0;
  EXPECT_EQ(f_write(&handle, bytes, sizeof(bytes), &written), FR_OK);
  EXPECT_EQ(written, sizeof(bytes));
  EXPECT_EQ(f_close(&handle), FR_OK);
  auto oversized = exec(std::string("bmp=lvgl.bitmap({w=4096,h=64,file='") +
                        file + "'})");
  bool rejected = oversized && !bitmap()->hasImage();
  auto valid = exec("bmp:set({w=2047,h=32})");
  int width = 0, height = 0;
  if (valid && bitmap()->hasImage()) {
    width = pixels()->header.w;
    height = pixels()->header.h;
  }
  EXPECT_EQ(f_unlink(file), FR_OK);
  ASSERT_TRUE(oversized);
  ASSERT_TRUE(valid);
  EXPECT_TRUE(rejected);
  EXPECT_EQ(width, 2047);
  EXPECT_EQ(height, 31);
}

TEST_F(LuaLvglBitmap, RejectedScalingPreservesOwnedAndBorrowedStorage)
{
  pixel_t pixels[] = {0x1234};
  for (auto format : {BMP_RGB565, BMP_ARGB4444}) {
    BitmapBuffer buffer(format, 64, 1);
    auto data = buffer.getData();
    ASSERT_NE(data, nullptr);
    EXPECT_FALSE(buffer.resizeToLVGL(2048, 32));
    EXPECT_EQ(buffer.getData(), data);
    EXPECT_EQ(buffer.width(), 64);
    EXPECT_EQ(buffer.height(), 1);
  }
  BitmapBuffer borrowed(BMP_RGB565, 1, 1, pixels);
  EXPECT_FALSE(borrowed.resizeToLVGL(2048, 2048));
  EXPECT_EQ(borrowed.getData(), pixels);
  EXPECT_EQ(pixels[0], 0x1234);
}

TEST_F(LuaLvglBitmap, DecodedImageDimensionsCannotNarrow)
{
  const char* file = "images/color/bitmap_source_dimension_limits.bmp";
  for (unsigned width : {65535u, 65536u}) {
    std::vector<uint8_t> bytes(54 + 196608);
    bytes[0] = 'B';
    bytes[1] = 'M';
    bytes[2] = 54;
    bytes[4] = 3;
    bytes[10] = 54;
    bytes[14] = 40;
    bytes[18] = width & 0xFF;
    bytes[19] = (width >> 8) & 0xFF;
    bytes[20] = width >> 16;
    bytes[22] = 1;
    bytes[26] = 1;
    bytes[28] = 24;
    bytes[36] = 3;
    FIL handle;
    ASSERT_EQ(f_open(&handle, file, FA_CREATE_NEW | FA_WRITE), FR_OK);
    UINT written = 0;
    EXPECT_EQ(f_write(&handle, bytes.data(), bytes.size(), &written), FR_OK);
    EXPECT_EQ(written, bytes.size());
    EXPECT_EQ(f_close(&handle), FR_OK);
    std::unique_ptr<BitmapBuffer> image(BitmapBuffer::loadBitmap(file));
    EXPECT_EQ(f_unlink(file), FR_OK);
    if (width == 65536) {
      EXPECT_EQ(image, nullptr);
    } else {
      ASSERT_NE(image, nullptr);
      EXPECT_EQ(image->width(), width);
      EXPECT_TRUE(image->resizeToLVGL(256, 64));
      EXPECT_EQ(image->width(), 256);
      EXPECT_EQ(image->height(), 1);
    }
  }
}

TEST_F(LuaLvglBitmap, GrayscalePngAlphaIsPreserved)
{
  const char* file = "images/color/bitmap_grayscale_alpha.png";
  const uint8_t bytes[] = {
      0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
      0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01,
      0x08, 0x04, 0x00, 0x00, 0x00, 0x5e, 0x2b, 0xb7, 0x01, 0x00, 0x00, 0x00,
      0x0d, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x63, 0x28, 0x67, 0xb8, 0xfb,
      0x1f, 0x00, 0x04, 0x9a, 0x02, 0x54, 0xd6, 0x57, 0x3f, 0x9f, 0x00, 0x00,
      0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};
  FIL handle;
  ASSERT_EQ(f_open(&handle, file, FA_CREATE_NEW | FA_WRITE), FR_OK);
  UINT written = 0;
  EXPECT_EQ(f_write(&handle, bytes, sizeof(bytes), &written), FR_OK);
  EXPECT_EQ(written, sizeof(bytes));
  EXPECT_EQ(f_close(&handle), FR_OK);
  auto result = exec(std::string("bmp=lvgl.bitmap({w=2,h=1,file='") + file + "'})");
  if (result && bitmap()->hasImage()) {
    EXPECT_EQ(pixels()->header.cf, LV_IMG_CF_TRUE_COLOR_ALPHA);
    EXPECT_EQ(pixels()->data[2], 0);
    EXPECT_EQ(pixels()->data[5], 255);
  } else {
    ADD_FAILURE() << "grayscale-alpha PNG did not load";
  }
  EXPECT_EQ(f_unlink(file), FR_OK);
  ASSERT_TRUE(result);
}

TEST_F(LuaLvglBitmap, ExtremeFrameSizesPreserveTheDecodedBuffer)
{
  for (auto format : {BMP_RGB565, BMP_ARGB4444}) {
    BitmapBuffer small(format, 1, 1);
    auto data = small.getData();
    ASSERT_NE(data, nullptr);
    EXPECT_FALSE(small.resizeToLVGL(std::numeric_limits<coord_t>::max(),
                                    std::numeric_limits<coord_t>::max()));
    EXPECT_EQ(small.getData(), data);
  }
}

TEST_F(LuaLvglBitmap, RejectedUpdatesReleaseStagedCallbacksImmediately)
{
  createCountedBitmap(std::string("{w=80,h=60,file='") + kFileA + "'}");
  ASSERT_NE(countedBitmap, nullptr);
  countedBitmap->loads = 0;
  countedBitmap->failAfterFileStage = true;
  markPixels();
  ASSERT_TRUE(exec(std::string("local callback=function() return '") + kFileB +
                   "' end; weak=setmetatable({callback},{__mode='v'});"
                   "assert(not pcall(bmp.set,bmp,{file=callback}))"));
  EXPECT_FALSE(countedBitmap->hasPendingFile());
  lua_gc(L, LUA_GCCOLLECT, 0);
  ASSERT_TRUE(exec("assert(weak[1] == nil)"));
  EXPECT_EQ(countedBitmap->loads, 0u);
  EXPECT_TRUE(pixelsMarked());
}

}  // namespace

#endif
