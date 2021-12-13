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
#include "listbox.h"

constexpr int LONG_PRESS_10MS = 40;
constexpr int ACTIVE_RADIUS = 5;

extern inline tmr10ms_t getTicks()
{
  return g_tmr10ms;
}

ListBase::ListBase(Window *parent, const rect_t &rect, std::vector<std::string> names,
          std::function<uint32_t()> getValue,
          std::function<void(uint32_t)> setValue,
          uint8_t lineHeight,
          WindowFlags windowFlags, LcdFlags lcdFlags) :
  FormField(parent, rect, windowFlags),
  names(names),
  _getValue(std::move(getValue)),
  _setValue(std::move(setValue)),
  lineHeight(lineHeight)
{
#if defined(HARDWARE_TOUCH)
  duration10ms = 0;
#endif
  setInnerHeight(names.size() * lineHeight);
  if (_getValue != nullptr)
    setSelected(_getValue());
  else 
    setSelected(0);
}

void ListBase::setSelected(int selected)
{
  if (selected >= 0 && selected < (int)names.size()  && selected != this->selected) {
    this->selected = selected;
    setScrollPositionY(lineHeight * this->selected - lineHeight);
    if (_setValue != nullptr) {
      _setValue(this->selected);
    }
    invalidate();
  }
}

void ListBase::drawLine(BitmapBuffer *dc, const rect_t &rect, uint32_t index, LcdFlags lcdFlags)
{
  std::string name = names[index];
  int x = rect.x;

  if ((uint32_t)activeIndex == index) {
    LcdFlags circleColor = index == (uint32_t) selected ? COLOR_THEME_PRIMARY2 : COLOR_THEME_PRIMARY1;
    auto textWidth = getTextWidth(name.c_str(), name.length(), lcdFlags);
    auto fontHeight = getFontHeight(FONT(STD));

    dc->drawFilledCircle(rect.x + 4 + textWidth + ACTIVE_RADIUS, rect.y + fontHeight / 2, ACTIVE_RADIUS, circleColor);
  }

  dc->drawText(x, rect.y, name.c_str(), lcdFlags);
}

void ListBase::paint(BitmapBuffer *dc)
{
  dc->clear(COLOR_THEME_SECONDARY3);

  int curY = 0;
  for (int n = 0; n < (int)names.size(); n++) {
    dc->drawSolidFilledRect(1, curY, rect.w - 2, lineHeight, n == selected ? COLOR_THEME_FOCUS : COLOR_THEME_PRIMARY2);

    LcdFlags textColor = n == selected ? COLOR_THEME_PRIMARY2 : COLOR_THEME_SECONDARY1;

    auto fontHeight = getFontHeight(FONT(STD));
    drawLine(dc, { 8, curY  + (lineHeight - fontHeight) / 2, rect.w, lineHeight}, n, textColor);

    curY += lineHeight;
  }
  if (!(windowFlags & (FORM_NO_BORDER | FORM_FORWARD_FOCUS))) {
    dc->drawSolidRect(0, getScrollPositionY(), rect.w, rect.h, 2,
                      COLOR_THEME_FOCUS);
  }
}

#if defined(HARDWARE_KEYS)
  void ListBase::onEvent(event_t event)
  {
    int oldSelected = selected;
    switch (event) {
      case EVT_ROTARY_RIGHT:
        oldSelected = (selected + 1) % names.size();
        setSelected(oldSelected);
        onKeyPress();
        break;
      case EVT_ROTARY_LEFT:
        oldSelected--;
        if (oldSelected < 0) oldSelected = names.size() - 1;
        setSelected(oldSelected);
        onKeyPress();
        break;
      case EVT_KEY_LONG(KEY_ENTER):
        if (longPressHandler) {
          killEvents(event);
          longPressHandler(event);
        }
        break;
      case EVT_KEY_BREAK(KEY_ENTER):
        if (pressHandler) {
          killEvents(event);
          pressHandler(event);
        }
        break;

      default:
        FormField::onEvent(event);
    }
  }
#endif

#if defined(HARDWARE_TOUCH)
bool ListBase::isLongPress()
{
  unsigned int curTimer = getTicks();
  return (slidingWindow == nullptr && duration10ms != 0 && curTimer - duration10ms > LONG_PRESS_10MS);
}


void ListBase::checkEvents(void)
{
  Window::checkEvents();

  if (isLongPress()) {
    if (longPressHandler) {
      setSelected(yDown / lineHeight);
      longPressHandler(0);
      killAllEvents();
      duration10ms = 0;
      return;
    }
  }
}

bool ListBase::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  if (touchState.event == TE_SLIDE_END) { 
    duration10ms = 0;
  }
  
  return FormField::onTouchSlide(x, y, startX, startY, slideX, slideY);
}

bool ListBase::onTouchStart(coord_t x, coord_t y)
{
  if (duration10ms == 0) {
    duration10ms = getTicks();
  }

  captureWindow(this);
  yDown = y;

  return true;  // stop the processing and say that i handled it
}

bool ListBase::onTouchEnd(coord_t x, coord_t y)
{
  if (!isEnabled()) return false;
  if (slidingWindow) 
    return false;  // if we slide then this is not a selection

  auto selected = yDown / lineHeight;
  setSelected(selected);

  duration10ms = 0;

  if (!hasFocus()) {
    setFocus(SET_FOCUS_DEFAULT);
  }

  if (pressHandler != nullptr) {
    pressHandler(0);
  }
  invalidate();

  return true;
}
#endif


