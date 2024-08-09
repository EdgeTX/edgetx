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

#ifndef _TELEMETRYPROVIDER_H_
#define _TELEMETRYPROVIDER_H_

#include "simulatorinterface.h"

class TelemetryProvider
{
  public:
    virtual ~TelemetryProvider() {}

    virtual void resetRssi() = 0;

    // Simulator has been updated externally, update the UI to match
    virtual void loadUiFromSimulator(SimulatorInterface * simulator) = 0;

    // do the work every however often
    virtual void generateTelemetryFrame(SimulatorInterface * simulator) = 0;
};

#endif
