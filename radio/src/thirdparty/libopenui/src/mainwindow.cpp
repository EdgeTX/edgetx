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
#include "keyboard_base.h"

#include "lvgl/lvgl.h"



MainWindow * MainWindow::_instance = nullptr;

MainWindow::MainWindow() :
    Window(nullptr, {0, 0, LCD_W, LCD_H}, OPAQUE), invalidatedRect(rect)
{
}

void MainWindow::emptyTrash()
{
  for (auto window : trash) {
    delete window;
  }
  trash.clear();
}


void MainWindow::invalidate(const rect_t& rect)
{
  if (invalidatedRect.w) {
    auto left = limit<coord_t>(0, rect.left(), invalidatedRect.left());
    auto right = limit<coord_t>(invalidatedRect.right(), rect.right(), LCD_W);
    auto top = limit<coord_t>(0, rect.top(), invalidatedRect.top());
    auto bottom =
        limit<coord_t>(rect.bottom(), invalidatedRect.bottom(), LCD_H);
    invalidatedRect = {left, top, right - left, bottom - top};
  } else {
    invalidatedRect = rect;
  }
}

void MainWindow::run(bool trash)
{
  auto start = ticksNow();

  auto opaque = Layer::getFirstOpaque();
  if (opaque) opaque->checkEvents();

  if (trash) emptyTrash();

  auto delta = ticksNow() - start;
  if (delta > 10 * SYSTEM_TICKS_1MS) {
    TRACE_WINDOWS("MainWindow::run took %dms",
                  (ticksNow() - start) / SYSTEM_TICKS_1MS);
  }
}
