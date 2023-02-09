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

#include "pxx2.h"
#include "popups.h"

class OtaUpdateInformation: public BindInformation {
  public:
    char filename[FF_MAX_LFN + 1];
    uint32_t address;
    uint32_t module;
};

class Pxx2OtaUpdate {
  public:
    Pxx2OtaUpdate(uint8_t module, const char * rxName):
      module(module),
      rxName(rxName)
    {
    }

    void flashFirmware(const char * filename, ProgressHandler progressHandler);

  protected:
    uint8_t module;
    const char * rxName;

    const char * doFlashFirmware(const char * filename, ProgressHandler progressHandler);
    bool waitStep(uint8_t step, uint8_t timeout);
    const char* nextStep(uint8_t step, const char* rxName, uint32_t address,
                         const uint8_t* buffer);
};

