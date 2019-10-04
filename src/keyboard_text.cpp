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

#include "keyboard_text.h"
#include "libopenui_globals.h"
#include "textedit.h"
#include "font.h"

constexpr coord_t KEYBOARD_HEIGHT = 160;

TextKeyboard * TextKeyboard::_instance = nullptr;

const uint8_t LBM_KEY_UPPERCASE[] = {
#include "mask_key_uppercase.lbm"
};

const uint8_t LBM_KEY_LOWERCASE[] = {
#include "mask_key_lowercase.lbm"
};

const uint8_t LBM_KEY_BACKSPACE[] = {
#include "mask_key_backspace.lbm"
};

const uint8_t LBM_KEY_LETTERS[] = {
#include "mask_key_letters.lbm"
};

const uint8_t LBM_KEY_NUMBERS[] = {
#include "mask_key_numbers.lbm"
};

const uint8_t LBM_KEY_SPACEBAR[] = {
#include "mask_key_spacebar.lbm"
};

const uint8_t * const LBM_SPECIAL_KEYS[] = {
  LBM_KEY_BACKSPACE,
  LBM_KEY_UPPERCASE,
  LBM_KEY_LOWERCASE,
  LBM_KEY_LETTERS,
  LBM_KEY_NUMBERS,
};

const char * const KEYBOARD_LOWERCASE[] = {
  "qwertyuiop",
  " asdfghjkl",
  "\201zxcvbnm\200",
  "\204\t\n"
};

const char * const KEYBOARD_UPPERCASE[] = {
  "QWERTYUIOP",
  " ASDFGHJKL",
  "\202ZXCVBNM\200",
  "\204\t\n"
};

const char * const KEYBOARD_NUMBERS[] = {
  "1234567890",
  "_-",
  "                 \200",
  "\203\t\n"
};

const char * const * const KEYBOARD_LAYOUTS[] = {
  KEYBOARD_UPPERCASE,
  KEYBOARD_LOWERCASE,
  KEYBOARD_LOWERCASE,
  KEYBOARD_NUMBERS,
};

TextKeyboard::TextKeyboard():
  Keyboard<TextEdit>(KEYBOARD_HEIGHT),
  layout(KEYBOARD_LOWERCASE)
{
}

TextKeyboard::~TextKeyboard()
{
  _instance = nullptr;
}

void TextKeyboard::setCursorPos(coord_t x)
{
  if (!field)
    return;

  uint8_t size = field->getMaxLength();
  char * data = field->getData();
  coord_t rest = x;
  for (cursorIndex = 0; cursorIndex < size; cursorIndex++) {
    if (data[cursorIndex] == '\0')
      break;
    char c = data[cursorIndex];
    uint8_t w = getCharWidth(c, fontspecsTable[0]);
    if (rest < w)
      break;
    rest -= w;
  }
  cursorPos = x - rest;
  field->invalidate();
}

void TextKeyboard::paint(BitmapBuffer * dc)
{
  lcdSetColor(RGB(0xE0, 0xE0, 0xE0));
  dc->clear(CUSTOM_COLOR);

  for (uint8_t i=0; i<4; i++) {
    coord_t y = 15 + i * 40;
    coord_t x = 15;
    const char * c = layout[i];
    while(*c) {
      if (*c == ' ') {
        x += 15;
      }
      else if (*c == '\t') {
        // spacebar
        dc->drawBitmapPattern(x, y, LBM_KEY_SPACEBAR, DEFAULT_COLOR);
        x += 135;
      }
      else if (*c == '\n') {
        // enter
        dc->drawSolidFilledRect(x, y-2, 80, 25, TEXT_DISABLE_COLOR);
        dc->drawText(x+40, y, "ENTER", CENTERED);
        x += 80;
      }
      else if (int8_t(*c) < 0) {
        dc->drawBitmapPattern(x, y, LBM_SPECIAL_KEYS[uint8_t(*c - 128)], DEFAULT_COLOR);
        x += 45;
      }
      else {
        dc->drawSizedText(x, y, c, 1);
        x += 30;
      }
      c++;
    }
  }
}

bool TextKeyboard::onTouchEnd(coord_t x, coord_t y)
{
  if (!field)
    return false;

  uint8_t size = field->getMaxLength();
  char * data = field->getData();

  char c = 0;

  uint8_t row = max<coord_t>(0, y - 5) / 40;
  const char * key = layout[row];
  while(*key) {
    if (*key == ' ') {
      x -= 15;
    }
    else if (*key == '\t') {
      if (x <= 135) {
        c = ' ';
        break;
      }
      x -= 135;
    }
    else if (*key == '\n') {
      if (x <= 80) {
        // enter
        disable(true);
        return true;
      }
      x -= 80;
    }
    else if (int8_t(*key) < 0) {
      if (x <= 45) {
        uint8_t specialKey = *key;
        if (specialKey == 128) {
          // backspace
          if (cursorIndex > 0) {
            char c = data[cursorIndex - 1];
            memmove(data + cursorIndex - 1, data + cursorIndex, size - cursorIndex);
            data[size - 1] = '\0';
            cursorPos -= getCharWidth(c, fontspecsTable[0]);
            --cursorIndex;
          }
        }
        else {
          layout = KEYBOARD_LAYOUTS[specialKey - 129];
          invalidate();
        }
        break;
      }
      x -= 45;
    }
    else {
      if (x <= 30) {
        c = *key;
        break;
      }
      x -= 30;
    }
    key++;
  }

  if (c && size - cursorIndex > 1) {
    memmove(data + cursorIndex + 1, data + cursorIndex, size - cursorIndex - 1);
    data[cursorIndex++] = c;
    cursorPos += getCharWidth(c, fontspecsTable[0]);
  }

  field->invalidate();
  return true;
}
