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

#include "form.h"

const char extra_chars_default[] = ":;<=>";

class TextEdit : public FormField {
  friend class TextKeyboard;

  public:
    TextEdit(Window * parent, const rect_t & rect, char * value, uint8_t length, LcdFlags windowFlags = 0, const char * _extra_chars = nullptr) :
      FormField(parent, rect, windowFlags),
      value(value),
      length(length)
    {
      extra_chars = (_extra_chars) ? _extra_chars : extra_chars_default;
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "TextEdit";
    }
#endif

    uint8_t getMaxLength() const
    {
      return length;
    }

    char * getData() const
    {
      return value;
    }

    void paint(BitmapBuffer * dc) override;

    void onEvent(event_t event) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

    void onFocusLost() override;

  protected:
    char * value;
    bool changed = false;
    uint8_t length;
    const char * extra_chars;
    uint8_t cursorPos = 0;

    void trim();

    void changeEnd(bool forceChanged = false) override
    {
      cursorPos = 0;
      if (changed || forceChanged) {
        changed = false;
        trim();
        if (changeHandler) {
          changeHandler();
        }
      }
    }

    uint8_t getNextChar(uint8_t c)
    {
      if (c == ' ' || c == '\0')
        return 'A';
      else if (c >= 'A' && c < 'Z')
        return c + 1;
      else if (c == 'Z')
        return 'a';
      else if (c >= 'a' && c < 'z')
        return c + 1;
      else if (c == 'z')
        return '0';
      else if (c >= '0' && c < '9')
        return c + 1;
      else if (c == '9')
        return extra_chars[0];
      else {
        for (uint8_t n = 0; n < strlen(extra_chars) - 1; n++)
          if (c == extra_chars[n]) return extra_chars[n + 1];
        return ' ';
      }
    }

    uint8_t getPreviousChar(uint8_t c)
    {
      if (c == ' ' || c == '\0')
        return extra_chars[strlen(extra_chars) - 1];
      else if (c == 'A')
        return ' ';
      else if (c > 'A' && c <= 'Z')
        return c - 1;
      else if (c == 'a')
        return 'Z';
      else if (c > 'a' && c <= 'z')
        return c - 1;
      else if (c == '0')
        return 'z';
      else if (c > '0' && c <= '9')
        return c - 1;
      else {
        for (uint8_t n = 1; n < strlen(extra_chars); n++)
          if (c == extra_chars[n]) return extra_chars[n - 1];
        return '9';
      }
    }

    static uint8_t toggleCase(uint8_t c)
    {
      if (c >= 'A' && c <= 'Z')
        return c + 32; // tolower
      else if (c >= 'a' && c <= 'z')
        return c - 32; // toupper
      else
        return c;
    }
};

