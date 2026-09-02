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

constexpr coord_t KEYBOARD_HEIGHT = LCD_H * 2 / 5;
TextKeyboard* TextKeyboard::_instance = nullptr;

TextKeyboard::TextKeyboard() : Keyboard(KEYBOARD_HEIGHT) {}

TextKeyboard::~TextKeyboard() { _instance = nullptr; }

#if defined(HARDWARE_KEYS)

void TextKeyboard::changeMode()
{
  // Change keyboard mode
  lv_keyboard_mode_t mode = lv_keyboard_get_mode(keyboard);
  mode = (mode + 1) & 3;
  lv_keyboard_set_mode(keyboard, mode);
}

void TextKeyboard::backspace()
{
  lv_keyboard_t* kb = (lv_keyboard_t*)keyboard;
  // Backspace
  lv_textarea_del_char(kb->ta);
}

void TextKeyboard::toggleCase()
{
  lv_keyboard_t* kb = (lv_keyboard_t*)keyboard;
  // Toggle case
  char c = lv_textarea_get_text(kb->ta)[lv_textarea_get_cursor_pos(kb->ta)];
  if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))) {
    c ^= 0x20;
    lv_textarea_del_char_forward(kb->ta);
    lv_textarea_add_char(kb->ta, c);
    lv_textarea_cursor_left(kb->ta);
  }
}

void TextKeyboard::deleteChar()
{
  lv_keyboard_t* kb = (lv_keyboard_t*)keyboard;
  // Delete
  lv_textarea_del_char_forward(kb->ta);
}

void TextKeyboard::cursorLeft()
{
  lv_keyboard_t* kb = (lv_keyboard_t*)keyboard;
  // Cursor left
  lv_textarea_cursor_left(kb->ta);
}

void TextKeyboard::cursorRight()
{
  lv_keyboard_t* kb = (lv_keyboard_t*)keyboard;
  // Cursor right
  lv_textarea_cursor_right(kb->ta);
}

void TextKeyboard::cursorStart()
{
  lv_keyboard_t* kb = (lv_keyboard_t*)keyboard;
  // Cursor to start
  while (lv_textarea_get_cursor_pos(kb->ta) > 0)
    lv_textarea_cursor_left(kb->ta);
}

void TextKeyboard::cursorEnd()
{
  lv_keyboard_t* kb = (lv_keyboard_t*)keyboard;
  // Cursor to end
  size_t l = strlen(lv_textarea_get_text(kb->ta));
  while (lv_textarea_get_cursor_pos(kb->ta) < l)
    lv_textarea_cursor_right(kb->ta);
}

void TextKeyboard::onPressSYS() { if (hasTwoPageKeys) changeMode(); else cursorLeft(); }
void TextKeyboard::onLongPressSYS() { if (!hasTwoPageKeys) cursorStart(); }
void TextKeyboard::onPressMDL() { if (!hasTwoPageKeys) changeMode(); }
void TextKeyboard::onLongPressMDL() { backspace(); }
void TextKeyboard::onPressTELE() { if (hasTwoPageKeys) toggleCase(); else cursorRight(); }
void TextKeyboard::onLongPressTELE() { if (hasTwoPageKeys) deleteChar(); else cursorEnd(); }
void TextKeyboard::onPressPGUP() { if (hasTwoPageKeys) cursorLeft(); else deleteChar(); }
void TextKeyboard::onPressPGDN() { if (hasTwoPageKeys) cursorRight(); else toggleCase(); }
void TextKeyboard::onLongPressPGUP() { cursorStart(); }
void TextKeyboard::onLongPressPGDN() { cursorEnd(); }
#endif

void TextKeyboard::open(FormField* field)
{
  if (!_instance) _instance = new TextKeyboard();

  lv_obj_clear_flag(_instance->lvobj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(_instance->keyboard, LV_OBJ_FLAG_HIDDEN);
  lv_keyboard_set_mode(_instance->keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);

  _instance->setField(field);
}
