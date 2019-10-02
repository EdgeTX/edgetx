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

#include "dialog.h"
#include "mainwindow.h"
#include "form.h"

Dialog::Dialog(std::string title, const rect_t rect):
  Window(&mainWindow, rect, OPAQUE),
  title(std::move(title)),
  previousFocus(focusWindow)
{
  bringToTop();
  FormField::clearCurrentField();
  setFocus();
}

void Dialog::paint(BitmapBuffer * dc)
{
  dc->drawRect(0, 0, width() - 1, height() - 1, 1, SOLID, TEXT_COLOR);
  dc->drawSolidFilledRect(1, 1, width() - 3, PAGE_LINE_HEIGHT, TITLE_BGCOLOR);
  dc->drawText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, title.c_str(), MENU_TITLE_COLOR);
  dc->drawSolidFilledRect(1, PAGE_LINE_HEIGHT + 1, width() - 3, height() - PAGE_LINE_HEIGHT - 3, TEXT_BGCOLOR_INDEX);
}

void Dialog::deleteLater()
{
  if (previousFocus) {
    previousFocus->setFocus();
  }

  Window::deleteLater();
}
