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

#pragma once

#include "form.h"
#include "strhelpers.h"

class BaseNumberEdit: public FormField
{
  public:
   BaseNumberEdit(Window* parent, const rect_t& rect, int vmin, int vmax,
                  std::function<int()> getValue,
                  std::function<void(int)> setValue = nullptr,
                  WindowFlags windowFlags = 0, LcdFlags textFlags = 0,
                  LvglCreate objConstruct = nullptr) :
       FormField(parent, rect, windowFlags, textFlags, objConstruct),
       vmin(vmin),
       vmax(vmax),
       _getValue(std::move(getValue)),
       _setValue(std::move(setValue))
   {
   }

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
        if (instantChange && _setValue != nullptr) {
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
      setValue(currentValue);
    }

    void setSuffix(std::string value)
    {
      suffix = std::move(value);
      setValue(currentValue);
    }

    void setZeroText(std::string value)
    {
      zeroText = std::move(value);
      setValue(currentValue);
    }

    void enableInstantChange(bool value)
    {
      instantChange = value;
    }

    void disableInstantChange()
    {
      enableInstantChange(false);
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
      return (editMode && !instantChange) ? currentValue
             : _getValue != nullptr       ? _getValue()
                                          : 0;
    }

    void setEditMode(bool newEditMode) override
    {
      auto previousEditMode = editMode;
      FormField::setEditMode(newEditMode);
      if (!instantChange) {
        if (!previousEditMode && newEditMode) {
          if (_getValue != nullptr) currentValue = _getValue();
        }
        else if (previousEditMode && !newEditMode) {
          if (_setValue != nullptr) _setValue(currentValue);
        }
      }
    }

    void setDisplayHandler(std::function<std::string(int value)> function)
    {
      displayFunction = std::move(function);
      setValue(currentValue);
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
  
    virtual void paint(BitmapBuffer* dc) override {};

  protected:
    int vdefault = 0;
    int vmin;
    int vmax;
    int step = 1;
    int fastStep = 10;
    int accelFactor = 4;
    int currentValue;
    bool instantChange = true;
    std::string prefix;
    std::string suffix;
    std::string zeroText;
    std::function<int()> _getValue;
    std::function<void(int)> _setValue;
    std::function<std::string(int)> displayFunction;
};
