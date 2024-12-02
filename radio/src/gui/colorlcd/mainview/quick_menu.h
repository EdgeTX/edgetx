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

#include "window.h"

class QuickMenuGroup;
class ButtonBase;

class QuickMenu : public Window
{
 public:
  enum SubMenu {
    NONE = 0,
    MODEL_SETUP,
    MODEL_HELI,
    MODEL_FLIGHTMODES,
    MODEL_INPUTS,
    MODEL_MIXES,
    MODEL_OUTPUTS,
    MODEL_CURVES,
    MODEL_GVARS,
    MODEL_LS,
    MODEL_SF,
    MODEL_SCRIPTS,
    MODEL_TELEMETRY,
    RADIO_TOOLSCRIPTS,
    RADIO_SD,
    RADIO_SETUP,
    RADIO_THEMES,
    RADIO_GF,
    RADIO_TRAINER,
    RADIO_HARDWARE,
    RADIO_VERSION,
  };

  QuickMenu(Window* parent, std::function<void()> cancelHandler,
            std::function<void()> selectHandler = nullptr);

  void onCancel() override;
  void onSelect();
  void closeMenu();
  void deleteLater(bool detach = true, bool trash = true) override;

  void setFocus(SubMenu selection);

  static LAYOUT_VAL(QM_COLS, 8, 5)
  static LAYOUT_VAL(QM_ROWS, 3, 5)
  static LAYOUT_VAL(QMMAIN_ROWS, 1, 2)

 protected:
  std::function<void()> cancelHandler = nullptr;
  std::function<void()> selectHandler = nullptr;
  bool inSubMenu = false;
  QuickMenuGroup* mainMenu = nullptr;
  QuickMenuGroup* modelSubMenu = nullptr;
  QuickMenuGroup* radioSubMenu = nullptr;
  ButtonBase* modelBtn = nullptr;
  ButtonBase* radioBtn = nullptr;
  coord_t w, h;
  Window* box = nullptr;

  void buildMainMenu();
  void buildModelMenu();
  void buildRadioMenu();
};
