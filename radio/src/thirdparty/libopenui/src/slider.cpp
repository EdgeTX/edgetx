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

#include "slider.h"
#include "theme.h"

#include "LvglWrapper.h"

static void slider_changed_cb(lv_event_t* e)
{
  lv_obj_t* target = lv_event_get_target(e);
  auto code = lv_event_get_code(e);

  Slider* sl = (Slider*)lv_obj_get_user_data(target);
  if (code == LV_EVENT_VALUE_CHANGED) {
    if (sl != nullptr) sl->setValue(lv_slider_get_value(target));
  }
}

Slider::Slider(Window* parent, coord_t width, int32_t vmin, int32_t vmax,
               std::function<int()> getValue,
               std::function<void(int)> setValue) :
    FormField(parent, {0, 0, width, 8}, 0, 0, etx_slider_create),
    vmin(vmin),
    vmax(vmax),
    _getValue(std::move(getValue)),
    _setValue(std::move(setValue))
{
  lv_obj_add_event_cb(lvobj, slider_changed_cb, LV_EVENT_VALUE_CHANGED, this);
  lv_obj_add_event_cb(lvobj, slider_changed_cb, LV_EVENT_PRESSED, this);
  lv_slider_set_range(lvobj, vmin, vmax);

  if (_getValue != nullptr)
    lv_slider_set_value(lvobj, _getValue(), LV_ANIM_OFF);
}


int Slider::value(coord_t x) const
{
  return vmin + ((vmax - vmin) * x + (rect.w / 2)) / rect.w;
}

