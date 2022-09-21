/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
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

#include "list_line_button.h"
#include "opentx.h"

#include "lvgl_widgets/input_mix_line.h"

void ListLineButton::value_changed(lv_event_t* e)
{
  auto obj = lv_event_get_target(e);
  auto btn = (ListLineButton*)lv_obj_get_user_data(obj);
  if (btn) btn->refresh();
}

ListLineButton::ListLineButton(Window* parent, uint8_t index) :
    Button(parent, rect_t{}, nullptr, 0, 0, input_mix_line_create),
    index(index)
{
  lv_obj_add_event_cb(lvobj, ListLineButton::value_changed, LV_EVENT_VALUE_CHANGED, nullptr);
}

void ListLineButton::checkEvents()
{
  check(isActive());
  Button::checkEvents();
}
