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
  if (!cells.empty()) {
    dc->clear(TABLE_HEADER_BGCOLOR);
    for (unsigned i = 0; i < cells.size(); i++) {
      auto cell = cells[i];
      if (cell) {
        cell->paint(dc, x, 0, TABLE_HEADER_FONT);
      }
      x += static_cast<Table *>(parent)->columnsWidth[i];
    }
  }
}

void Table::Body::checkEvents()
{
  Window::checkEvents();

  if (_deleted)
    return;

  coord_t y = 0;
  for (auto line: lines) {
    coord_t x = 10;
    for (unsigned i = 0; i < line->cells.size(); i++) {
      auto cell = line->cells[i];
      auto width = static_cast<Table *>(parent)->columnsWidth[i];
      if (cell && cell->needsInvalidate()) {
        invalidate({x, y, width, TABLE_LINE_HEIGHT - 2});
      }
      x += width;
    }
    y += TABLE_LINE_HEIGHT;
  }
}

void Table::Body::paint(BitmapBuffer * dc)
{
  coord_t y = 0;
  int index = 0;
  dc->clear(DEFAULT_BGCOLOR);
  for (auto line: lines) {
    bool highlight = (index == selection);
    dc->drawSolidFilledRect(0, y, width(), TABLE_LINE_HEIGHT - 2, highlight ? MENU_HIGHLIGHT_BGCOLOR : TABLE_BGCOLOR);
    coord_t x = 10;
    for (unsigned i = 0; i < line->cells.size(); i++) {
      auto cell = line->cells[i];
      if (cell) {
        cell->paint(dc, x, y, line->flags | (highlight ? MENU_HIGHLIGHT_COLOR : DEFAULT_COLOR));
      }
      x += static_cast<Table *>(parent)->columnsWidth[i];
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
    auto onPress = lines[index]->onPress;
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
      auto onPress = lines[selection]->onPress;
      if (onPress)
        onPress();
    }
  }
  if (event == EVT_ROTARY_RIGHT) {
    auto table = static_cast<Table *>(parent);
    if (table->getWindowFlags() & FORWARD_SCROLL) {
      auto index = selection + 1;
      if (index < int(lines.size())) {
        select(index, true);
      }
      else {
        auto next = table->getNextField();
        if (next) {
          select(-1, false);
          next->setFocus(SET_FOCUS_FORWARD);
        }
      }
    }
    else {
      if (!lines.empty()) {
        select((selection + 1) % lines.size(), true);
      }
    }
  }
  else if (event == EVT_ROTARY_LEFT) {
    auto table = static_cast<Table *>(parent);
    if (table->getWindowFlags() & FORWARD_SCROLL) {
      auto index = selection - 1;
      if (index >= 0) {
        select(index, true);
      }
      else {
        auto previous = table->getPreviousField();
        if (previous) {
          select(-1, false);
          previous->setFocus(SET_FOCUS_BACKWARD);
        }
      }
    }
    else {
      if (!lines.empty()) {
        select(selection <= 0 ? lines.size() - 1 : selection - 1, true);
      }
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
