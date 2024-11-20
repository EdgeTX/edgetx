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
#include "setup_timer.h"
#include "setup_module.h"
#include "setup_function_switches.h"

namespace Ui {
  class Setup;
}

class AutoLineEdit;

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
    void throttleReverseChanged();

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
    void on_checklistInteractive_toggled(bool checked);
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
    void onTimerChanged();
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
