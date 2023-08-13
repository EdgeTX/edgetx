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

#include "static.h"
#include "font.h"

StaticText::StaticText(Window* parent, const rect_t& rect, std::string txt,
                       WindowFlags windowFlags, LcdFlags textFlags) :
    Window(parent, rect, windowFlags, textFlags, lv_label_create),
    text(std::move(txt))
{
  lv_obj_enable_style_refresh(false);
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);

  lv_obj_set_style_text_font(lvobj, getFont(textFlags), LV_PART_MAIN);
  lv_obj_set_style_text_color(lvobj, makeLvColor(textFlags), LV_PART_MAIN);

  if (textFlags & CENTERED)
    lv_obj_set_style_text_align(lvobj, LV_TEXT_ALIGN_CENTER, 0);
  else if (textFlags & RIGHT)
    lv_obj_set_style_text_align(lvobj, LV_TEXT_ALIGN_RIGHT, 0);

  lv_obj_set_style_grid_cell_x_align(lvobj, LV_GRID_ALIGN_STRETCH, 0);
  lv_label_set_text(lvobj, text.c_str());
  lv_obj_set_height(lvobj, LV_SIZE_CONTENT);
  lv_obj_enable_style_refresh(true);
  lv_obj_refresh_style(lvobj, LV_PART_ANY, LV_STYLE_PROP_ANY);
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

void StaticText::setBackgroudOpacity(uint32_t opa)
{
  lv_obj_set_style_bg_opa(lvobj,  opa, LV_PART_MAIN);
}

void StaticText::setFont(LcdFlags font)
{
  lv_obj_set_style_text_font(lvobj, getFont(font), 0);
}

template <>
void DynamicNumber<uint32_t>::updateText()
{
  if (lvobj) {
    const char* p = prefix ? prefix : "";
    const char* s = suffix ? suffix : "";
    if ((textFlags & PREC2) == PREC2) {
      lv_label_set_text_fmt(lvobj, "%s%" PRIu32 ".%02" PRIu32 "%s", p,
                            value / 100, value % 100, s);
    } else if (textFlags & PREC1) {
      lv_label_set_text_fmt(lvobj, "%s%" PRIu32 ".%01" PRIu32 "%s", p,
                            value / 10, value % 10, s);
    } else {
      lv_label_set_text_fmt(lvobj, "%s%" PRIu32 "%s", p, value, s);
    }
  }
}

template <>
void DynamicNumber<int32_t>::updateText()
{
  if (lvobj) {
    const char* p = prefix ? prefix : "";
    const char* s = suffix ? suffix : "";
    if ((textFlags & PREC2) == PREC2) {
      lv_label_set_text_fmt(lvobj, "%s%" PRId32 ".%02" PRIu32 "%s", p,
                            value / 100, (uint32_t)abs(value % 100), s);
    } else if (textFlags & PREC1) {
      lv_label_set_text_fmt(lvobj, "%s%" PRId32 ".%01" PRIu32 "%s", p,
                            value / 10, (uint32_t)abs(value % 10), s);
    } else {
      lv_label_set_text_fmt(lvobj, "%s%" PRId32 "%s", p, value, s);
    }
  }
}

template <>
void DynamicNumber<uint16_t>::updateText()
{
  if (lvobj) {
    const char* p = prefix ? prefix : "";
    const char* s = suffix ? suffix : "";
    if ((textFlags & PREC2) == PREC2) {
      lv_label_set_text_fmt(lvobj, "%s%" PRIu16 ".%02" PRIu16 "%s", p,
                            (uint16_t)(value / 100), (uint16_t)(value % 100), s);
    } else if (textFlags & PREC1) {
      lv_label_set_text_fmt(lvobj, "%s%" PRIu16 ".%01" PRIu16 "%s", p,
                            (uint16_t)(value / 10), (uint16_t)(value % 10), s);
    } else {
      lv_label_set_text_fmt(lvobj, "%s%" PRIu16 "%s", p, value, s);
    }
  }
}

template <>
void DynamicNumber<int16_t>::updateText()
{
  if (lvobj) {
    const char* p = prefix ? prefix : "";
    const char* s = suffix ? suffix : "";
    if ((textFlags & PREC2) == PREC2) {
      lv_label_set_text_fmt(lvobj, "%s%" PRId16 ".%02" PRIu16 "%s", p,
                            (int16_t)(value / 100), (uint16_t)abs(value % 100), s);
    } else if (textFlags & PREC1) {
      lv_label_set_text_fmt(lvobj, "%s%" PRId16 ".%01" PRIu16 "%s", p,
                            (int16_t)(value / 10), (uint16_t)abs(value % 10), s);
    } else {
      lv_label_set_text_fmt(lvobj, "%s%" PRId16 "%s", p, value, s);
    }
  }
}
