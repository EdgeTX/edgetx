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

class QMPageChoiceMenuToolbar;

class QMPageChoice : public Choice
{
 public:
  QMPageChoice(Window* parent, const rect_t& rect,
               std::vector<std::string> values,
               int16_t vmin, int16_t vmax,
               std::function<int16_t()> getValue,
               std::function<void(int16_t)> setValue,
               const char *title);

  static LAYOUT_ORIENTATION(TABLE_WIDTH, LCD_W * 5 / 8, LCD_W / 2 + PAD_LARGE * 3)

 protected:
  friend QMPageChoiceMenuToolbar;

  void openMenu() override;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "QMPageChoice"; }
#endif
};
