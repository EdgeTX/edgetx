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

class QTableView;
class CompoundItemModelFactory;
class FilteredItemModel;
class LogicalSwitchesTableModel;

constexpr char MIMETYPE_LOGICAL_SWITCH[] = "application/x-companion-logical-switch";

class LogicalSwitchesPanel : public ModelPanel
{
    Q_OBJECT

  public:
    LogicalSwitchesPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings,
                         Firmware * firmware, CompoundItemModelFactory * sharedItemModels);
    virtual ~LogicalSwitchesPanel();

    virtual void update();

  private slots:
    void onFunctionFamilyChanged(int row);
    void onV1SourceTypeChanged(int row);
    void onCustomContextMenuRequested(QPoint pos);
    void onLogicalSwitchStateChanged();
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();

    void cmCopy();
    void cmPaste();
    void cmCut();
    void cmDelete();
    void cmInsert();
    void cmMoveUp();
    void cmMoveDown();
    void cmClear(bool prompt = true);
    void cmClearAll();

  private:
    QTableView * m_tableView;
    LogicalSwitchesTableModel * m_tableModel;
    CompoundItemModelFactory * sharedItemModels;
    FilteredItemModel * rawSwitchFilteredModel;
    FilteredItemModel * rawSourceFilteredModel;
    int selectedIndex;
    int modelsUpdateCnt;

    void openPersistentEditorsForRow(int row);
    void recreateRowEditors(int row);
    void openAllPersistentEditors();
    bool hasClipboardData(QByteArray * data = nullptr) const;
    bool insertAllowed() const;
    bool moveDownAllowed() const;
    bool moveUpAllowed() const;
    void connectItemModelEvents(const FilteredItemModel * itemModel);
};
