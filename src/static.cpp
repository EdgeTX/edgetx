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
  if (flags & BUTTON_BACKGROUND)
    dc->drawSolidFilledRect(0, 0, rect.w, rect.h, DISABLE_COLOR);
  if (flags & CENTERED)
    dc->drawText(rect.w / 2, (rect.h - getFontHeight(flags)) / 2, text.c_str(), CENTERED | flags);
  else
    dc->drawText(0, 0, text.c_str(), flags);
}
