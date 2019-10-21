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

#ifndef _STATIC_H_
#define _STATIC_H_

#include "window.h"
#include "button.h" // TODO just for BUTTON_BACKGROUND

class StaticText : public Window {
  public:
    StaticText(Window * parent, const rect_t & rect, std::string text = "", WindowFlags windowFlags = 0, LcdFlags textFlags = 0) :
      Window(parent, rect, windowFlags, textFlags),
      text(std::move(text))
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "StaticText \"" + text + "\"";
    }
#endif

    void paint(BitmapBuffer * dc) override;

    void setText(std::string value)
    {
      text = std::move(value);
      invalidate();
    }

  protected:
    std::string text;
};

class Subtitle: public StaticText {
  public:
    Subtitle(Window * parent, const rect_t & rect, const char * text):
      StaticText(parent, rect, text, 0, BOLD)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "Subtitle \"" + text + "\"";
    }
#endif
};

class StaticBitmap: public Window {
  public:
    StaticBitmap(Window * parent, const rect_t & rect, const char * filename):
      Window(parent, rect),
      bitmap(BitmapBuffer::loadBitmap(filename))
    {
    }

    StaticBitmap(Window * parent, const rect_t & rect, const BitmapBuffer * bitmap):
      Window(parent, rect),
      bitmap(bitmap)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "StaticBitmap";
    }
#endif

    void paint(BitmapBuffer * dc) override
    {
      dc->drawBitmap(0, 0, bitmap);
    }

  protected:
    const BitmapBuffer * bitmap;
};

#endif
