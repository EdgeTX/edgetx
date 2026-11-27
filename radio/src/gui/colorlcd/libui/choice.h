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
#include <climits>

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

  void setTextHandler(std::function<std::string(int)> handler);

  int getMin() const { return vmin; }
  void setMin(int value) { vmin = value; }

  int getMax() const { return vmax; }
  void setMax(int value) { vmax = value; }

  const char* getTitle() const { return menuTitle; }

  void setPopupWidth(coord_t w) { popupWidth = w; }

  static LAYOUT_VAL_SCALED(ICON_W, 18)

 protected:
  lv_obj_t *label;
  coord_t popupWidth = 0;
  int vmin = 0;
  int vmax = 0;
  const char *menuTitle = nullptr;
  ChoiceType type;
  int currentValue = INT_MAX;
  std::function<int()> _getValue;
  std::function<void(int)> _setValue;
  std::function<std::string(int)> textHandler;

  virtual std::string getLabelText() = 0;

  void checkEvents() override;
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

  void setFillMenuHandler(std::function<void(Menu *, int, int &)> handler);
  void setSetValueHandler(std::function<void(int)> handler);
  void setGetValueHandler(std::function<int()> handler);
  void setAvailableHandler(std::function<bool(int)> handler);

  unsigned getIndexFromValue(int value) const;
  int getValueFromIndex(int index) const;

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
