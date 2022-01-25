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

constexpr coord_t KEYBOARD_HEIGHT = LCD_H * 2 / 5;

TextKeyboard * TextKeyboard::_instance = nullptr;

// const uint8_t LBM_KEY_UPPERCASE[] = {
// #include "mask_key_uppercase.lbm"
// };

// const uint8_t LBM_KEY_LOWERCASE[] = {
// #include "mask_key_lowercase.lbm"
// };

// const uint8_t LBM_KEY_BACKSPACE[] = {
// #include "mask_key_backspace.lbm"
// };

// const uint8_t LBM_KEY_LETTERS[] = {
// #include "mask_key_letters.lbm"
// };

// const uint8_t LBM_KEY_NUMBERS[] = {
// #include "mask_key_numbers.lbm"
// };

// const uint8_t LBM_KEY_SPACEBAR[] = {
// #include "mask_key_spacebar.lbm"
// };

// const uint8_t * const LBM_SPECIAL_KEYS[] = {
//   LBM_KEY_BACKSPACE,
//   LBM_KEY_UPPERCASE,
//   LBM_KEY_LOWERCASE,
//   LBM_KEY_LETTERS,
//   LBM_KEY_NUMBERS,
// };

// const char * const KEYBOARD_LOWERCASE[] = {
//   "qwertyuiop",
//   " asdfghjkl",
//   KEYBOARD_SET_UPPERCASE "zxcvbnm" KEYBOARD_BACKSPACE,
//   KEYBOARD_SET_NUMBERS KEYBOARD_SPACE KEYBOARD_ENTER
// };

// const char * const KEYBOARD_UPPERCASE[] = {
//   "QWERTYUIOP",
//   " ASDFGHJKL",
//   KEYBOARD_SET_LOWERCASE "ZXCVBNM" KEYBOARD_BACKSPACE,
//   KEYBOARD_SET_NUMBERS KEYBOARD_SPACE KEYBOARD_ENTER
// };

// const char * const KEYBOARD_NUMBERS[] = {
//   "1234567890",
//   "_-",
//   "                 " KEYBOARD_BACKSPACE,
//   KEYBOARD_SET_LETTERS KEYBOARD_SPACE KEYBOARD_ENTER
// };

// #define NUMBER_OF_KEYBOARD_LAYOUTS 4
// const char * const * const KEYBOARD_LAYOUTS[NUMBER_OF_KEYBOARD_LAYOUTS] = {
//   KEYBOARD_UPPERCASE,
//   KEYBOARD_LOWERCASE,
//   KEYBOARD_LOWERCASE,
//   KEYBOARD_NUMBERS,
// };

TextKeyboard::TextKeyboard():
  Keyboard(KEYBOARD_HEIGHT)
  // layout(KEYBOARD_LOWERCASE)
{
  if (keyboard == nullptr) {
    keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(keyboard, LCD_W, LCD_H * 2 / 5);
    lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
  }
}

TextKeyboard::~TextKeyboard()
{
  _instance = nullptr;

}

int TextKeyboard::getCharWidth(const char c)
{
  // if (int8_t(c) < 0) return KEYBOARD_BITMAP_WIDTH;

  // switch (c)
  // {
  //   case ' ': return 15;
  //   case KEYBOARD_SPACE[0]: return KEYBOARD_SPACE_WIDTH;
  //   case KEYBOARD_ENTER[0]: return KEYBOARD_ENTER_WIDTH;
  //   default: return KEYBOARD_CHAR_WIDTH;
  // }
}


int TextKeyboard::calculateMaxWidth()
{
  // int maxWidth = 0;
  // for (uint8_t i = 0; i < NUMBER_OF_KEYBOARD_LAYOUTS; i++) {
  //   int lineWidth = 0;
  //   const char * c = layout[i];
  //   while (*c) {
  //     lineWidth += getCharWidth(*c);
  //     maxWidth = max(lineWidth, maxWidth);
  //     c++;
  //   }
  // }

  // return maxWidth;
}

void TextKeyboard::paint(BitmapBuffer * dc)
{
  // lcdSetColor(RGB(0xE0, 0xE0, 0xE0));
  // dc->clear(CUSTOM_COLOR);

  // // center the keyboard
  // coord_t start_x = (dc->width() - calculateMaxWidth()) / 2;

  // for (uint8_t i = 0; i < NUMBER_OF_KEYBOARD_LAYOUTS; i++) {
  //   coord_t y = KEYBOARD_OFFSET_Y + i * KEYBOARD_ROW_HEIGHT;
  //   coord_t x = start_x;
  //   const char * c = layout[i];

  //   while (*c) {
  //     if (*c == KEYBOARD_SPACE[0]) {
  //       // spacebar
  //       dc->drawBitmapPattern(x, y, LBM_KEY_SPACEBAR, COLOR_THEME_SECONDARY1);
  //     }
  //     else if (*c == KEYBOARD_ENTER[0]) {
  //       // enter
  //       dc->drawSolidFilledRect(x, y - 2, KEYBOARD_ENTER_WIDTH, 25, COLOR_THEME_DISABLED);
  //       dc->drawText(x + KEYBOARD_ENTER_WIDTH / 2, y + 2, "ENTER", CENTERED);
  //     }
  //     else if (int8_t(*c) < 0) {
  //       const uint8_t *pBitmap =  LBM_SPECIAL_KEYS[uint8_t(*c - 128)];
  //       uint8_t bitmapWidth = *pBitmap, bitmapHeight = *(pBitmap + 2);
  //       if (touched && *c == touch_key) {
  //         dc->drawSolidFilledRect(x - 2, y + 3, bitmapWidth + 4, bitmapHeight + 4, COLOR_THEME_SECONDARY2);
  //       }

  //       dc->drawBitmapPattern(x, y + 5, pBitmap, COLOR_THEME_SECONDARY1);
  //     }
  //     else {
  //       if (touched && *c == touch_key) {
  //         dc->drawSolidFilledRect(x - KEYBOARD_CHAR_WIDTH / 2, y - ((KEYBOARD_ROW_HEIGHT - 16) / 2), KEYBOARD_CHAR_WIDTH, KEYBOARD_ROW_HEIGHT, COLOR_THEME_FOCUS);
  //       }
  //       dc->drawSizedText(x, y, c, 1, CENTERED);
  //     }
  //     x += getCharWidth(*c);
  //     c++;
  //   }
  // }
}

