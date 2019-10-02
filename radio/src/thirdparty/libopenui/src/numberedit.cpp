/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include "numberedit.h"

#if defined(HARDWARE_TOUCH)
#include "keyboard_number.h"
#endif

NumberEdit::NumberEdit(Window * parent, const rect_t & rect, int32_t vmin, int32_t vmax, std::function<int32_t()> getValue, std::function<void(int32_t)> setValue, LcdFlags flags):
  BaseNumberEdit(parent, rect, vmin, vmax, std::move(getValue), std::move(setValue), flags)
{
}

void NumberEdit::paint(BitmapBuffer * dc)
{
  FormField::paint(dc);

  LcdFlags textColor;
  if (editMode)
    textColor = TEXT_INVERTED_COLOR;
  else if (hasFocus())
    textColor = TEXT_INVERTED_BGCOLOR;
  else if (enabled)
    textColor = TEXT_COLOR;
  else
    textColor = CURVE_AXIS_COLOR;

  int32_t value = getValue();
  if (displayFunction)
    displayFunction(dc, textColor, value);
  else if (value == 0 && !zeroText.empty())
    dc->drawText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, zeroText.c_str(), textColor | flags);
  else
    dc->drawNumber(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, value, textColor | flags, 0, prefix.c_str(), suffix.c_str());
}

#if defined(HARDWARE_KEYS)
void NumberEdit::onKeyEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (editMode) {
    if (event == EVT_ROTARY_RIGHT) {
      int value = getValue();
      do {
        value += rotencSpeed * step;
      } while (isValueAvailable && !isValueAvailable(value) && value <= vmax);
      if (value <= vmax)
        setValue(value);
      else
        onKeyError();
      return;
    }
    else if (event == EVT_ROTARY_LEFT) {
      int value = getValue();
      do {
        value -= rotencSpeed * step;
      } while (isValueAvailable && !isValueAvailable(value) && value >= vmin);
      if (value >= vmin)
        setValue(value);
      else
        onKeyError();
      return;
    }
  }

  FormField::onKeyEvent(event);
}
#endif

#if defined(HARDWARE_TOUCH)
bool NumberEdit::onTouchEnd(coord_t, coord_t)
{
  if (!enabled) {
    return true;
  }

  if (!hasFocus()) {
    setFocus();
  }

  NumberKeyboard * keyboard = NumberKeyboard::instance();
  if (keyboard->getField() != this) {
    keyboard->setField(this);
  }

  return true;
}
#endif

void NumberEdit::onFocusLost()
{
#if defined(HARDWARE_TOUCH)
  NumberKeyboard::instance()->disable(true);
#endif

  FormField::onFocusLost();
}
