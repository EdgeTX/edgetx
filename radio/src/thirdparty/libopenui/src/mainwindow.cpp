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
#include "themes/etx_lv_theme.h"

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
  auto start = ticksNow();

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

  auto delta = ticksNow() - start;
  if (delta > 10 * SYSTEM_TICKS_1MS) {
    TRACE_WINDOWS("MainWindow::run took %dms",
                  (ticksNow() - start) / SYSTEM_TICKS_1MS);
  }
}

void MainWindow::setBackground(const BitmapBuffer* bitmap)
{
  lv_canvas_set_buffer(background, bitmap->getData(), bitmap->width(),
                       bitmap->height(), LV_IMG_CF_TRUE_COLOR);
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
