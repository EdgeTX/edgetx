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
#include "menu.h"
#include "static.h"
#include "button.h"
#include "numberedit.h"

#if defined(BLUETOOTH)
#include "bluetooth.h"
#endif

class TrChoice;

class TrainerModuleWindow : public FormGroup
{
 public:
  TrainerModuleWindow(FormWindow *parent, const rect_t &rect);

  void checkEvents() override;
  void update();

 protected:
  TrChoice *trainerChoice = nullptr;
  NumberEdit *channelStart = nullptr;
  NumberEdit *channelEnd = nullptr;
  bool trChoiceOpen = false;
#if defined(BLUETOOTH)
  StaticText *btChannelEnd = nullptr;
  StaticText *btDistAddress = nullptr;
  TextButton *btMasterButton = nullptr;
  Menu *btPopUpMenu = nullptr;
  bool btCanceled = false;

 private:
  bool popupopen = false;
  int devicecount = 0;
  uint8_t lastbluetoothstate = BLUETOOTH_STATE_OFF;

  void btDiscoverMenuItemChosen();
  void btDiscoverMenuAddItem(const char *itm);

#endif
};
