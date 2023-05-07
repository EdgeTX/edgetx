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

#include "textedit.h"
#include "font.h"

#include "widgets/field_edit.h"

#if !defined(STR_EDIT)
#define STR_EDIT "Edit"
#endif

#if !defined(STR_CLEAR)
#define STR_CLEAR "Clear"
#endif

#include "keyboard_text.h"

#if defined(HARDWARE_KEYS)
#include "menu.h"
#endif

#if defined(CLIPBOARD)
#include "clipboard.h"
#endif

TextEdit::TextEdit(Window *parent, const rect_t &rect, char *value,
                   uint8_t length, LcdFlags windowFlags) :
    FormField(parent, rect, windowFlags, 0,
              field_edit_create),
    value(value),
    length(length)
{
  // properties
  lv_obj_set_scrollbar_mode(lvobj, LV_SCROLLBAR_MODE_OFF);
  lv_textarea_set_password_mode(lvobj, false);
  lv_textarea_set_one_line(lvobj, true);

  lv_textarea_set_placeholder_text(lvobj, "---");
  lv_textarea_set_max_length(lvobj, length);

  setHeight(33);
  padTop(5);
  padLeft(4);
  lv_obj_set_style_radius(lvobj, 4, 0);

  if (width() == 0) {
    lv_obj_set_width(lvobj, LV_DPI_DEF);
  }

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

void TextEdit::onFocusLost()
{
  changeEnd();
  FormField::onFocusLost();
}
