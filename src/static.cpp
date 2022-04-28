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

#include "static.h"
#include "font.h"

StaticText::StaticText(Window* parent, const rect_t& rect, std::string txt,
                       WindowFlags windowFlags, LcdFlags textFlags) :
    Window(parent, rect, windowFlags, textFlags, lv_label_create),
    text(std::move(txt))
{
  lv_label_set_text(lvobj, text.c_str());
  lv_obj_set_height(lvobj, LV_SIZE_CONTENT);
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);

  lv_obj_set_style_text_font(lvobj, getFont(textFlags), LV_PART_MAIN);
  lv_obj_set_style_text_color(lvobj, makeLvColor(textFlags), LV_PART_MAIN);

  if (textFlags & CENTERED)
    lv_obj_set_style_text_align(lvobj, LV_TEXT_ALIGN_CENTER, 0);
  else if (textFlags & RIGHT)
    lv_obj_set_style_text_align(lvobj, LV_TEXT_ALIGN_RIGHT, 0);

  if (windowFlags & BUTTON_BACKGROUND) {
    lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, LV_PART_MAIN);
    setBackgroundColor(COLOR_THEME_SECONDARY2);
  }
}

#if defined(DEBUG_WINDOWS)
std::string StaticText::getName() const
{
  return "StaticText \"" + text + "\"";
}
#endif

void StaticText::setText(std::string value)
{
  if (text != value) {
    text = std::move(value);
    if (lvobj) lv_label_set_text(lvobj, text.c_str());
  }
}

const std::string& StaticText::getText() const
{
  return text;
}

void StaticText::setBackgroundColor(LcdFlags color)
{
  bgColor = color;

  if (!lvobj) return;

  lv_color_t c = makeLvColor(bgColor);
  lv_obj_set_style_bg_color(lvobj, c, LV_PART_MAIN);
}
