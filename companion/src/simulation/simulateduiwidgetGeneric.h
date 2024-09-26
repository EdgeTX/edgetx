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

#include "simulateduiwidget.h"
#include "ui_simulateduiwidgetGeneric.h"

/*
    Note: This class is not expected to be instantiated directly but as the base for each firmware that uses it
          due to the methods and functions surrounding the building and loading of radio simulators
*/

SimulatedUIWidgetGeneric::SimulatedUIWidgetGeneric(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetGeneric)
{
  ui->setupUi(this);

  addGenericPushButtons(ui->leftbuttons, ui->rightbuttons);

  addScrollActions();

  setLcd(ui->lcd);
}

SimulatedUIWidgetGeneric::~SimulatedUIWidgetGeneric()
{
  delete ui;
}
