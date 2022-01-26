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

#include "basenumberedit.h"
#include <string>

class NumberEdit : public BaseNumberEdit
{
  public:
    NumberEdit(Window * parent, const rect_t & rect, int vmin, int vmax, std::function<int()> getValue, std::function<void(int)> setValue = nullptr, WindowFlags windowFlags = 0, LcdFlags textFlags = 0);

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

    void setPrefix(std::string value)
    {
      prefix = std::move(value);
    }

    void setSuffix(std::string value)
    {
      suffix = std::move(value);
    }

    void setZeroText(std::string value)
    {
      zeroText = std::move(value);
    }

    void setDisplayHandler(std::function<void(BitmapBuffer *, LcdFlags, int)> function)
    {
      displayFunction = std::move(function);
    }

    void onEvent(event_t event) override;
    void onFocusLost() override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

  protected:
    std::function<void(BitmapBuffer *, LcdFlags, int)> displayFunction;
    std::string prefix;
    std::string suffix;
    std::string zeroText;
    std::function<bool(int)> isValueAvailable;
};

