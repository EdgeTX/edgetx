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

#include "menu.h"
#include "font.h"
#include "theme.h"

void MenuBody::select(int index)
{
  selectedIndex = index;
  if (innerHeight > height()) {
    setScrollPositionY(MENUS_LINE_HEIGHT * index - 3 * MENUS_LINE_HEIGHT);
  }
  invalidate();
}

#if defined(HARDWARE_KEYS)
void MenuBody::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_ROTARY_RIGHT) {
    select(int((selectedIndex + 1) % lines.size()));
  }
  else if (event == EVT_ROTARY_LEFT) {
    select(int(selectedIndex == 0 ? lines.size() - 1 : selectedIndex - 1));
  }
  else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    if (selectedIndex < 0) {
      select(0);
    }
    else {
      Window::onEvent(event); // the window above will be closed on event
      lines[selectedIndex].onPress();
    }
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    if (onCancel)
      onCancel();
    Window::onEvent(event);
  }
  else {
    Window::onEvent(event);
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool MenuBody::onTouchEnd(coord_t /*x*/, coord_t y)
{
  int index = y / MENUS_LINE_HEIGHT;
  if (index < lines.size()) {
    lines[index].onPress();
  }
  getParent()->getParent()->deleteLater();
  return true;
}
#endif

void MenuBody::paint(BitmapBuffer * dc)
{
  for (unsigned i = 0; i < lines.size(); i++) {
    auto & line = lines[i];
    LcdFlags flags = MENU_COLOR | MENU_FONT;
    if (selectedIndex == (int)i) {
      flags = MENU_HIGHLIGHT_COLOR | MENU_FONT;
      if (MENU_HIGHLIGHT_BGCOLOR != MENU_BGCOLOR) {
        dc->drawSolidFilledRect(0, i * MENUS_LINE_HEIGHT, width(), MENUS_LINE_HEIGHT, MENU_HIGHLIGHT_BGCOLOR);
      }
    }
    if (line.drawLine) {
      line.drawLine(dc, 0, i * MENUS_LINE_HEIGHT /*+ (lineHeight - 20) / 2*/, flags);
    }
    else {
      const char * text = line.text.data();
      dc->drawText(10, i * MENUS_LINE_HEIGHT + (MENUS_LINE_HEIGHT - 20) / 2, text[0] == '\0' ? "---" : text, flags);
    }
    if (i > 0) {
      dc->drawSolidHorizontalLine(0, i * MENUS_LINE_HEIGHT - 1, MENUS_WIDTH, MENU_LINE_COLOR);
    }
  }
}

MenuWindowContent::MenuWindowContent(Menu * parent):
  ModalWindowContent(parent, {(LCD_W - MENUS_WIDTH) / 2, (LCD_H - MENUS_WIDTH) / 2, MENUS_WIDTH, 0}),
  body(this, {0, 0, width(), height()})
{
  body.setFocus(SET_FOCUS_DEFAULT);
}

void MenuWindowContent::paint(BitmapBuffer * dc)
{
  // the background
  dc->clear(MENU_BGCOLOR);

  // the title
  if (!title.empty()) {
    dc->drawText(MENUS_WIDTH / 2, (POPUP_HEADER_HEIGHT - getFontHeight(MENU_HEADER_FONT)) / 2, title.c_str(), CENTERED | MENU_HEADER_FONT);
    dc->drawSolidHorizontalLine(0, POPUP_HEADER_HEIGHT - 1, MENUS_WIDTH, MENU_LINE_COLOR);
  }
}

Menu::Menu(Window * parent):
  ModalWindow(parent),
  content(createMenuWindow(this))
{
}

void Menu::updatePosition()
{
  if (!toolbar) {
    // there is no navigation bar at the left, we may center the window on screen
    auto headerHeight = content->title.empty() ? 0 : POPUP_HEADER_HEIGHT;
    auto bodyHeight = limit<coord_t>(MENUS_MIN_HEIGHT, content->body.lines.size() * MENUS_LINE_HEIGHT - 1, MENUS_MAX_HEIGHT);
    content->setTop((LCD_H - headerHeight - bodyHeight) / 2 + MENUS_OFFSET_TOP);
    content->setHeight(headerHeight + bodyHeight);
    content->body.setTop(headerHeight);
    content->body.setHeight(bodyHeight);
  }
  content->body.setInnerHeight(content->body.lines.size() * MENUS_LINE_HEIGHT - 1);
}

void Menu::setTitle(std::string text)
{
  content->setTitle(std::move(text));
  updatePosition();
}

void Menu::addLine(const std::string & text, std::function<void()> onPress)
{
  content->body.addLine(text, std::move(onPress));
  updatePosition();
}

void Menu::addCustomLine(std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> drawLine, std::function<void()> onPress)
{
  content->body.addCustomLine(std::move(drawLine), std::move(onPress));
  updatePosition();
}

void Menu::removeLines()
{
  content->body.removeLines();
  updatePosition();
}

#if defined(HARDWARE_KEYS)
void Menu::onEvent(event_t event)
{
  if (toolbar && (event == EVT_KEY_BREAK(KEY_PGDN) || event == EVT_KEY_LONG(KEY_PGDN))) {
    toolbar->onEvent(event);
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT) || event == EVT_KEY_BREAK(KEY_ENTER)) {
    deleteLater();
  }
}
#endif
