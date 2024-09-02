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

class ToggleSwitch : public FormField
{
 public:
  ToggleSwitch(Window* parent, const rect_t& rect,
           std::function<uint8_t()> getValue,
           std::function<void(uint8_t)> setValue);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ToggleSwitch"; }
#endif

  void onClicked() override;

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

  static LAYOUT_VAL(TOGGLE_W, 52, 52)

 protected:
  std::function<uint8_t()> _getValue;
  std::function<void(uint8_t)> _setValue;

  static void toggleswitch_event_handler(lv_event_t* e);
};
