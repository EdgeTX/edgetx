/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "select_fab_button.h"

#include "font.h"
#include "static.h"
#include "theme.h"

const uint8_t __alpha_button_off[]{
#include "alpha_button_off.lbm"
};
LZ4Bitmap ALPHA_BUTTON_OFF(BMP_ARGB4444, __alpha_button_off);

SelectFabButton::SelectFabButton(FormWindow* parent, uint8_t icon,
                                 const char* title,
                                 std::function<uint8_t(void)> pressHandler,
                                 WindowFlags windowFlags) :
    Button(parent, {}, pressHandler, windowFlags, 0, etx_quick_button_create),
    icon(icon),
    title(title)
{
  padAll(0);
  new StaticText(this, {1, FAB_BUTTON_SIZE - 14, FAB_BUTTON_SIZE - 4, 0}, title, 0,
                 COLOR2FLAGS(WHITE) | CENTERED);
}

void SelectFabButton::paint(BitmapBuffer* dc)
{
  const BitmapBuffer* bitmap = &ALPHA_BUTTON_OFF;

  dc->drawBitmap((FAB_BUTTON_SIZE - bitmap->width()) / 2,
                 (FAB_BUTTON_SIZE - bitmap->height()) / 2 - 6, bitmap);

  const BitmapBuffer* mask = EdgeTxTheme::instance()->getIconMask(icon);
  if (mask) {
    dc->drawMask((FAB_BUTTON_SIZE - mask->width()) / 2,
                 (FAB_BUTTON_SIZE - mask->height()) / 2 - 6, mask,
                 COLOR2FLAGS(WHITE));
  }
}

void SelectFabButton::onEvent(event_t event)
{
#if defined(HARDWARE_KEYS)
  switch (event) {
    case EVT_KEY_BREAK(KEY_EXIT):
      killEvents(event);
      parent->deleteLater();
      return;
  }
#endif
  Button::onEvent(event);
}
