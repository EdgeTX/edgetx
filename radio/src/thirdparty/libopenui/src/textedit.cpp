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

#include "keyboard_text.h"
#include "myeeprom.h"
#include "storage/storage.h"
#include "themes/etx_lv_theme.h"

#if defined(HARDWARE_KEYS)
#include "menu.h"
#endif

TextEdit::TextEdit(Window* parent, const rect_t& rect, char* value,
                   uint8_t length) :
    FormField(rect, 0), value(value), length(length)
{
  lv_obj_enable_style_refresh(false);

  // Workaround for performance issues with lv_textarea - create on top layer
  // not this window then reparent to this window after setup finished
  this->parent = parent;
  lvobj = lv_textarea_create(lv_layer_top());

  // Do this first - before any styles are applied, otherwise it is very slow
  update();

  etx_textarea_style(lvobj);

  lv_textarea_set_max_length(lvobj, length);
  lv_textarea_set_placeholder_text(lvobj, "---");

  lv_obj_set_parent(lvobj, parent->getLvObj());
  setupLVGL();

  if (rect.w == 0) setWidth(100);

  lv_obj_enable_style_refresh(true);
  lv_obj_refresh_style(lvobj, LV_PART_ANY, LV_STYLE_PROP_ANY);
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

void TextEdit::onClicked() { TextKeyboard::show(this); }

ModelTextEdit::ModelTextEdit(Window* parent, const rect_t& rect, char* value,
                             uint8_t length) :
    TextEdit(parent, rect, value, length)
{
  setChangeHandler([]() { storageDirty(EE_MODEL); });
}

RadioTextEdit::RadioTextEdit(Window* parent, const rect_t& rect, char* value,
                             uint8_t length) :
    TextEdit(parent, rect, value, length)
{
  setChangeHandler([]() { storageDirty(EE_GENERAL); });
}
