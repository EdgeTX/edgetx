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

#include "window.h"

class ModalWindow: public Window
{
  public:
    explicit ModalWindow(Window * parent);

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "ModalWindow";
    }
#endif

    void deleteLater()
    {
      if (previousFocus) {
        previousFocus->setFocus();
      }
      Window::deleteLater();
    }

    void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchStart(coord_t x, coord_t y) override
    {
      return true;
    }

    bool onTouchEnd(coord_t x, coord_t y) override
    {
      if (!Window::onTouchEnd(x, y)) {
        deleteLater();
      }
      return true;
    }

    bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY) override
    {
      Window::onTouchSlide(x, y, startX, startY, slideX, slideY);
      return true;
    }
#endif

  protected:
    Window * previousFocus = nullptr;
};

class ModalWindowContent: public Window {
  public:
    explicit ModalWindowContent(ModalWindow * parent, const rect_t & rect):
      Window(parent, rect, OPAQUE)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "ModalWindowContent";
    }
#endif

    void setTitle(std::string text)
    {
      title = std::move(text);
    }

    void paint(BitmapBuffer * dc) override;

  protected:
    std::string title;
};

#endif // _MODAL_WINDOW_H_
