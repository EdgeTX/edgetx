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

#include "slider.h"
#include "theme.h"

void Slider::paint(BitmapBuffer * dc)
{
  theme->drawSlider(dc, vmin, vmax, getValue(), rect, editMode, hasFocus());
}

int Slider::value(coord_t x) const
{
  return vmin + ((vmax - vmin) * x + (rect.w / 2)) / rect.w;
}

#if defined(HARDWARE_KEYS)
void Slider::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (editMode) {
    if (event == EVT_ROTARY_RIGHT) {
      setValue(getValue() + ROTARY_ENCODER_SPEED());
      onKeyPress();
      return;
    }
    else if (event == EVT_ROTARY_LEFT) {
      setValue(getValue() - ROTARY_ENCODER_SPEED());
      onKeyPress();
      return;
    } else if (event == EVT_KEY_FIRST(KEY_EXIT))
      return;
  }

  FormField::onEvent(event);
}
#endif

#if defined(HARDWARE_TOUCH)
bool Slider::onTouchStart(coord_t x, coord_t y)
{
  if(!enabled)
    return true;

  if (!hasFocus()) {
    onKeyPress();
    setFocus(SET_FOCUS_DEFAULT);
  }
  sliding = (value(x) == getValue());
  return true;
}

bool Slider::onTouchEnd(coord_t x, coord_t y)
{
  if(!enabled)
    return true;

  setValue(value(x));
  invalidate();
  onKeyPress();
  return true;
}

bool Slider::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  if(!enabled)
    return true;

  if (hasFocus() && sliding) {
    int newValue = value(x);
    if (getValue() != newValue) {
      setValue(newValue);
      invalidate();
      onKeyPress();
    }
  }
  return true;
}
#endif
