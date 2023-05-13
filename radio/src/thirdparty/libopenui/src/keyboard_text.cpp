/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
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

void TextKeyboard::show(FormField* field)
{
  if (!_instance) _instance = new TextKeyboard();

  lv_obj_clear_flag(_instance->lvobj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(_instance->keyboard, LV_OBJ_FLAG_HIDDEN);
  lv_keyboard_set_mode(_instance->keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);

  _instance->setField(field);
}
