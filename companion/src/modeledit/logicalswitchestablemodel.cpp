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

#include "logicalswitchestablemodel.h"
#include "eeprominterface.h"

LogicalSwitchesTableModel::LogicalSwitchesTableModel(
    ModelData * model, GeneralSettings & generalSettings, Firmware * firmware,
    FilteredItemModel * rawSwitchModel, FilteredItemModel * rawSourceModel,
    QObject * parent)
  : QAbstractTableModel(parent),
    m_model(model),
    m_generalSettings(generalSettings),
    m_firmware(firmware),
    m_rawSwitchModel(rawSwitchModel),
    m_rawSourceModel(rawSourceModel)
{
  int cap = firmware->getCapability(LogicalSwitches);
  m_rowCount = qMin(cap, (int)CPN_MAX_LOGICAL_SWITCHES);
}

int LogicalSwitchesTableModel::rowCount(const QModelIndex & parent) const
{
  return parent.isValid() ? 0 : m_rowCount;
}

int LogicalSwitchesTableModel::columnCount(const QModelIndex & parent) const
{
  return parent.isValid() ? 0 : COL_COUNT;
}

QVariant LogicalSwitchesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    switch (section) {
      case COL_INDEX:      return tr("#");
      case COL_FUNCTION:   return tr("Function");
      case COL_V1:         return tr("V1");
      case COL_V2:         return tr("V2");
      case COL_AND_SWITCH: return tr("AND Switch");
      case COL_DURATION:   return tr("Duration");
      case COL_DELAY:      return tr("Delay");
      case COL_PERSISTENT: return tr("Persistent");
      default:             return QVariant();
    }
  }
  return QVariant();
}

Qt::ItemFlags LogicalSwitchesTableModel::flags(const QModelIndex & index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;

  Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  if (index.column() != COL_INDEX)
    f |= Qt::ItemIsEditable;
  if (index.column() == COL_PERSISTENT)
    f |= Qt::ItemIsUserCheckable;
  return f;
}

unsigned int LogicalSwitchesTableModel::computeVisibilityMask(int row) const
{
  const LogicalSwitchData & lsw = m_model->logicalSw[row];
  unsigned int mask = 0;

  if (!lsw.isEmpty()) {
    mask = LINE_ENABLED | DELAY_ENABLED | DURATION_ENABLED;

    switch (lsw.getFunctionFamily()) {
      case LS_FAMILY_VOFS:
        mask |= SOURCE1_VISIBLE;
        if (RawSource(lsw.val1).type == SOURCE_TYPE_TIMER)
          mask |= VALUE_TO_VISIBLE;
        else
          mask |= VALUE2_VISIBLE;
        break;

      case LS_FAMILY_STICKY:
        mask |= PERSIST_ENABLED;
        // fall through
      case LS_FAMILY_VBOOL:
        mask |= SOURCE1_VISIBLE | SOURCE2_VISIBLE;
        break;

      case LS_FAMILY_EDGE:
        mask |= SOURCE1_VISIBLE | VALUE2_VISIBLE | VALUE3_VISIBLE;
        mask &= ~DELAY_ENABLED;
        break;

      case LS_FAMILY_VCOMP:
        mask |= SOURCE1_VISIBLE | SOURCE2_VISIBLE;
        break;

      case LS_FAMILY_TIMER:
        mask |= VALUE1_VISIBLE | VALUE2_VISIBLE;
        break;
    }
  }
  return mask;
}

