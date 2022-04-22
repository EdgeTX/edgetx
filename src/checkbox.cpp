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


static void checkbox_event_handler(lv_event_t* e)
{
  lv_obj_t* target = lv_event_get_target(e);
  CheckBox* cb = (CheckBox*)lv_obj_get_user_data(target);

  onKeyPress();
  if (cb) cb->setValue(lv_obj_get_state(target) & LV_STATE_CHECKED);
}

CheckBox::CheckBox(Window* parent, const rect_t& rect,
                   std::function<uint8_t()> getValue,
                   std::function<void(uint8_t)> setValue, WindowFlags flags) :
    FormField(parent, rect, flags, 0, lv_switch_create),
    _getValue(std::move(getValue)),
    _setValue(std::move(setValue))
{
  // TODO: migrate to default theme

  if (height() > 0)
    lv_obj_set_width(lvobj, (lv_coord_t)(height() * 1.7f));

  // PART_MAIN
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_DISABLED), LV_PART_MAIN);
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_DISABLED), LV_STATE_FOCUSED);

  if (_getValue()) lv_obj_add_state(lvobj, LV_STATE_CHECKED);

  lv_obj_add_event_cb(lvobj, checkbox_event_handler,
                      LV_EVENT_VALUE_CHANGED, this);
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

#if defined(HARDWARE_TOUCH)
bool CheckBox::onTouchEnd(coord_t x, coord_t y)
{
  if (!hasFocus()) {
    setFocus(SET_FOCUS_DEFAULT);
  }

  return true;
}
#endif

