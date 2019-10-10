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

#ifndef _THEME_H_
#define _THEME_H_

#include "bitmapbuffer.h"
#include "checkbox.h"

class PageHeader;
class ChoiceBase;

enum IconState {
  STATE_DEFAULT,
  STATE_PRESSED,
};

class Theme {
  public:
    virtual void drawMessageBox(const char * title, const char * text, const char * action, LcdFlags flags) const = 0;
    virtual void drawProgressBar(BitmapBuffer * dc, coord_t x, coord_t y, coord_t w, coord_t h, int value) const = 0;
    virtual void drawCheckBox(BitmapBuffer * dc, CheckBox * checkBox) const
    {
      return drawCheckBox(dc, checkBox->getValue(), 0, 0, checkBox->hasFocus());
    }
    virtual void drawCheckBox(BitmapBuffer * dc, bool checked, coord_t x, coord_t y, bool focus = false) const = 0;
    virtual void drawChoice(BitmapBuffer * dc, ChoiceBase * choice, const char * str) const = 0;
    virtual void drawSlider(BitmapBuffer * dc, int vmin, int vmax, int value, const rect_t & rect, bool edit, bool focus) const = 0;
    virtual const BitmapBuffer * getIcon(uint8_t index, IconState state) const = 0;
    virtual const BitmapBuffer * getIconMask(uint8_t index) const = 0;
};

extern Theme * theme;

#endif // _THEME_H_