QVariant LogicalSwitchesTableModel::data(const QModelIndex & index, int role) const
{
  if (!index.isValid() || index.row() >= m_rowCount)
    return QVariant();

  const int row = index.row();
  const int col = index.column();
  const LogicalSwitchData & lsw = m_model->logicalSw[row];

  if (role == VisibilityMaskRole)
    return computeVisibilityMask(row);

  if (role == FunctionFamilyRole)
    return lsw.isEmpty() ? -1 : (int)lsw.getFunctionFamily();

  if (role == SourceTypeRole && col == COL_V1)
    return RawSource(lsw.val1).type;

  switch (col) {
    case COL_INDEX:
      if (role == Qt::DisplayRole)
        return lsw.nameToString(row);
      break;

    case COL_FUNCTION:
      if (role == Qt::EditRole)
        return (int)lsw.func;
      if (role == Qt::DisplayRole)
        return lsw.funcToString();
      break;

    case COL_V1:
      if (role == Qt::EditRole)
        return lsw.val1;
      break;

    case COL_V2:
      if (role == Qt::EditRole) {
        QVariantMap m;
        m["val2"] = lsw.val2;
        m["val3"] = lsw.val3;
        return m;
      }
      break;

    case COL_AND_SWITCH:
      if (role == Qt::EditRole)
        return lsw.andsw;
      break;

    case COL_DURATION:
      if (role == Qt::EditRole || role == Qt::DisplayRole)
        return lsw.duration / 10.0;
      break;

    case COL_DELAY:
      if (role == Qt::EditRole || role == Qt::DisplayRole)
        return lsw.delay / 10.0;
      break;

    case COL_PERSISTENT:
      if (role == Qt::CheckStateRole)
        return lsw.lsPersist ? Qt::Checked : Qt::Unchecked;
      if (role == Qt::EditRole)
        return lsw.lsPersist;
      break;
  }

  return QVariant();
}

bool LogicalSwitchesTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
  if (!index.isValid() || index.row() >= m_rowCount)
    return false;

  const int row = index.row();
  const int col = index.column();
  LogicalSwitchData & lsw = m_model->logicalSw[row];

  switch (col) {
    case COL_FUNCTION: {
      unsigned int newFunc = value.toUInt();
      if (lsw.func == newFunc)
        return false;

      unsigned int oldFunc = lsw.func;
      CSFunctionFamily oldFamily = lsw.getFunctionFamily();
      lsw.func = newFunc;
      CSFunctionFamily newFamily = lsw.getFunctionFamily();

      if (oldFamily != newFamily || newFunc == LS_FN_OFF) {
        lsw.clear();
        lsw.func = newFunc;
        if (newFamily == LS_FAMILY_TIMER) {
          lsw.val1 = -119;
          lsw.val2 = -119;
        }
        else if (newFamily == LS_FAMILY_EDGE) {
          lsw.val2 = -129;
        }
      }

      emit dataChanged(this->index(row, 0), this->index(row, COL_COUNT - 1));

      if (oldFamily != newFamily || newFunc == LS_FN_OFF)
        emit functionFamilyChanged(row);
      if (oldFunc == LS_FN_OFF || newFunc == LS_FN_OFF)
        emit logicalSwitchStateChanged();

      emit modified();
      return true;
    }

    case COL_V1: {
      int newVal = value.toInt();
      if (lsw.val1 == newVal)
        return false;

      int oldSourceType = RawSource(lsw.val1).type;
      lsw.val1 = newVal;
      int newSourceType = RawSource(lsw.val1).type;

      emit dataChanged(index, index);

      if (lsw.getFunctionFamily() == LS_FAMILY_VOFS && oldSourceType != newSourceType)
        emit v1SourceTypeChanged(row);

      emit modified();
      return true;
    }

    case COL_V2: {
      if (value.type() == QVariant::Map) {
        QVariantMap m = value.toMap();
        lsw.val2 = m["val2"].toInt();
        lsw.val3 = m["val3"].toInt();
      }
      else {
        lsw.val2 = value.toInt();
      }
      emit dataChanged(index, index);
      emit modified();
      return true;
    }

    case COL_AND_SWITCH:
      lsw.andsw = value.toInt();
      emit dataChanged(index, index);
      emit modified();
      return true;

    case COL_DURATION:
      lsw.duration = (uint8_t)qRound(value.toDouble() * 10);
      emit dataChanged(index, index);
      emit modified();
      return true;

    case COL_DELAY:
      lsw.delay = (uint8_t)qRound(value.toDouble() * 10);
      emit dataChanged(index, index);
      emit modified();
      return true;

    case COL_PERSISTENT:
      if (role == Qt::CheckStateRole)
        lsw.lsPersist = (value.toInt() == Qt::Checked);
      else
        lsw.lsPersist = value.toBool();
      emit dataChanged(index, index);
      emit modified();
      return true;
  }
  return false;
}

