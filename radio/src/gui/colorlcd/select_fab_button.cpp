/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

SelectFabButton::SelectFabButton(FormGroup* parent, coord_t x, coord_t y,
                                 uint8_t icon, const char* title,
                                 std::function<uint8_t(void)> pressHandler,
                                 WindowFlags windowFlags) :
    // FabButton uses center coordinates, we want top left corner:
    FabButton(parent, x + FAB_BUTTON_SIZE / 2, y + FAB_BUTTON_SIZE / 2, icon,
              pressHandler, windowFlags),
    title(title)
{
  // Add some space on either side on the button
  setWidth(FAB_BUTTON_SIZE + SELECT_BUTTON_BORDER);

  // 2 Lines extra for the text + half border
  setHeight(FAB_BUTTON_SIZE + 2 * PAGE_LINE_HEIGHT + SELECT_BUTTON_BORDER / 2);

  // if (height() < (width() / 2) * 3) {
  //   setHeight((width() / 2) * 3);
  // }
}

void SelectFabButton::paint(BitmapBuffer* dc)
{
  FabButton::paint(dc);
  auto pos = title.find('\n');

  auto y_pos = FAB_BUTTON_SIZE;// + (height() - FAB_BUTTON_SIZE) / 5;
  dc->drawSizedText(width() / 2, y_pos,
                    title.c_str(), pos,
                    FOCUS_COLOR | CENTERED | VCENTERED);

  if (pos != std::string::npos) {
    // y_pos += (height() - FAB_BUTTON_SIZE) / 3;
    y_pos += PAGE_LINE_HEIGHT;
    dc->drawSizedText(width() / 2, y_pos, title.substr(pos+1).c_str(),
                      255, FOCUS_COLOR | CENTERED | VCENTERED);
  }

  if (hasFocus()) {
    dc->drawSolidRect(0, 0, width(), height(), 2, CHECKBOX_COLOR);
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
  FabButton::onEvent(event);
}
