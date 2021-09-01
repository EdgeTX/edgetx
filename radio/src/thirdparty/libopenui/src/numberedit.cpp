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

#if defined(SOFTWARE_KEYBOARD)
#include "keyboard_number.h"
#endif

NumberEdit::NumberEdit(Window * parent, const rect_t & rect, int vmin, int vmax, std::function<int()> getValue, std::function<void(int)> setValue, WindowFlags windowFlags, LcdFlags textFlags):
  BaseNumberEdit(parent, rect, vmin, vmax, std::move(getValue), std::move(setValue), windowFlags, textFlags)
{
}

void NumberEdit::paint(BitmapBuffer * dc)
{
  FormField::paint(dc);

  auto value = getValue();

  LcdFlags textColor;
  if (editMode)
    textColor = COLOR_THEME_PRIMARY2;
  else if (hasFocus())
    textColor = COLOR_THEME_PRIMARY2;
  // else if (enabled && (value != 0 || zeroText.empty()))
  //   textColor = COLOR_THEME_SECONDARY1;
  else
    textColor = COLOR_THEME_SECONDARY1;

  if (displayFunction) {
    displayFunction(dc, textColor, value);
  } else if (value == 0 && !zeroText.empty()) {
    dc->drawText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, zeroText.c_str(),
                 textColor | textFlags);
    if (textFlags & RIGHT)
      dc->drawText(rect.w - FIELD_PADDING_LEFT, FIELD_PADDING_TOP,
                   zeroText.c_str(), textColor | textFlags);
    else
      dc->drawText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, zeroText.c_str(),
                   textColor | textFlags);
  } else {
    if (textFlags & RIGHT)
      dc->drawNumber(rect.w - FIELD_PADDING_LEFT, FIELD_PADDING_TOP, value,
                     textColor | textFlags, 0, prefix.c_str(), suffix.c_str());
    else
      dc->drawNumber(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, value,
                     textColor | textFlags, 0, prefix.c_str(), suffix.c_str());
  }
}

void NumberEdit::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (editMode) {
    switch (event) {
#if defined(HARDWARE_KEYS)
      case EVT_ROTARY_RIGHT: {
        int value = getValue();
        do {
          value += ROTARY_ENCODER_SPEED() * step;
        } while (isValueAvailable && !isValueAvailable(value) && value <= vmax);
        if (value <= vmax) {
          setValue(value);
          onKeyPress();
        }
        else {
          onKeyError();
        }
        return;
      }

      case EVT_ROTARY_LEFT: {
        int value = getValue();
        do {
          value -= ROTARY_ENCODER_SPEED() * step;
        } while (isValueAvailable && !isValueAvailable(value) && value >= vmin);
        if (value >= vmin) {
          setValue(value);
          onKeyPress();
        }
        else {
          onKeyError();
        }
        return;
      }
#endif

    case EVT_KEY_FIRST(KEY_EXIT):
#if defined(SOFTWARE_KEYBOARD)
      Keyboard::hide();
      return;
#endif
      break;


#if defined(HARDWARE_TOUCH)
      case EVT_VIRTUAL_KEY_PLUS:
        setValue(getValue() + getStep());
        break;

      case EVT_VIRTUAL_KEY_MINUS:
        setValue(getValue() - getStep());
        break;

      case EVT_VIRTUAL_KEY_FORWARD:
        setValue(getValue() + 10 * getStep());
        break;

      case EVT_VIRTUAL_KEY_BACKWARD:
        setValue(getValue() - 10 * getStep());
        break;

      case EVT_VIRTUAL_KEY_DEFAULT:
        setValue(getDefault());
        break;

      case EVT_VIRTUAL_KEY_MAX:
        setValue(getMax());
        break;

      case EVT_VIRTUAL_KEY_MIN:
        setValue(getMin());
        break;

      case EVT_VIRTUAL_KEY_SIGN:
        setValue(-getValue());
        break;
#endif
    }
  }

  FormField::onEvent(event);
}

void NumberEdit::onFocusLost()
{
#if defined(SOFTWARE_KEYBOARD)
  Keyboard::hide();
#endif

  FormField::onFocusLost();
}


#if defined(HARDWARE_TOUCH)
bool NumberEdit::onTouchEnd(coord_t, coord_t)
{
  if (!enabled) {
    return true;
  }

  if (hasFocus()) {
    setEditMode(true);
  }
  else {
    setFocus(SET_FOCUS_DEFAULT);
  }

#if defined(SOFTWARE_KEYBOARD)
  NumberKeyboard::show(this);
#endif

  return true;
}
#endif

