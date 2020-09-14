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
    bool highlight = (index == selection);
    dc->drawSolidFilledRect(0, y, width(), TABLE_LINE_HEIGHT - 2, highlight ? MENU_HIGHLIGHT_BGCOLOR : TABLE_BGCOLOR);
    x = 10;
    for (unsigned i = 0; i < line.values.size(); i++) {
      dc->drawText(x, y + (TABLE_LINE_HEIGHT - getFontHeight(TABLE_BODY_FONT)) / 2 + 3, line.values[i].c_str(), line.flags | (highlight ? MENU_HIGHLIGHT_COLOR : DEFAULT_COLOR));
      x += columnsWidth[i];
    }
    y += TABLE_LINE_HEIGHT;
    index += 1;
  }
}

#if defined(HARDWARE_TOUCH)
bool Table::Body::onTouchEnd(coord_t x, coord_t y)
{
  unsigned index = y / TABLE_LINE_HEIGHT;
  if (index < lines.size()) {
    setFocus(SET_FOCUS_DEFAULT);
    auto onPress = lines[index].onPress;
    if (onPress)
      onPress();
  }
  return true;
}
#endif

#if defined(HARDWARE_KEYS)
void Table::Body::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    if (selection >= 0) {
      auto onPress = lines[selection].onPress;
      if (onPress)
        onPress();
    }
  }
  if (event == EVT_ROTARY_RIGHT) {
    if (lines.empty())
      return;

    int newSelection = (selection + 1) % lines.size();
    select(newSelection, true);
    auto onSelect = lines[selection].onSelect;
    if (onSelect) {
      onSelect();
    }
  }
  else if (event == EVT_ROTARY_LEFT) {
    if (lines.empty())
      return;

    int newSelection = selection <= 0 ? lines.size() - 1 : selection - 1;
    select(newSelection, true);
    auto onSelect = lines[selection].onSelect;
    if (onSelect) {
      onSelect();
    }
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT) && selection >= 0) {
    select(-1, true);
    Window::onEvent(event);
  }
  else {
    Window::onEvent(event);
  }
}
#endif
