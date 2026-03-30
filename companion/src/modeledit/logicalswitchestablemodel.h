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

#include <QAbstractTableModel>

class ModelData;
class GeneralSettings;
class Firmware;
class FilteredItemModel;

class LogicalSwitchesTableModel : public QAbstractTableModel
{
  Q_OBJECT

  public:
    enum Column {
      COL_INDEX = 0,
      COL_FUNCTION,
      COL_V1,
      COL_V2,
      COL_AND_SWITCH,
      COL_DURATION,
      COL_DELAY,
      COL_PERSISTENT,
      COL_COUNT
    };

    enum Role {
      FunctionFamilyRole = Qt::UserRole + 1,
      VisibilityMaskRole,
      SourceTypeRole
    };

    // Visibility mask bits (same semantics as the old #defines in logicalswitches.cpp)
    enum VisibilityMask {
      SOURCE1_VISIBLE  = 0x1,
      SOURCE2_VISIBLE  = 0x2,
      VALUE1_VISIBLE   = 0x4,
      VALUE2_VISIBLE   = 0x8,
      VALUE3_VISIBLE   = 0x10,
      VALUE_TO_VISIBLE = 0x20,
      DELAY_ENABLED    = 0x40,
      DURATION_ENABLED = 0x80,
      LINE_ENABLED     = 0x100,
      PERSIST_ENABLED  = 0x200
    };

    LogicalSwitchesTableModel(ModelData * model, GeneralSettings & generalSettings,
                              Firmware * firmware,
                              FilteredItemModel * rawSwitchModel,
                              FilteredItemModel * rawSourceModel,
                              QObject * parent = nullptr);

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Clipboard helpers
    void clearRow(int row);
    void lsInsert(int row);
    void lsDelete(int row);
    void swapRows(int row1, int row2);
    void clearAllRows();
    void pasteRow(int row, const void * data, int size);

    // Delegate access
    FilteredItemModel * rawSwitchModel() const { return m_rawSwitchModel; }
    FilteredItemModel * rawSourceModel() const { return m_rawSourceModel; }
    ModelData * modelData() const { return m_model; }
    GeneralSettings & generalSettingsRef() const { return m_generalSettings; }

    void refreshAllRows();

    unsigned int computeVisibilityMask(int row) const;

  signals:
    void functionFamilyChanged(int row);
    void v1SourceTypeChanged(int row);
    void logicalSwitchStateChanged();
    void modified();

  private:
    ModelData * m_model;
    GeneralSettings & m_generalSettings;
    Firmware * m_firmware;
    FilteredItemModel * m_rawSwitchModel;
    FilteredItemModel * m_rawSourceModel;
    int m_rowCount;
};
