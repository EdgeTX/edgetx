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
  KEYBOARD_SET_UPPERCASE "zxcvbnm" KEYBOARD_BACKSPACE,
  KEYBOARD_SET_NUMBERS KEYBOARD_SPACE KEYBOARD_ENTER
};

const char * const KEYBOARD_UPPERCASE[] = {
  "QWERTYUIOP",
  " ASDFGHJKL",
  KEYBOARD_SET_LOWERCASE "ZXCVBNM" KEYBOARD_BACKSPACE,
  KEYBOARD_SET_NUMBERS KEYBOARD_SPACE KEYBOARD_ENTER
};

const char * const KEYBOARD_NUMBERS[] = {
  "1234567890",
  "_-",
  "                 " KEYBOARD_BACKSPACE,
  KEYBOARD_SET_LETTERS KEYBOARD_SPACE KEYBOARD_ENTER
};

const char * const * const KEYBOARD_LAYOUTS[] = {
  KEYBOARD_UPPERCASE,
  KEYBOARD_LOWERCASE,
  KEYBOARD_LOWERCASE,
  KEYBOARD_NUMBERS,
};

TextKeyboard::TextKeyboard():
  Keyboard(KEYBOARD_HEIGHT),
  layout(KEYBOARD_LOWERCASE)
{
}

TextKeyboard::~TextKeyboard()
{
  _instance = nullptr;
}

void TextKeyboard::paint(BitmapBuffer * dc)
{
  lcdSetColor(RGB(0xE0, 0xE0, 0xE0));
  dc->clear(CUSTOM_COLOR);

  for (uint8_t i=0; i<4; i++) {
    coord_t y = 15 + i * 40;
    coord_t x = 15;
    const char * c = layout[i];
    while (*c) {
      if (*c == ' ') {
        x += 15;
      }
      else if (*c == KEYBOARD_SPACE[0]) {
        // spacebar
        dc->drawBitmapPattern(x, y, LBM_KEY_SPACEBAR, COLOR_THEME_SECONDARY1);
        x += 135;
      }
      else if (*c == KEYBOARD_ENTER[0]) {
        // enter
        dc->drawSolidFilledRect(x, y-2, 80, 25, COLOR_THEME_DISABLED);
        dc->drawText(x+40, y, "ENTER", CENTERED);
        x += 80;
      }
      else if (int8_t(*c) < 0) {
        dc->drawBitmapPattern(x, y, LBM_SPECIAL_KEYS[uint8_t(*c - 128)], COLOR_THEME_SECONDARY1);
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
  onKeyPress();

  uint8_t row = max<coord_t>(0, y - 5) / 40;
  const char * key = layout[row];
  while (*key) {
    if (*key == ' ') {
      x -= 15;
    }
    else if (*key == KEYBOARD_SPACE[0]) {
      if (x <= 135) {
        pushEvent(EVT_VIRTUAL_KEY(' '));
        return true;
      }
      x -= 135;
    }
    else if (*key == KEYBOARD_ENTER[0]) {
      if (x <= 80) {
        // enter
        hide();
        return true;
      }
      x -= 80;
    }
    else if (int8_t(*key) < 0) {
      if (x <= 45) {
        uint8_t specialKey = *key;
        if (specialKey == 128) {
          // backspace
          pushEvent(EVT_VIRTUAL_KEY(KEYBOARD_BACKSPACE[0]));
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
        pushEvent(EVT_VIRTUAL_KEY(*key));
        return true;
      }
      x -= 30;
    }
    key++;
  }

  return true;
}
