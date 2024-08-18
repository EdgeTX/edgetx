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

  int32_t getMax() const { return vmax; }

  void setMin(int value) { vmin = value; }
  void setMax(int value) { vmax = value; }
  void setDefault(int value) { vdefault = value; }
  void setStep(int value) { step = value; }
  void setFastStep(int value) { fastStep = value; }
  void setAccelFactor(int value) { accelFactor = value; }
  void setValue(int value);

  void setPrefix(std::string value)
  {
    prefix = std::move(value);
    update();
  }

  void setSuffix(std::string value)
  {
    suffix = std::move(value);
    update();
  }

  void setZeroText(std::string value)
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
    update();
  }

  void setSetValueHandler(std::function<void(int)> handler)
  {
    _setValue = std::move(handler);
  }

  void setGetValueHandler(std::function<int()> handler)
  {
    _getValue = std::move(handler);
  }

  int32_t getValue() const { return _getValue != nullptr ? _getValue() : 0; }

  static LAYOUT_VAL(DEF_W, 100, 100)

 protected:
  friend class NumberArea;

  NumberArea* edit = nullptr;
  std::function<int()> _getValue;
  std::function<void(int)> _setValue;
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

  std::string getDisplayVal();

  void updateDisplay();
  void openEdit();
};
