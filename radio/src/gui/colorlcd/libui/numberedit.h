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

#include "form.h"
#include "button.h"

class NumberArea;

class NumberEdit : public TextButton
{
 public:
  NumberEdit(Window* parent, const rect_t& rect, int vmin, int vmax,
                   std::function<int()> getValue,
                   std::function<void(int)> setValue = nullptr,
                   LcdFlags textFlags = 0);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override
  {
    return "NumberEdit";
  }
#endif

  virtual void update();

  int32_t getMin() const { return vmin; }
  int32_t getMax() const { return vmax; }
  bool hasDecimalPrecision() const;
  bool useDirectKeyboard() const;
  const std::string& getEditTitle() const { return editTitle; }

  void setMin(int value) { vmin = value; }
  void setMax(int value) { vmax = value; }
  void setDefault(int value) { vdefault = value; }
  void setStep(int value) { step = value; }
  void setFastStep(int value) { fastStep = value; }
  void setAccelFactor(int value) { accelFactor = value; }
  void setValue(int value);

  void setPrefix(const std::string& value)
  {
    prefix = std::move(value);
    update();
  }

  void setSuffix(const std::string& value)
  {
    suffix = std::move(value);
    update();
  }

  void setZeroText(const std::string& value)
  {
    zeroText = std::move(value);
    update();
  }

  void setAvailableHandler(std::function<bool(int)> handler)
  {
    isValueAvailable = std::move(handler);
  }

  void setDisplayHandler(std::function<std::string(int value)> function)
  {
    displayFunction = std::move(function);
    // Custom display strings are not always safe to parse as direct numeric input.
    directKeyboard = displayFunction == nullptr;
    update();
  }

  void setDirectKeyboard(bool value) { directKeyboard = value; }
  void setEditTitle(std::string value) { editTitle = std::move(value); }

  void setSetValueHandler(std::function<void(int)> handler)
  {
    _setValue = std::move(handler);
  }

  void setGetValueHandler(std::function<int()> handler)
  {
    _getValue = std::move(handler);
  }

  void setOnEditedHandler(std::function<void(int)> handler)
  {
    onEdited = std::move(handler);
  }

  int32_t getValue() const { return _getValue != nullptr ? _getValue() : 0; }
  std::string getDisplayVal() const;
  std::string getEditVal() const;
  void setValueFromEditVal(const char* text);

 protected:
  friend class NumberArea;

  NumberArea* edit = nullptr;
  std::function<int()> _getValue;
  std::function<void(int)> _setValue;
  std::function<void(int)> onEdited;
  int vdefault = 0;
  int vmin;
  int vmax;
  int step = 1;
  int fastStep = 10;
  int accelFactor = 4;
  int currentValue;
  std::string prefix;
  std::string suffix;
  std::string zeroText;
  std::function<std::string(int)> displayFunction;
  std::function<bool(int)> isValueAvailable;
  bool directKeyboard = true;
  std::string editTitle;

  void updateDisplay();
  void openEdit();

  void checkEvents() override;
};
