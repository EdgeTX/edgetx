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

static lv_style_t style_main;
static lv_style_t style_ind;
static lv_style_t style_knob;
static lv_style_t style_focused;
static lv_style_t style_edit;

constexpr coord_t SLIDER_PADDING = 6;

Slider::Slider(Window* parent, const rect_t& rect, int32_t vmin, int32_t vmax,
               std::function<int()> getValue,
               std::function<void(int)> setValue) :
    FormField(parent,
              {rect.x + SLIDER_PADDING, rect.y + SLIDER_PADDING,
               rect.w - 2 * SLIDER_PADDING, rect.h - 2 * SLIDER_PADDING},
              0, 0, lv_slider_create),
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

  // LV_PART_MAIN
  lv_style_init(&style_main);
  lv_style_set_bg_opa(&style_main, LV_OPA_100);
  lv_style_set_bg_color(&style_main, makeLvColor(COLOR_THEME_SECONDARY1));
  lv_style_set_radius(&style_main, LV_RADIUS_CIRCLE);
  lv_obj_add_style(lvobj, &style_main, LV_PART_MAIN);

  // LV_PART_INDICATOR
  lv_style_init(&style_ind);
  lv_style_set_bg_opa(&style_ind, LV_OPA_100);
  lv_style_set_bg_color(&style_ind, makeLvColor(COLOR_THEME_SECONDARY1));
  lv_obj_add_style(lvobj, &style_ind, LV_PART_INDICATOR);

  // Knob
  lv_style_init(&style_knob);
  lv_style_set_bg_color(&style_knob, makeLvColor(COLOR_THEME_PRIMARY2));
  lv_style_set_border_color(&style_knob, makeLvColor(COLOR_THEME_SECONDARY1));
  lv_style_set_border_width(&style_knob, 2);
  lv_obj_add_style(lvobj, &style_knob, LV_PART_KNOB);

  // LV_STATE_FOCUSED
  lv_style_init(&style_focused);
  lv_style_set_bg_color(&style_focused, makeLvColor(COLOR_THEME_FOCUS));
  lv_style_set_outline_width(&style_focused, 0);

  lv_obj_add_style(lvobj, &style_focused, LV_PART_MAIN | LV_STATE_FOCUSED);
  lv_obj_add_style(lvobj, &style_focused, LV_PART_MAIN | LV_STATE_FOCUSED | LV_STATE_EDITED);
  lv_obj_add_style(lvobj, &style_focused, LV_PART_INDICATOR | LV_STATE_FOCUSED);
  lv_obj_add_style(lvobj, &style_focused, LV_PART_INDICATOR | LV_STATE_FOCUSED | LV_STATE_EDITED);

  lv_style_init(&style_edit);
  lv_style_set_bg_color(&style_edit, makeLvColor(COLOR_THEME_EDIT));
  lv_obj_add_style(lvobj, &style_edit, LV_PART_KNOB | LV_STATE_FOCUSED | LV_STATE_EDITED);

  // always fill a grid cell in x-direction
  lv_obj_set_style_grid_cell_x_align(lvobj, LV_GRID_ALIGN_STRETCH, 0);
}


int Slider::value(coord_t x) const
{
  return vmin + ((vmax - vmin) * x + (rect.w / 2)) / rect.w;
}

