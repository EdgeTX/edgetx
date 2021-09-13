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
#include "choiceex.h"

constexpr int LONG_PRESS_10MS = 40;

extern inline tmr10ms_t getTicks()
{
  return g_tmr10ms;
}



void ChoiceEx::setLongPressHandler(std::function<void(event_t)> handler)
{
  longPressHandler = handler;
}

ChoiceEx::ChoiceEx(FormGroup * parent, const rect_t & rect, int16_t vmin, int16_t vmax, std::function<int16_t()> getValue, 
                   std::function<void(int16_t)> setValue, WindowFlags windowFlags) :
  Choice(parent, rect, vmin, vmax, getValue, setValue, windowFlags)
{
#if defined(HARDWARE_TOUCH)
  duration10ms = 0;
#endif
}

#if defined(HARDWARE_KEYS)
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
#endif

#if defined(HARDWARE_TOUCH)
bool ChoiceEx::isLongPress()
{
  unsigned int curTimer = getTicks();
  return (!longPressed && duration10ms != 0 && curTimer - duration10ms > LONG_PRESS_10MS);
}

void ChoiceEx::checkEvents(void)
{
  event_t event = getEvent();

  if (isLongPress()) {
    if (!longPressed && longPressHandler) {
      longPressHandler(event);
      killEvents(event);
      duration10ms = 0;
      longPressed = true;
      return;
    }
  }

  if (hasFocus())
    onEvent(event);
  else
    pushEvent(event);
}

bool ChoiceEx::onTouchStart(coord_t x, coord_t y)
{
  if (!longPressed && duration10ms == 0) {
    duration10ms = getTicks();
  }

  return Choice::onTouchStart(x, y);
}


bool ChoiceEx::onTouchEnd(coord_t x, coord_t y)
{
  if (longPressed) {
    longPressed = false;
    return false;
  }

  duration10ms = 0;
  return Choice::onTouchEnd(x,y);
}
#endif



