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

#include "libopenui_config.h"
#include "fab_button.h"
#include "font.h"
#include "theme.h"

const uint8_t __alpha_button_on[] {
#include "alpha_button_on.lbm"
};
Bitmap ALPHA_BUTTON_ON(BMP_ARGB4444, (const uint16_t*)__alpha_button_on);

const uint8_t __alpha_button_off[] {
#include "alpha_button_off.lbm"
};
Bitmap ALPHA_BUTTON_OFF(BMP_ARGB4444, (const uint16_t*)__alpha_button_off);

FabButton::FabButton(FormGroup* parent, coord_t x, coord_t y, uint8_t icon,
                     std::function<uint8_t(void)> pressHandler,
                     WindowFlags windowFlags) :
    Button(parent,
           {x - FAB_BUTTON_SIZE / 2, y - FAB_BUTTON_SIZE / 2, FAB_BUTTON_SIZE,
            FAB_BUTTON_SIZE},
           pressHandler, windowFlags),
    icon(icon)
{
}

void FabButton::paint(BitmapBuffer * dc)
{
  auto bitmap = checked() ? &ALPHA_BUTTON_ON : &ALPHA_BUTTON_OFF;

  dc->drawBitmap((width() - bitmap->width()) / 2,
                 (FAB_BUTTON_SIZE - bitmap->height()) / 2, bitmap);

  const BitmapBuffer* mask = theme->getIconMask(icon);
  if (mask) {
    dc->drawMask((width() - mask->width()) / 2,
                 (FAB_BUTTON_SIZE - mask->height()) / 2, mask, DEFAULT_BGCOLOR);
  }
}
