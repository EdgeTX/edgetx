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

#include "textedit.h"
#include "font.h"
#include "libopenui_globals.h"

#if defined(HARDWARE_TOUCH)
#include "keyboard_text.h"
#endif

void TextEdit::paint(BitmapBuffer * dc)
{
  FormField::paint(dc);

  if (editMode) {
    dc->drawSizedText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, value, length, FOCUS_COLOR);
    coord_t left = (cursorPos == 0 ? 0 : getTextWidth(value, cursorPos));
#if defined(HARDWARE_TOUCH)
    dc->drawSolidFilledRect(left + 2, 2, 2, height() - 4, FOCUS_COLOR);
#else
    char s[] = { value[cursorPos], '\0' };
    dc->drawSolidFilledRect(FIELD_PADDING_LEFT + left - 1, FIELD_PADDING_TOP - 1, getTextWidth(s, 1) + 1, height() - 2, FOCUS_COLOR);
    dc->drawText(FIELD_PADDING_LEFT + left, FIELD_PADDING_TOP, s, DEFAULT_COLOR);
#endif
  }
  else {
    const char * displayedValue = value;
    LcdFlags textColor;
    if (hasFocus()) {
      if (strlen(value) == 0) {
        displayedValue = "---";
      }
      textColor = FOCUS_BGCOLOR;
    }
    else {
      if (strlen(value) == 0) {
        displayedValue = "---";
        textColor = DISABLE_COLOR;
      }
      else {
        textColor = DEFAULT_COLOR;
      }
    }
    dc->drawSizedText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, displayedValue, length, textColor);
  }
}

void TextEdit::trim()
{
  for (int i = length - 1; i >= 0; i--) {
    if (value[i] == ' ' || value[i] == '\0')
      value[i] = '\0';
    else
      break;
  }
}

void TextEdit::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

#if defined(HARDWARE_TOUCH)
  if (IS_VIRTUAL_KEY_EVENT(event)) {
    uint8_t c = event & 0xFF;
    if (c == (uint8_t)KEYBOARD_BACKSPACE[0]) {
      if (cursorPos > 0) {
        memmove(value + cursorPos - 1, value + cursorPos, length - cursorPos);
        value[length - 1] = '\0';
        --cursorPos;
        invalidate();
      }
    }
    else if (cursorPos < length) {
      memmove(value + cursorPos + 1, value + cursorPos, length - cursorPos - 1);
      value[cursorPos++] = c;
      invalidate();
    }
  }
#endif

#if defined(HARDWARE_KEYS)
  if (editMode) {
    int c = value[cursorPos];
    int v = c;

    switch (event) {
      case EVT_ROTARY_RIGHT:
        for (int i = 0; i < rotencSpeed; i++) {
          v = getNextChar(v);
        }
        break;

      case EVT_ROTARY_LEFT:
        for (int i = 0; i < rotencSpeed; i++) {
          v = getPreviousChar(v);
        }
        break;

      case EVT_KEY_BREAK(KEY_LEFT):
        if (cursorPos > 0) {
          cursorPos--;
          invalidate();
        }
        break;

      case EVT_KEY_BREAK(KEY_RIGHT):
        if (cursorPos < length - 1 && value[cursorPos + 1] != '\0') {
          cursorPos++;
          invalidate();
        }
        break;

      case EVT_KEY_BREAK(KEY_ENTER):
        if (cursorPos < length - 1) {
          cursorPos++;
          if (value[cursorPos] == '\0')
            value[cursorPos] = ' ';
          invalidate();
        }
        else {
          changeEnd();
          FormField::onEvent(event);
        }
        break;

      case EVT_KEY_BREAK(KEY_EXIT):
        changeEnd();
        FormField::onEvent(event);
        break;

      case EVT_KEY_LONG(KEY_ENTER):
        if (v == 0) {
          killEvents(event);
          FormField::onEvent(EVT_KEY_BREAK(KEY_ENTER));
          break;
        }
        // no break

      case EVT_KEY_LONG(KEY_LEFT):
      case EVT_KEY_LONG(KEY_RIGHT):
        if (v >= -26 && v <= 26) {
          v = -v; // toggle case
          if (event == EVT_KEY_LONG(KEY_LEFT)) {
            killEvents(KEY_LEFT);
          }
        }
        break;
    }

    if (c != v) {
      // TRACE("value[%d] = %d", cursorPos, v);
      value[cursorPos] = v;
      invalidate();
      // TODO storageDirty(...);
    }
  }
  else {
    cursorPos = 0;
    FormField::onEvent(event);
  }
#endif
}

#if defined(HARDWARE_TOUCH)
bool TextEdit::onTouchEnd(coord_t x, coord_t y)
{
  if (!hasFocus()) {
    setFocus();
  }

#if defined(SOFTWARE_KEYBOARD)
  TextKeyboard::show(this);
#endif

  coord_t rest = x;
  for (cursorPos = 0; cursorPos < length; cursorPos++) {
    char c = value[cursorPos];
    if (c == '\0')
      break;
    uint8_t w = getCharWidth(c, fontspecsTable[0]) + 1;
    if (rest < w)
      break;
    rest -= w;
  }

  invalidate();
  return true;
}
#endif

void TextEdit::onFocusLost()
{
#if defined(SOFTWARE_KEYBOARD)
  TextKeyboard::hide();
#endif

  changeEnd();

  // TODO storageDirty(...);
  FormField::onFocusLost();
}
