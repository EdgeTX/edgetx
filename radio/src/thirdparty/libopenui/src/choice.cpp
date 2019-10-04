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

#include "choice.h"
#include "menu.h"
#include "theme.h"

Choice::Choice(Window * parent, const rect_t & rect, const char * values, int16_t vmin, int16_t vmax,
               std::function<int16_t()> getValue, std::function<void(int16_t)> setValue, LcdFlags flags) :
  ChoiceBase(parent, rect),
  values(values),
  vmin(vmin),
  vmax(vmax),
  getValue(std::move(getValue)),
  setValue(std::move(setValue)),
  flags(flags)
{
}

void Choice::paint(BitmapBuffer * dc)
{
  FormField::paint(dc);
  theme->drawChoice(dc, this, textHandler ? textHandler(getValue()).c_str() : TEXT_AT_INDEX(values, getValue() - vmin).c_str());
//
//
//  LcdFlags textColor;
//  if (editMode)
//    textColor = TEXT_INVERTED_COLOR;
//  else if (hasFocus())
//    textColor = TEXT_INVERTED_BGCOLOR;
//  else
//    textColor = 0;
//
//  if (textHandler)
//    dc->drawText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, , textColor);
//  else
//    dc->drawTextAtIndex(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, values, getValue() - vmin, flags | textColor);
//
//  dc->drawBitmapPattern(rect.w - 14, (rect.h - 5) / 2, LBM_DROPDOWN, textColor);
}

#if defined(HARDWARE_KEYS)
void Choice::onKeyEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    editMode = true;
    invalidate();
    openMenu();
  }
  else {
    FormField::onKeyEvent(event);
  }
}
#endif

void Choice::openMenu()
{
  auto menu = new Menu();
  auto value = getValue();
  int count = 0;
  int current = -1;

  for (int i = vmin; i <= vmax; ++i) {
    if (isValueAvailable && !isValueAvailable(i))
      continue;
    if (textHandler) {
      menu->addLine(textHandler(i), [=]() {
          setValue(i);
      });
    }
    else {
      menu->addLine(TEXT_AT_INDEX(values, i - vmin), [=]() {
          setValue(i);
      });
    }
    if (value == i) {
      current = count;
    }
    ++count;
  }

  if (current >= 0) {
    menu->select(current);
  }

  menu->setCloseHandler([=]() {
      editMode = false;
      setFocus();
  });
}

#if defined(HARDWARE_TOUCH)
bool Choice::onTouchEnd(coord_t, coord_t)
{
  openMenu();
  setFocus();
  return true;
}
#endif
