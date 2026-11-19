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

#include "startup_shutdown.h"

#include "edgetx.h"
#include "hal/abnormal_reboot.h"
#include "inactivity_timer.h"
#include "mainwindow.h"
#include "os/sleep.h"
#include "stamp.h"
#include "theme_manager.h"

constexpr const char* strip_leading_hyphen(const char* str) {
    return (str[0] == '-') ? str + 1 : str;
}

#if defined(VERSION_TAG)
const std::string ver_str = "" VERSION_TAG;
const std::string nam_str = "" CODENAME;
#else
const std::string ver_str = "" VERSION_PREFIX VERSION;
const std::string nam_str = strip_leading_hyphen("" VERSION_SUFFIX);
const std::string git_str = "(" GIT_STR ")";
#endif

const uint8_t __bmp_splash_logo[] __FLASH = {
#include "bmp_logo_edgetx_splash.lbm"
};

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
    coord_t x = (LANDSCAPE ? LCD_W / 3 : LCD_W / 2) - logo->width / 2;
    coord_t y = (LANDSCAPE ? LCD_H / 2 : LCD_H * 2 / 5) - logo->height / 2;
    new StaticLZ4Image(splashScreen, x, y, logo);

    coord_t w = LAYOUT_SCALE(200);
    x = (LANDSCAPE ? LCD_W * 4 / 5 : LCD_W / 2) - w / 2;
    y = LCD_H - EdgeTxStyles::STD_FONT_HEIGHT * 4;
    new StaticText(splashScreen, {x, y, w, EdgeTxStyles::STD_FONT_HEIGHT}, ver_str.c_str(), COLOR_GREY_INDEX, CENTERED);
    new StaticText(splashScreen, {x, y + EdgeTxStyles::STD_FONT_HEIGHT, w, EdgeTxStyles::STD_FONT_HEIGHT},
                   nam_str.c_str(), COLOR_GREY_INDEX, CENTERED);
#if !defined(VERSION_TAG)
    new StaticText(splashScreen, {x, y + EdgeTxStyles::STD_FONT_HEIGHT * 2, w, EdgeTxStyles::STD_FONT_HEIGHT},
                   git_str.c_str(), COLOR_GREY_INDEX, CENTERED);
#endif
  }

  // Refresh to show splash screen
  MainWindow::instance()->run();
}

static tmr10ms_t splashStartTime = 0;
static bool splashRunning = false;

void startSplash()
{
  if (!UNEXPECTED_SHUTDOWN()) {
    splashStartTime = get_tmr10ms();
    splashRunning = true;
    drawSplash();
  }
}

void cancelSplash()
{
  if (splashScreen) {
    splashScreen->closeWindow();
    splashScreen = nullptr;
    splashStartTime = 0;
  }
}

void waitSplash()
{
  // Handle color splash screen
  if (splashRunning) {
    inactivityCheckInputs();
    splashStartTime += SPLASH_TIMEOUT;

#if defined(PWR_BUTTON_PRESS)
    // Wait for the power button(s) to be released,
    // otherwise pwrCheck() starts the shutdown countdown
    while (pwrPressed()) {
      WDG_RESET();
      sleep_ms(10);
    }
#if defined(PWR_BUTTON_DUAL)
    // Kill SYS and/or MDL events
    killAllEvents();
#endif
    MainWindow::instance()->run();
#endif

    MainWindow::instance()->blockUntilClose(true, [=]() {
      if (splashStartTime < get_tmr10ms())
        return true;
      auto evt = getEvent();
      if (evt || inactivityCheckInputs()) {
        if (evt) killEvents(evt);
        return true;
      }
      return false;
    });

    // Reset timer so special/global functions set to !1x don't get triggered
    START_SILENCE_PERIOD();
  }

  cancelSplash();
}

static LAYOUT_VAL_SCALED_ODD(SHUTDOWN_CIRCLE_RADIUS, 75)
static LAYOUT_VAL_SCALED_EVEN(SHUTDOWN_ICON_SIZE, 90)
static LAYOUT_VAL_SCALED_EVEN(SHUTDOWN_ICON_SIZE_INNER, 62)

static Window* shutdownWindow = nullptr;
static lv_obj_t* shutdownAnim[4] = {nullptr};
static BitmapBuffer* shutdownSplashImg = nullptr;

