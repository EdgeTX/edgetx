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

class ToggleSwitch;

struct ModuleData;
struct MPMProtoOption;
struct MPMSubtype;
struct MPMDSMCloned;
struct MPMServoRate;
struct MPMAutobind;
struct MPMChannelMap;

class MultimoduleSettings : public Window, public ModuleOptions
{
  ModuleData* md;
  uint8_t moduleIdx;

  MPMSubtype* st_line;
  MPMProtoOption* opt_line;
  MPMDSMCloned* cl_line;
  MPMServoRate* sr_line;
  MPMAutobind* ab_line;
  ToggleSwitch* lp_mode;
#if defined(MANUFACTURER_FRSKY)
  ToggleSwitch* disable_telem;
#endif
  MPMChannelMap* cm_line;

 public:
  MultimoduleSettings(Window* parent, const FlexGridLayout& g,
                      uint8_t moduleIdx);
  void update() override;
};
