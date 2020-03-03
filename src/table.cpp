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

#include "table.h"
#include "font.h"

void Table::Header::paint(BitmapBuffer * dc)
{
  coord_t x = 10;
  if (!header.values.empty()) {
    dc->clear(TABLE_HEADER_BGCOLOR);
    for (unsigned i = 0; i < header.values.size(); i++) {
      dc->drawText(x, (TABLE_LINE_HEIGHT - getFontHeight(TABLE_HEADER_FONT)) / 2, header.values[i].c_str(), TABLE_HEADER_FONT);
      x += columnsWidth[i];
    }
  }
}

void Table::Body::paint(BitmapBuffer * dc)
{
  coord_t y = 0;
  coord_t x;
  int index = 0;
  for (auto & line: lines) {
    dc->drawSolidFilledRect(0, y, width(), TABLE_LINE_HEIGHT - 2, index == selection ? FOCUS_BGCOLOR : TABLE_BGCOLOR);
    x = 10;
    for (unsigned i = 0; i < line.values.size(); i++) {
      dc->drawText(x, y + (TABLE_LINE_HEIGHT - getFontHeight(TABLE_BODY_FONT)) / 2 + 3, line.values[i].c_str(), TABLE_BODY_FONT | (index == selection ? FOCUS_COLOR : DEFAULT_COLOR));
      x += columnsWidth[i];
    }
    y += TABLE_LINE_HEIGHT;
    index += 1;
  }
}
