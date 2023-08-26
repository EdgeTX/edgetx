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
  if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
    Slider* sl = (Slider*)lv_event_get_user_data(e);
    if (sl != nullptr) {
      lv_obj_t* target = lv_event_get_target(e);
      sl->setValue(lv_slider_get_value(target));
    }
  }
}

Slider::Slider(Window* parent, coord_t width, int32_t vmin, int32_t vmax,
               std::function<int()> getValue,
               std::function<void(int)> setValue) :
    Window(parent, {0, 0, width, 32}, 0, 0, window_create),
    vmin(vmin),
    vmax(vmax),
    _getValue(std::move(getValue)),
    _setValue(std::move(setValue))
{
  padAll(7);

  auto slider = (new FormField(this, rect_t{}, 0, 0, etx_slider_create))->getLvObj();
  lv_obj_set_width(slider, lv_pct(100));

  lv_obj_add_event_cb(slider, slider_changed_cb, LV_EVENT_VALUE_CHANGED, this);
  lv_slider_set_range(slider, vmin, vmax);

  if (_getValue != nullptr)
    lv_slider_set_value(slider, _getValue(), LV_ANIM_OFF);
}
