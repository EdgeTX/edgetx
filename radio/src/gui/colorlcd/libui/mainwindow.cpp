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
#include "keyboard_base.h"
#include "layout.h"
#include "etx_lv_theme.h"
#include "sdcard.h"

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
  auto start = timersGetMsTick();

  auto opaque = Layer::getFirstOpaque();
  if (opaque) {
    opaque->checkEvents();
  }

  auto copy = children;
  for (auto child : copy) {
    if (!child->deleted() && child->isBubblePopup()) {
      child->checkEvents();
    }
  }

  if (trash) emptyTrash();

  auto delta = timersGetMsTick() - start;
  if (delta > 10) {
    TRACE_WINDOWS("MainWindow::run took %dms", delta);
  }
}

void MainWindow::shutdown()
{
  // Called when USB is connected in SD card mode

  // Delete main view screens
  LayoutFactory::deleteCustomScreens();

  // clear layer stack first
  for (Window* w = Layer::back(); w; w = Layer::back()) w->deleteLater();

  children.clear();
  clear();
  emptyTrash();

  // Re-add background canvas
  background = lv_canvas_create(lvobj);
  lv_obj_center(background);
}

void MainWindow::setBackgroundImage(const char* fileName)
{
  // ensure you delete old bitmap
  if (backgroundBitmap != nullptr) delete backgroundBitmap;

  if (fileName == nullptr) fileName = "";

  backgroundImageFileName = fileName;

  // Try to load bitmap. If this fails backgroundBitmap will be NULL and default
  // will be loaded in update() method
  backgroundBitmap =
      BitmapBuffer::loadBitmap(backgroundImageFileName.c_str(), BMP_RGB565);

  if (!backgroundBitmap) {
    backgroundBitmap = BitmapBuffer::loadBitmap(
        THEMES_PATH "/EdgeTX/background.png", BMP_RGB565);
  }

  if (backgroundBitmap) {
    lv_canvas_set_buffer(background, backgroundBitmap->getData(), backgroundBitmap->width(),
                         backgroundBitmap->height(), LV_IMG_CF_TRUE_COLOR);
  }
}
