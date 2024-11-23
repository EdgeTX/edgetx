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

constexpr char FIM_ANTENNAMODE[]   {"Antenna Mode"};
constexpr char FIM_CRSFARMSWITCH[] {"CRSF Arming Switch"};

namespace Ui {
  class Module;
}

class ModulePanel : public ModelPanel
{
  Q_OBJECT

  public:
    ModulePanel(QWidget * parent, ModelData & model, ModuleData & module, GeneralSettings & generalSettings, Firmware * firmware, int moduleIdx,
                FilteredItemModelFactory * panelFilteredItemModels = nullptr, CompoundItemModelFactory * panelItemModels = nullptr);
    virtual ~ModulePanel();

    virtual void update();

  public slots:
    void onExtendedLimitsToggled();
    void onFailsafeModified(unsigned index);
    void updateTrainerModeItemModel();

  signals:
    void channelsRangeChanged();
    void failsafeModified(unsigned index);
    void updateItemModels();
    void protocolChanged();

  private slots:
    void setupFailsafes();
    void onProtocolChanged(int index);
    void on_ppmDelay_editingFinished();
    void on_channelsCount_editingFinished();
    void on_channelsStart_editingFinished();
    void on_ppmPolarity_currentIndexChanged(int index);
    void on_ppmOutputType_currentIndexChanged(int index);
    void on_ppmFrameLength_editingFinished();
    void on_rxNumber_editingFinished();
    void on_failsafeMode_currentIndexChanged(int value);
    void onMultiProtocolChanged(int index);
    void onSubTypeChanged();
    void on_autoBind_stateChanged(int state);
    void on_disableChMap_stateChanged(int state);
    void on_raw12bits_stateChanged(int state);
    void on_racingMode_stateChanged(int state);
    void on_disableTelem_stateChanged(int state);
    void on_lowPower_stateChanged(int state);
    void on_r9mPower_currentIndexChanged(int index);
    void setChannelFailsafeValue(const int channel, const int value, quint8 updtSb = 0);
    void onFailsafeComboIndexChanged(int index);
    void onFailsafeUsecChanged(int value);
    void onFailsafePercentChanged(double value);
    void onFailsafesDisplayValueTypeChanged(int type);
    void onRfFreqChanged(int freq);
    void updateFailsafe(unsigned channel);
    void on_optionValue_valueChanged(int value);
    void onClearAccessRxClicked();
    void on_chkOption_stateChanged(int state);
    void on_cboOption_currentIndexChanged(int value);

  private:
    enum FailsafeValueDisplayTypes { FAILSAFE_DISPLAY_PERCENT = 1, FAILSAFE_DISPLAY_USEC = 2 };

    struct ChannelFailsafeWidgetsGroup {
        QLabel * label;
        QComboBox * combo;
        QSpinBox * sbUsec;
        QDoubleSpinBox * sbPercent;
    };

    ModuleData & module;
    int moduleIdx;
    Ui::Module *ui;
    QMap<int, ChannelFailsafeWidgetsGroup> failsafeGroupsMap;
    static quint8 failsafesValueDisplayType;  // FailsafeValueDisplayTypes
    void updateFailsafeUI(unsigned channel, quint8 updtSb);
    FilteredItemModel *trainerModeItemModel;
    static bool isTrainerModule(int index) { return index < 0; }
    static bool isInternalModule(int index) { return index == 0; }
    static bool isExternalModule(int index) { return index > 0; }
};
