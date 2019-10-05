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

#ifndef _PAGE_H_
#define _PAGE_H_

#include "window.h"
#include "button.h"

class Page;

class PageHeader: public Window {
  public:
    PageHeader(Page * parent, uint8_t icon);

    void paint(BitmapBuffer * dc) override;

    uint8_t getIcon() const
    {
      return icon;
    }

  protected:
    uint8_t icon;
#if defined(HARDWARE_TOUCH)
    IconButton back;
#endif
};

class Page: public Window {
  public:
    explicit Page(unsigned icon);

    ~Page() override;

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "Page";
    }
#endif

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchStart(coord_t x, coord_t y) override
    {
      Window::onTouchStart(x, y);
      return true;
    }

    bool onTouchEnd(coord_t x, coord_t y) override;

    bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY) override
    {
      Window::onTouchSlide(x, y, startX, startY, slideX, slideY);
      return true;
    }
#endif

    void deleteLater();

    void paint(BitmapBuffer * dc) override;

  protected:
    PageHeader header;
    FormWindow body;
    Window * previousFocus;
};

#endif // _PAGE_H_
