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

class BaseNumberEdit: public FormField
{
  public:
    BaseNumberEdit(Window * parent, const rect_t &rect, int vmin, int vmax,
                   std::function<int()> getValue, std::function<void(int)> setValue = nullptr, WindowFlags windowFlags = 0, LcdFlags textFlags = 0, LvglWidgetFactory *factory = nullptr):
      FormField(parent, rect, windowFlags, textFlags, factory),
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

    void setValue(int value)
    {
      currentValue = limit(vmin, value, vmax);
      if (instantChange) {
        _setValue(currentValue);
      }
      invalidate();
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

    int32_t getValue() const
    {
      return (editMode && !instantChange) ? currentValue : _getValue();
    }

    void setEditMode(bool newEditMode) override
    {
      auto previousEditMode = editMode;
      FormField::setEditMode(newEditMode);
      if (!instantChange) {
        if (!previousEditMode && newEditMode) {
          currentValue = _getValue();
        }
        else if (previousEditMode && !newEditMode) {
          _setValue(currentValue);
        }
      }
    }

  protected:
    int vdefault = 0;
    int vmin;
    int vmax;
    int step = 1;
    int currentValue;
    bool instantChange = true;
    std::function<int()> _getValue;
    std::function<void(int)> _setValue;
};
