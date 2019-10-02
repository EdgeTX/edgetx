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

#ifndef _BASENUMBEREDIT_H_
#define _BASENUMBEREDIT_H_

#include "form.h"

class BaseNumberEdit : public FormField {
  public:
    BaseNumberEdit(Window * parent, const rect_t &rect, int32_t vmin, int32_t vmax,
                   std::function<int32_t()> getValue, std::function<void(int32_t)> setValue = nullptr, LcdFlags flags = 0):
      FormField(parent, rect),
      vmin(vmin),
      vmax(vmax),
      _getValue(std::move(getValue)),
      _setValue(std::move(setValue)),
      flags(flags)
    {
    }

    void setMin(int32_t value)
    {
      vmin = value;
    }

    void setMax(int32_t value)
    {
      vmax = value;
    }

    void setDefault(int32_t value)
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

    void setStep(int32_t value)
    {
      step = value;
    }

    int32_t getStep() const
    {
      return step;
    }

    void setValue(int32_t value)
    {
      _setValue(limit(vmin, value, vmax));
      invalidate();
    }

    void setSetValueHandler(std::function<void(int32_t)> handler)
    {
      _setValue = handler;
    }

    int32_t getValue()
    {
      return _getValue();
    }

  protected:
    int32_t vdefault = 0;
    int32_t vmin;
    int32_t vmax;
    int32_t step = 1;
    std::function<int32_t()> _getValue;
    std::function<void(int32_t)> _setValue;
    LcdFlags flags;
};

#endif // _BASENUMBEREDIT_H_
