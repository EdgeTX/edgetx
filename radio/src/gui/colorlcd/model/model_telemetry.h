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

#include "tabsgroup.h"
#include "edgetx.h"

class ModelTelemetryPage : public PageTab
{
 public:
  ModelTelemetryPage();

  bool isVisible() const override { return modelTelemetryEnabled(); }

  void build(Window* window) override;

  static LAYOUT_VAL(NUM_EDIT_W, 100, 65)

 protected:
  int lastKnownIndex = 0;
  Window* window = nullptr;
  Window* sensorWindow = nullptr;
  TextButton* discover = nullptr;
  TextButton* deleteAll = nullptr;

  void checkEvents() override;

  void editSensor(Window* window, uint8_t index);
  void rebuild(Window* window, int8_t focusSensorIndex = -1);
  void buildSensorList(int8_t focusSensorIndex = -1);
};
