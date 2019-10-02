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

#include "page.h"
#include "mainwindow.h"
#include "keyboard_number.h"
#include "keyboard_text.h"
#include "keyboard_curve.h"
#include "opentx.h" // TODO for constants

PageHeader::PageHeader(Page * parent, uint8_t icon):
  Window(parent, { 0, 0, LCD_W, MENU_HEADER_HEIGHT }, OPAQUE),
  icon(icon)
#if defined(HARDWARE_TOUCH)
  , back(this, { 0, 0, MENU_HEADER_BUTTON_WIDTH, MENU_HEADER_BUTTON_WIDTH }, ICON_BACK,
       [=]() -> uint8_t {
         parent->deleteLater();
         return 0;
       }, BUTTON_NOFOCUS)
#endif
{
}

void PageHeader::paint(BitmapBuffer * dc)
{
  theme->drawPageHeader(dc, this);
}

Page::Page(unsigned icon):
  Window(&mainWindow, {0, 0, LCD_W, LCD_H}, OPAQUE),
  header(this, icon),
  body(this, { 0, MENU_HEADER_HEIGHT, LCD_W, LCD_H - MENU_HEADER_HEIGHT }),
  previousFocus(focusWindow)
{
  setFocus();
}

Page::~Page()
{
#if defined(HARDWARE_TOUCH)
  TextKeyboard::instance()->disable(false);
  NumberKeyboard::instance()->disable(false);
  CurveKeyboard::instance()->disable(false);
#endif
}

void Page::deleteLater()
{
  if (previousFocus) {
    previousFocus->setFocus();
  }

  Window::deleteLater();
}

void Page::paint(BitmapBuffer * dc)
{
  dc->clear(TEXT_BGCOLOR);
}

#if defined(HARDWARE_KEYS)
void Page::onKeyEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_LONG(KEY_EXIT) || event == EVT_KEY_BREAK(KEY_EXIT)) {
    killEvents(event);
    deleteLater();
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool Page::onTouchEnd(coord_t x, coord_t y)
{
  if (Window::onTouchEnd(x, y))
    return true;

  TextKeyboard::instance()->disable(true);
  NumberKeyboard::instance()->disable(true);
  CurveKeyboard::instance()->disable(true);
  return true;
}
#endif
