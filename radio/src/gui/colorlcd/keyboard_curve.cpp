/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "keyboard_curve.h"
#include "button.h"

#include "translations/untranslated.h"

constexpr coord_t KEYBOARD_HEIGHT = 110;

CurveKeyboard * CurveKeyboard::_instance = nullptr;

CurveKeyboard::CurveKeyboard() :
  Keyboard(KEYBOARD_HEIGHT)
{
  // up
  new TextButton(this, {LCD_W / 2 - 20, 5, 40, 40}, STR_CHAR_UP,
                 [=]() -> uint8_t {
                     pushEvent(EVT_VIRTUAL_KEY_UP);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  // down
  new TextButton(this, {LCD_W / 2 - 20, 65, 40, 40}, STR_CHAR_DOWN,
                 [=]() -> uint8_t {
                     pushEvent(EVT_VIRTUAL_KEY_DOWN);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  // left
  left = new TextButton(this, {LCD_W / 2 - 70, 35, 40, 40}, STR_CHAR_LEFT,
                        [=]() -> uint8_t {
                            pushEvent(EVT_VIRTUAL_KEY_LEFT);
                            return 0;
                        }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  // right
  right = new TextButton(this, {LCD_W / 2 + 30, 35, 40, 40}, STR_CHAR_RIGHT,
                         [=]() -> uint8_t {
                             pushEvent(EVT_VIRTUAL_KEY_RIGHT);
                             return 0;
                         }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  // next
  new TextButton(this, {LCD_W / 2 + 80, 35, 60, 40}, "Next",
                 [=]() -> uint8_t {
                     pushEvent(EVT_VIRTUAL_KEY_NEXT);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  // previous
  new TextButton(this, {LCD_W / 2 - 140, 35, 60, 40}, "Prev",
                 [=]() -> uint8_t {
                     pushEvent(EVT_VIRTUAL_KEY_PREVIOUS);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

}

void CurveKeyboard::enableRightLeft(bool enable)
{
  TRACE("STOP %d", enable);
  left->enable(enable);
  right->enable(enable);
}

CurveKeyboard::~CurveKeyboard()
{
  _instance = nullptr;
}

void CurveKeyboard::paint(BitmapBuffer * dc)
{
  lcdSetColor(RGB(0xE0, 0xE0, 0xE0));
  dc->clear(CUSTOM_COLOR);
}
