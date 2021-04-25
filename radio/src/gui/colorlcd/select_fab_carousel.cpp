/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "select_fab_carousel.h"

SelectFabCarousel::SelectFabCarousel(Window* parent):
    FormGroup(parent, {}, OPAQUE | FORM_FORWARD_FOCUS | NO_SCROLLBAR)
{
    setPageWidth(FAB_BUTTON_SIZE + SELECT_BUTTON_BORDER);
    setHeight(FAB_BUTTON_SIZE + 2 * PAGE_LINE_HEIGHT + SELECT_BUTTON_BORDER / 2);
}

void SelectFabCarousel::setMaxButtons(uint8_t max)
{
  maxButtons = max;
  setWidth(maxButtons * pageWidth);
}

void SelectFabCarousel::addButton(uint8_t icon, const char* title,
                                  std::function<uint8_t(void)> pressHandler)
{
  coord_t y_pos = 0;//SELECT_BUTTON_BORDER;
  coord_t x_pos = pageWidth * buttons;// + SELECT_BUTTON_BORDER / 2;
  buttons++;

  auto button = new SelectFabButton(this, x_pos, y_pos, icon, title, pressHandler);
  setInnerWidth(pageWidth * buttons);

  // y_pos += button->width() + SELECT_BUTTON_BORDER;
  // setHeight(y_pos);
  // setInnerHeight(y_pos);
}
