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

#if defined(HARDWARE_TOUCH)
TouchState touchState;
#endif

static LvglWidgetFactory mainWindowFactory = { lv_obj_create, nullptr };

MainWindow * MainWindow::_instance = nullptr;

MainWindow::MainWindow() :
    Window(nullptr, {0, 0, LCD_W, LCD_H}, 0, 0, &mainWindowFactory),
    invalidatedRect(rect)
{
  Layer::push(this);
}

void MainWindow::emptyTrash()
{
  for (auto window: trash) {
    delete window;
  }
  trash.clear();
}

#if defined(HARDWARE_TOUCH)
void MainWindow::setTouchEnabled(bool enable)
{
  switch(touchEnableState)
  {
  case TouchOn:
    if(!enable)
      touchEnableState = TouchOff;
    break;
  case TouchOn_Touching:
    if(!enable)
      touchEnableState = TouchOnToOff_Touching;
    break;
  case TouchOnToOff_Touching:
    if(enable)
      touchEnableState = TouchOn_Touching;
    break;
  case TouchOff:
    if(enable)
      touchEnableState = TouchOn;
    break;
  case TouchOff_Touching:
    if(enable)
      touchEnableState = TouchOffToOn_Touching;
    break;
  case TouchOffToOn_Touching:
    if(!enable)
      touchEnableState = TouchOff_Touching;
    break;
  }
}
#endif

void MainWindow::invalidate(const rect_t & rect)
{
  if (invalidatedRect.w) {
    auto left = limit<coord_t>(0, rect.left(), invalidatedRect.left());
    auto right = limit<coord_t>(invalidatedRect.right(), rect.right(), LCD_W);
    auto top = limit<coord_t>(0, rect.top(), invalidatedRect.top());
    auto bottom = limit<coord_t>(rect.bottom(), invalidatedRect.bottom(), LCD_H);
    invalidatedRect = {left, top, right - left, bottom - top};
  }
  else {
    invalidatedRect = rect;
  }
}
#include "lvgl/lvgl.h"
extern lv_obj_t * canvas;
extern BitmapBuffer canBuf;
bool MainWindow::refresh()
{
  if (invalidatedRect.w) {
    canBuf.setOffset(0, 0);
    canBuf.setClippingRect(0,LCD_W,0, LCD_H);
    fullPaint(&canBuf);
    lv_obj_invalidate(canvas);
  }
  return false;

  if (invalidatedRect.w) {
    if (invalidatedRect.x > 0 || invalidatedRect.y > 0 || invalidatedRect.w < LCD_W || invalidatedRect.h < LCD_H) {
      TRACE_WINDOWS("Refresh rect: left=%d top=%d width=%d height=%d", invalidatedRect.left(), invalidatedRect.top(), invalidatedRect.w, invalidatedRect.h);
      lcdCopy(lcd->getData(), lcdFront->getData());
    }
    else {
      TRACE_WINDOWS("Refresh full screen");
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
  auto start = ticksNow();

  // KLK: removed for now.  This is now
  // called from lvgl event processing when
  // necessary
  checkEvents();

  if (trash) {
    emptyTrash();
  }
  
  if (refresh()) {
    lcdRefresh();
  }

  auto delta = ticksNow() - start;
  if (delta > 10 * SYSTEM_TICKS_1MS) {
    TRACE_WINDOWS("MainWindow::run took %dms", (ticksNow() - start) / SYSTEM_TICKS_1MS);
  }
}
