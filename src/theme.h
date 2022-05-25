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

#include "bitmapbuffer.h"
#include "checkbox.h"
#include "button.h"

class Menu;
class MenuWindowContent;
class Dialog;
class DialogWindowContent;
class ChoiceBase;

enum IconState {
  STATE_DEFAULT,
  STATE_PRESSED,
};

class Theme {
  public:
    virtual void drawProgressBar(BitmapBuffer * dc, coord_t x, coord_t y,
                                 coord_t w, coord_t h, int value, int total) const = 0;

    virtual void drawCheckBox(BitmapBuffer* dc, CheckBox* checkBox) const
    {
      drawCheckBox(dc, checkBox->getValue(), 0, FIELD_PADDING_TOP,
                   checkBox->hasFocus());
      const char* label = checkBox->getLabel();
      if (label) dc->drawText(22, FIELD_PADDING_TOP, label);
    }

    virtual void drawCheckBox(BitmapBuffer * dc, bool checked, coord_t x, coord_t y, bool focus = false) const = 0;
    virtual const BitmapBuffer * getIcon(uint8_t index, IconState state) const = 0;
    virtual const BitmapBuffer * getIconMask(uint8_t index) const = 0;

    virtual TextButton * createTextButton(FormGroup * parent, const rect_t & rect, std::string text, std::function<uint8_t(void)> pressHandler = nullptr, WindowFlags windowFlags = OPAQUE | BUTTON_BACKGROUND) const
    {
      return new TextButton(parent, rect, text, pressHandler, windowFlags);
    }
};

extern Theme * theme;

MenuWindowContent * createMenuWindow(Menu * menu);
DialogWindowContent * createDialogWindow(Dialog * dialog, const rect_t & rect);

