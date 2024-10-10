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

#include "form.h"
#include "etx_lv_theme.h"

// Slider
const lv_style_const_prop_t slider_knob_props[] = {
    LV_STYLE_CONST_PAD_TOP(9),  LV_STYLE_CONST_PAD_BOTTOM(9),
    LV_STYLE_CONST_PAD_LEFT(4), LV_STYLE_CONST_PAD_RIGHT(4),
    LV_STYLE_CONST_RADIUS(4),   LV_STYLE_CONST_BORDER_WIDTH(2),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(slider_knob, slider_knob_props);

static void slider_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_solid_bg(obj, COLOR_THEME_SECONDARY1_INDEX);
  etx_obj_add_style(obj, styles->rounded, LV_PART_MAIN);
  etx_bg_color(obj, COLOR_THEME_FOCUS_INDEX, LV_PART_MAIN | LV_STATE_FOCUSED);
  etx_bg_color(obj, COLOR_THEME_EDIT_INDEX,
               LV_PART_MAIN | LV_STATE_FOCUSED | LV_STATE_EDITED);

  etx_bg_color(obj, COLOR_THEME_FOCUS_INDEX,
               LV_PART_INDICATOR | LV_STATE_FOCUSED);
  etx_bg_color(obj, COLOR_THEME_EDIT_INDEX,
               LV_PART_INDICATOR | LV_STATE_FOCUSED | LV_STATE_EDITED);

  etx_obj_add_style(obj, styles->bg_opacity_cover, LV_PART_KNOB);
  etx_obj_add_style(obj, slider_knob, LV_PART_KNOB);
  etx_obj_add_style(obj, styles->border_color[COLOR_THEME_SECONDARY1_INDEX], LV_PART_MAIN);
  etx_obj_add_style(obj, styles->border_color[COLOR_THEME_FOCUS_INDEX],
                    LV_PART_KNOB | LV_STATE_FOCUSED);
}

static const lv_obj_class_t slider_class = {
    .base_class = &lv_slider_class,
    .constructor_cb = slider_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 8,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_slider_t),
};

static lv_obj_t* slider_create(lv_obj_t* parent)
{
  return etx_create(&slider_class, parent);
}

void Slider::slider_changed_cb(lv_event_t* e)
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
    Window(parent, {0, 0, width, 32}, window_create),
    vmin(vmin),
    vmax(vmax),
    _getValue(std::move(getValue)),
    _setValue(std::move(setValue))
{
  padTop(9);
  padLeft(8);
  padRight(8);

  slider = (new FormField(this, rect_t{}, slider_create))->getLvObj();
  lv_obj_set_width(slider, lv_pct(100));

  lv_obj_add_event_cb(slider, Slider::slider_changed_cb, LV_EVENT_VALUE_CHANGED, this);
  lv_slider_set_range(slider, vmin, vmax);

  lv_obj_add_event_cb(lvobj, Slider::on_draw, LV_EVENT_DRAW_MAIN_BEGIN,
                      nullptr);

  int range = vmax - vmin;
  if (range < 10) {
    tickPts = new lv_obj_t*[range - 1];
    for (int n = 1; n < range; n += 1) {
      lv_obj_t* p = lv_obj_create(lvobj);
      lv_obj_set_size(p, 2, 6);
      etx_solid_bg(p, COLOR_THEME_PRIMARY2_INDEX);
      tickPts[n - 1] = p;
    }
  }

  update();
}

void Slider::on_draw(lv_event_t* e)
{
  auto dlg = (Slider*)lv_obj_get_user_data(lv_event_get_target(e));
  if (dlg) dlg->delayedInit();
}

void Slider::delayedInit()
{
  if (!loaded) {
    loaded = true;

    coord_t w = lv_obj_get_width(lvobj) - 16;
    coord_t x = -1;
    int range = vmax - vmin;
    if (range < 10) {
      for (int n = 1; n < range; n += 1) {
        lv_obj_set_pos(tickPts[n - 1], x + (w * n) / range, 1);
      }
    }
  }
}

void Slider::update()
{
  if (_getValue != nullptr) {
    // Fix for lv_slider_set_value not working when using the rotary encoder to
    // update value
    auto bar = (lv_bar_t*)slider;
    bar->cur_value_anim.anim_state = -1;
    bar->cur_value_anim.anim_end = _getValue();

    lv_slider_set_value(slider, _getValue(), LV_ANIM_OFF);
  }
}

void Slider::deleteLater(bool detach, bool trash)
{
  if (!deleted()) {
    if (tickPts) delete tickPts;
    Window::deleteLater(detach, trash);
  }
}