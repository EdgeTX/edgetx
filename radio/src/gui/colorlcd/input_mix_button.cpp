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

InputMixButton::InputMixButton(Window* parent, const rect_t& rect, uint8_t index) :
    Button(parent, rect, nullptr, 0, COLOR_THEME_PRIMARY1,
           input_mix_line_create),
    index(index)
{
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