lv_obj_t* createArc(lv_obj_t* parent, uint16_t pw, uint16_t ph, 
                    uint16_t rotation, uint16_t size, uint16_t startAngle, uint16_t endAngle,
                    uint16_t thickness, LcdColorIndex color)
{
  lv_obj_t* arc = lv_arc_create(parent);
  lv_arc_set_rotation(arc, rotation);
  lv_arc_set_bg_angles(arc, 0, 360);
  lv_arc_set_range(arc, 0, 360);
  lv_arc_set_angles(arc, startAngle, endAngle);
  lv_arc_set_start_angle(arc, 0);
  lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_pos(arc, (pw - size) / 2, (ph - size) / 2);
  lv_obj_set_size(arc, size, size);
  lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_arc_opa(arc, LV_OPA_COVER, LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(arc, thickness, LV_PART_INDICATOR);
  lv_obj_set_style_arc_rounded(arc, thickness, LV_PART_INDICATOR);
  etx_arc_color(arc, color, LV_PART_INDICATOR);

  return arc;
}

lv_obj_t* shutdownArc(uint16_t rotation)
{
  return createArc(shutdownWindow->getLvObj(), LCD_W, LCD_H, rotation + 10, SHUTDOWN_CIRCLE_RADIUS * 2, 20, 70, PAD_LARGE * 2, COLOR_THEME_PRIMARY2_INDEX);
}

void shutdownIcon()
{
  // Center circle
  lv_obj_t* icn = lv_obj_create(shutdownWindow->getLvObj());
  lv_obj_set_size(icn, SHUTDOWN_ICON_SIZE, SHUTDOWN_ICON_SIZE);
  lv_obj_set_pos(icn, (LCD_W - SHUTDOWN_ICON_SIZE) / 2, (LCD_H - SHUTDOWN_ICON_SIZE) / 2);
  etx_obj_add_style(icn, styles->circle, LV_PART_MAIN);
  etx_solid_bg(icn, COLOR_THEME_PRIMARY2_INDEX);

  // Center arc
  createArc(icn, SHUTDOWN_ICON_SIZE, SHUTDOWN_ICON_SIZE, 300, SHUTDOWN_ICON_SIZE_INNER, 0, 300, PAD_LARGE, COLOR_THEME_PRIMARY1_INDEX);

  // Vertical line
  icn = lv_obj_create(icn);
  lv_obj_set_size(icn, PAD_LARGE, SHUTDOWN_ICON_SIZE / 2 - PAD_LARGE);
  lv_obj_set_pos(icn, (SHUTDOWN_ICON_SIZE - PAD_LARGE) / 2, PAD_LARGE);
  etx_obj_add_style(icn, styles->rounded, LV_PART_MAIN);
  etx_solid_bg(icn, COLOR_THEME_PRIMARY1_INDEX);
}

void drawSleepBitmap()
{
  cancelShutdownAnimation();

  shutdownWindow = new Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H});
  shutdownWindow->setWindowFlag(OPAQUE);
  etx_solid_bg(shutdownWindow->getLvObj(), COLOR_THEME_PRIMARY1_INDEX);

  shutdownIcon();

  // Force screen refresh
  lv_refr_now(nullptr);
}

void cancelShutdownAnimation()
{
  if (shutdownWindow)
    shutdownWindow->closeWindow();
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
    if (shutdownSplashImg)
      shutdownSplashImg->addCanvas(shutdownWindow);

    shutdownIcon();

    shutdownAnim[0] = shutdownArc(270);
    shutdownAnim[1] = shutdownArc(0);
    shutdownAnim[2] = shutdownArc(90);
    shutdownAnim[3] = shutdownArc(180);

    shutdownWindow->onClosing([=]() {
      // Detach before the parent deletes the canvas
      if (shutdownSplashImg) shutdownSplashImg->removeCanvas();
      shutdownWindow = nullptr;
      for (int i = 0; i < 4; i += 1) shutdownAnim[i] = nullptr;
    });
  }

  int quarter = 4 - (duration * 5) / totalDuration;
  if (quarter < 0) quarter = 0;
  for (int i = 3; i >= quarter; i -= 1)
    lv_obj_add_flag(shutdownAnim[i], LV_OBJ_FLAG_HIDDEN);

  MainWindow::instance()->run();
}

Window* drawFatalErrorScreen(const char* message)
{
  auto w = new Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H});
  w->setWindowFlag(OPAQUE);
  etx_solid_bg(w->getLvObj(), COLOR_BLACK_INDEX);

  new StaticText(w, rect_t{0, LCD_H / 2 - EdgeTxStyles::STD_FONT_HEIGHT, LCD_W, EdgeTxStyles::STD_FONT_HEIGHT * 2},
                 message, COLOR_WHITE_INDEX, FONT(XL) | CENTERED);

  return w;
}

void runFatalErrorScreen(const char* message)
{
  drawFatalErrorScreen(message);

  MainWindow::instance()->blockUntilClose(true, []() {
    return false;
  }, true);
}
