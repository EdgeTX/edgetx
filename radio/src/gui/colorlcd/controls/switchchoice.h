/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
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

#include "choice.h"
#include "form.h"

class SwitchChoiceMenuToolbar;

class SwitchChoice : public Choice
{
 public:
  SwitchChoice(Window* parent, const rect_t& rect, int vmin, int vmax,
               std::function<int16_t()> getValue,
               std::function<void(int16_t)> setValue);

 protected:
  friend SwitchChoiceMenuToolbar;

  bool inMenu = false;

  void setValue(int value) override;
  int getIntValue() const override;
  bool onLongPress() override;

  void invertChoice();

  void openMenu() override;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "SwitchChoice"; }
#endif
};
