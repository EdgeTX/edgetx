/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include "mainwindow.h"
#include "keyboard_base.h"

#if defined(HARDWARE_TOUCH)
#include "touch.h"
#endif

MainWindow mainWindow;

#if defined(HARDWARE_TOUCH)
TouchState touchState;
Keyboard * Keyboard::activeKeyboard = nullptr;
#endif

void MainWindow::emptyTrash()
{
  for (auto window: trash) {
    delete window;
  }
  trash.clear();
}

void MainWindow::checkEvents()
{
#if defined(HARDWARE_TOUCH)
  if (touchState.event == TE_DOWN) {
    onTouchStart(touchState.x + scrollPositionX, touchState.y + scrollPositionY);
    // touchState.Event = TE_NONE;
  }
  else if (touchState.event == TE_UP) {
    touchState.event = TE_NONE;
    onTouchEnd(touchState.startX + scrollPositionX, touchState.startY + scrollPositionY);
  }
  else if (touchState.event == TE_SLIDE) {
    coord_t x = touchState.x - touchState.lastX;
    coord_t y = touchState.y - touchState.lastY;
    onTouchSlide(touchState.x, touchState.y, touchState.startX, touchState.startY, x, y);
    touchState.lastX = touchState.x;
    touchState.lastY = touchState.y;
  }
#endif

  Window::checkEvents();
}

void MainWindow::invalidate(const rect_t & rect)
{
  if (invalidatedRect.w) {
    coord_t left = min(invalidatedRect.left(), rect.left());
    coord_t right = max(invalidatedRect.right(), rect.right());
    coord_t top = min(invalidatedRect.top(), rect.top());
    coord_t bottom = max(invalidatedRect.bottom(), rect.bottom());
    invalidatedRect = {left, top, right - left, bottom - top};
  }
  else {
    invalidatedRect = rect;
  }
}

bool MainWindow::refresh()
{
  if (invalidatedRect.w) {
    if (invalidatedRect.x > 0 || invalidatedRect.y > 0 || invalidatedRect.w < LCD_W || invalidatedRect.h < LCD_H) {
      TRACE_WINDOWS("Refresh rect: left=%d top=%d width=%d height=%d", invalidatedRect.left(), invalidatedRect.top(), invalidatedRect.w, invalidatedRect.h);
      BitmapBuffer * previous = lcd;
      lcdNextLayer();
      lcdCopy(lcd->getData(), previous->getData());
    }
    else {
      TRACE_WINDOWS("Refresh full screen");
      lcdNextLayer();
    }
    lcd->setOffset(0, 0);
    lcd->setClippingRect(invalidatedRect.left(), invalidatedRect.right(), invalidatedRect.top(), invalidatedRect.bottom());
    fullPaint(lcd);
    invalidatedRect.w = 0;
    return true;
  }
  else {
    return false;
  }
}

void MainWindow::run(bool trash)
{
  checkEvents();

  if (trash) {
    emptyTrash();
  }
  
  if (refresh()) {
    lcdRefresh();
  }
}
