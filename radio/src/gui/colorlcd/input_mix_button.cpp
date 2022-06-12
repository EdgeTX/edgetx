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

#include "input_mix_button.h"
#include "opentx.h"

#include "lvgl_widgets/input_mix_line.h"

void InputMixButton::self_size(lv_event_t* e)
{
  auto obj = lv_event_get_target(e);
  auto p = (lv_point_t*)lv_event_get_param(e);

  auto btn = (InputMixButton*)lv_obj_get_user_data(obj);
  if (!btn) return;

  p->y = btn->calcHeight();
}

void InputMixButton::value_changed(lv_event_t* e)
{
  auto obj = lv_event_get_target(e);
  lv_obj_refresh_self_size(obj);
}

InputMixButton::InputMixButton(Window* parent, uint8_t index) :
    Button(parent, rect_t{}, nullptr, 0, 0/*COLOR_THEME_PRIMARY1*/,
           input_mix_line_create),
    index(index)
{
  lv_obj_add_event_cb(lvobj, InputMixButton::self_size, LV_EVENT_GET_SELF_SIZE, nullptr);
  lv_obj_add_event_cb(lvobj, InputMixButton::value_changed, LV_EVENT_VALUE_CHANGED, nullptr);
}

void InputMixButton::checkEvents()
{
  check(isActive());
  Button::checkEvents();
}

void InputMixButton::drawFlightModes(BitmapBuffer *dc, FlightModesType value,
                                     LcdFlags textColor, coord_t x, coord_t y)
{
  dc->drawMask(x, y + 2, mixerSetupFlightmodeIcon, textColor);
  x += 20;

  for (int i = 0; i < MAX_FLIGHT_MODES; i++) {
    char s[] = " ";
    s[0] = '0' + i;
    if (value & (1 << i)) {
      dc->drawText(x, y + 2, s, FONT(XS) | COLOR_THEME_DISABLED);
    } else {
      dc->drawSolidFilledRect(x, y + 2, 8, 3, COLOR_THEME_FOCUS);
      dc->drawText(x, y + 2, s, FONT(XS) | textColor);
    }
    x += 8;
  }
}

lv_coord_t InputMixButton::calcHeight() const
{
  auto h = getFontHeight(FONT(STD));
  // h += 2 * lv_obj_get_style_border_width(lvobj, LV_PART_MAIN);
  // h += lv_obj_get_style_pad_top(lvobj, LV_PART_MAIN);
  // h += lv_obj_get_style_pad_bottom(lvobj, LV_PART_MAIN);

  size_t lines = getLines();
  if (lines > 1) {
    lines -= 1;
    h += lines * lv_obj_get_style_text_line_space(lvobj, LV_PART_MAIN);
    h += lines * getFontHeight(FONT(STD));
  }

  return h;
}
