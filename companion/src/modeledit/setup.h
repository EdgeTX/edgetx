/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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
#include "compounditemmodels.h"
#include "filtereditemmodels.h"

constexpr char MIMETYPE_TIMER[] = "application/x-companion-timer";

namespace Ui {
  class Setup;
  class Timer;
  class Module;
  class FunctionSwitches;
}

class AutoLineEdit;

class TimerPanel : public ModelPanel
{
    Q_OBJECT

  public:
    TimerPanel(QWidget * parent, ModelData & model, TimerData & timer, GeneralSettings & generalSettings, Firmware * firmware,
               QWidget * prevFocus, FilteredItemModelFactory * panelFilteredModels, CompoundItemModelFactory * panelItemModels);
    virtual ~TimerPanel();

    virtual void update();
    QWidget * getLastFocus();

  private slots:
    void onNameChanged();
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();
    void onCountdownBeepChanged(int index);
    void onModeChanged(int index);

  signals:
    void nameChanged();

  private:
    TimerData & timer;
    Ui::Timer * ui;
    void connectItemModelEvents(const FilteredItemModel * itemModel);
    int modelsUpdateCnt;
};

class ModulePanel : public ModelPanel
{
  Q_OBJECT

  public:
    ModulePanel(QWidget * parent, ModelData & model, ModuleData & module, GeneralSettings & generalSettings, Firmware * firmware, int moduleIdx,
                FilteredItemModelFactory * panelFilteredItemModels = nullptr);
    virtual ~ModulePanel();
    virtual void update();

  public slots:
    void onExtendedLimitsToggled();
    void onFailsafeModified(unsigned index);

  signals:
    void channelsRangeChanged();
    void failsafeModified(unsigned index);
    void updateItemModels();

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
    void on_optionValue_editingFinished();
    void onClearAccessRxClicked();

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
};

class FunctionSwitchesPanel : public ModelPanel
{
    Q_OBJECT

  public:
    FunctionSwitchesPanel(QWidget * parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware);
    virtual ~FunctionSwitchesPanel();

    virtual void update();
    void update(int index);

  signals:
    void updateDataModels();

  private slots:
    void on_nameEditingFinished();
    void on_configCurrentIndexChanged(int index);
    void on_startPosnCurrentIndexChanged(int index);
    void on_groupChanged(int value);
    void on_alwaysOnGroupChanged(int value);

  private:
    Ui::FunctionSwitches * ui;
    QVector<AutoLineEdit *> aleNames;
    QVector<QComboBox *> cboConfigs;
    QVector<QComboBox *> cboStartupPosns;
    QVector<QSpinBox *> sbGroups;
    QVector<QCheckBox *> cbAlwaysOnGroups;
    int switchcnt;
};

class SetupPanel : public ModelPanel
{
    Q_OBJECT

  public:
    SetupPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware, CompoundItemModelFactory * sharedItemModels);
    virtual ~SetupPanel();

    virtual void update();

  signals:
    void extendedLimitsToggled();
    void updated();

  private slots:
    void on_name_editingFinished();
    void on_throttleTrimSwitch_currentIndexChanged(int index);
    void on_throttleTrim_toggled(bool checked);
    void on_extendedLimits_toggled(bool checked);
    void on_extendedTrims_toggled(bool checked);
    void on_throttleWarning_toggled(bool checked);
    void on_enableCustomThrottleWarning_toggled(bool checked);
    void on_customThrottleWarningPosition_valueChanged(int value);
    void on_throttleReverse_toggled(bool checked);
    void on_displayText_toggled(bool checked);
    void on_gfEnabled_toggled(bool checked);
    void on_image_currentIndexChanged(int index);
    void on_trimIncrement_currentIndexChanged(int index);
    void onBeepCenterToggled(bool checked);
    void startupSwitchEdited(int value);
    void startupSwitchToggled(bool checked);
    void potWarningToggled(bool checked);
    void on_potWarningMode_currentIndexChanged(int index);
    void on_editText_clicked();
    void onTimerCustomContextMenuRequested(QPoint pos);
    void cmTimerClear(bool prompt = true);
    void cmTimerClearAll();
    void cmTimerCopy();
    void cmTimerCut();
    void cmTimerDelete();
    void cmTimerInsert();
    void cmTimerPaste();
    void cmTimerMoveDown();
    void cmTimerMoveUp();
    void onTimerNameChanged();
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();
    void onModuleUpdateItemModels();
    void onFunctionSwitchesUpdateItemModels();
    void on_jitterFilter_currentIndexChanged(int index);

  private:
    Ui::Setup *ui;
    QVector<QSlider *> startupSwitchesSliders;
    QVector<QCheckBox *> startupSwitchesCheckboxes;
    QVector<QCheckBox *> potWarningCheckboxes;
    QVector<QCheckBox *> centerBeepCheckboxes;
    ModulePanel * modules[CPN_MAX_MODULES + 1];
    TimerPanel * timers[CPN_MAX_TIMERS];
    FunctionSwitchesPanel * funcswitches;

    void updateStartupSwitches();
    void updatePotWarnings();
    void updateBeepCenter();
    void populateThrottleTrimSwitchCB();
    int timersCount;
    int selectedTimerIndex;
    bool hasTimerClipboardData(QByteArray * data = nullptr) const;
    bool insertTimerAllowed() const;
    bool moveTimerDownAllowed() const;
    bool moveTimerUpAllowed() const;
    void swapTimerData(int idx1, int idx2);
    CompoundItemModelFactory * sharedItemModels;
    void updateItemModels();
    void connectItemModelEvents(const FilteredItemModel * itemModel);
    CompoundItemModelFactory * panelItemModels;
    FilteredItemModelFactory * panelFilteredModels;
};
