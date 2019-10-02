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

MenuWindow::MenuWindow(Menu * parent):
  Window(parent, {LCD_W / 2 - 100, LCD_H / 2, 200, 0}, OPAQUE)
{
  setFocus();
}

void MenuWindow::select(int index)
{
  selectedIndex = index;
  if (innerHeight > height()) {
    setScrollPositionY(lineHeight * index - 3 * lineHeight);
  }
  invalidate();
}

#if defined(HARDWARE_KEYS)
void MenuWindow::onKeyEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_ROTARY_RIGHT) {
    select((selectedIndex + 1) % lines.size());
  }
  else if (event == EVT_ROTARY_LEFT) {
    select(selectedIndex == 0 ? lines.size() - 1 : selectedIndex - 1);
  }
  else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    Window::onKeyEvent(event); // the window above will be closed on event
    lines[selectedIndex].onPress();
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    if (onCancel)
      onCancel();
    Window::onKeyEvent(event);
  }
  else {
    Window::onKeyEvent(event);
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool MenuWindow::onTouchEnd(coord_t x, coord_t y)
{
  int index = y / lineHeight;
  lines[index].onPress();
  return false; // = close the menu (inverted so that click outside the menu closes it)
}
#endif

void MenuWindow::paint(BitmapBuffer * dc)
{
  int width = (innerHeight > height() ? 195 : 200);
  dc->clear(HEADER_BGCOLOR);
  for (unsigned i = 0; i < lines.size(); i++) {
    auto & line = lines[i];
    LcdFlags flags = (selectedIndex == (int) i ? WARNING_COLOR : MENU_TITLE_COLOR);
    if (line.drawLine) {
      line.drawLine(dc, 0, i * lineHeight /*+ (lineHeight - 20) / 2*/, flags);
    }
    else {
      const char *text = line.text.data();
      dc->drawText(10, i * lineHeight + (lineHeight - 20) / 2, text[0] == '\0' ? "---" : text, flags);
    }
    if (i > 0) {
      dc->drawSolidHorizontalLine(0, i * lineHeight - 1, width, CURVE_AXIS_COLOR);
    }
  }
}

void Menu::updatePosition()
{
  if (!toolbar) {
    // there is no navigation bar at the left, we may center the window on screen
    int count = min<int>(MenuWindow::maxLines, menuWindow.lines.size());
    coord_t h = count * MenuWindow::lineHeight - 1;
    menuWindow.setTop((LCD_H - h) / 2 + 20);
    menuWindow.setHeight(h);
  }
  menuWindow.setInnerHeight(menuWindow.lines.size() * MenuWindow::lineHeight - 1);
}

void Menu::addLine(const std::string & text, std::function<void()> onPress)
{
  menuWindow.addLine(text, std::move(onPress));
  updatePosition();
}

void Menu::addLine(std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> drawLine, std::function<void()> onPress)
{
  menuWindow.addLine(drawLine, std::move(onPress));
  updatePosition();
}

void Menu::removeLines()
{
  menuWindow.removeLines();
  updatePosition();
}

#if defined(HARDWARE_KEYS)
void Menu::onKeyEvent(event_t event)
{
  if (toolbar && (event == EVT_KEY_BREAK(KEY_PGDN) || event == EVT_KEY_LONG(KEY_PGDN))) {
    toolbar->onKeyEvent(event);
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT) || event == EVT_KEY_BREAK(KEY_ENTER)) {
    deleteLater();
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool Menu::onTouchEnd(coord_t x, coord_t y)
{
  if (!Window::onTouchEnd(x, y)) {
    deleteLater();
  }
  return true;
}

bool Menu::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  Window::onTouchSlide(x, y, startX, startY, slideX, slideY);
  return true;
}
#endif
