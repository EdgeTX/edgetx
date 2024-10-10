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

#if (defined(KEYS_GPIO_REG_PAGEUP) || defined(USE_HATS_AS_KEYS)) && \
    !defined(PCBX12S)
// Radios with both PGUP and PGDN buttons except X12S
void TextKeyboard::onPressSYS() { changeMode(); }
void TextKeyboard::onLongPressSYS() {}
void TextKeyboard::onPressMDL() {}
void TextKeyboard::onLongPressMDL() { backspace(); }
void TextKeyboard::onPressTELE() { toggleCase(); }
void TextKeyboard::onLongPressTELE() { deleteChar(); }
void TextKeyboard::onPressPGUP() { cursorLeft(); }
void TextKeyboard::onPressPGDN() { cursorRight(); }
void TextKeyboard::onLongPressPGUP() { cursorStart(); }
void TextKeyboard::onLongPressPGDN() { cursorEnd(); }
#else
// Radios witb only a single PGUP/DN button or X12S
void TextKeyboard::onPressSYS() { cursorLeft(); }
void TextKeyboard::onLongPressSYS() { cursorStart(); }
void TextKeyboard::onPressMDL() { changeMode(); }
void TextKeyboard::onLongPressMDL() { backspace(); }
void TextKeyboard::onPressTELE() { cursorRight(); }
void TextKeyboard::onLongPressTELE() { cursorEnd(); }
#if defined(PCBX12S)
void TextKeyboard::onPressPGUP() { toggleCase(); }
#else
void TextKeyboard::onPressPGUP() { deleteChar(); }
#endif
void TextKeyboard::onPressPGDN() { toggleCase(); }
void TextKeyboard::onLongPressPGUP() {}
void TextKeyboard::onLongPressPGDN() { deleteChar(); }
#endif
#endif

void TextKeyboard::open(FormField* field)
{
  if (!_instance) _instance = new TextKeyboard();

  lv_obj_clear_flag(_instance->lvobj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(_instance->keyboard, LV_OBJ_FLAG_HIDDEN);
  lv_keyboard_set_mode(_instance->keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);

  _instance->setField(field);
}