void LogicalSwitchesTableModel::refreshAllRows()
{
  if (m_rowCount > 0)
    emit dataChanged(index(0, 0), index(m_rowCount - 1, COL_COUNT - 1));
}

void LogicalSwitchesTableModel::clearRow(int row)
{
  if (row < 0 || row >= m_rowCount) return;
  m_model->logicalSw[row].clear();
  m_model->updateAllReferences(ModelData::REF_UPD_TYPE_LOGICAL_SWITCH, ModelData::REF_UPD_ACT_CLEAR, row);
  emit dataChanged(index(row, 0), index(row, COL_COUNT - 1));
  emit functionFamilyChanged(row);
  emit logicalSwitchStateChanged();
  emit modified();
}

void LogicalSwitchesTableModel::lsInsert(int row)
{
  if (row < 0 || row >= m_rowCount) return;
  memmove(&m_model->logicalSw[row + 1], &m_model->logicalSw[row],
          (m_rowCount - (row + 1)) * sizeof(LogicalSwitchData));
  m_model->logicalSw[row].clear();
  m_model->updateAllReferences(ModelData::REF_UPD_TYPE_LOGICAL_SWITCH, ModelData::REF_UPD_ACT_SHIFT, row, 0, 1);
  refreshAllRows();
  emit logicalSwitchStateChanged();
  emit modified();
}

void LogicalSwitchesTableModel::lsDelete(int row)
{
  if (row < 0 || row >= m_rowCount) return;
  memmove(&m_model->logicalSw[row], &m_model->logicalSw[row + 1],
          (m_rowCount - (row + 1)) * sizeof(LogicalSwitchData));
  m_model->logicalSw[m_rowCount - 1].clear();
  m_model->updateAllReferences(ModelData::REF_UPD_TYPE_LOGICAL_SWITCH, ModelData::REF_UPD_ACT_SHIFT, row, 0, -1);
  refreshAllRows();
  emit logicalSwitchStateChanged();
  emit modified();
}

void LogicalSwitchesTableModel::swapRows(int row1, int row2)
{
  if (row1 < 0 || row1 >= m_rowCount || row2 < 0 || row2 >= m_rowCount) return;
  if (m_model->logicalSw[row1].isEmpty() && m_model->logicalSw[row2].isEmpty()) return;

  LogicalSwitchData tmp = m_model->logicalSw[row2];
  memcpy(&m_model->logicalSw[row2], &m_model->logicalSw[row1], sizeof(LogicalSwitchData));
  memcpy(&m_model->logicalSw[row1], &tmp, sizeof(LogicalSwitchData));
  m_model->updateAllReferences(ModelData::REF_UPD_TYPE_LOGICAL_SWITCH, ModelData::REF_UPD_ACT_SWAP, row1, row2);

  emit dataChanged(index(row1, 0), index(row1, COL_COUNT - 1));
  emit dataChanged(index(row2, 0), index(row2, COL_COUNT - 1));
  emit functionFamilyChanged(row1);
  emit functionFamilyChanged(row2);
  emit logicalSwitchStateChanged();
  emit modified();
}

void LogicalSwitchesTableModel::clearAllRows()
{
  for (int i = 0; i < m_rowCount; i++) {
    m_model->logicalSw[i].clear();
    m_model->updateAllReferences(ModelData::REF_UPD_TYPE_LOGICAL_SWITCH, ModelData::REF_UPD_ACT_CLEAR, i);
  }
  refreshAllRows();
  emit logicalSwitchStateChanged();
  emit modified();
}

void LogicalSwitchesTableModel::pasteRow(int row, const void * data, int size)
{
  if (row < 0 || row >= m_rowCount) return;
  if (size != sizeof(LogicalSwitchData)) return;
  memcpy(&m_model->logicalSw[row], data, sizeof(LogicalSwitchData));
  emit dataChanged(index(row, 0), index(row, COL_COUNT - 1));
  emit functionFamilyChanged(row);
  emit logicalSwitchStateChanged();
  emit modified();
}
