/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#pragma once

#include <vector>

#include "form.h"

class Menu;

enum ChoiceType {
  CHOICE_TYPE_DROPOWN,
  CHOICE_TYPE_FOLDER,
};

class ChoiceBase : public FormField
{
 public:
  ChoiceBase(Window *parent, const rect_t &rect, int vmin, int vmax, const char* title,
             std::function<int()> _getValue, std::function<void(int)> _setValue,
             ChoiceType type = CHOICE_TYPE_DROPOWN);

  void update();

  void setTextHandler(std::function<std::string(int)> handler)
  {
    textHandler = std::move(handler);
    update();
  }

  int getMin() const { return vmin; }
  void setMin(int value) { vmin = value; }

  int getMax() const { return vmax; }
  void setMax(int value) { vmax = value; }

  const char* getTitle() const { return menuTitle; }

  static LAYOUT_VAL(ICON_W, 18, 18)

 protected:
  lv_obj_t *label;
  int vmin = 0;
  int vmax = 0;
  const char *menuTitle = nullptr;
  ChoiceType type;
  std::function<int()> _getValue;
  std::function<void(int)> _setValue;
  std::function<std::string(int)> textHandler;

  virtual std::string getLabelText() = 0;
};

class Choice : public ChoiceBase
{
 public:
  Choice(Window *parent, const rect_t &rect, int vmin, int vmax,
         std::function<int()> _getValue,
         std::function<void(int)> _setValue = nullptr,
         const char *title = nullptr, ChoiceType type = CHOICE_TYPE_DROPOWN);
  Choice(Window *parent, const rect_t &rect, std::vector<std::string> values,
         int vmin, int vmax, std::function<int()> _getValue,
         std::function<void(int)> _setValue = nullptr,
         const char *title = nullptr);
  Choice(Window *parent, const rect_t &rect, const char *const values[],
         int vmin, int vmax, std::function<int()> _getValue,
         std::function<void(int)> _setValue = nullptr,
         const char *title = nullptr);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "Choice"; }
#endif

  void addValue(const char *value);

  void setValues(std::vector<std::string> values);
  void setValues(const char *const values[]);

  void onClicked() override;

  void setFillMenuHandler(std::function<void(Menu *, int, int &)> handler)
  {
    fillMenuHandler = std::move(handler);
  }

  void setSetValueHandler(std::function<void(int)> handler)
  {
    _setValue = std::move(handler);
  }

  void setGetValueHandler(std::function<int()> handler)
  {
    _getValue = std::move(handler);
  }

  void setAvailableHandler(std::function<bool(int)> handler)
  {
    isValueAvailable = std::move(handler);
  }

  unsigned getIndexFromValue(int value) const
  {
    if (!isValueAvailable) {
      return value - vmin;
    }

    unsigned index = 0;
    for (int i = vmin; i < value; i++) {
      if (isValueAvailable(i)) {
        index++;
      }
    }
    return index;
  }

  int getValueFromIndex(int index) const
  {
    if (!isValueAvailable) {
      return vmin + index;
    }

    int value = vmin - 1;
    while (index >= 0) {
      index--;
      value++;
      while (value < vmax && !isValueAvailable(value)) {
        value++;
      }
    }
    return value;
  }

  virtual void setValue(int val);
  virtual int getIntValue() const { return _getValue(); }

  std::string getString(int val) { return values[val]; }

  typedef std::function<bool(int16_t)> FilterFct;

  void fillMenu(Menu *menu, const FilterFct &filter = nullptr);

 protected:
  friend class MenuToolbar;

  bool inverted = false;
  int selectedIx0 = 0;

  std::vector<std::string> values;
  std::function<bool(int)> isValueAvailable;
  std::function<void(Menu *, int, int &)> fillMenuHandler;

  std::string getLabelText() override;

  virtual void openMenu();
};