bool TextKeyboard::onTouchStart(coord_t x, coord_t y)
{
  // // remove the centering.  If x < 0 then this is an invalid touch event
  // x -= (this->width() - calculateMaxWidth()) / 2;
  // if (x < 0) {
  //   if (touched) {
  //     touched = false;
  //     invalidate();
  //   }
  //   return true;
  // }

  // touched = false;
  
  // uint8_t row = max<coord_t>(0, y - 5) / KEYBOARD_ROW_HEIGHT;
  // const char * key = layout[row];

  // while(*key) {
  //   if (*key == KEYBOARD_SPACE[0] && x <= KEYBOARD_SPACE_WIDTH) {
  //     touch_key = KEYBOARD_SPACE[0];
  //     touched = true;
  //     break;
  //   } 
  //   else if (*key == KEYBOARD_ENTER[0] && x <= KEYBOARD_ENTER_WIDTH) {
  //     touch_key = KEYBOARD_ENTER[0];
  //     touched = true;
  //     break;
  //   }
  //   else if (int8_t(*key) < 0 && x <= KEYBOARD_BITMAP_WIDTH) {
  //     if (uint8_t(*key) == 128) {
  //       touch_key = KEYBOARD_BACKSPACE[0];
  //       touched = true;
  //       break;
  //     } else {
  //       touched = false;
  //       break;
  //     }
  //   }
  //   else if (*key != ' ' && (x >= -(KEYBOARD_CHAR_WIDTH / 2) && x <= KEYBOARD_CHAR_WIDTH / 2)) {
  //     touch_key = *key;
  //     touched = true;
  //     break;
  //   }

  //   x -= getCharWidth(*key);
  //   key++;

  //   if (touched) {
  //     invalidate();
  //     break;
  //   }
  // }
  // return true;
}

bool TextKeyboard::onTouchEnd(coord_t x, coord_t y)
{
  // // remove the centering width, if x < 0 then this is an invalid touch on a centered keyboard.
  // x -= (this->width() - calculateMaxWidth()) / 2;  
  // if (x < 0) {
  //   touched = false;
  //   return true;
  // }

  // onKeyPress();

  // if (touched)
  //   invalidate();
  // touched = false;

  // uint8_t row = max<coord_t>(0, y - 5) / KEYBOARD_ROW_HEIGHT;

  // const char * key = layout[row];
  // while (*key) {
  //   if (*key == KEYBOARD_SPACE[0]) {
  //     if (x <= KEYBOARD_SPACE_WIDTH) {
  //       pushEvent(EVT_VIRTUAL_KEY(' '));
  //       return true;
  //     }
  //   }
  //   else if (*key == KEYBOARD_ENTER[0]) {
  //     if (x <= KEYBOARD_ENTER_WIDTH) {
  //       pushEvent(EVT_VIRTUAL_KEY(KEYBOARD_ENTER[0]));
  //       // enter
  //       hide();
  //       return true;
  //     }
  //   }
  //   else if (int8_t(*key) < 0) {
  //     if (x <= KEYBOARD_BITMAP_WIDTH) {
  //       uint8_t specialKey = *key;
  //       if (specialKey == 128) {
  //         // backspace
  //         pushEvent(EVT_VIRTUAL_KEY(KEYBOARD_BACKSPACE[0]));
  //       }
  //       else {
  //         layout = KEYBOARD_LAYOUTS[specialKey - 129];
  //         invalidate();
  //       }
  //       break;
  //     }
  //   }
  //   else {
  //     if (*key != ' ' && (x >= -(KEYBOARD_CHAR_WIDTH / 2) && x <= KEYBOARD_CHAR_WIDTH / 2)) {
  //       pushEvent(EVT_VIRTUAL_KEY(*key));
  //       return true;
  //     }
  //   }
  //   x -= getCharWidth(*key);
  //   key++;
  // }

  // return true;
}

bool TextKeyboard::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  // if (touched)
  //   invalidate();

  // touched = false;
  // return false;
}

#if defined(HARDWARE_KEYS)
void TextKeyboard::onEvent(event_t event)
{
}
#endif
