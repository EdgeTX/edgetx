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
#include <vector>

class QuickMenuGroup;
class PageGroup;
class ButtonBase;
class QuickSubMenu;

class QuickMenu : public Window
{
 public:
  enum SubMenu {
    NONE = 0,
    MODEL_FIRST,
    MODEL_SETUP = MODEL_FIRST,
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
    MODEL_LAST = MODEL_TELEMETRY,
    RADIO_FIRST,
    RADIO_TOOLSCRIPTS = RADIO_FIRST,
    RADIO_SD,
    RADIO_SETUP,
    RADIO_THEMES,
    RADIO_GF,
    RADIO_TRAINER,
    RADIO_HARDWARE,
    RADIO_VERSION,
    RADIO_LAST = RADIO_VERSION,
    CHANNELS_FIRST,
    CHANNELS_PG1 = CHANNELS_FIRST,
    CHANNELS_PG2,
    CHANNELS_PG3,
    CHANNELS_PG4,
    CHANNELS_LS,
    CHANNELS_LAST = CHANNELS_LS,
    SCREENS_FIRST,
    SCREENS_UI_SETUP = SCREENS_FIRST,
    SCREENS_PG1,
    SCREENS_PG2,
    SCREENS_PG3,
    SCREENS_PG4,
    SCREENS_PG5,
    SCREENS_PG6,
    SCREENS_PG7,
    SCREENS_PG8,
    SCREENS_PG9,
    SCREENS_PG10,
    SCREENS_ADD_PG,
    SCREENS_LAST = SCREENS_ADD_PG,
    STATS_FIRST,
    STATS_STATS = STATS_FIRST,
    STATS_DEBUG,
    STATS_LAST = STATS_DEBUG,
  };

  QuickMenu(Window* parent, std::function<void()> cancelHandler,
            std::function<void(bool close)> selectHandler = nullptr,
            PageGroup* pageGroup = nullptr, SubMenu curPage = NONE);

  void onCancel() override;
  void onSelect(bool close);
  void closeMenu();
  void deleteLater(bool detach = true, bool trash = true) override;
  void onEvent(event_t event) override;

  void setFocus(SubMenu selection);

  void enableSubMenu();

  SubMenu currentPage() const { return curPage; }

  static LAYOUT_VAL2(QM_COLS, 8, 5)
  static LAYOUT_VAL2(QM_ROWS, 3, 6)
  static LAYOUT_VAL2(QMMAIN_ROWS, 1, 2)

 protected:
  std::function<void()> cancelHandler = nullptr;
  std::function<void(bool close)> selectHandler = nullptr;
  bool inSubMenu = false;
  QuickMenuGroup* mainMenu = nullptr;
  std::vector<QuickSubMenu*> subMenus;
  Window* box = nullptr;
  PageGroup* pageGroup = nullptr;
  SubMenu curPage;

  void buildMainMenu();

  void onClicked() override;
};
