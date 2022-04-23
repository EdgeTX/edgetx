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
#include "choice.h"
#include "button.h"
#include "gridlayout.h"

class ModuleWindow : public FormGroup
{
 public:
  ModuleWindow(FormWindow *parent, const rect_t &rect, uint8_t moduleIdx);

 protected:
  uint8_t moduleIdx;
  bool hasFailsafe = false;

  Choice *moduleChoice = nullptr;
  Choice *rfChoice = nullptr;
  TextButton *bindButton = nullptr;
  TextButton *rangeButton = nullptr;
  TextButton *registerButton = nullptr;
  Choice *failSafeChoice = nullptr;

  void addChannelRange(FormGridLayout &grid);
  void startRSSIDialog(std::function<void()> closeHandler = nullptr);

  void update();
  void checkEvents() override;
};
