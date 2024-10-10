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
#include "choice.h"
#include "module_setup.h"

struct ModuleData;

class AFHDS3Settings : public Window, public ModuleOptions
{
  uint8_t moduleIdx;
  ModuleData* md;
  FlexGridLayout grid;
  tmr10ms_t lastRefresh = 0;

  Window* afhds3StatusLabel = nullptr;
  Window* afhds3StatusText = nullptr;
  Window* afhds3TypeLabel = nullptr;
  Window* afhds3TypeForm = nullptr;
  Choice *afhds3PhyMode = nullptr;
  Choice *afhds3Emi = nullptr;
  Choice *afhds3RfPower = nullptr;
  void hideAFHDS3Options();
  void showAFHDS3Options();

  void checkEvents() override;
  void update() override;
    
public:
  AFHDS3Settings(Window* parent, const FlexGridLayout& g, uint8_t moduleIdx);
};
