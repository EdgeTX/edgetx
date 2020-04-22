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

#include "libopenui_config.h"
#include "button.h"
#include "font.h"
#include "theme.h"

void Button::onPress()
{
  bool check = (pressHandler && pressHandler());
  if (check != bool(windowFlags & BUTTON_CHECKED)) {
    windowFlags ^= BUTTON_CHECKED;
    invalidate();
  }
}

#if defined(HARDWARE_KEYS)
void Button::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString("Button").c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    onPress();
  }
  else {
    FormField::onEvent(event);
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool Button::onTouchEnd(coord_t x, coord_t y)
{
  if (enabled) {
    if (!(windowFlags & NO_FOCUS)) {
      setFocus(SET_FOCUS_DEFAULT);
    }
    onPress();
  }
  return true;
}
#endif

void Button::checkEvents()
{
  Window::checkEvents();
  if (checkHandler)
    checkHandler();
}

void TextButton::paint(BitmapBuffer * dc)
{
  FormField::paint(dc);

  auto textColor = DEFAULT_COLOR;

  if (checked()) {
    if (hasFocus()) {
      dc->drawSolidRect(0, 0, rect.w, rect.h, 2, FOCUS_BGCOLOR);
      dc->drawSolidFilledRect(3, 3, rect.w - 6, rect.h - 6, FOCUS_BGCOLOR);
    }
    else {
      dc->drawSolidRect(0, 0, rect.w, rect.h, 1, DISABLE_COLOR);
      dc->drawSolidFilledRect(2, 2, rect.w - 4, rect.h - 4, FOCUS_BGCOLOR);
    }
    textColor = FOCUS_COLOR;
  }
  else {
    if (windowFlags & BUTTON_BACKGROUND) {
      if (hasFocus()) {
        dc->drawSolidRect(0, 0, rect.w, rect.h, 2, FOCUS_BGCOLOR);
        dc->drawSolidFilledRect(2, 2, rect.w - 4, rect.h - 4, DISABLE_COLOR);
      }
      else {
        dc->drawSolidFilledRect(0, 0, rect.w, rect.h, DISABLE_COLOR);
      }
    }
    else {
      if (hasFocus()) {
        dc->drawSolidRect(0, 0, rect.w, rect.h, 2, FOCUS_BGCOLOR);
      }
      else {
        dc->drawSolidRect(0, 0, rect.w, rect.h, 1, DISABLE_COLOR);
      }
    }
  }

  dc->drawText(rect.w / 2, (rect.h - getFontHeight(textFlags)) / 2, text.c_str(), CENTERED | textFlags | textColor);
}

void IconButton::paint(BitmapBuffer * dc)
{
  dc->drawBitmap(0, 0, theme->getIcon(icon, checked() ? STATE_PRESSED : STATE_DEFAULT));
}
