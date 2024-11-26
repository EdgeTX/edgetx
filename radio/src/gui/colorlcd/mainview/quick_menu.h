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

#include "form.h"
#include "bitmaps.h"
#include <vector>

class ButtonBase;

class QuickMenuGroup : public Window
{
 public:
  QuickMenuGroup(Window* parent, const rect_t &rect, bool createGroup);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "QuickMenuGroup"; }
#endif

  // Add a new button to the carousel
  ButtonBase* addButton(EdgeTxIcon icon, const char* title,
                 std::function<uint8_t(void)> pressHandler);

  void defocus();
  void setGroup();
  void setFocus();
  void setDisabled(bool all);
  void setEnabled();
  void setCurrent(ButtonBase* b) { curBtn = b; }

  static LAYOUT_VAL(FAB_BUTTON_WIDTH, 50, 50)
  static LAYOUT_VAL(FAB_BUTTON_HEIGHT, 74, 74)

  static LAYOUT_VAL(FAB_ICON_SIZE, 38, 38)
  static constexpr coord_t FAB_BUTTON_INNER_WIDTH = FAB_BUTTON_WIDTH;

 protected:
  std::vector<ButtonBase*> btns;
  ButtonBase* curBtn = nullptr;
  lv_group_t* group = nullptr;

  void deleteLater(bool detach = true, bool trash = true) override;
};
