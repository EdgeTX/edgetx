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

Choice::Choice(FormGroup * parent, const rect_t & rect, int16_t vmin, int16_t vmax,
  std::function<int16_t()> getValue, std::function<void(int16_t)> setValue, WindowFlags windowFlags) :
  ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
  vmin(vmin),
  vmax(vmax),
  getValue(std::move(getValue)),
  setValue(std::move(setValue))
{
}

Choice::Choice(FormGroup * parent, const rect_t & rect, const char * values[], int16_t vmin, int16_t vmax,
               std::function<int16_t()> getValue, std::function<void(int16_t)> setValue, WindowFlags windowFlags) :
  ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
  vmin(vmin),
  vmax(vmax),
  getValue(std::move(getValue)),
  setValue(std::move(setValue))
{
  if (values) {
    const char ** value = &values[0];
    for (int i = vmin; i <= vmax; i++) {
      this->values.emplace_back(*value++);
    }
  }
}

Choice::Choice(FormGroup * parent, const rect_t & rect, std::vector<std::string> values, int16_t vmin, int16_t vmax,
               std::function<int16_t()> getValue, std::function<void(int16_t)> setValue, WindowFlags windowFlags) :
  ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
  values(std::move(values)),
  vmin(vmin),
  vmax(vmax),
  getValue(std::move(getValue)),
  setValue(std::move(setValue))
{
}

Choice::Choice(FormGroup * parent, const rect_t & rect, const char * values, int16_t vmin, int16_t vmax,
               std::function<int16_t()> getValue, std::function<void(int16_t)> setValue, WindowFlags windowFlags) :
  ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
  vmin(vmin),
  vmax(vmax),
  getValue(std::move(getValue)),
  setValue(std::move(setValue))
{
  if (values) {
    uint8_t len = values[0];
    const char * value = &values[1];
    for (int i = vmin; i <= vmax; i++) {
      this->values.emplace_back(std::string(value, min<uint8_t>(len, strlen(value))));
      value += len;
    }
  }
}

void Choice::addItem(const char * item)
{
  values.emplace_back(item);
  vmax += 1;
}

void Choice::addItems(const char * items[], uint8_t count)
{
  values.reserve(values.size() + count);
  for (uint8_t i = 0; i < count; i++)
    values.emplace_back(items[i]);
  vmax += count;
}

void Choice::paint(BitmapBuffer * dc)
{
  FormField::paint(dc);
  theme->drawChoice(dc, this, textHandler ? textHandler(getValue()).c_str() : values[getValue() - vmin].c_str());
}

#if defined(HARDWARE_KEYS)
void Choice::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    openMenu();
  }
  else {
    FormField::onEvent(event);
  }
}
#endif

void Choice::openMenu()
{
  auto menu = new Menu(this);
  if (!menuTitle.empty())
    menu->setTitle(menuTitle);

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
      menu->addLine(values[i - vmin], [=]() {
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

  setEditMode(true);
  invalidate();
}

#if defined(HARDWARE_TOUCH)
bool Choice::onTouchEnd(coord_t, coord_t)
{
  onKeyPress();
  openMenu();
  return true;
}
#endif
