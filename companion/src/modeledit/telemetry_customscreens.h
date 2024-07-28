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

class AutoComboBox;
class TimerEdit;

namespace Ui {
  class TelemetryCustomScreen;
}

class CustomScreen: public ModelPanel
{
    Q_OBJECT

  public:
    CustomScreen(QWidget *parent, ModelData & model, FrSkyScreenData & screen, GeneralSettings & generalSettings, Firmware * firmware,
                          const bool & parentLock, FilteredItemModelFactory * panelFilteredItemModels);
    ~CustomScreen();
    void update();

  private slots:
    void on_screenType_currentIndexChanged(int index);
    void scriptNameEdited();
    void customFieldChanged(int index);
    void barSourceChanged(int index);
    void barMinChanged(double value);
    void barMaxChanged(double value);
    void barTimeChanged();
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();

  private:
    void updateBar(int line);
    Ui::TelemetryCustomScreen * ui;
    FrSkyScreenData & screen;
    QComboBox * fieldsCB[4][3];
    QComboBox * barsCB[4];
    QDoubleSpinBox * minSB[4];
    QDoubleSpinBox * maxSB[4];
    TimerEdit * minTime[4];
    TimerEdit * maxTime[4];
    int modelsUpdateCnt;
    const bool &parentLock;

    inline bool isLocked() { return lock | parentLock; }
    void connectItemModelEvents(const FilteredItemModel * itemModel);
};

class TelemetryCustomScreensPanel : public ModelPanel
{
    Q_OBJECT

  public:
    TelemetryCustomScreensPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware,
                   CompoundItemModelFactory * sharedItemModels);
    virtual ~TelemetryCustomScreensPanel();
    virtual void update();

  signals:
    void updated();

  private slots:
    void onModified();
    void on_dataModifiedSensor();
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();

  private:
    CompoundItemModelFactory *sharedItemModels;
    FilteredItemModelFactory *panelFilteredItemModels;
    int modelsUpdateCnt;

    QTabWidget *tabWidget;
    QGridLayout *grid;
    QVector<GenericPanel *> panels;

    QString getTabName(int index);

    void setup();
    void telBarUpdate();
    void connectItemModelEvents(const int id);
    inline bool isLocked() { return lock; }
};
