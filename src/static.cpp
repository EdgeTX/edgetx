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
  if (windowFlags & BUTTON_BACKGROUND)
    dc->drawSolidFilledRect(0, 0, rect.w, rect.h, DISABLE_COLOR);
  if (textFlags & CENTERED)
    dc->drawText(rect.w / 2, 1 + (rect.h - getFontHeight(textFlags)) / 2, text.c_str(), textFlags);
  else if (textFlags & RIGHT)
    dc->drawText(rect.w, FIELD_PADDING_TOP, text.c_str(), textFlags);
  else
    dc->drawText(0, 0 + FIELD_PADDING_TOP, text.c_str(), textFlags);
}
