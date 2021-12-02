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


// ensure index is in range and also handle wrapping index
int MenuBody::rangeCheck(int index)
{
    if (index < 0) 
      index = lines.size() - 1;
    else if (index > (signed)lines.size() - 1)
      index = 0;

    return index;
}

void MenuBody::setIndex(int index)
{
  selectedIndex = index;
  // calculate the correct scroll postion based on there being separators
  int scrollY = 0;
  for (int i = 0; i < selectedIndex; i++)
    scrollY += lines[i].lineHeight();

  if (innerHeight > height()) {
    setScrollPositionY(scrollY - 3 * MENUS_LINE_HEIGHT);
  }

  invalidate();
}

void MenuBody::select(int index)
{
  // adjust the selection based on separators
  for (int i = 0; i <= index; i++) {
    if (lines[i].isSeparator)
      index++;
    index = rangeCheck(index);
  }

  setIndex(index);
}

void MenuBody::selectNext(MENU_DIRECTION direction)
{
  // look for the next non separator line
  int index = selectedIndex + direction;
  index = rangeCheck(index);
  while (lines[index].isSeparator) {
    index += direction;
    index = rangeCheck(index);
  }

  setIndex(index);
}


#if defined(HARDWARE_KEYS)
void MenuBody::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_ROTARY_RIGHT) {
    if (!lines.empty()) {
      selectNext(DIRECTION_UP);
      onKeyPress();
    }
  }
  else if (event == EVT_ROTARY_LEFT) {
    if (!lines.empty()) {
      selectNext(DIRECTION_DOWN);
      onKeyPress();
    }
  }
  else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    if (!lines.empty()) {
      onKeyPress();
      if (selectedIndex < 0) {
        select(0);
      }
      else {
        Menu * menu = getParentMenu();
        if (menu->multiple) {
          lines[selectedIndex].onPress();
          menu->invalidate();
        }
        else {
          menu->deleteLater();
          lines[selectedIndex].onPress();
        }
      }
    }
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    onKeyPress();
    if (onCancel) {
      onCancel();
    }
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
  Menu * menu = getParentMenu();

  int index = 0;

  // dynamically find the selected element based on variable line heights
  coord_t yAccumulator = 0;
  for (int i = 0; i < (signed)lines.size(); i++) {
    if (y >= yAccumulator && y <= yAccumulator + lines[i].lineHeight()) {
      index = i;
      break;
    }
    yAccumulator += lines[i].lineHeight();
  }

  // dont allow selecting separators
  if (lines[index].isSeparator)
    return false;

  if (index < (int)lines.size()) {
    onKeyPress();
    if (menu->multiple) {
      if (selectedIndex == index)
        lines[index].onPress();
      else
        setIndex(index);
      menu->invalidate();
    }
    else {
      setIndex(index);
      menu->deleteLater();
      lines[index].onPress();
    }
  }

  return true;
}
#endif

void MenuBody::paint(BitmapBuffer * dc)
{
  dc->clear(COLOR_THEME_PRIMARY2);

  int y = 0;
  for (int i = 0; i < (signed)lines.size(); i++) {
    auto& line = lines[i];
    LcdFlags flags = COLOR_THEME_PRIMARY3 | MENU_FONT;

    // draw selection if appropriate
    if (selectedIndex == i && ! line.isSeparator) {
      flags = COLOR_THEME_PRIMARY2 | MENU_FONT;
      if (COLOR_THEME_FOCUS != COLOR_THEME_PRIMARY2) {
        dc->drawSolidFilledRect(0, y, width(),
                                MENUS_LINE_HEIGHT, COLOR_THEME_FOCUS);
      }
    }

    if (line.isSeparator) {
      dc->drawHorizontalLine(5, y + MENUS_SEPARATOR_HEIGHT / 2, width() - 10, 255, COLOR_THEME_SECONDARY1);
    } else if (line.drawLine) {
      line.drawLine(dc, 0, y, COLOR_THEME_SECONDARY3);
    } else {
      const char* text = line.text.data();
      dc->drawText(10,
                   y + (MENUS_LINE_HEIGHT - getFontHeight(MENU_FONT)) / 2,
                   text[0] == '\0' ? "---" : text, flags);
    }

    Menu* menu = getParentMenu();
    if (menu->multiple && line.isChecked) {
      theme->drawCheckBox(dc, line.isChecked(), width() - 35,
                          y + (MENUS_LINE_HEIGHT - 20) / 2,
                          0);
    }

    y += line.lineHeight();
  }
}

