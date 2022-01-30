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

#include "checkbox.h"
#include "theme.h"

LvglWidgetFactory checkBoxFactory = { lv_checkbox_create, nullptr };
lv_style_t style_indicator;
lv_style_t style_edit;


static void checkbox_event_handler(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * target = lv_event_get_target(e);
  CheckBox* cb = (CheckBox *)lv_obj_get_user_data(target);
 
  if(code == LV_EVENT_VALUE_CHANGED) {
    onKeyPress();
    cb->setValue(lv_obj_get_state(target) & LV_STATE_CHECKED);
  }
}

CheckBox::CheckBox(Window * parent, const rect_t & rect, std::function<uint8_t()> getValue, std::function<void(uint8_t)> setValue, WindowFlags flags) :
  FormField(parent, rect, flags, 0, &checkBoxFactory),
  _getValue(std::move(getValue)),
  _setValue(std::move(setValue))
{
  coord_t size = min(rect.w, rect.h);
  setWidth(size);
  setHeight(size);

  // BORDER
  lv_obj_set_style_border_color(lvobj, makeLvColor(COLOR_THEME_SECONDARY2), LV_PART_INDICATOR);
  lv_obj_set_style_border_width(lvobj, 1, LV_PART_INDICATOR);

  lv_obj_set_style_border_color(lvobj, makeLvColor(COLOR_THEME_FOCUS), LV_PART_INDICATOR | LV_STATE_FOCUSED);
  lv_obj_set_style_border_width(lvobj, 2, LV_PART_INDICATOR | LV_STATE_FOCUSED);

  // UNCHECKED BACKGROUND
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_PRIMARY2), LV_PART_INDICATOR);

  if (_getValue())
    lv_obj_add_state(lvobj, LV_STATE_CHECKED);

  lv_obj_add_event_cb(lvobj, checkbox_event_handler, LV_EVENT_ALL, this);
}

#if defined(HARDWARE_KEYS)
void CheckBox::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString("CheckBox").c_str(), event);

  if (enabled && event == EVT_KEY_BREAK(KEY_ENTER)) {
    onKeyPress();
    if (lv_obj_get_state(lvobj) & LV_STATE_CHECKED)
      lv_obj_clear_state(lvobj, LV_STATE_CHECKED);
    else 
      lv_obj_add_state(lvobj, LV_STATE_CHECKED);

    _setValue(lv_obj_get_state(lvobj) & LV_STATE_CHECKED);
  }
  else {
    FormField::onEvent(event);
  }
}
#endif

