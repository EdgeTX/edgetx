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

#include "static.h"
#include "font.h"

void StaticText::paint(BitmapBuffer * dc)
{
  // if (bgColor) {
  //   dc->drawSolidFilledRect(0, 0, rect.w, rect.h, bgColor);
  // }

  // coord_t x;
  // if (textFlags & CENTERED)
  //   x = rect.w / 2;
  // else if (textFlags & RIGHT)
  //   x = rect.w;
  // else
  //   x = 0;

  // coord_t y = (textFlags & VCENTERED) ? (rect.h - getFontHeight(textFlags)) / 2
  //                                     : FIELD_PADDING_TOP;

  // auto start = text.c_str();
  // auto nextline = findNextLine(start);
  // if (nextline) {
  //   auto current = start;
  //   do {
  //     dc->drawText(x, y,
  //                  text.substr(current - start, nextline - current).c_str(),
  //                  textFlags);
  //     current = nextline + 1;
  //     nextline = findNextLine(current);
  //     y += getFontHeight(textFlags) + 2;
  //   } while (nextline);
  //   dc->drawText(x, y, current, textFlags);
  // } else {
  //   dc->drawText(x, y, start, textFlags);
  // }
}
