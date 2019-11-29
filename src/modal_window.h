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

#ifndef _MODAL_WINDOW_H_
#define _MODAL_WINDOW_H_

#include <vector>
#include <functional>
#include <utility>
#include "mainwindow.h"

class ModalWindow: public Window
{
  public:
    ModalWindow():
      Window(&mainWindow, {0, 0, LCD_W, LCD_H})
#if !defined(HARDWARE_TOUCH)
      , previousFocus(focusWindow)
#endif
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "ModalWindow";
    }
#endif

    void deleteLater()
    {
#if !defined(HARDWARE_TOUCH)
      if (previousFocus) {
        previousFocus->setFocus();
      }
#endif
      Window::deleteLater();
    }

    void paint(BitmapBuffer * dc) override
    {
      dc->drawFilledRect(0, 0, width(), height(), SOLID, OVERLAY_COLOR | OPACITY(5));
    }

#if defined(HARDWARE_TOUCH)
    bool onTouchStart(coord_t x, coord_t y) override
    {
      return true;
    }

    bool onTouchEnd(coord_t x, coord_t y)
    {
      if (!Window::onTouchEnd(x, y)) {
        deleteLater();
      }
      return true;
    }

    bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
    {
      Window::onTouchSlide(x, y, startX, startY, slideX, slideY);
      return true;
    }
#endif

  protected:
    Window * previousFocus = nullptr;
};

#endif // _MODAL_WINDOW_H_
