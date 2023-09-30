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

#if defined(SPLASH)

#include "opentx.h"
#include "stamp.h"

#if defined(VERSION_TAG)
const std::string ver_str = "" VERSION_TAG;
const std::string nam_str = "" CODENAME;
#else
const std::string ver_str = "" VERSION;
const std::string nam_str = "" VERSION_SUFFIX;
#endif

#define BK_COLOR  COLOR2FLAGS(BLACK)
#define TXT_COLOR COLOR2FLAGS(RGB(128,128,128))

const uint8_t __bmp_splash_logo[] {
#include "splash_logo.lbm"
};

void draw_splash_cb(lv_event_t * e)
{
  auto draw_ctx = lv_event_get_draw_ctx(e);
  auto splashImg = (BitmapBuffer*)lv_event_get_user_data(e);

  // lcd->clear(splash_background_color);

  if (splashImg) {
    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc);

    lv_area_t coords;
    coords.x1 = (LCD_W/2) - (splashImg->width()/2);
    coords.y1 = (LCD_H/2) - (splashImg->height()/2);
    coords.x2 = coords.x1 + splashImg->width()-1;
    coords.y2 = coords.y1 + splashImg->height()-1;

    lv_draw_img_decoded(draw_ctx, &img_dsc, &coords,
                        (const uint8_t *)splashImg->getData(),
                        LV_IMG_CF_TRUE_COLOR);
  }
}

void drawSplash()
{
  static bool loadSplashImg = true;
  static BitmapBuffer* splashImg = nullptr;
  static lv_obj_t* splashScreen = nullptr;

  // try splash from SD card first
  if (loadSplashImg && splashImg == nullptr) {
    if (!sdMounted()) sdInit();
    splashImg = BitmapBuffer::loadBitmap(BITMAPS_PATH "/" SPLASH_FILE, BMP_RGB565);
    loadSplashImg = false;

    if (splashImg == nullptr) {
      splashImg = new BitmapBuffer(BMP_RGB565, LCD_W, LCD_H);
      splashImg->clear(BK_COLOR);
      BitmapBuffer* splashLogo = new LZ4Bitmap(BMP_ARGB4444, __bmp_splash_logo);
      splashImg->drawBitmap((LCD_W/2) - (splashLogo->width()/2),
                            (LCD_H/2) - (splashLogo->height()/2),
                            splashLogo);
#if LCD_W > LCD_H
      splashImg->drawText(LCD_W / 5, 220, ver_str.c_str(), TXT_COLOR | CENTERED);
      splashImg->drawText(LCD_W * 4 / 5, 220, nam_str.c_str(), TXT_COLOR | CENTERED);
#else
      splashImg->drawText(LCD_W / 2, 390, ver_str.c_str(), TXT_COLOR | CENTERED);
      splashImg->drawText(LCD_W / 2, 420, nam_str.c_str(), TXT_COLOR | CENTERED);
#endif
    }

    splashScreen = window_create(nullptr);
    if (splashScreen) {
      lv_obj_add_event_cb(splashScreen, draw_splash_cb, LV_EVENT_DRAW_MAIN, splashImg);
    }
  }

  if (splashScreen) {
    lv_scr_load(splashScreen);
    lv_refr_now(nullptr);
  }

  MainWindow::instance()->setActiveScreen();
}
#endif
