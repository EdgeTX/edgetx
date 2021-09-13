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

Choice::Choice(FormGroup * parent, const rect_t & rect, int vmin, int vmax,
  std::function<int()> getValue, std::function<void(int)> setValue, WindowFlags windowFlags) :
  ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
  vmin(vmin),
  vmax(vmax),
  getValue(std::move(getValue)),
  setValue(std::move(setValue))
{
}

Choice::Choice(FormGroup * parent, const rect_t & rect, const char * const values[], int vmin, int vmax,
               std::function<int()> getValue, std::function<void(int)> setValue, WindowFlags windowFlags) :
  ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
  vmin(vmin),
  vmax(vmax),
  getValue(std::move(getValue)),
  setValue(std::move(setValue))
{
  setValues(values);
}

Choice::Choice(FormGroup * parent, const rect_t & rect, std::vector<std::string> values, int vmin, int vmax,
               std::function<int()> getValue, std::function<void(int)> setValue, WindowFlags windowFlags) :
  ChoiceBase(parent, rect, CHOICE_TYPE_DROPOWN, windowFlags),
  values(std::move(values)),
  vmin(vmin),
  vmax(vmax),
  getValue(std::move(getValue)),
  setValue(std::move(setValue))
{
}

Choice::Choice(FormGroup * parent, const rect_t & rect, const char * values, int vmin, int vmax,
               std::function<int()> getValue, std::function<void(int)> setValue, WindowFlags windowFlags) :
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

void Choice::addValue(const char * value)
{
  values.emplace_back(value);
  vmax += 1;
}

void Choice::addValues(const char * const values[], uint8_t count)
{
  this->values.reserve(this->values.size() + count);
  for (uint8_t i = 0; i < count; i++)
    this->values.emplace_back(values[i]);
  vmax += count;
}

void Choice::setValues(std::vector<std::string> values)
{
  this->values = std::move(values);
}

void Choice::setValues(const char * const values[])
{
  this->values.clear();
  if (values) {
    auto value = &values[0];
    for (int i = vmin; i <= vmax; i++) {
      this->values.emplace_back(*value++);
    }
  }
}

void Choice::paint(BitmapBuffer * dc)
{
  FormField::paint(dc);
  theme->drawChoice(dc, this,
                    textHandler ? textHandler(getValue()).c_str()
                                : values[getValue() - vmin].c_str());
}

#if defined(HARDWARE_KEYS)
void Choice::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    onKeyPress();
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
    else if (unsigned(i - vmin) < values.size()) {
      menu->addLine(values[i - vmin], [=]() {
        setValue(i);
      });
    }
    else {
      menu->addLine(std::to_string(i), [=]() {
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

  if (beforeDisplayMenuHandler) {
    beforeDisplayMenuHandler(menu);
  }


  menu->setCloseHandler([=]() {
    setEditMode(false);
  });

  setEditMode(true);
  invalidate();
}

#if defined(HARDWARE_TOUCH)
bool Choice::onTouchEnd(coord_t, coord_t)
{
  if (enabled) {
    if (!hasFocus()) {
      setFocus(SET_FOCUS_DEFAULT);
    }
    onKeyPress();
    openMenu();
  }
  return true;
}
#endif
