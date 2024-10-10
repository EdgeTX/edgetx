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

#include "hal/abnormal_reboot.h"
#include "inactivity_timer.h"
#include "edgetx.h"
#include "stamp.h"
#include "theme_manager.h"

extern void checkSpeakerVolume();

#if defined(VERSION_TAG)
const std::string ver_str = "" VERSION_TAG;
const std::string nam_str = "" CODENAME;
#if PORTRAIT_LCD
#define TXT_Y 404
#else
#define TXT_Y (LCD_H * 3 / 4)
#endif
#else
const std::string ver_str = "" VERSION;
const std::string nam_str = "" VERSION_SUFFIX;
const std::string git_str = "(" GIT_STR ")";
#if PORTRAIT_LCD
#define TXT_Y 380
#else
#define TXT_Y (LCD_H * 2 / 3)
#endif
#endif

static LAYOUT_VAL(TXT_H, 24, 24)

#if !PORTRAIT_LCD
#define TXT_X (LCD_W * 4 / 5)
#define IMG_X (LCD_W / 3)
#define IMG_Y (LCD_H / 2)
#else
#define TXT_X (LCD_W / 2)
#define IMG_X (LCD_W / 2)
#define IMG_Y (LCD_H * 2 / 5)
#endif

const uint8_t __bmp_splash_logo[]{
#include "splash_logo.lbm"
};

void draw_splash_cb(lv_event_t* e)
{
  auto draw_ctx = lv_event_get_draw_ctx(e);
  auto splashImg = (BitmapBuffer*)lv_event_get_user_data(e);

  if (splashImg) {
    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc);

    lv_area_t coords;
    coords.x1 = (LCD_W / 2) - (splashImg->width() / 2);
    coords.y1 = (LCD_H / 2) - (splashImg->height() / 2);
    coords.x2 = coords.x1 + splashImg->width() - 1;
    coords.y2 = coords.y1 + splashImg->height() - 1;

    lv_draw_img_decoded(draw_ctx, &img_dsc, &coords,
                        (const uint8_t*)splashImg->getData(),
                        LV_IMG_CF_TRUE_COLOR);
  }
}

static Window* splashScreen = nullptr;

void drawSplash()
{
  if (!sdMounted()) sdInit();

  splashScreen = new Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H});
  lv_obj_set_parent(splashScreen->getLvObj(), lv_layer_top());

  etx_solid_bg(splashScreen->getLvObj(), COLOR_BLACK_INDEX);

  auto bg = new StaticImage(splashScreen, {0, 0, LCD_W, LCD_H},
                            BITMAPS_PATH "/" SPLASH_FILE);
  bg->show(bg->hasImage());

  if (!bg->hasImage()) {
    LZ4Bitmap* logo = (LZ4Bitmap*)__bmp_splash_logo;
    new StaticLZ4Image(splashScreen, IMG_X - logo->width / 2,
                       IMG_Y - logo->height / 2, logo);

    new StaticText(splashScreen, {TXT_X - 100, TXT_Y, 200, 24}, ver_str.c_str(), COLOR_GREY_INDEX, CENTERED);
    new StaticText(splashScreen, {TXT_X - 100, TXT_Y + TXT_H, 200, TXT_H},
                   nam_str.c_str(), COLOR_GREY_INDEX, CENTERED);
#if !defined(VERSION_TAG)
    new StaticText(splashScreen, {TXT_X - 100, TXT_Y + TXT_H * 2, 200, TXT_H},
                   git_str.c_str(), COLOR_GREY_INDEX, CENTERED);
#endif
  }

  MainWindow::instance()->setActiveScreen();
  lv_refr_now(nullptr);
}

static tmr10ms_t splashStartTime = 0;

void startSplash()
{
  if (!UNEXPECTED_SHUTDOWN()) {
    splashStartTime = get_tmr10ms();
    drawSplash();
  }
}

void cancelSplash()
{
  if (splashScreen) {
    splashScreen->deleteLater();
    splashScreen = nullptr;
    MainWindow::instance()->setActiveScreen();
    splashStartTime = 0;
  }
}

void waitSplash()
{
  // Handle color splash screen
  if (splashStartTime) {
#if defined(SIMU)
    // Simulator - inputsMoved() returns true immediately without this!
    RTOS_WAIT_TICKS(30);
#endif  // defined(SIMU)

    splashStartTime += SPLASH_TIMEOUT;
    while (splashStartTime >= get_tmr10ms()) {
      LvglWrapper::instance()->run();
      MainWindow::instance()->run();
      WDG_RESET();
      checkSpeakerVolume();
      checkBacklight();
      RTOS_WAIT_TICKS(10);
      auto evt = getEvent();
      if (evt || inactivityCheckInputs()) {
        if (evt) killEvents(evt);
        break;
      }
#if defined(SIMU)
      // Allow simulator to exit if closed while splash showing
      uint32_t pwr_check = pwrCheck();
      if (pwr_check == e_power_off) {
        break;
      }
#endif  // defined(SIMU)
    }

    // Reset timer so special/global functions set to !1x don't get triggered
    START_SILENCE_PERIOD();
  }

  cancelSplash();
}

