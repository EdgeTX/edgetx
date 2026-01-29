/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#include "mainwindow.h"

#include "board.h"
#include "debug.h"
#include "edgetx.h"
#include "keyboard_base.h"
#include "layout.h"
#include "LvglWrapper.h"
#include "etx_lv_theme.h"
#include "os/sleep.h"
#include "sdcard.h"
#include "view_main.h"

// timers_driver.h
uint32_t timersGetMsTick();

MainWindow* MainWindow::_instance = nullptr;

MainWindow* MainWindow::instance()
{
  if (!_instance) _instance = new MainWindow();
  return _instance;
}

MainWindow::MainWindow() : Window(nullptr, {0, 0, LCD_W, LCD_H})
{
  setWindowFlag(OPAQUE);

  etx_solid_bg(lvobj);

  background = lv_canvas_create(lvobj);
  lv_obj_center(background);
}

void MainWindow::emptyTrash()
{
  for (auto window : trash) {
    delete window;
  }
  trash.clear();
}

void MainWindow::run(bool trash)
{
  LvglWrapper::instance()->run();

#if defined(DEBUG_WINDOWS)
  auto start = timersGetMsTick();
#endif

  if (widgetRefreshEnable)
    ViewMain::refreshWidgets();

  auto opaque = Window::firstOpaque();
  if (opaque) {
    opaque->checkEvents();
  }

  auto copy = children;
  for (auto child : copy) {
    if (!child->deleted() && child->isBubblePopup()) {
      child->checkEvents();
    }
  }

  if (trash)
    emptyTrash();

#if defined(DEBUG_WINDOWS)
  auto delta = timersGetMsTick() - start;
  if (delta > 10) {
    TRACE_WINDOWS("MainWindow::run took %dms", delta);
  }
#endif
}

void MainWindow::shutdown()
{
  // Called when USB is connected in SD card mode

  // Delete main view screens
  LayoutFactory::deleteCustomScreens();

  // clear layer stack first
  for (Window* w = Window::topWindow(); w; w = Window::topWindow())
    w->deleteLater();

  clear();
  emptyTrash();

  // Re-add background canvas
  background = lv_canvas_create(lvobj);
  lv_obj_center(background);
}

bool MainWindow::setBackgroundImage(std::string& fileName)
{
  if (fileName.empty()) return false;

  // ensure you delete old bitmap
  if (backgroundBitmap != nullptr) delete backgroundBitmap;

  // Try to load bitmap.
  backgroundBitmap = BitmapBuffer::loadBitmap(fileName.c_str(), BMP_RGB565);

  if (backgroundBitmap) {
    lv_canvas_set_buffer(background, backgroundBitmap->getData(), backgroundBitmap->width(),
                         backgroundBitmap->height(), LV_IMG_CF_TRUE_COLOR);
    return true;
  }

  return false;
}

void MainWindow::blockUntilClose(bool checkPwr, std::function<bool(void)> closeCondition, bool isError)
{
  // reset input devices to avoid
  // RELEASED/CLICKED to be called in a loop
  lv_indev_reset(nullptr, nullptr);

  if (isError) {
    // refresh screen and turn backlight on
    backlightEnable(BACKLIGHT_LEVEL_MAX);
    // On startup error wait for power button to be released
    while (pwrPressed()) {
      WDG_RESET();
      run(false);
      sleep_ms(10);
    }
  }

  while (!closeCondition()) {
    if (checkPwr) {
      auto check = pwrCheck();
      if (check == e_power_off) {
        boardOff();
#if defined(SIMU)
        return;
#endif
      } else if (check == e_power_press) {
        WDG_RESET();
        sleep_ms(1);
        continue;
      }
    }

    resetBacklightTimeout();
    if (isError)
      backlightEnable(BACKLIGHT_LEVEL_MAX);
    else
      checkBacklight();

    WDG_RESET();

    run(false);
    sleep_ms(10);
  }
}
