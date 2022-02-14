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

#include "menutoolbar.h"
#include "font.h"

void MenuToolbarButton::paint(BitmapBuffer * dc)
{
  if (checked()) {
    dc->drawSolidFilledRect(
        MENUS_TOOLBAR_BUTTON_PADDING, MENUS_TOOLBAR_BUTTON_PADDING,
        MENUS_TOOLBAR_BUTTON_WIDTH - 2 * MENUS_TOOLBAR_BUTTON_PADDING,
        MENUS_TOOLBAR_BUTTON_WIDTH - 2 * MENUS_TOOLBAR_BUTTON_PADDING,
        COLOR_THEME_FOCUS);
    dc->drawText(rect.w / 2, (rect.h - getFontHeight(FONT(STD))) / 2 + 1,
                 picto, CENTERED | COLOR_THEME_PRIMARY2);
  } else {
    dc->drawText(rect.w / 2, (rect.h - getFontHeight(FONT(STD))) / 2 + 1,
                 picto, CENTERED | COLOR_THEME_PRIMARY1);
  }
}
