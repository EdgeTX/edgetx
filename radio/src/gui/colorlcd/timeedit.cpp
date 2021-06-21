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

#include "libopenui_config.h"
#include "timeedit.h"
#include "keyboard_number.h"
#include "strhelpers.h"

TimeEdit::TimeEdit(Window * parent, const rect_t & rect, int32_t vmin, int32_t vmax, std::function<int32_t()> getValue, std::function<void(int32_t)> setValue):
  BaseNumberEdit(parent, rect, vmin, vmax, std::move(getValue), std::move(setValue))
{
}

void TimeEdit::paint(BitmapBuffer * dc)
{
  FormField::paint(dc);

  LcdFlags textColor;
  if (editMode)
    textColor = FOCUS_COLOR;
  else if (hasFocus())
    textColor = FOCUS_COLOR;
  else
    textColor = DEFAULT_COLOR;

  dc->drawText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP,
               getTimerString(getValue(), (textFlags & TIMEHOUR) != 0),
               textColor);
}

#if defined(HARDWARE_KEYS)
// TODO could be moved to BaseNumberEdit
void TimeEdit::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (editMode) {
    if (event == EVT_ROTARY_RIGHT) {
      int value = getValue();
      value += rotencSpeed * step;
      if (value <= vmax)
        setValue(value);
      else
        onKeyError();
      return;
    }
    else if (event == EVT_ROTARY_LEFT) {
      int value = getValue();
      value -= rotencSpeed * step;
      if (value >= vmin)
        setValue(value);
      else
        onKeyError();
      return;
    }
  }

  FormField::onEvent(event);
}
#endif

#if defined(HARDWARE_TOUCH)
bool TimeEdit::onTouchEnd(coord_t x, coord_t y)
{
  if (!hasFocus()) {
    setFocus(SET_FOCUS_DEFAULT);
  }

//  NumberKeyboard * keyboard = NumberKeyboard::instance();
//  if (keyboard->getField() != this) {
//    keyboard->setField(this);
//  }

  return true;
}

void TimeEdit::onFocusLost()
{
  BaseNumberEdit::onFocusLost();
//  NumberKeyboard::instance()->disable(true);
}
#endif
