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

#include "keyboard_number.h"
#include "button.h"
#include "libopenui_globals.h"

constexpr coord_t KEYBOARD_HEIGHT = 90;

NumberKeyboard * NumberKeyboard::_instance = nullptr;

NumberKeyboard::NumberKeyboard() :
  Keyboard(KEYBOARD_HEIGHT)
{
  new TextButton(this, {LCD_W / 2 - 115, 10, 50, 30}, "<<",
                 [=]() -> uint8_t {
                     pushEvent(EVT_VIRTUAL_KEY_BACKWARD);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(this, {LCD_W / 2 - 55, 10, 50, 30}, "-",
                 [=]() -> uint8_t {
                     pushEvent(EVT_VIRTUAL_KEY_MINUS);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(this, {LCD_W / 2 + 5, 10, 50, 30}, "+",
                 [=]() -> uint8_t {
                     pushEvent(EVT_VIRTUAL_KEY_PLUS);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(this, {LCD_W / 2 + 65, 10, 50, 30}, ">>",
                 [=]() -> uint8_t {
                     pushEvent(EVT_VIRTUAL_KEY_FORWARD);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(this, {LCD_W / 2 - 115, 50, 50, 30}, "MIN",
                 [=]() -> uint8_t {
                     pushEvent(EVT_VIRTUAL_KEY_MIN);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(this, {LCD_W / 2 + 65, 50, 50, 30}, "MAX",
                 [=]() -> uint8_t {
                     pushEvent(EVT_VIRTUAL_KEY_MAX);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(this, { LCD_W/2 - 55, 50, 50, 30 }, "DEF",
                 [=]() -> uint8_t {
                   pushEvent(EVT_VIRTUAL_KEY_DEFAULT);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);
				 
  new TextButton(this, { LCD_W/2 + 5, 50, 50, 30 }, "+/-",
                 [=]() -> uint8_t {
                   pushEvent(EVT_VIRTUAL_KEY_SIGN);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);     
}

NumberKeyboard::~NumberKeyboard()
{
  _instance = nullptr;
}

void NumberKeyboard::paint(BitmapBuffer * dc)
{
  lcdSetColor(RGB(0xE0, 0xE0, 0xE0));
  dc->clear(CUSTOM_COLOR);
}
