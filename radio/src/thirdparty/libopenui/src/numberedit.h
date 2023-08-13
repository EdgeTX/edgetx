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
#include "strhelpers.h"
#include <string>

class NumberEdit: public FormField
{
  public:
    NumberEdit(Window* parent, const rect_t& rect, int vmin, int vmax,
               std::function<int()> getValue,
               std::function<void(int)> setValue = nullptr,
               WindowFlags windowFlags = 0, LcdFlags textFlags = 0);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "NumberEdit(" + std::to_string(getValue()) + ")";
    }
#endif

    void setAvailableHandler(std::function<bool(int)> handler)
    {
      isValueAvailable = std::move(handler);
    }

    void onEvent(event_t event) override;
    void onClicked() override;

    void setMin(int value)
    {
      vmin = value;
    }

    void setMax(int value)
    {
      vmax = value;
    }

    void setDefault(int value)
    {
      vdefault = value;
    }

    int32_t getMin() const
    {
      return vmin;
    }

    int32_t getMax() const
    {
      return vmax;
    }

    int32_t getDefault() const
    {
      return vdefault;
    }

    void setStep(int value)
    {
      step = value;
    }

    int32_t getStep() const
    {
      return step;
    }

    void setFastStep(int value)
    {
      fastStep = value;
    }

    int32_t getFastStep() const
    {
      return fastStep;
    }

    void setAccelFactor(int value)
    {
      accelFactor = value;
    }

    int32_t getAccelFactor() const
    {
      return accelFactor;
    }

    void setValue(int value)
    {
      auto newValue = limit(vmin, value, vmax);
      if (newValue != currentValue) {
        currentValue = newValue;
        if (_setValue != nullptr) {
          _setValue(currentValue);
        }
      }
      if (lvobj != nullptr) {
        std::string str;
        if (displayFunction != nullptr) {
          str = displayFunction(currentValue);
        } else {
          str = formatNumberAsString(currentValue, textFlags, 0, prefix.c_str(),
                                     suffix.c_str());
        }
        lv_textarea_set_text(lvobj, str.c_str());
      }
    }

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

    void setSetValueHandler(std::function<void(int)> handler)
    {
      _setValue = std::move(handler);
    }

    void setGetValueHandler(std::function<int()> handler)
    {
      _getValue = std::move(handler);
    }

    int32_t getValue() const
    {
      return _getValue != nullptr ? _getValue() : 0;
    }

    void setDisplayHandler(std::function<std::string(int value)> function)
    {
      displayFunction = std::move(function);
      update();
    }

    virtual void update()
    {
      if (_getValue == nullptr) return;
      auto newValue = _getValue();
      if (newValue != currentValue) {
        currentValue = newValue;
      }
      if (lvobj != nullptr) {
        std::string str;
        if (displayFunction != nullptr) {
          str = displayFunction(currentValue);
        } else {
          str = formatNumberAsString(currentValue, textFlags, 0, prefix.c_str(),
                                     suffix.c_str());
        }
        lv_textarea_set_text(lvobj, str.c_str());
      }
    }

  protected:
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
    std::function<int()> _getValue;
    std::function<void(int)> _setValue;
    std::function<std::string(int)> displayFunction;
    std::function<bool(int)> isValueAvailable;
};
