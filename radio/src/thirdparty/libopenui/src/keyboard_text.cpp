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

#include "keyboard_text.h"
#include "textedit.h"
#include "font.h"

constexpr coord_t KEYBOARD_HEIGHT = LCD_H * 2 / 5;
TextKeyboard * TextKeyboard::_instance = nullptr;

TextKeyboard::TextKeyboard() :
  Keyboard(KEYBOARD_HEIGHT)
{
}

TextKeyboard::~TextKeyboard()
{
  _instance = nullptr;
}

#if defined(HARDWARE_KEYS)
void TextKeyboard::onEvent(event_t event)
{
}
#endif
