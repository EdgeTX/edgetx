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

#include "font.h"
#include "textedit.h"

constexpr coord_t KEYBOARD_HEIGHT = LCD_H * 2 / 5;
TextKeyboard* TextKeyboard::_instance = nullptr;

TextKeyboard::TextKeyboard() : Keyboard(KEYBOARD_HEIGHT) {}

TextKeyboard::~TextKeyboard() { _instance = nullptr; }

#if defined(HARDWARE_KEYS)
void TextKeyboard::onEvent(event_t event)
{
  lv_keyboard_t* kb = (lv_keyboard_t*)keyboard;

#if (defined(KEYS_GPIO_REG_PAGEUP) || defined(USE_HATS_AS_KEYS)) && !defined(PCBX12S)
  // Radios with both PGUP and PGDN buttons except X12S
  switch (event) {
    case EVT_KEY_BREAK(KEY_SYS): {
      // Change keyboard mode
      lv_keyboard_mode_t mode = lv_keyboard_get_mode(keyboard);
      mode = (mode + 1) & 3;
      lv_keyboard_set_mode(keyboard, mode);
    } break;

    case EVT_KEY_LONG(KEY_MODEL):
      killEvents(event);
      // Backspace
      lv_textarea_del_char(kb->ta);
      break;

    case EVT_KEY_BREAK(KEY_PAGEDN):
      // Cursor right
      lv_textarea_cursor_right(kb->ta);
      break;

    case EVT_KEY_BREAK(KEY_PAGEUP):
      // Cursor left
      lv_textarea_cursor_left(kb->ta);
      break;

    case EVT_KEY_LONG(KEY_PAGEDN): {
      killEvents(event);
      // Cursor to end
      int l = strlen(lv_textarea_get_text(kb->ta));
      while (lv_textarea_get_cursor_pos(kb->ta) < l)
        lv_textarea_cursor_right(kb->ta);
    } break;

    case EVT_KEY_LONG(KEY_PAGEUP):
      killEvents(event);
      // Cursor to start
      while (lv_textarea_get_cursor_pos(kb->ta) > 0)
        lv_textarea_cursor_left(kb->ta);
      break;

    case EVT_KEY_BREAK(KEY_TELE): {
      // Toggle case
      char c = lv_textarea_get_text(kb->ta)[lv_textarea_get_cursor_pos(kb->ta)];
      if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))) {
        c ^= 0x20;
        lv_textarea_del_char_forward(kb->ta);
        lv_textarea_add_char(kb->ta, c);
        lv_textarea_cursor_left(kb->ta);
      }
    } break;

    case EVT_KEY_LONG(KEY_TELE):
      killEvents(event);
      // Delete
      lv_textarea_del_char_forward(kb->ta);
      break;

    default:
      break;
  }

#else
  // Radios witb only a single PGUP/DN button or X12S
  switch (event) {
    case EVT_KEY_BREAK(KEY_SYS):
      // Cursor left
      lv_textarea_cursor_left(kb->ta);
      break;

    case EVT_KEY_LONG(KEY_SYS):
      killEvents(event);
      // Cursor to start
      while (lv_textarea_get_cursor_pos(kb->ta) > 0)
        lv_textarea_cursor_left(kb->ta);
      break;

    case EVT_KEY_BREAK(KEY_MODEL): {
      // Change keyboard mode
      lv_keyboard_mode_t mode = lv_keyboard_get_mode(keyboard);
      mode = (mode + 1) & 3;
      lv_keyboard_set_mode(keyboard, mode);
    } break;

    case EVT_KEY_LONG(KEY_MODEL):
      killEvents(event);
      // Backspace
      lv_textarea_del_char(kb->ta);
      break;

#if defined(PCBX12S)
    case EVT_KEY_BREAK(KEY_PAGEUP):
#endif
    case EVT_KEY_BREAK(KEY_PAGEDN): {
      // Toggle case
      char c = lv_textarea_get_text(kb->ta)[lv_textarea_get_cursor_pos(kb->ta)];
      if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))) {
        c ^= 0x20;
        lv_textarea_del_char_forward(kb->ta);
        lv_textarea_add_char(kb->ta, c);
        lv_textarea_cursor_left(kb->ta);
      }
    } break;

    case EVT_KEY_LONG(KEY_PAGEDN):
      killEvents(event);
      // Delete
      lv_textarea_del_char_forward(kb->ta);
      break;

    case EVT_KEY_BREAK(KEY_TELE): {
      // Cursor right
      lv_textarea_cursor_right(kb->ta);
    } break;

    case EVT_KEY_LONG(KEY_TELE): {
      killEvents(event);
      // Cursor to end
      int l = strlen(lv_textarea_get_text(kb->ta));
      while (lv_textarea_get_cursor_pos(kb->ta) < l)
        lv_textarea_cursor_right(kb->ta);
    } break;

    default:
      break;
  }

#endif
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
