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

#include <utility>
#include "form.h"

class ToggleSwitch : public FormField
{
 public:
  ToggleSwitch(Window* parent, const rect_t& rect,
           std::function<uint8_t()> getValue,
           std::function<void(uint8_t)> setValue, WindowFlags flags = 0);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ToggleSwitch"; }
#endif

  void onClicked() override;

  const char* getLabel() const { return label.c_str(); }
  void setLabel(std::string newLabel) { label = std::move(newLabel); }

  uint8_t getValue() const { return _getValue(); }
  void setValue(uint8_t value) { _setValue(value); }

  void setSetValueHandler(std::function<void(uint8_t)> handler)
  {
    _setValue = std::move(handler);
  }

  void setGetValueHandler(std::function<uint8_t()> handler)
  {
    _getValue = std::move(handler);
  }

  void update() const;
  
 protected:
  std::string label;
  std::function<uint8_t()> _getValue;
  std::function<void(uint8_t)> _setValue;
};
