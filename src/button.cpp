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
  if (check != bool(flags & BUTTON_CHECKED)) {
    flags ^= BUTTON_CHECKED;
    invalidate();
  }
}

#if defined(HARDWARE_KEYS)
void Button::onKeyEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    onPress();
  }
  else {
    FormField::onKeyEvent(event);
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool Button::onTouchEnd(coord_t x, coord_t y)
{
  if (enabled()) {
    onPress();
    if (!(flags & BUTTON_NOFOCUS)) {
      setFocus();
    }
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

  LcdFlags textColor = TEXT_COLOR;

  if (checked()) {
    if (hasFocus()) {
      dc->drawSolidRect(0, 0, rect.w, rect.h, 2, TEXT_INVERTED_BGCOLOR);
      dc->drawSolidFilledRect(3, 3, rect.w - 6, rect.h - 6, TEXT_INVERTED_BGCOLOR);
    }
    else {
      dc->drawSolidRect(0, 0, rect.w, rect.h, 1, CURVE_AXIS_COLOR);
      dc->drawSolidFilledRect(2, 2, rect.w - 4, rect.h - 4, TEXT_INVERTED_BGCOLOR);
    }
    textColor = TEXT_INVERTED_COLOR;
  }
  else {
    if (flags & BUTTON_BACKGROUND) {
      if (hasFocus()) {
        dc->drawSolidRect(0, 0, rect.w, rect.h, 2, TEXT_INVERTED_BGCOLOR);
        dc->drawSolidFilledRect(2, 2, rect.w - 4, rect.h - 4, CURVE_AXIS_COLOR);
      }
      else {
        dc->drawSolidFilledRect(0, 0, rect.w, rect.h, CURVE_AXIS_COLOR);
      }
    }
    else {
      if (hasFocus()) {
        dc->drawSolidRect(0, 0, rect.w, rect.h, 2, TEXT_INVERTED_BGCOLOR);
      }
      else {
        dc->drawSolidRect(0, 0, rect.w, rect.h, 1, CURVE_AXIS_COLOR);
      }
    }
  }

  dc->drawText(rect.w / 2, (rect.h - getFontHeight(flags)) / 2, text.c_str(), CENTERED | textColor);
}

void IconButton::paint(BitmapBuffer * dc)
{
  dc->drawBitmap(0, 0, theme->getIcon(icon, checked() ? STATE_PRESSED : STATE_DEFAULT));
}


const uint8_t __alpha_button_on[] {
#include "alpha_button_on.lbm"
};
Bitmap ALPHA_BUTTON_ON(BMP_ARGB4444, (const uint16_t*)__alpha_button_on);

const uint8_t __alpha_button_off[] {
#include "alpha_button_off.lbm"
};
Bitmap ALPHA_BUTTON_OFF(BMP_ARGB4444, (const uint16_t*)__alpha_button_off);


FabIconButton::FabIconButton(Window * parent, coord_t x, coord_t y, uint8_t icon, std::function<uint8_t(void)> pressHandler, uint8_t flags):
  Button(parent, { x - 34, y - 34, 68, 68 }, pressHandler, flags),
  icon(icon)
{
}

void FabIconButton::paint(BitmapBuffer * dc)
{
  dc->drawBitmap(0, 0, checked() ? &ALPHA_BUTTON_ON : &ALPHA_BUTTON_OFF);
  const BitmapBuffer * mask = theme->getIconMask(icon);
  if (mask) {
    dc->drawMask((68 - mask->getWidth()) / 2, (68 - mask->getHeight()) / 2, mask, TEXT_BGCOLOR);
  }
}
