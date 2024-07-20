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

#include <gtest/gtest.h>
#include <math.h>

#define SWAP_DEFINED
#include "location.h"
#include "edgetx.h"

#if defined(COLORLCD)

#include "bitmaps.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

void convert_RGB565_to_RGB888(uint8_t* dst, const BitmapBuffer* src, coord_t w,
                              coord_t h)
{
  for (int y = 0; y < src->height(); y++) {
    for (int x = 0; x < src->width(); x++) {
      RGB_SPLIT(*src->getPixelPtrAbs(x, y), r, g, b);
      *(dst++) = (uint8_t)(r << 3);
      *(dst++) = (uint8_t)(g << 2);
      *(dst++) = (uint8_t)(b << 3);
    }
  }
}

void dumpImage(const std::string& filename, const BitmapBuffer* dc)
{
  std::string fullpath = TESTS_PATH "/images/color/failed_" + filename;

  TRACE("dumping image '%s'", fullpath.c_str());

  // allocate enough for 3 channels
  auto pixels = dc->width() * dc->height();
  auto stride = dc->width() * 3;
  uint8_t* img = (uint8_t*)malloc(pixels * 3);
  convert_RGB565_to_RGB888(img, dc, dc->width(), dc->height());
  stbi_write_png(fullpath.c_str(), dc->width(), dc->height(), 3, img, stride);
  free(img);
}

bool checkScreenshot_colorlcd(const BitmapBuffer* dc, const char* test)
{
  if (dc->width() != LCD_W || dc->height() != LCD_H) {
    return false;
  }

  std::string filename = std::string(test);
  filename += '_' + std::to_string(LCD_W);
  filename += 'x' + std::to_string(LCD_H);
  filename += ".png";

  std::string fullpath = TESTS_PATH "/images/color/" + filename;

  std::unique_ptr<BitmapBuffer> testPict(
      BitmapBuffer::loadBitmap(fullpath.c_str()));
  if (!testPict || testPict->width() != LCD_W || testPict->height() != LCD_H) {
    dumpImage(filename, dc);
    return false;
  }

  for (int y = 0; y < LCD_H; y++) {
    for (int x = 0; x < LCD_W; x++) {
      if (*testPict->getPixelPtrAbs(x, y) != *dc->getPixelPtrAbs(x, y)) {
        dumpImage(filename, dc);
        return false;
      }
    }
  }

  return true;
}

TEST(Lcd_colorlcd, lines)
{
  BitmapBuffer dc(BMP_RGB565, LCD_W, LCD_H);

  dc.clear(COLOR_THEME_SECONDARY3);
  dc.setClippingRect(50, 400, 50, 230);

  for (int x = 10; x <= 100; x += 10) {
    dc.drawLine(x, x, x + 200, x + 50, DOTTED, COLOR_THEME_SECONDARY1);
  }

  for (int x = 200; x >= 100; x -= 10) {
    dc.drawLine(x, x, x - 50, x + 100, DOTTED, COLOR_THEME_SECONDARY1);
  }

  dc.clearClippingRect();
  EXPECT_TRUE(checkScreenshot_colorlcd(&dc, "lines"));
}

TEST(Lcd_colorlcd, vline)
{
  BitmapBuffer dc(BMP_RGB565, LCD_W, LCD_H);

  dc.clear(COLOR_THEME_SECONDARY3);

  for (int x = 0; x < 100; x += 2) {
    dc.drawVerticalLine(x, x / 2, 12, SOLID, COLOR_THEME_SECONDARY1);
  }
  EXPECT_TRUE(checkScreenshot_colorlcd(&dc, "vline"));
}

