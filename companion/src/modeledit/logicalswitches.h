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

#ifndef _LOGICALSWITCHES_H_
#define _LOGICALSWITCHES_H_

#include "modeledit.h"
#include "radiodata.h"

class CompoundItemModelFactory;
class FilteredItemModel;
class TimerEdit;

constexpr char MIMETYPE_LOGICAL_SWITCH[] = "application/x-companion-logical-switch";

class LogicalSwitchesPanel : public ModelPanel
{
    Q_OBJECT

  public:
    LogicalSwitchesPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware, CompoundItemModelFactory * sharedItemModels);
    virtual ~LogicalSwitchesPanel();

    virtual void update();

  private slots:
    void onFunctionChanged();
    void onV1Changed(int value);
    void onV2Changed(int value);
    void onAndSwitchChanged(int value);
    void onDurationChanged(double duration);
    void onDelayChanged(double delay);
    void onPersistChanged();
    void onOffsetChanged();
    bool offsetChangedAt(int index);
    void updateLine(int index);
    void onCustomContextMenuRequested(QPoint pos);
    void cmDelete();
    void cmCopy();
    void cmPaste();
    void cmCut();
    void cmMoveUp();
    void cmMoveDown();
    void cmInsert();
    void cmClear(bool prompt = true);
    void cmClearAll();
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();

  private:
    QComboBox * cbFunction[CPN_MAX_LOGICAL_SWITCHES];
    QDoubleSpinBox * dsbValue[CPN_MAX_LOGICAL_SWITCHES];
    QDoubleSpinBox * dsbOffset[CPN_MAX_LOGICAL_SWITCHES];
    QDoubleSpinBox * dsbOffset2[CPN_MAX_LOGICAL_SWITCHES];
    TimerEdit * teOffset[CPN_MAX_LOGICAL_SWITCHES];
    QComboBox * cbAndSwitch[CPN_MAX_LOGICAL_SWITCHES];
    QDoubleSpinBox * dsbDuration[CPN_MAX_LOGICAL_SWITCHES];
    QDoubleSpinBox * dsbDelay[CPN_MAX_LOGICAL_SWITCHES];
    QComboBox * cbSource1[CPN_MAX_LOGICAL_SWITCHES];
    QComboBox * cbSource2[CPN_MAX_LOGICAL_SWITCHES];
    QCheckBox * cbPersist[CPN_MAX_LOGICAL_SWITCHES];
    CompoundItemModelFactory * sharedItemModels;
    FilteredItemModel * rawSwitchFilteredModel;
    FilteredItemModel * rawSourceFilteredModel;
    int selectedIndex;
    void populateFunctionCB(QComboBox *b);
    void updateTimerParam(QDoubleSpinBox *sb, int timer, double minimum=0, double maximum=175.0);
    int lsCapability;
    int lsCapabilityExt;
    void swapData(int idx1, int idx2);
    bool hasClipboardData(QByteArray * data = nullptr) const;
    bool insertAllowed() const;
    bool moveDownAllowed() const;
    bool moveUpAllowed() const;
    int modelsUpdateCnt;
    void updateItemModels();
    void connectItemModelEvents(const FilteredItemModel * itemModel);
};

#endif // _LOGICALSWITCHES_H_
