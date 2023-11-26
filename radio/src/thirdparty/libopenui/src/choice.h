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

typedef void (*lvHandler_t)(void *);

typedef struct {
  void *userData;
  bool isLongPressed;
  lvHandler_t lv_LongPressHandler;
} lv_eventData_t;

class Menu;

constexpr int CHOICE_LABEL_MARGIN_RIGHT = 10;

enum ChoiceType {
  CHOICE_TYPE_DROPOWN,
  CHOICE_TYPE_FOLDER,
};

class ChoiceBase : public FormField
{
 public:
  ChoiceBase(Window *parent, const rect_t &rect,
             ChoiceType type = CHOICE_TYPE_DROPOWN,
             WindowFlags windowFlags = 0);

 protected:
  ChoiceType type;
  lv_obj_t *label;

  friend void choice_changed_cb(lv_event_t *e);
  virtual std::string getLabelText() { return ""; };
};

class Choice : public ChoiceBase
{
 public:
  Choice(Window *parent, const rect_t &rect, int vmin, int vmax,
         std::function<int()> _getValue,
         std::function<void(int)> _setValue = nullptr,
         WindowFlags windowFlags = 0);
  Choice(Window *parent, const rect_t &rect, std::vector<std::string> values,
         int vmin, int vmax, std::function<int()> _getValue,
         std::function<void(int)> _setValue = nullptr,
         WindowFlags windowFlags = 0);
  Choice(Window *parent, const rect_t &rect, const char *const values[],
         int vmin, int vmax, std::function<int()> _getValue,
         std::function<void(int)> _setValue = nullptr,
         WindowFlags windowFlags = 0);
  Choice(Window *parent, const rect_t &rect, const char *values, int vmin,
         int vmax, std::function<int()> _getValue,
         std::function<void(int)> _setValue = nullptr,
         WindowFlags windowFlags = 0);

  void addValue(const char *value);

  void addValues(const char *const values[], uint8_t count);

  void setValues(std::vector<std::string> values);

  void setValues(const char *const values[]);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "Choice"; }
#endif

  void onClicked() override;

  void setFillMenuHandler(std::function<void(Menu*, int, int&)> handler)
  {
    fillMenuHandler = std::move(handler);
  }

  void setBeforeDisplayMenuHandler(std::function<void(Menu *)> handler)
  {
    beforeDisplayMenuHandler = std::move(handler);
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

  unsigned getValuesCount() const { return getIndexFromValue(vmax + 1); }

  void setTextHandler(std::function<std::string(int)> handler)
  {
    textHandler = std::move(handler);
    lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
  }

  void setMenuTitle(std::string value) { menuTitle = std::move(value); }
  std::string getMenuTitle() const { return menuTitle; }

  void setMin(int value)
  {
    vmin = value;
    invalidate();
  }

  void setMax(int value)
  {
    vmax = value;
    invalidate();
  }

  int getMin() const { return vmin; }
  int getMax() const { return vmax; }

  void set_lv_LongPressHandler(lvHandler_t longPressHandler, void *data);

  int selectedIx0 = 0;

 protected:
  friend class MenuToolbar;

  bool inverted = false;

  std::string getLabelText() override;
  std::vector<std::string> values;
  int vmin = 0;
  int vmax = 0;
  std::string menuTitle;
  std::function<int()> _getValue;
  std::function<void(int)> _setValue;
  std::function<bool(int)> isValueAvailable;
  std::function<std::string(int)> textHandler;
  std::function<void(Menu *, int, int&)> fillMenuHandler;
  std::function<void(Menu *)> beforeDisplayMenuHandler;

  typedef std::function<bool(int16_t)> FilterFct;
  void fillMenu(Menu *menu, const FilterFct &filter = nullptr);

  virtual void openMenu();

  lv_eventData_t longPressData;
  std::function<void(event_t)> longPressHandler = nullptr;

  static void ClickHandler(lv_event_t *e)
  {
    auto ch = (Choice *)lv_event_get_user_data(e);
    if (ch) ch->longPressData.isLongPressed = false;
  }
};
