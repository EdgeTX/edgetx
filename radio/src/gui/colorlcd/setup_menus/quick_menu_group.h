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
  QuickMenuGroup(Window* parent);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "QuickMenuGroup"; }
#endif

  // Add a new button to the carousel
  ButtonBase* addButton(EdgeTxIcon icon, const char* title,
                 std::function<void(void)> pressHandler,
                 std::function<bool(void)> visibleHandler = nullptr,
                 std::function<void(void)> focusHandler = nullptr);

  void setGroup();
  void setFocus();
  void clearFocus();
  void setDisabled(bool all);
  void setEnabled();
  void setCurrent(ButtonBase* b);
  void setCurrent(int b) { setCurrent(btns[b]); }
  void doLayout(int cols);
  void nextEntry();
  void prevEntry();
  ButtonBase* getFocusedButton();

#if PORTRAIT
  static LAYOUT_VAL_SCALED(QM_BUTTON_WIDTH, 72)
#else
  static LAYOUT_SIZE_SCALED(QM_BUTTON_WIDTH, 72, 60)
#endif
  static LAYOUT_VAL_SCALED(QM_BUTTON_HEIGHT, 70)

  static LAYOUT_VAL_SCALED(QM_ICON_SIZE, 30)
  static LAYOUT_ORIENTATION(QM_ICON_PAD, PAD_MEDIUM, PAD_SMALL)

 protected:
  std::vector<ButtonBase*> btns;
  ButtonBase* curBtn = nullptr;
  lv_group_t* group = nullptr;

  void deleteLater(bool detach = true, bool trash = true) override;
};
