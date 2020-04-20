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

template <class T>
class BufferedWindow: public T
{
  public:
    using T::T;

    ~BufferedWindow() override
    {
      delete bitmap;
    }

    void invalidate(const rect_t & rect) override
    {
      paintUpdateNeeded = true;
      T::invalidate(rect);
    }

  protected:
    BitmapBuffer * bitmap = nullptr;
    bool paintUpdateNeeded = false;
    virtual void paintUpdate(BitmapBuffer * dc) = 0;
};

template <class T>
class OpaqueBufferedWindow: public BufferedWindow<T>
{
  public:
    using BufferedWindow<T>::BufferedWindow;

    void paint(BitmapBuffer * dc) override
    {
      if (!this->bitmap) {
        this->bitmap = new BitmapBuffer(BMP_RGB565, T::width(), T::height());
        this->paintUpdateNeeded = true;
      }

      if (this->paintUpdateNeeded) {
        this->paintUpdate(this->bitmap);
        this->paintUpdateNeeded = false;
      }

      dc->drawBitmap(0, 0, this->bitmap);
    }
};

template <class T>
class TransparentBufferedWindow: public BufferedWindow<T>
{
  public:
    using BufferedWindow<T>::BufferedWindow;

    void paint(BitmapBuffer * dc) override
    {
      if (!BufferedWindow<T>::bitmap) {
        this->bitmap = new BitmapBuffer(BMP_RGB565, T::width(), T::height());
        this->paintUpdateNeeded = true;
      }

      if (this->paintUpdateNeeded) {
        this->paintUpdate(dc);
        // we can only store the bitmap if fully drawn
        coord_t xmin, xmax, ymin, ymax;
        dc->getClippingRect(xmin, xmax, ymin, ymax);
        if (xmax - xmin >= this->width() && ymax - ymin >= this->height()) {
          this->bitmap->drawBitmap(0, 0, dc, dc->getOffsetX(), dc->getOffsetY());
          this->paintUpdateNeeded = false;
        }
      }
      else {
        dc->drawBitmap(0, 0, this->bitmap);
      }
    }
};

class TransparentBitmapBackground: public TransparentBufferedWindow<Window>
{
  public:
    TransparentBitmapBackground(Window * parent, const rect_t & rect, const BitmapBuffer * bitmap):
      TransparentBufferedWindow<Window>(parent, rect, OPAQUE),
      background(bitmap)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "TransparentBitmapBackground";
    }
#endif

  protected:
    const BitmapBuffer * background;

    void paintUpdate(BitmapBuffer * dc) override
    {
      if (background) {
        dc->drawBitmap((width() - background->width()) / 2, (height() - background->height()) / 2, background);
      }
    }
};

#endif // _BUFFERED_WINDOW_H_
