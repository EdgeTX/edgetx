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

#ifndef _MODEL_TELEMETRY_H
#define _MODEL_TELEMETRY_H

#include "tabsgroup.h"

class ModelTelemetryPage: public PageTab {
  public:
    ModelTelemetryPage();

    void build(FormWindow * window) override
    {
      build(window, -1);
    }

    void checkEvents() override;

  protected:
    FormWindow * window = nullptr;
    void editSensor(FormWindow * window, uint8_t index);
    int lastKnownIndex = 0;
    void build(FormWindow * window, int8_t focusSensorIndex=-1);
    void rebuild(FormWindow * window, int8_t focusSensorIndex=-1);
};

#endif //_MODEL_TELEMETRY_H