TEST(Lcd_colorlcd, primitives)
{
  BitmapBuffer dc(BMP_RGB565, LCD_W, LCD_H);

  dc.clear(COLOR_THEME_SECONDARY3);

  dc.drawText(8, 8, "The quick brown fox jumps over the lazy dog",
              COLOR_THEME_DISABLED);
  dc.drawText(5, 5, "The quick brown fox jumps over the lazy dog",
              COLOR_THEME_SECONDARY1);

  dc.drawFilledRect(10, 30, 30, 30, SOLID, COLOR_THEME_SECONDARY1);
  dc.drawFilledRect(50, 30, 30, 30, DOTTED, COLOR_THEME_SECONDARY1);

  dc.drawRect(90, 30, 30, 30, 1, SOLID, COLOR_THEME_SECONDARY1);
  dc.drawRect(130, 30, 30, 30, 2, SOLID, COLOR_THEME_SECONDARY1);
  dc.drawRect(170, 30, 30, 30, 5, SOLID, COLOR_THEME_SECONDARY1);

  dc.drawVerticalLine(10, 70, 100, SOLID, COLOR_THEME_SECONDARY1);
  dc.drawVerticalLine(15, 70, 90, SOLID, COLOR_THEME_SECONDARY1);
  dc.drawVerticalLine(20, 70, 80, SOLID, COLOR_THEME_SECONDARY1);
  dc.drawVerticalLine(25, 70, 70, SOLID, COLOR_THEME_SECONDARY1);

  dc.drawHorizontalLine(30, 70, 100, SOLID, COLOR_THEME_SECONDARY1);
  dc.drawHorizontalLine(30, 75, 90, SOLID, COLOR_THEME_SECONDARY1);
  dc.drawHorizontalLine(30, 80, 80, SOLID, COLOR_THEME_SECONDARY1);
  dc.drawHorizontalLine(30, 85, 70, SOLID, COLOR_THEME_SECONDARY1);

  EXPECT_TRUE(checkScreenshot_colorlcd(&dc, "primitives_" TRANSLATIONS));
}

TEST(Lcd_colorlcd, transparency)
{
  BitmapBuffer dc(BMP_RGB565, LCD_W, LCD_H);

  dc.clear(COLOR_THEME_SECONDARY3);

  dc.drawText(8, 8, "The quick brown fox jumps over the lazy dog",
              COLOR_THEME_SECONDARY1);
  dc.drawText(5, 5, "The quick brown fox jumps over the lazy dog",
              COLOR_THEME_SECONDARY1);

  dc.drawFilledRect(10, 30, 30, 30, SOLID, COLOR_THEME_SECONDARY1, OPACITY(8));
  dc.drawFilledRect(50, 30, 30, 30, DOTTED, COLOR_THEME_SECONDARY1,
                    OPACITY(10));

  dc.drawRect(90, 30, 30, 30, 1, SOLID, COLOR_THEME_SECONDARY1, OPACITY(8));
  dc.drawRect(130, 30, 30, 30, 2, SOLID, COLOR_THEME_SECONDARY1, OPACITY(8));
  dc.drawRect(170, 30, 30, 30, 5, SOLID, COLOR_THEME_SECONDARY1, OPACITY(8));

  dc.drawVerticalLine(10, 70, 100, SOLID, COLOR_THEME_SECONDARY1, OPACITY(2));
  dc.drawVerticalLine(15, 70, 90, SOLID, COLOR_THEME_SECONDARY1, OPACITY(6));
  dc.drawVerticalLine(20, 70, 80, SOLID, COLOR_THEME_SECONDARY1, OPACITY(10));
  dc.drawVerticalLine(25, 70, 70, SOLID, COLOR_THEME_SECONDARY1,
                      OPACITY(OPACITY_MAX));

  dc.drawHorizontalLine(30, 70, 100, SOLID, COLOR_THEME_SECONDARY1, OPACITY(2));
  dc.drawHorizontalLine(30, 75, 90, SOLID, COLOR_THEME_SECONDARY1, OPACITY(6));
  dc.drawHorizontalLine(30, 80, 80, SOLID, COLOR_THEME_SECONDARY1, OPACITY(10));
  dc.drawHorizontalLine(30, 85, 70, SOLID, COLOR_THEME_SECONDARY1,
                        OPACITY(OPACITY_MAX));

  for (int n = 0; n < 10; n++) {
    int x = 120 + n * 20;
    int y = 80 + n * 10;
    int color = COLOR((n / 2 + 4));
    int size = 100;
    dc.drawFilledRect(x, y, size, size, SOLID, color, OPACITY(8));
  }

  EXPECT_TRUE(checkScreenshot_colorlcd(&dc, "transparency_" TRANSLATIONS));
}

