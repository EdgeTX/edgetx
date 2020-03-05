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

#ifndef _BUFFERED_WINDOW_H_
#define _BUFFERED_WINDOW_H_

#include "window.h"

class BufferedWindow: public Window
{
  public:
    BufferedWindow(Window * parent, const rect_t & rect):
      Window(parent, rect, OPAQUE)
    {
    }

    ~BufferedWindow() override
    {
      delete bitmap;
    }

    void paint(BitmapBuffer * dc) override
    {
      if (!bitmap) {
        paintInit(dc);
        bitmap = new BitmapBuffer(BMP_RGB565, width(), height());
        bitmap->drawBitmap(0, 0, dc, dc->getOffsetX(), dc->getOffsetY());
      }
      else {
        dc->drawBitmap(0, 0, bitmap);
      }
    }

  protected:
    BitmapBuffer * bitmap = nullptr;
    virtual void paintInit(BitmapBuffer * dc) = 0;
};

#endif // _BUFFERED_WINDOW_H_
