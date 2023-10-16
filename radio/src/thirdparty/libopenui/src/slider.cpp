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

#include "LvglWrapper.h"
#include "theme.h"

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
  padTop(9);
  padLeft(8);
  padRight(8);

  slider = (new FormField(this, rect_t{}, 0, 0, etx_slider_create))->getLvObj();
  lv_obj_set_width(slider, lv_pct(100));

  lv_obj_add_event_cb(slider, slider_changed_cb, LV_EVENT_VALUE_CHANGED, this);
  lv_slider_set_range(slider, vmin, vmax);

  if (_getValue != nullptr)
    lv_slider_set_value(slider, _getValue(), LV_ANIM_OFF);
}

void Slider::paint(BitmapBuffer* dc)
{
  coord_t w = lv_obj_get_width(slider);
  coord_t x = (lv_obj_get_width(lvobj) - w) / 2 - 1;
  int range = vmax - vmin;
  LcdFlags color = lv_obj_has_state(slider, LV_STATE_EDITED) ? COLOR_THEME_EDIT
                   : lv_obj_has_state(slider, LV_STATE_FOCUSED)
                       ? COLOR_THEME_FOCUS
                       : COLOR_THEME_SECONDARY1;

  if (range < 10) {
    for (int n = 1; n < range; n += 1) {
      dc->drawSolidRect(x + (w * n) / range, 6, 2, 2, 1, color);
    }
  }
}