//
// Fonts test are disabled, as they cause
// too much trouble (font are generated and never
// really the same based on which software / radio
// combination.
//
#if 0
TEST(Lcd_colorlcd, fonts)
{
  BitmapBuffer dc(BMP_RGB565, LCD_W, LCD_H);
  
  dc.clear(COLOR_THEME_SECONDARY3);

  dc.drawText(8, 8, "The quick brown fox jumps over the lazy dog", COLOR_THEME_SECONDARY1);
  dc.drawText(5, 5, "The quick brown fox jumps over the lazy dog", COLOR_THEME_SECONDARY1);

  dc.drawText(50, 25, "The quick", COLOR_THEME_SECONDARY1 | FONT(XXS));
  dc.drawText(50, 40, "The quick", COLOR_THEME_SECONDARY1 | FONT(XS));
  dc.drawText(50, 55, "The quick", COLOR_THEME_SECONDARY1 | FONT(L));
  dc.drawText(50, 80, "The quick", COLOR_THEME_SECONDARY1 | FONT(XL));
  dc.drawText(50, 120, "The quick", COLOR_THEME_SECONDARY1 | FONT(XXL));

  dc.drawText(8, 208, "The quick brown fox jumps over the lazy dog", COLOR_THEME_SECONDARY1);
  dc.drawText(5, 205, "The quick brown fox jumps over the lazy dog", COLOR_THEME_SECONDARY1);

  EXPECT_TRUE(checkScreenshot_colorlcd(&dc, "fonts_" TRANSLATIONS));
}
#endif

TEST(Lcd_colorlcd, clipping)
{
  BitmapBuffer dc(BMP_RGB565, LCD_W, LCD_H);

  dc.clear(COLOR_THEME_SECONDARY3);

  coord_t x1 = LCD_W / 4;
  coord_t x2 = x1 * 3;
  coord_t y1 = LCD_H / 4;
  coord_t y2 = y1 * 3;

  dc.drawVerticalLine(x1, 0, LCD_H, SOLID, COLOR_THEME_SECONDARY1);
  dc.drawVerticalLine(x2, 0, LCD_H, SOLID, COLOR_THEME_SECONDARY1);

  dc.drawHorizontalLine(0, y1, LCD_W, SOLID, COLOR_THEME_SECONDARY1);
  dc.drawHorizontalLine(0, y2, LCD_W, SOLID, COLOR_THEME_SECONDARY1);

  dc.setClippingRect(x1, x2, y1, y2);

  dc.drawHorizontalLine(0, y1 + 30, LCD_W, SOLID, COLOR_THEME_SECONDARY1);
  dc.drawHorizontalLine(0, y1 + 31, LCD_W, SOLID, COLOR_THEME_SECONDARY1);

  dc.drawVerticalLine(x1 + 50, 0, LCD_H, SOLID, COLOR_THEME_SECONDARY1);
  dc.drawVerticalLine(x1 + 51, 0, LCD_H, SOLID, COLOR_THEME_SECONDARY1);

  dc.drawSolidRect(x1 - 30, y1 - 30, 50, 50, 2, COLOR_THEME_SECONDARY1);
  dc.drawRect(x2 - 20, y1 - 30, 50, 50, 2, SOLID, COLOR_THEME_SECONDARY1);

  dc.clearClippingRect();
  EXPECT_TRUE(checkScreenshot_colorlcd(&dc, "clipping"));
}

TEST(Lcd_colorlcd, bitmap)
{
  BitmapBuffer dc(BMP_RGB565, LCD_W, LCD_H);

  dc.clear(COLOR_THEME_SECONDARY3);

  dc.setClippingRect(100, 400, 50, 200);
  std::unique_ptr<BitmapBuffer> bmp(
      BitmapBuffer::loadBitmap(TESTS_PATH "/images/color/edgetx.png"));
  dc.drawBitmap(0, 0, bmp.get());
  dc.drawBitmap(320, 0, bmp.get());
  dc.drawBitmap(0, 150, bmp.get());

  dc.clearClippingRect();
  EXPECT_TRUE(checkScreenshot_colorlcd(&dc, "bitmap"));
}

static const uint8_t mask_menu_radio[] = {
#include "images/color/mask_menu_radio.lbm"
};

extern MaskBitmap* _decompressed_mask(const uint8_t* lz4_compressed);

