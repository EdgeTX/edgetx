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

#include "toggleswitch.h"

#include "etx_lv_theme.h"

// Animation
LV_STYLE_CONST_SINGLE_INIT(anim_fast, LV_STYLE_ANIM_TIME, 120);

// Toggle switch
const lv_style_const_prop_t switch_knob_props[] = {
    LV_STYLE_CONST_PAD_TOP(-4),  LV_STYLE_CONST_PAD_BOTTOM(-4),
    LV_STYLE_CONST_PAD_LEFT(-4), LV_STYLE_CONST_PAD_RIGHT(-4),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(switch_knob, switch_knob_props);

static void switch_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_std_style(obj, LV_PART_MAIN, PAD_ZERO);
  etx_obj_add_style(obj, styles->circle, LV_PART_MAIN);
  etx_obj_add_style(obj, anim_fast, LV_PART_MAIN);

  etx_std_style(obj, LV_PART_INDICATOR, PAD_ZERO);
  etx_obj_add_style(obj, styles->circle, LV_PART_INDICATOR);

  etx_obj_add_style(obj, styles->bg_opacity_cover, LV_PART_KNOB);
  etx_obj_add_style(obj, styles->circle, LV_PART_KNOB);
  etx_obj_add_style(obj, switch_knob, LV_PART_KNOB);
  etx_bg_color(obj, COLOR_THEME_SECONDARY1_INDEX, LV_PART_KNOB);

  etx_obj_add_style(obj, styles->disabled, LV_PART_KNOB | LV_STATE_DISABLED);
}

static const lv_obj_class_t switch_class = {
    .base_class = &lv_switch_class,
    .constructor_cb = switch_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = ToggleSwitch::TOGGLE_W,
    .height_def = EdgeTxStyles::UI_ELEMENT_HEIGHT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_switch_t),
};

static lv_obj_t* switch_create(lv_obj_t* parent)
{
  return etx_create(&switch_class, parent);
}

void ToggleSwitch::toggleswitch_event_handler(lv_event_t* e)
{
  lv_obj_t* target = lv_event_get_target(e);
  ToggleSwitch* cb = (ToggleSwitch*)lv_obj_get_user_data(target);

  if (cb) cb->setValue(lv_obj_get_state(target) & LV_STATE_CHECKED);
}

ToggleSwitch::ToggleSwitch(Window* parent, const rect_t& rect,
                           std::function<uint8_t()> getValue,
                           std::function<void(uint8_t)> setValue) :
    FormField(parent, rect, switch_create),
    _getValue(std::move(getValue)),
    _setValue(std::move(setValue))
{
  update();

  lv_obj_add_event_cb(lvobj, ToggleSwitch::toggleswitch_event_handler, LV_EVENT_VALUE_CHANGED,
                      this);
}

void ToggleSwitch::update() const
{
  if (!_getValue) return;
  if (_getValue())
    lv_obj_add_state(lvobj, LV_STATE_CHECKED);
  else
    lv_obj_clear_state(lvobj, LV_STATE_CHECKED);
}

void ToggleSwitch::onClicked()
{
  // prevent FormField::onClicked()
}
