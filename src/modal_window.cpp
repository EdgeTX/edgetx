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


#include "modal_window.h"
#include "mainwindow.h"
#include "font.h"

ModalWindow::ModalWindow():
  Window(&mainWindow, {0, 0, LCD_W, LCD_H})
#if !defined(HARDWARE_TOUCH)
  , previousFocus(focusWindow)
#endif
{
}

void ModalWindow::paint(BitmapBuffer * dc)
{
  dc->drawFilledRect(0, 0, width(), height(), SOLID, OVERLAY_COLOR | OPACITY(5));
}

void ModalWindowContent::paint(BitmapBuffer * dc)
{
  dc->drawSolidFilledRect(0, 0, width(), PAGE_LINE_HEIGHT, FOCUS_BGCOLOR);
  dc->drawText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, title.c_str(), FOCUS_COLOR);
  dc->drawSolidFilledRect(0, PAGE_LINE_HEIGHT, width(), height() - PAGE_LINE_HEIGHT, DEFAULT_BGCOLOR);

/*
  // the background
  dc->clear(MENU_BGCOLOR);

  // the title
  if (!title.empty()) {
    dc->drawText(MENUS_WIDTH / 2, (MENUS_HEADER_HEIGHT - getFontHeight(MENU_HEADER_FONT)) / 2, title.c_str(), CENTERED | MENU_HEADER_FONT);
    dc->drawSolidHorizontalLine(0, MENUS_HEADER_HEIGHT - 1, MENUS_WIDTH, MENU_LINE_COLOR);
  }*/
}
