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

extern void lv_stb_init();

namespace
{

class ImageScriptManager : public LuaScriptManager
{
 public:
  Window* root = nullptr;
  Window* getCurrentParent() const override
  {
    return (tempParent && tempParent->getWindow()) ? tempParent->getWindow()
                                                    : root;
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
  int destroyedImages = 0;

  class TrackedImage : public LvglWidgetImage
  {
   public:
    explicit TrackedImage(int* count) : count(count) {}
    ~TrackedImage() override { ++*count; }
   private:
    int* count;
  };

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
    lua_pushlightuserdata(L, &destroyedImages);
    lua_pushcclosure(L, [](lua_State* L) {
      auto count = (int*)lua_touserdata(L, lua_upvalueindex(1));
      auto obj = new TrackedImage(count);
      obj->create(L, 1);
      obj->push(L);
      return 1;
    }, 1);
    lua_setglobal(L, "trackedImage");
    ASSERT_TRUE(exec(
        "function lateParams(key,value) "
        "for i=1,1000 do "
        "local n=8+i%64; local p={};"
        "for j=1,n do p['padding'..j]=true end;"
        "p['bad'..i]=1; p[key]=value;"
        "for j=1,n do p['padding'..j]=nil end;"
        "if next(p)==key then return p end end;"
        "error('Cannot arrange source before invalid property') end"));
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

  int imageRef()
  {
    lua_getglobal(L, "image");
    int ref = LUA_REFNIL;
    lua_pushnil(L);
    while (lua_next(L, LUA_REGISTRYINDEX)) {
      if (lua_isinteger(L, -2) && lua_rawequal(L, -1, -3))
        ref = lua_tointeger(L, -2);
      lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return ref;
  }

  int registryResources()
  {
    int count = 0;
    lua_pushnil(L);
    while (lua_next(L, LUA_REGISTRYINDEX)) {
      if (lua_isfunction(L, -1) || lua_isuserdata(L, -1)) ++count;
      lua_pop(L, 1);
    }
    return count;
  }
};

TEST_F(LuaBitmapImage, BorrowsRgb565AndKeepsUserdataAlive)
{
  auto bitmap = addBitmap("bitmap");
  ASSERT_TRUE(
      exec("image = lvgl.image({bitmap=bitmap, w=2, h=1});"
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

TEST_F(LuaBitmapImage, BorrowsDecodedJpegWithoutCopying)
{
  ASSERT_TRUE(exec("bitmap=Bitmap.open('images/color/bitmap_2x2.jpg');"
                   "local w,h=Bitmap.getSize(bitmap); assert(w==2 and h==2);"
                   "image=lvgl.image({bitmap=bitmap,w=2,h=2})"));
  lua_getglobal(L, "bitmap");
  auto bitmap = checkBitmap(L, -1);
  lua_pop(L, 1);
  ASSERT_NE(bitmap, nullptr);
  EXPECT_EQ(bitmap->getFormat(), BMP_RGB565);
  EXPECT_EQ(source()->data, (const uint8_t*)bitmap->getData());
  EXPECT_EQ(source()->header.cf, LV_IMG_CF_TRUE_COLOR);
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage + bitmap->getDataSize());
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
           "image:show(); image:hide(); lvgl.enable(image); lvgl.disable(image);"
           "assert(not pcall(image.set, image, {bitmap=false}))"));
  EXPECT_NE(image(), nullptr);
  EXPECT_EQ(image()->getWindow(), nullptr);
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
  manager.clearRefs(L);
  ASSERT_TRUE(exec("assert(not pcall(image.show, image));"
                   "image=nil; collectgarbage()"));
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

TEST_F(LuaBitmapImage, ParentClearPreservesRegistrySlotUntilDeferredCleanup)
{
  addBitmap("bitmap", BMP_ARGB4444);
  ASSERT_TRUE(exec("box=lvgl.box({w=100,h=100})"));
  lua_getglobal(L, "box");
  manager.setTempParent(LvglWidgetObjectBase::checkLvgl(L, -1, true));
  lua_pop(L, 1);
  auto created = exec("image=trackedImage({bitmap=bitmap,w=2,h=1}); bitmap=nil");
  manager.setTempParent(nullptr);
  ASSERT_TRUE(created);
  int ref = imageRef();
  ASSERT_GT(ref, 0);
  ASSERT_TRUE(exec("box:clear(); collectgarbage();"
                   "label=lvgl.label({text='unrelated'});"
                   "image:show(); image:hide()"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
  EXPECT_EQ(destroyedImages, 0);
  EXPECT_EQ(imageRef(), ref);
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_EQ(destroyedImages, 1);
  ASSERT_TRUE(exec("label:set({text='still valid'});"
                   "assert(not pcall(image.show,image))"));
  lua_pushboolean(L, true);
  int reused = luaL_ref(L, LUA_REGISTRYINDEX);
  EXPECT_EQ(reused, ref);
  luaL_unref(L, LUA_REGISTRYINDEX, reused);
}

TEST_F(LuaBitmapImage, FilenameOnlyNativeDeletionKeepsDeferredHandle)
{
  ASSERT_TRUE(exec("image=trackedImage({file='images/color/edgetx.png',"
                   "w=2,h=1})"));
  int ref = imageRef();
  image()->getWindow()->closeWindow();
  EXPECT_EQ(imageRef(), ref);
  EXPECT_EQ(destroyedImages, 0);
  ASSERT_TRUE(exec("image:show(); image:hide(); lvgl.enable(image);"
                   "lvgl.disable(image);"
                   "assert(not pcall(image.set,image,{file=''}));"
                   "label=lvgl.label({text='unrelated'})"));
  EXPECT_TRUE(manager.callRefs(L));
  manager.clearRefs(L);
  EXPECT_EQ(destroyedImages, 1);
  manager.clearRefs(L);
  ASSERT_TRUE(exec("image=nil; collectgarbage(); collectgarbage()"));
  EXPECT_EQ(destroyedImages, 1);
}

TEST_F(LuaBitmapImage, NativeDeletedImageRejectsSourcesAndSkipsCallbacks)
{
  addBitmap("bitmap");
  ASSERT_TRUE(exec("calls=0; image=lvgl.image({bitmap=bitmap,w=2,h=1,"
                   "visible=function() calls=calls+1; return true end});"
                   "calls=0"));
  image()->getWindow()->closeWindow();
  ASSERT_TRUE(exec("assert(not pcall(image.set,image,{bitmap=bitmap}));"
                   "assert(not pcall(lvgl.image,image,{bitmap=bitmap}));"
                   "assert(not pcall(lvgl.build,image,"
                   "{{type='image',bitmap=bitmap}}));"
                   "bitmap=nil; collectgarbage()"));
  EXPECT_TRUE(manager.callRefs(L));
  ASSERT_TRUE(exec("assert(calls==0)"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
}

TEST_F(LuaBitmapImage, CollectedCreatorCoroutineDoesNotOwnDeletionState)
{
  addBitmap("bitmap", BMP_ARGB4444);
  auto co = lua_newthread(L);
  lua_setglobal(L, "creator");
  ASSERT_EQ(luaL_dostring(co,
      "image=trackedImage({bitmap=bitmap,w=2,h=1})"), LUA_OK);
  ASSERT_TRUE(exec("weak=setmetatable({creator},{__mode='v'});"
                   "creator=nil; bitmap=nil; collectgarbage(); collectgarbage();"
                   "assert(weak[1]==nil)"));
  image()->getWindow()->closeWindow();
  ASSERT_TRUE(exec("collectgarbage()"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
  manager.clearRefs(L);
  EXPECT_EQ(destroyedImages, 1);
}

TEST_F(LuaBitmapImage, NativePageAndDialogCloseReleasePixelsBeforeHandles)
{
  ViewMain::instance();
  for (const char* constructor : {"page", "dialog"}) {
    addBitmap("bitmap", BMP_ARGB4444);
    lua_pushstring(L, constructor);
    lua_setglobal(L, "constructor");
    ASSERT_TRUE(exec("container=lvgl[constructor]({title='Bitmap test'});"
                     "image=lvgl.image(container,{bitmap=bitmap,w=2,h=1});"
                     "bitmap=nil"));
    int ref = imageRef();
    lua_getglobal(L, "container");
    auto container = LvglWidgetObjectBase::checkLvgl(L, -1, true);
    lua_pop(L, 1);
    container->getWindow()->getParent()->closeWindow();
    ASSERT_TRUE(exec("collectgarbage(); image:show(); image:hide();"
                     "assert(not pcall(image.set,image,{bitmap=false}))"));
    EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
    EXPECT_EQ(imageRef(), ref);
    EXPECT_EQ(image()->getWindow(), nullptr);
    manager.clearRefs(L);
    ASSERT_TRUE(exec("image=nil; container=nil; collectgarbage()"));
    MainWindow::instance()->run();
  }
}

TEST_F(LuaBitmapImage, ParentCleanupDeletesWrapperExactlyOnceBeforeNativeClose)
{
  addBitmap("bitmap", BMP_ARGB4444);
  ASSERT_TRUE(exec("image=trackedImage({bitmap=bitmap,w=2,h=1}); bitmap=nil"));
  auto window = image()->getWindow();
  manager.clearRefs(L);
  EXPECT_EQ(destroyedImages, 1);
  window->closeWindow();
  ASSERT_TRUE(exec("image=nil; collectgarbage(); collectgarbage()"));
  manager.clearRefs(L);
  EXPECT_EQ(destroyedImages, 1);
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
}

TEST_F(LuaBitmapImage, RejectedUpdatesPreservePixelsAndReleaseCandidates)
{
  addBitmap("bitmap", BMP_ARGB4444);
  addBitmap("second", BMP_ARGB4444);
  ASSERT_TRUE(exec("image=lvgl.image({bitmap=bitmap,w=2,h=1})"));
  auto descriptor = source();
  auto pixels = descriptor->data;
  auto usage = luaExtraMemoryUsage;
  auto refs = registryResources();
  ASSERT_TRUE(exec("image:set({x=1,y=2})"));
  EXPECT_EQ(source(), descriptor);
  EXPECT_EQ(source()->data, pixels);
  ASSERT_TRUE(exec(
      "for i=1,100 do "
      "assert(not pcall(image.set,image,{file={}}));"
      "assert(not pcall(image.set,image,{file=function() return '' end,bad=1}));"
      "assert(not pcall(image.set,image,{bitmap=second,bad=1}));"
      "assert(not pcall(image.set,image,lateParams('bitmap',second)));"
      "assert(not pcall(image.set,image,{bitmap=second,x={}}));"
      "assert(not pcall(image.set,image,{bitmap=false,borderPad={left={}}}));"
      "end; collectgarbage()"));
  EXPECT_EQ(source(), descriptor);
  EXPECT_EQ(source()->data, pixels);
  EXPECT_EQ(luaExtraMemoryUsage, usage);
  EXPECT_EQ(registryResources(), refs);
  const uint8_t expected[] = {0x00, 0xf8, 0x88, 0xe0, 0x07, 0xff};
  EXPECT_EQ(memcmp(source()->data, expected, sizeof(expected)), 0);
}

TEST_F(LuaBitmapImage, RejectedUpdatePreservesFilenameCallback)
{
  ASSERT_TRUE(exec(
      "calls=0; local old=function() calls=calls+1;"
      "return 'images/color/edgetx.png' end;"
      "weak=setmetatable({old},{__mode='v'});"
      "image=lvgl.image({file=old,w=2,h=1}); old=nil;"
      "for i=1,100 do "
      "local rejected=function() return 'images/color/bitmap_320x240.png' end;"
      "local params={file=rejected,x={}};"
      "assert(not pcall(image.set,image,params)); end;"
      "collectgarbage(); assert(weak[1]); calls=0"));
  EXPECT_TRUE(manager.callRefs(L));
  ASSERT_TRUE(exec("assert(calls==1)"));
  EXPECT_STREQ((const char*)source(), "A/images/color/edgetx.png");
  ASSERT_TRUE(exec("image:set({file='images/color/bitmap_320x240.png'});"
                   "collectgarbage(); assert(weak[1]==nil); calls=0"));
  EXPECT_TRUE(manager.callRefs(L));
  ASSERT_TRUE(exec("assert(calls==0)"));
  EXPECT_STREQ((const char*)source(), "A/images/color/bitmap_320x240.png");
}

TEST_F(LuaBitmapImage, FailedConstructorsReleaseRefsBudgetAndWrappers)
{
  for (auto format : {BMP_RGB565, BMP_ARGB4444}) {
    addBitmap("bitmap", format);
    auto refs = registryResources();
    ASSERT_TRUE(exec(
        "weak=setmetatable({bitmap},{__mode='v'});"
        "for i=1,100 do "
        "assert(not pcall(trackedImage,{bitmap=bitmap,badkey=1}));"
        "assert(not pcall(trackedImage,lateParams('bitmap',bitmap)));"
        "assert(not pcall(trackedImage,{bitmap=bitmap,x={}}));"
        "assert(not pcall(trackedImage,{bitmap=bitmap,borderPad={left={}}}));"
        "assert(not pcall(trackedImage,{bitmap=bitmap,w=2,h=1,"
        "file=function() return '' end}));"
        "assert(not pcall(trackedImage,{bitmap=bitmap,w=2,h=1,"
        "size=function() return {} end}));"
        "end; bitmap=nil; collectgarbage(); collectgarbage();"
        "assert(weak[1]==nil)"));
    EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
    EXPECT_EQ(registryResources(), refs);
    EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 0);
  }
  EXPECT_EQ(destroyedImages, 1200);
}

TEST_F(LuaBitmapImage, FailedFilenameConstructorReleasesCapturedBitmap)
{
  addBitmap("bitmap", BMP_ARGB4444);
  auto refs = registryResources();
  ASSERT_TRUE(exec(
      "weak=setmetatable({bitmap},{__mode='v'});"
      "local function source(b) return function() return b end end;"
      "for i=1,100 do "
      "assert(not pcall(trackedImage,{file=source(bitmap),bad=1}));"
      "assert(not pcall(trackedImage,{file=source(bitmap),w=2,h=1}));"
      "end; bitmap=nil; collectgarbage(); collectgarbage();"
      "assert(weak[1]==nil)"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
  EXPECT_EQ(destroyedImages, 200);
  EXPECT_EQ(registryResources(), refs);
}

TEST_F(LuaBitmapImage, FailedLayoutConstructorsReleaseBitmapResources)
{
  addBitmap("bitmap", BMP_ARGB4444);
  auto refs = registryResources();
  ASSERT_TRUE(exec("for i=1,100 do "
                   "assert(not pcall(lvgl.build,"
                   "{{type='image',bitmap=bitmap,badkey=1}}));"
                   "end; bitmap=nil; collectgarbage()"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
  EXPECT_EQ(registryResources(), refs);
}

TEST_F(LuaBitmapImage, FailedExplicitParentConstructorRestoresParent)
{
  addBitmap("bitmap", BMP_ARGB4444);
  ASSERT_TRUE(exec("box=lvgl.box({w=100,h=100});"
                   "assert(not pcall(lvgl.image,box,{bitmap=bitmap,bad=1}));"
                   "image=lvgl.image({bitmap=bitmap,w=2,h=1})"));
  EXPECT_EQ(image()->getWindow()->getParent(), manager.root);
}

TEST_F(LuaBitmapImage, FailedChildConstructionReleasesImageAndRestoresParent)
{
  addBitmap("bitmap", BMP_ARGB4444);
  auto refs = registryResources();
  ASSERT_TRUE(exec("for i=1,100 do "
                   "assert(not pcall(lvgl.image,{bitmap=bitmap,w=2,h=1,"
                   "children={{type='image',bitmap=bitmap,badkey=1}}}));"
                   "end; bitmap=nil; collectgarbage()"));
  EXPECT_EQ(manager.getTempParent(), nullptr);
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
  EXPECT_EQ(registryResources(), refs);
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 0);
}

TEST_F(LuaBitmapImage, FileAndFalseResolveIndependentlyOfIterationOrder)
{
  addBitmap("bitmap");
  ASSERT_TRUE(exec(
      "local tested={};"
      "for i=8,256 do "
      "local p={w=2,h=1};"
      "for j=1,i do p['padding'..j]=true end;"
      "if i%2==0 then p.bitmap=false; p.file='images/color/edgetx.png' "
      "else p.file='images/color/edgetx.png'; p.bitmap=false end;"
      "for j=1,i do p['padding'..j]=nil end;"
      "local first; for key in pairs(p) do "
      "if key=='file' or key=='bitmap' then first=key; break end end;"
      "image=lvgl.image(p);"
      "image:set({bitmap=bitmap}); image:set(p); tested[first]=true;"
      "if tested.file and tested.bitmap then break end end;"
      "assert(tested.file and tested.bitmap)"));
  EXPECT_STREQ((const char*)source(), "A/images/color/edgetx.png");
  ASSERT_TRUE(exec("image:set({bitmap=false,file=function()"
                   "return 'images/color/bitmap_320x240.png' end})"));
  EXPECT_TRUE(manager.callRefs(L));
  EXPECT_STREQ((const char*)source(), "A/images/color/bitmap_320x240.png");
  ASSERT_TRUE(exec("assert(not pcall(image.set,image,"
                   "{bitmap=bitmap,file='images/color/edgetx.png'}));"
                   "assert(not pcall(image.set,image,"
                   "{file=function() return '' end,bitmap=bitmap}))"));
  EXPECT_STREQ((const char*)source(), "A/images/color/bitmap_320x240.png");
}

TEST_F(LuaBitmapImage, ClearedImageChildrenCanBeReplacedAndReleased)
{
  auto bitmap = addBitmap("bitmap");
  ASSERT_TRUE(exec("image=lvgl.image({bitmap=bitmap,w=2,h=1});"
                   "lvgl.clear(image)"));
  EXPECT_EQ(lv_obj_get_child_cnt(image()->getWindow()->getLvObj()), 0u);
  ASSERT_TRUE(exec("image:set({bitmap=bitmap})"));
  EXPECT_EQ(source()->data, (const uint8_t*)bitmap->getData());
  ASSERT_TRUE(exec("lvgl.clear(image); image:set({bitmap=false});"
                   "image:set({file='images/color/edgetx.png'});"
                   "lvgl.clear(image); image:set({bitmap=bitmap});"
                   "lvgl.clear(image)"));
  EXPECT_EQ(lv_obj_get_child_cnt(image()->getWindow()->getLvObj()), 0u);
  manager.clearRefs(L);
  ASSERT_TRUE(exec("image=nil; bitmap=nil; collectgarbage()"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage);
}

TEST_F(LuaBitmapImage, WindowlessObjectsRemainValidParents)
{
  ASSERT_TRUE(exec("parent=lvgl.label({text='parent'});"
                   "child=lvgl.label(parent,{text='child'});"
                   "built=lvgl.build(parent,{{type='label',name='label',"
                   "text='built'}}); assert(child and built.label)"));
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), 3u);
  EXPECT_EQ(manager.getTempParent(), nullptr);
}

TEST_F(LuaBitmapImage, UpdatesIgnoreSourceMetamethods)
{
  addBitmap("first", BMP_ARGB4444);
  addBitmap("second", BMP_ARGB4444);
  ASSERT_TRUE(exec("image=lvgl.image({bitmap=first,w=2,h=1})"));
  auto usage = luaExtraMemoryUsage;
  ASSERT_TRUE(exec(
      "calls=0; image:set(setmetatable({x=1},{__index=function(t,k)"
      "calls=calls+1; image:set({bitmap=second}); return second end}));"
      "assert(calls==0)"));
  EXPECT_EQ(luaExtraMemoryUsage, usage);
  ASSERT_TRUE(exec("image:set({bitmap=false})"));
  EXPECT_EQ(luaExtraMemoryUsage, initialUsage + 8);
  ASSERT_TRUE(exec("image:set({bitmap=first})"));
  const uint8_t expected[] = {0x00, 0xf8, 0x88, 0xe0, 0x07, 0xff};
  EXPECT_EQ(memcmp(source()->data, expected, sizeof(expected)), 0);
}

TEST_F(LuaBitmapImage, FailedBuildChildrenRestoreParent)
{
  addBitmap("bitmap");
  ASSERT_TRUE(exec("box=lvgl.box({w=100,h=100});"
                   "assert(not pcall(lvgl.build,{{type='image',bitmap=bitmap,"
                   "w=2,h=1,children={{type='label',bad=1}}}}));"
                   "assert(not pcall(lvgl.build,box,{{type='box',"
                   "children={{type='label',bad=1}}}}))"));
  EXPECT_EQ(manager.getTempParent(), nullptr);
  manager.clearRefs(L);
  auto children = lv_obj_get_child_cnt(manager.root->getLvObj());
  ASSERT_TRUE(exec("label=lvgl.label({text='after'})"));
  EXPECT_EQ(lv_obj_get_child_cnt(manager.root->getLvObj()), children + 1);
}

}  // namespace

#endif
