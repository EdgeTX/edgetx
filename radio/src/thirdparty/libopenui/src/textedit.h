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

#ifndef _TEXTEDIT_H_
#define _TEXTEDIT_H_

#include "form.h"

class TextEdit : public FormField {
  friend class TextKeyboard;

  public:
    TextEdit(Window * parent, const rect_t &rect, char * value, uint8_t length, LcdFlags flags = 0) :
      FormField(parent, rect),
      value(value),
      length(length)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "TextEdit";
    }
#endif

    uint8_t getMaxLength()
    {
      return length;
    }

    char * getData()
    {
      return value;
    }

    void paint(BitmapBuffer * dc) override;

    void onEvent(event_t event) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;

    void onFocusLost() override;
#endif

  protected:
    char * value;
    uint8_t length;
    uint8_t cursorPos = 0;

    static uint8_t getPreviousChar(uint8_t c)
    {
      if (c == 'A')
        return ' ';
      else if (c == 'a')
        return 'Z';
      else if (c == '0')
        return 'z';
      else if (c == ' ')
        return '>';
      else
        return c - 1;
    }

    static uint8_t getNextChar(uint8_t c)
    {
      if (c == ' ')
        return 'A';
      else if (c == 'Z')
        return 'a';
      else if (c == 'z')
        return '0';
      else if (c == '>')
        return ' ';
      else
        return c + 1;
    }

};

#endif // _TEXTEDIT_H_
