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

//  This class could be merged into the base class however in the future there maybe a requirement
//  for another generic or specialised simulator ui so keep the separation

namespace Ui {
  class SimulatedUIWidgetGeneric;
}

class SimulatedUIWidgetGeneric: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetGeneric(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetGeneric();

  public slots:
    virtual void shrink();

  private:
    Ui::SimulatedUIWidgetGeneric * ui;
};
