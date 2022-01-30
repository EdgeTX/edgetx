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

#include <utility>
#include "form.h"

class CheckBox : public FormField {
  public:
    CheckBox(Window * parent, const rect_t & rect, std::function<uint8_t()> getValue, std::function<void(uint8_t)> setValue, WindowFlags flags = 0);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "CheckBox";
    }
#endif

    const char * getLabel() const
    {
      return label.c_str();
    }

    void setLabel(std::string newLabel)
    {
      label = std::move(newLabel);
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

    uint8_t getValue() const
    {
      return _getValue();
    }

    void setValue(uint8_t value)
    {
      _setValue(value);
    }

    void setSetValueHandler(std::function<void(uint8_t)> handler)
    {
      _setValue = std::move(handler);
    }

  protected:
    std::string label;
    std::function<uint8_t()> _getValue;
    std::function<void(uint8_t)> _setValue;
};
