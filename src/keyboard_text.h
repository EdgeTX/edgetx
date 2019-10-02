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

#ifndef _KEYBOARD_TEXT_H_
#define _KEYBOARD_TEXT_H_

#include "keyboard_base.h"
#include "textedit.h"

class TextKeyboard : public Keyboard<TextEdit> {
  friend class TextEdit;

  public:
    TextKeyboard();

    ~TextKeyboard() override;

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "TextKeyboard";
    }
#endif

    static TextKeyboard * instance() {
      if (!_instance)
        _instance = new TextKeyboard();
      return _instance;
    }

    coord_t getCursorPos() const
    {
      return cursorPos;
    }

    void setCursorPos(coord_t x);

    void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

  protected:
    static TextKeyboard * _instance;
    coord_t cursorPos = 0;
    uint8_t cursorIndex = 0;
    const char * const * layout;
};

#endif // _KEYBOARD_TEXT_H_