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

#include "textedit.h"
#include "font.h"

#include "keyboard_text.h"

#if defined(HARDWARE_KEYS)
#include "menu.h"
#endif

TextEdit::TextEdit(Window *parent, const rect_t &rect, char *value,
                   uint8_t length, LcdFlags windowFlags) :
    FormField(parent, rect, windowFlags, 0,
              etx_text_edit_create),
    value(value),
    length(length)
{
  lv_textarea_set_placeholder_text(lvobj, "---");
  lv_textarea_set_max_length(lvobj, length);

  update();
}

void TextEdit::update()
{
  // value may not be null-terminated
  std::string txt(value, length);
  lv_textarea_set_text(lvobj, txt.c_str());  
}

void TextEdit::trim()
{
  for (int i = length - 1; i >= 0; i--) {
    if (value[i] == ' ' || value[i] == '\0')
      value[i] = '\0';
    else
      break;
  }
}

void TextEdit::changeEnd(bool forceChanged)
{
  if (lvobj == nullptr) return;

  bool changed = false;
  auto text = lv_textarea_get_text(lvobj);
  if (strncmp(value, text, length) != 0) {
    changed = true;
  }

  if (changed || forceChanged) {
    strncpy(value, text, length);
    trim();
    FormField::changeEnd();
  }
}

void TextEdit::onClicked()
{
  TextKeyboard::show(this);
}
