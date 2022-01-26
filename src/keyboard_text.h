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

#include "keyboard_base.h"

#define KEYBOARD_SPACE         "\t"
#define KEYBOARD_ENTER         "\n"
#define KEYBOARD_BACKSPACE     "\200"
#define KEYBOARD_SET_UPPERCASE "\201"
#define KEYBOARD_SET_LOWERCASE "\202"
#define KEYBOARD_SET_LETTERS   "\203"
#define KEYBOARD_SET_NUMBERS   "\204"

#define KEYBOARD_OFFSET_Y 15
#define KEYBOARD_ROW_HEIGHT 40
#define KEYBOARD_CHAR_WIDTH 30
#define KEYBOARD_SPACE_WIDTH 135
#define KEYBOARD_ENTER_WIDTH 80
#define KEYBOARD_BITMAP_WIDTH 45

class TextKeyboard : public Keyboard {
  public:
    TextKeyboard();

    ~TextKeyboard() override;

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "TextKeyboard";
    }
#endif

    static void show(FormField * field)
    {
      if (!_instance)
        _instance = new TextKeyboard();
      else
        lv_obj_clear_flag(_instance->keyboard, LV_OBJ_FLAG_HIDDEN);

      lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
      _instance->setField(field);
    }

  protected:
#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

    static TextKeyboard * _instance;
    const char * const * layout;
  private:
    int calculateMaxWidth();
    int getCharWidth(const char c);

    char touch_key = -128;
    bool touched = false;
};

