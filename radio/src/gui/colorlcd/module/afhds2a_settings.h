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
#include "module_setup.h"

class Choice;
struct ModuleData;

class AFHDS2ASettings : public Window, public ModuleOptions
{
  uint8_t moduleIdx;
  ModuleData* md;
  FlexGridLayout grid;

  Window* afhds2OptionsLabel = nullptr;
  Window* afhds2ProtoOpts = nullptr;
#if defined(PCBNV14)
  bool hasRFPower = false;
  Window* afhds2RFPowerText = nullptr;
  Choice* afhds2RFPowerChoice = nullptr;
#endif  
  void hideAFHDS2Options();
  void showAFHDS2Options();

  void checkEvents() override;
  void update() override;
    
public:
  AFHDS2ASettings(Window* parent, const FlexGridLayout& g, uint8_t moduleIdx);
};