TEST(Lcd_colorlcd, masks)
{
  BitmapBuffer dc(BMP_RGB565, LCD_W, LCD_H);

  dc.clear(COLOR_THEME_SECONDARY3);

  MaskBitmap* mask = _decompressed_mask(mask_menu_radio);

  for (int i = 0; i < LCD_W; i += mask->width) {
    for (int j = 0; j < LCD_H; j += mask->height) {
      dc.drawBitmapPattern(i, j, mask, COLOR_BLACK);
    }
  }

  EXPECT_TRUE(checkScreenshot_colorlcd(&dc, "masks"));
  free(mask);
}

#if 0
// clang-format off
#define TEST_CHAR_RIGHT     "\302\200"
#define TEST_CHAR_LEFT      "\302\201"
#define TEST_CHAR_UP        "\302\202"
#define TEST_CHAR_DOWN      "\302\203"

#define TEST_CHAR_DELTA     "\302\210"
#define TEST_CHAR_STICK     "\302\211"
#define TEST_CHAR_POT       "\302\212"
#define TEST_CHAR_SLIDER    "\302\213"
#define TEST_CHAR_SWITCH    "\302\214"
#define TEST_CHAR_TRIM      "\302\215"
#define TEST_CHAR_INPUT     "\302\216"
#define TEST_CHAR_FUNCTION  "\302\217"
#define TEST_CHAR_CYC       "\302\220"
#define TEST_CHAR_TRAINER   "\302\221"
#define TEST_CHAR_CHANNEL   "\302\222"
#define TEST_CHAR_TELEMETRY "\302\223"
#define TEST_CHAR_LUA       "\302\224"
// clang-format on

//#define EXTRA_TEST TEST_CHAR_RIGHT TEST_CHAR_LEFT TEST_CHAR_UP TEST_CHAR_DOWN
#define EXTRA_TEST                                               \
  TEST_CHAR_DELTA TEST_CHAR_STICK TEST_CHAR_POT TEST_CHAR_SLIDER \
      TEST_CHAR_SWITCH TEST_CHAR_TRIM TEST_CHAR_INPUT

#define EXTRA_TEST2                                                    \
  TEST_CHAR_FUNCTION TEST_CHAR_CYC TEST_CHAR_TRAINER TEST_CHAR_CHANNEL \
      TEST_CHAR_TELEMETRY TEST_CHAR_LUA

#define EXTRA_FULL EXTRA_TEST EXTRA_TEST2

TEST(Lcd_colorlcd, extra_font)
{
  BitmapBuffer dc(BMP_RGB565, LCD_W, LCD_H);
  dc.clear(COLOR_THEME_SECONDARY3);

  dc.drawText(10, 25, EXTRA_FULL, COLOR_THEME_SECONDARY1 | FONT(XXS));
  dc.drawText(10, 40, EXTRA_FULL, COLOR_THEME_SECONDARY1 | FONT(XS));
  dc.drawText(10, 55, EXTRA_FULL, COLOR_THEME_SECONDARY1 | FONT(L));
  dc.drawText(10, 80, EXTRA_FULL, COLOR_THEME_SECONDARY1 | FONT(XL));
  dc.drawText(10, 120, EXTRA_TEST, COLOR_THEME_SECONDARY1 | FONT(XXL));
  dc.drawText(10, 184, EXTRA_TEST2, COLOR_THEME_SECONDARY1 | FONT(XXL));

  EXPECT_TRUE(checkScreenshot_colorlcd(&dc, "extra"));
}

constexpr coord_t LBM_USB_PLUGGED_W = 211;
constexpr coord_t LBM_USB_PLUGGED_H = 110;

const uint8_t LBM_USB_PLUGGED[] = {
#include "mask_usb_symbol.lbm"
};

TEST(Lcd_colorlcd, darkmode)
{
  BitmapBuffer dc(BMP_RGB565, LCD_W, LCD_H);

  dc.clear(COLOR_BLACK);

  dc.drawText(20, 28, "The quick brown fox jumps over the lazy dog", COLOR_WHITE);
  dc.drawSolidFilledRect(28, 56, 422, 2, COLOR2FLAGS(RGB(0xFF, 0, 0)));
  dc.drawBitmapPattern(LCD_W/2 - LBM_USB_PLUGGED_W/2,
                       LCD_H/2 - LBM_USB_PLUGGED_H/2,
                       LBM_USB_PLUGGED,
                       COLOR2FLAGS(RGB(0, 0, 0xFF)));

  EXPECT_TRUE(checkScreenshot_colorlcd(&dc, "darkmode_" TRANSLATIONS));
}
#endif

#endif
