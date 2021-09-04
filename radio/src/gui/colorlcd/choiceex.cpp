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

#include "choiceex.h"

void ChoiceEx::setLongPressHandler(std::function<void(event_t)> handler)
{
  longPressHandler = handler;
}

ChoiceEx::ChoiceEx(FormGroup * parent, const rect_t & rect, int vmin, int vmax, std::function<int()> getValue, std::function<void(int)> setValue, WindowFlags windowFlags) :
  Choice(parent, rect, vmin, vmax, getValue, setValue, windowFlags)
{
}

bool ChoiceEx::onTouchEnd(coord_t x, coord_t y)
{
  return Choice::onTouchEnd(x,y);
}

void ChoiceEx::onEvent(event_t event)
{
  if (event == EVT_KEY_LONG(KEY_ENTER)) {
    if (longPressHandler) {
      killEvents(event);
      longPressHandler(event);
      return;
    }
  }

  Choice::onEvent(event);
}