MenuWindowContent::MenuWindowContent(Menu* parent) :
    ModalWindowContent(parent, {(LCD_W - MENUS_WIDTH) / 2,
                                (LCD_H - MENUS_WIDTH) / 2, MENUS_WIDTH, 0}),
    body(this, {0, 0, width(), height()})
{
  body.setFocus(SET_FOCUS_DEFAULT);
}

void MenuWindowContent::paint(BitmapBuffer * dc)
{
  // the background
  dc->clear(COLOR_THEME_SECONDARY1);

  // the title
  if (!title.empty()) {
    dc->drawText(MENUS_WIDTH / 2,
                 (POPUP_HEADER_HEIGHT - getFontHeight(MENU_HEADER_FONT)) / 2,
                 title.c_str(), CENTERED | MENU_HEADER_FONT | COLOR_THEME_PRIMARY2);
    dc->drawSolidHorizontalLine(0, POPUP_HEADER_HEIGHT - 1, MENUS_WIDTH,
                                COLOR_THEME_SECONDARY2);
  }
}

Menu::Menu(Window * parent, bool multiple):
  ModalWindow(parent, true),
  content(createMenuWindow(this)),
  multiple(multiple)
{
}

void Menu::updatePosition()
{
  // calcualte the correct menu height given that the line heights are variable
  coord_t height = 0;
  for (int i = 0; i < (signed)content->body.lines.size(); i++)
    height += content->body.lines[i].lineHeight();

  if (!toolbar) {
    // there is no navigation bar at the left, we may center the window on screen
    auto headerHeight = content->title.empty() ? 0 : POPUP_HEADER_HEIGHT;
    auto bodyHeight = limit<coord_t>(MENUS_MIN_HEIGHT, height, MENUS_MAX_HEIGHT);
    content->setTop((LCD_H - headerHeight - bodyHeight) / 2 + MENUS_OFFSET_TOP);
    content->setHeight(headerHeight + bodyHeight);
    content->body.setTop(headerHeight);
    content->body.setHeight(bodyHeight);
  }

  content->body.setInnerHeight(height);
}

void Menu::setTitle(std::string text)
{
  content->setTitle(std::move(text));
  updatePosition();
}

void Menu::addLine(const std::string & text, std::function<void()> onPress, std::function<bool()> isChecked)
{
  content->body.addLine(text, std::move(onPress), std::move(isChecked));
  updatePosition();
}

void Menu::addSeparator()
{
  content->body.addSeparator();
  updatePosition();
}

void Menu::addCustomLine(std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> drawLine, std::function<void()> onPress, std::function<bool()> isChecked)
{
  content->body.addCustomLine(std::move(drawLine), std::move(onPress), std::move(isChecked));
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
  if (toolbar && (event == EVT_KEY_BREAK(KEY_PGDN) || event == EVT_KEY_LONG(KEY_PGDN) || 
      event == EVT_KEY_BREAK(KEY_PGUP))) {
    toolbar->onEvent(event);
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    deleteLater();
  }
  else if (event == EVT_KEY_BREAK(KEY_ENTER) && !multiple) {
    deleteLater();
  }
}
#endif

void Menu::paint(BitmapBuffer * dc)
{
  ModalWindow::paint(dc);

  rect_t r(content->getRect());
  if (toolbar) {
    r.x = toolbar->left();
    r.w += toolbar->width();
  }
  dc->drawSolidRect(r.x - 1, r.y - 1, r.w + 2, r.h + 2, 1, COLOR_THEME_SECONDARY1);
}
