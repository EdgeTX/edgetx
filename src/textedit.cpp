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

#if defined(HARDWARE_TOUCH)
#include "keyboard_text.h"
#endif

void TextEdit::paint(BitmapBuffer * dc)
{
  FormField::paint(dc);

  if (editMode) {
    dc->drawSizedText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, value, length, TEXT_INVERTED_COLOR);
    coord_t left = (cursorPos == 0 ? 0 : getTextWidth(value, cursorPos));
    char s[] = { value[cursorPos], '\0' };
    dc->drawSolidFilledRect(FIELD_PADDING_LEFT + left - 1, FIELD_PADDING_TOP + 1, getTextWidth(s, 1) + 1, height() - 2, TEXT_INVERTED_COLOR);
    dc->drawText(FIELD_PADDING_LEFT + left, FIELD_PADDING_TOP, s, TEXT_COLOR);
  }
  else {
    const char * displayedValue = value;
    LcdFlags textColor;
    if (strlen(value) == 0) {
      displayedValue = "---";
      textColor = hasFocus() ? TEXT_INVERTED_BGCOLOR : CURVE_AXIS_COLOR;
    }
    else if (hasFocus()) {
      textColor = TEXT_INVERTED_BGCOLOR;
    }
    else {
      textColor = TEXT_COLOR;
    }
    dc->drawSizedText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, displayedValue, length, textColor);
  }

#if defined(HARDWARE_TOUCH)
  auto keyboard = TextKeyboard::instance();
  if (hasFocus() && keyboard->getField() == this) {
    coord_t cursorPos = keyboard->getCursorPos();
    dc->drawSolidFilledRect(cursorPos + 2, FIELD_PADDING_TOP, 2, 21, 0); // TEXT_INVERTED_BGCOLOR);
  }
#endif
}

#if defined(HARDWARE_KEYS)
void TextEdit::onKeyEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

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
        if (cursorPos < length - 1) {
          cursorPos++;
          invalidate();
        }
        break;

      case EVT_KEY_BREAK(KEY_ENTER):
        if (cursorPos < length - 1) {
          cursorPos++;
          invalidate();
        }
        else {
          FormField::onKeyEvent(event);
        }
        break;

      case EVT_KEY_BREAK(KEY_EXIT):
        cursorPos = 0;
        FormField::onKeyEvent(event);
        break;

      case EVT_KEY_LONG(KEY_ENTER):
        if (v == 0) {
          killEvents(event);
          FormField::onKeyEvent(EVT_KEY_BREAK(KEY_ENTER));
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
    FormField::onKeyEvent(event);
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool TextEdit::onTouchEnd(coord_t x, coord_t y)
{
  if (!hasFocus()) {
    setFocus();
  }

  auto keyboard = TextKeyboard::instance();
  if (keyboard->getField() != this) {
    keyboard->setField(this);
  }

  keyboard->setCursorPos(x);

  return true;
}

void TextEdit::onFocusLost()
{
  TextKeyboard::instance()->disable(true);
  // TODO storageDirty(...);
}
#endif
