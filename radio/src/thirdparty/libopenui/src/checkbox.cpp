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

#include "checkbox.h"
#include "theme.h"

void CheckBox::paint(BitmapBuffer * dc)
{
  theme->drawCheckBox(dc, this);
}

#if defined(HARDWARE_KEYS)
void CheckBox::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString("CheckBox").c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    onKeyPress();
    _setValue(!getValue());
    invalidate();
  }
  else {
    FormField::onEvent(event);
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool CheckBox::onTouchEnd(coord_t x, coord_t y)
{
  onKeyPress();
  _setValue(!getValue());
  setFocus(SET_FOCUS_DEFAULT);
  invalidate();
  return true;
}
#endif