#define SHUTDOWN_CIRCLE_RADIUS 75

const int8_t bmp_shutdown_xo[] = {0, 0, -SHUTDOWN_CIRCLE_RADIUS,
                                  -SHUTDOWN_CIRCLE_RADIUS};
const int8_t bmp_shutdown_yo[] = {-SHUTDOWN_CIRCLE_RADIUS, 0, 0,
                                  -SHUTDOWN_CIRCLE_RADIUS};

static Window* shutdownWindow = nullptr;
static StaticIcon* shutdownAnim[4] = {nullptr};
static BitmapBuffer* shutdownSplashImg = nullptr;
static lv_obj_t* shutdownCanvas = nullptr;

void drawSleepBitmap()
{
  if (shutdownWindow) {
    shutdownWindow->clear();
  } else {
    shutdownWindow =
        new Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H});
    shutdownWindow->setWindowFlag(OPAQUE);
    etx_solid_bg(shutdownWindow->getLvObj(), COLOR_THEME_PRIMARY1_INDEX);
  }

  (new StaticIcon(shutdownWindow, 0, 0, ICON_SHUTDOWN, COLOR_THEME_PRIMARY2_INDEX))
      ->center(LCD_W, LCD_H);

  LvglWrapper::instance()->run();
}

void cancelShutdownAnimation()
{
  if (shutdownWindow) {
    shutdownWindow->deleteLater();
    shutdownWindow = nullptr;
    shutdownCanvas = nullptr;
    for (int i = 0; i < 4; i += 1) shutdownAnim[i] = nullptr;
  }
}

void drawShutdownAnimation(uint32_t duration, uint32_t totalDuration,
                           const char* message)
{
  if (totalDuration == 0) return;

  if (shutdownWindow == nullptr) {
    shutdownWindow =
        new Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H});
    shutdownWindow->setWindowFlag(OPAQUE);
    etx_solid_bg(shutdownWindow->getLvObj(), COLOR_THEME_PRIMARY1_INDEX);

    if (sdMounted() && !shutdownSplashImg)
      shutdownSplashImg = BitmapBuffer::loadBitmap(
          BITMAPS_PATH "/" SHUTDOWN_SPLASH_FILE, BMP_RGB565);

    if (shutdownSplashImg) {
      shutdownCanvas = lv_canvas_create(shutdownWindow->getLvObj());
      lv_obj_center(shutdownCanvas);
      lv_canvas_set_buffer(shutdownCanvas, shutdownSplashImg->getData(),
                           shutdownSplashImg->width(),
                           shutdownSplashImg->height(), LV_IMG_CF_TRUE_COLOR);
    }
    (new StaticIcon(shutdownWindow, 0, 0, ICON_SHUTDOWN, COLOR_THEME_PRIMARY2_INDEX))
        ->center(LCD_W, LCD_H);

    for (int i = 0; i < 4; i += 1) {
      shutdownAnim[i] = new StaticIcon(
          shutdownWindow, LCD_W / 2 + bmp_shutdown_xo[i],
          LCD_H / 2 + bmp_shutdown_yo[i],
          (EdgeTxIcon)(ICON_SHUTDOWN_CIRCLE0 + i), COLOR_THEME_PRIMARY2_INDEX);
    }
  }

  int quarter = 4 - (duration * 5) / totalDuration;
  if (quarter < 0) quarter = 0;
  for (int i = 3; i >= quarter; i -= 1) shutdownAnim[i]->hide();

  LvglWrapper::instance()->run();
}

void drawFatalErrorScreen(const char* message)
{
  static Window* fatalErrorWindow = nullptr;

  if (!fatalErrorWindow) {
    fatalErrorWindow =
        new Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H});
    fatalErrorWindow->setWindowFlag(OPAQUE);
    etx_solid_bg(fatalErrorWindow->getLvObj(), COLOR_BLACK_INDEX);

    new StaticText(fatalErrorWindow, rect_t{0, LCD_H / 2 - 20, LCD_W, 40},
                   message, COLOR_WHITE_INDEX, FONT(XL) | CENTERED);
  }

  backlightEnable(100);
  LvglWrapper::instance()->run();
}

void runFatalErrorScreen(const char* message)
{
  lcdInitDisplayDriver();

  while (true) {
    drawFatalErrorScreen(message);
    WDG_RESET();

    // loop as long as PWR button is pressed
    while (true) {
      uint32_t pwr_check = pwrCheck();
      if (pwr_check == e_power_off) {
        boardOff();
        return;  // only happens in SIMU, required for proper shutdown
      } else if (pwr_check == e_power_on) {
        break;
      }
      WDG_RESET();
    }
  }
}
