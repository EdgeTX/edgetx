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

constexpr char MIMETYPE_TELE_SENSOR[] {"application/x-companion-tele-sensor"};

class AutoComboBox;
class TimerEdit;

namespace Ui {
  class TelemetrySensor;
  class Telemetry;
}

class TelemetrySensorPanel: public ModelPanel
{
    Q_OBJECT

  public:
    TelemetrySensorPanel(QWidget *parent, SensorData & sensor, int sensorIndex, int sensorCapability, ModelData & model,
                         GeneralSettings & generalSettings, Firmware * firmware, const bool & parentLock,
                         FilteredItemModelFactory * panelFilteredItemModels);
    ~TelemetrySensorPanel();
    void update();

  signals:
    void dataModified();
    void clearAllSensors();
    void insertSensor(int index);
    void deleteSensor(int index);
    void moveUpSensor(int index);
    void moveDownSensor(int index);

  protected slots:
    void on_customContextMenuRequested(QPoint pos);
    bool hasClipboardData(QByteArray * data = nullptr) const;
    void cmCopy();
    void cmCut();
    void cmPaste();
    void cmClear(bool prompt = true);
    void cmClearAll();
    void cmInsert();
    void cmDelete();
    void cmMoveUp();
    void cmMoveDown();
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();
    void on_nameDataChanged();
    void on_formulaDataChanged();
    void on_unitDataChanged();
    void on_precDataChanged();

  private:
    Ui::TelemetrySensor * ui;
    SensorData & sensor;
    int sensorIndex = 0;
    int selectedIndex = 0;
    int sensorCapability;
    bool insertAllowed() const;
    bool moveDownAllowed() const;
    bool moveUpAllowed() const;
    int modelsUpdateCnt;
    const bool &parentLock;

    inline bool isLocked() { return lock | parentLock; }
    void connectItemModelEvents(const FilteredItemModel * itemModel);
};

class TelemetryPanel : public ModelPanel
{
    Q_OBJECT

  public:
    TelemetryPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware,
                   CompoundItemModelFactory * sharedItemModels);
    virtual ~TelemetryPanel();
    virtual void update();

  signals:
    void updated();

  private slots:
    void onModified();
    void on_frskyProtoCB_currentIndexChanged(int index);
    void on_bladesCount_editingFinished();
    void on_rssiAlarmWarningSB_editingFinished();
    void on_rssiAlarmCriticalSB_editingFinished();
    void on_varioLimitMin_DSB_editingFinished();
    void on_varioLimitMax_DSB_editingFinished();
    void on_varioLimitCenterMin_DSB_editingFinished();
    void on_varioLimitCenterMax_DSB_editingFinished();
    void on_fasOffset_DSB_editingFinished();
    void on_mahCount_SB_editingFinished();
    void on_mahCount_ChkB_toggled(bool checked);
    void on_clearAllSensors();
    void on_insertSensor(int index);
    void on_deleteSensor(int index);
    void on_moveUpSensor(int index);
    void on_moveDownSensor(int index);
    void on_dataModifiedSensor();
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();

  private:
    Ui::Telemetry *ui;
    TelemetrySensorPanel *sensorPanels[CPN_MAX_SENSORS];
    int sensorCapability;
    CompoundItemModelFactory *sharedItemModels;
    CompoundItemModelFactory *panelItemModels;
    FilteredItemModelFactory *panelFilteredItemModels;
    int modelsUpdateCnt;

    void setup();
    void telBarUpdate();
    void swapData(int idx1, int idx2);
    void connectItemModelEvents(const int id);
    inline bool isLocked() { return lock; }
};
