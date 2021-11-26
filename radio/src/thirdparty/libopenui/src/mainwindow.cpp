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

MainWindow * MainWindow::_instance = nullptr;

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

void MainWindow::checkEvents()
{
#if defined(HARDWARE_TOUCH)

  if (touchPanelEventOccured()) {
    short lastDeltaX = touchState.lastDeltaX;
    short lastDeltaY = touchState.lastDeltaY;;
    touchState = touchPanelRead();
    touchState.lastDeltaX = lastDeltaX;
    touchState.lastDeltaY = lastDeltaY;
  }

  TouchEnableState currentState = touchEnableState;

  switch(touchState.event)
  {
  case TE_DOWN:
  case TE_SLIDE:
      if(touchEnableState == TouchOn)
        touchEnableState = TouchOn_Touching;
      else if (touchEnableState == TouchOff)
        touchEnableState = TouchOff_Touching;
      break;
  case TE_UP:
  case TE_SLIDE_END:
    if(touchEnableState == TouchOn_Touching)
      touchEnableState = TouchOn;
    else if (touchEnableState == TouchOff_Touching)
      touchEnableState = TouchOff;
    else if(touchEnableState == TouchOnToOff_Touching)
      touchEnableState = TouchOff;
    else if(touchEnableState == TouchOffToOn_Touching)
      touchEnableState = TouchOn;
    break;
  default:
      break;
  }

  if(touchState.event != TE_NONE)
    _touchEventOccured = true;

  if  (  currentState == TouchOff
      || currentState == TouchOff_Touching
      || currentState == TouchOffToOn_Touching)
  {
    touchState.event = TE_NONE;
  }

  if (touchState.event == TE_DOWN) {
    onTouchStart(touchState.x + scrollPositionX, touchState.y + scrollPositionY);
    slidingWindow = nullptr;
  }
  else if (touchState.event == TE_UP) {
    touchState.event = TE_NONE;
    onTouchEnd(touchState.startX + scrollPositionX, touchState.startY + scrollPositionY);
  }
  else if (touchState.event == TE_SLIDE) {
    if (touchState.deltaX || touchState.deltaY) {
      onTouchSlide(touchState.x, touchState.y, touchState.startX, touchState.startY, touchState.deltaX, touchState.deltaY);
      touchState.lastDeltaX = touchState.deltaX;
      touchState.lastDeltaY = touchState.deltaY;
      touchState.deltaX = 0;
      touchState.deltaY = 0;
    }
  }
  else if (touchState.event == TE_SLIDE_END && slidingWindow) {
    if (touchState.lastDeltaX > SLIDE_SPEED_REDUCTION)
      touchState.lastDeltaX -= SLIDE_SPEED_REDUCTION;
    else if (touchState.lastDeltaX < -SLIDE_SPEED_REDUCTION)
      touchState.lastDeltaX += SLIDE_SPEED_REDUCTION;
    else
      touchState.lastDeltaX = 0;
    if (touchState.lastDeltaY > SLIDE_SPEED_REDUCTION)
      touchState.lastDeltaY -= SLIDE_SPEED_REDUCTION;
    else if (touchState.lastDeltaY < -SLIDE_SPEED_REDUCTION)
      touchState.lastDeltaY += SLIDE_SPEED_REDUCTION;
    else
      touchState.lastDeltaY = 0;

    onTouchSlide(touchState.x, touchState.y, touchState.startX, touchState.startY, touchState.lastDeltaX, touchState.lastDeltaY);
  } else if (touchState.event == TE_SLIDE_END && slidingWindow == nullptr) {
    onTouchEnd(touchState.startX + scrollPositionX, touchState.startY + scrollPositionY);
    touchState.event = TE_NONE;
  }
#endif

  Window::checkEvents();
}

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

bool MainWindow::refresh()
{
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
