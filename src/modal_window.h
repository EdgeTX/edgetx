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

#pragma once

#include "window.h"

class ModalWindow: public Window
{
  public:
    explicit ModalWindow(Window * parent, bool closeWhenClickOutside = false);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "ModalWindow";
    }
#endif

    void deleteLater(bool detach = true, bool trash = true) override; // NOLINT(google-default-arguments)

    void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

    void setCloseWhenClickOutside(bool value = true)
    {
      closeWhenClickOutside = value;
    }

  protected:
    bool closeWhenClickOutside;
};

class ModalWindowContent: public Window
{
  public:
    explicit ModalWindowContent(ModalWindow * parent, const rect_t & rect):
      Window(parent, rect, OPAQUE)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "ModalWindowContent";
    }
#endif

    void setTitle(std::string text)
    {
      title = std::move(text);
    }

    const std::string & getTitle()
    {
      return title;
    }

    void paint(BitmapBuffer * dc) override;

  protected:
    std::string title;
};

