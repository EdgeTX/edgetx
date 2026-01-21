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

#include "modeledit.h"
#include "eeprominterface.h"
#include "filtereditemmodels.h"

constexpr char FIM_TIMERSWITCH[] {"Timer Switch"};
constexpr char MIMETYPE_TIMER[]  {"application/x-companion-timer"};

namespace Ui {
  class Timer;
}

class TimerPanel : public ModelPanel
{
    Q_OBJECT

  public:
    TimerPanel(QWidget * parent, ModelData & model, TimerData & timer, GeneralSettings & generalSettings, Firmware * firmware,
               QWidget * prevFocus, FilteredItemModelFactory * panelFilteredModels, CompoundItemModelFactory * panelItemModels);
    virtual ~TimerPanel();

    virtual void update();
    QWidget * getLastFocus();

  public slots:
    void onNameChanged();
    void onModeChanged(int index);

  private slots:
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();
    void onCountdownBeepChanged(int index);

  signals:
    void nameChanged();
    void modeChanged();

  private:
    TimerData & timer;
    Ui::Timer * ui;
    void connectItemModelEvents(const FilteredItemModel * itemModel);
    int modelsUpdateCnt;
};
