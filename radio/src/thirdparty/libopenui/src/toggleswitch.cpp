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
#include "theme.h"


static void toggleswitch_event_handler(lv_event_t* e)
{
  lv_obj_t* target = lv_event_get_target(e);
  ToggleSwitch* cb = (ToggleSwitch*)lv_obj_get_user_data(target);

  if (cb) cb->setValue(lv_obj_get_state(target) & LV_STATE_CHECKED);
}

ToggleSwitch::ToggleSwitch(Window* parent, const rect_t& rect,
                   std::function<uint8_t()> getValue,
                   std::function<void(uint8_t)> setValue, WindowFlags flags) :
    FormField(parent, rect, flags, 0, etx_switch_create),
    _getValue(std::move(getValue)),
    _setValue(std::move(setValue))
{
  update();

  lv_obj_add_event_cb(lvobj, toggleswitch_event_handler,
                      LV_EVENT_VALUE_CHANGED, this);
}

void ToggleSwitch::update() const
{
  if (!_getValue) return;
  if (_getValue()) lv_obj_add_state(lvobj, LV_STATE_CHECKED);
  else lv_obj_clear_state(lvobj, LV_STATE_CHECKED);
}

void ToggleSwitch::onClicked()
{
  // prevent FormField::onClicked()
}
