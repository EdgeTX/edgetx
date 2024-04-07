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

#include "repodatamodels.h"
#include "constants.h"

/*
    RepoRawItemModel
*/

RepoRawItemModel::RepoRawItemModel(QString name) :
  QStandardItemModel(),
  RepoItemModelBase(name)
{
  invalidate();
}

void RepoRawItemModel::invalidate()
{
  clear();
  setRefreshRequired(true);
}

void RepoRawItemModel::parseAll(QJsonDocument * json)
{
  invalidate();

  if (json->isArray()) {
    const QJsonArray &array = json->array();
    foreach (const QJsonValue &v, array)
    {
      if (v.isObject()) {
        const QJsonObject &obj = v.toObject();
        parseJsonObject(obj);
      }
    }
    setRefreshRequired(false);
  }
}

void RepoRawItemModel::parseMetaData(const int mdt, QJsonDocument * json)
{
  switch (mdt) {
    case MDT_Build_Asset:
    case MDT_Build_Assets:
    case MDT_Build_Release:
    case MDT_Build_Releases:
    case MDT_GitHub_Asset:
      parseOne(json);
      break;
    case MDT_GitHub_Assets:
    case MDT_GitHub_Releases:
      parseAll(json);
      break;
    default:
      qDebug() << "Unexpected meta data type" << mdt;
      break;
  }
}

void RepoRawItemModel::parseOne(QJsonDocument * json)
{
  invalidate();

  if (json->isObject()) {
    const QJsonObject &obj = json->object();
    parseJsonObject(obj);
    setRefreshRequired(false);
  }
}

bool RepoRawItemModel::setValue(const int id, const int role, const QVariant value)
{
  //qDebug() << "id:" << id << "role:" << role << "value:" << value;
  const QModelIndexList idxlst = match(index(0, 0), RIMR_Id, QVariant(id), 1, Qt::MatchExactly);

  if (!idxlst.isEmpty())
    return setData(idxlst.at(0), value, role);

  return false;
}

//  static
QString RepoRawItemModel::typeToString(const int val)
{
  const QStringList strl = { "Asset", "Assets", "Release", "Releases" };
  return strl.value(val, CPN_STR_UNKNOWN_ITEM);
}

void RepoRawItemModel::update()
{
  for (int i = 0; i < rowCount(); ++i)
    setDynamicItemData(item(i));
}

/*
    RepoFilteredItemModel
*/

RepoFilteredItemModel::RepoFilteredItemModel(QString name) :
  QSortFilterProxyModel(),
  RepoItemModelBase(name)
{
  m_filterFlags = 0;
  m_filterText.setPattern("");
  m_filterText.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
  setFilterRole(RIMR_Available);
  setFilterKeyColumn(0);
  setDynamicSortFilter(true);
}

const QString RepoFilteredItemModel::date(const int id) const
{
  return value(id, RIMR_Date).toString();
}

bool RepoFilteredItemModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
  const QModelIndex & srcIdx = sourceModel()->index(sourceRow, 0, sourceParent);

  if (!srcIdx.isValid() || !sourceModel()->data(srcIdx, filterRole()).toBool())
    return false;

  if (!m_filterFlags && m_filterText.pattern().isEmpty())
    return true;

  if (m_filterFlags) {
    bool ok;
    const int flags = sourceModel()->data(srcIdx, RIMR_Flags).toInt(&ok);
    return (ok && (m_filterFlags & flags));
  }
  else {
    return sourceModel()->data(srcIdx, Qt::DisplayRole).toString().contains(m_filterText);
  }
}

const int RepoFilteredItemModel::getFlags(const int id) const
{
  return value(id, RIMR_Flags).toInt();
}

const int RepoFilteredItemModel::id(const int row) const
{
  const QModelIndex & idx = index(row, 0);

  if (!idx.isValid())
    return 0;

  bool ok;
  const int id = data(idx, RIMR_Id).toInt(&ok);

  if (ok)
    return id;

  return 0;
}

const int RepoFilteredItemModel::id(const QVariant & value, const Qt::MatchFlags flags, const int role) const
{
  const QModelIndexList idxlst = match(index(0, 0), role, value, 1, flags);

  if (!idxlst.isEmpty()) {
    bool ok;
    const int id = data(idxlst.at(0), RIMR_Id).toInt(&ok);

    if (ok)
      return id;
  }

  return 0;
}

const QStringList RepoFilteredItemModel::list() const
{
  QStringList strl;

  for (int i = 0; i < rowCount(); i++)
    strl << data(index(i, 0)).toString();

  return strl;
}

const QString RepoFilteredItemModel::name(const int id) const
{
  return value(id, Qt::DisplayRole).toString();
}

void RepoFilteredItemModel::setFilterFlags(const int flags)
{
  if (m_filterFlags != flags) {
    m_filterFlags = flags;
    m_filterText.setPattern("");
    invalidateFilter();
  }
}

void RepoFilteredItemModel::setFilterPattern(const QString & pattern)
{
  if (m_filterText.pattern() != pattern) {
    m_filterText.setPattern(pattern);
    m_filterFlags = 0;
    invalidateFilter();
  }
}

bool RepoFilteredItemModel::setFlags(const int id, const int flags)
{
  return setValue(id, RIMR_Flags, QVariant(flags));
}

bool RepoFilteredItemModel::setValue(const int id, const int role, const QVariant value)
{
  //qDebug() << "id:" << id << "role:" << role << "value:" << value;
  const QModelIndexList idxlst = match(index(0, 0), RIMR_Id, QVariant(id), 1, Qt::MatchExactly);

  if (!idxlst.isEmpty())
    return setData(idxlst.at(0), value, role);

  return false;
}

const QVariant RepoFilteredItemModel::value(const int id, const int role) const
{
  //qDebug() << "id:" << id << "role:";
  const QModelIndexList idxlst = match(index(0, 0), RIMR_Id, QVariant(id), 1, Qt::MatchExactly);

  if (!idxlst.isEmpty())
    return data(idxlst.at(0), role);

  return QVariant();
}

const QMap<int, QVariant> RepoFilteredItemModel::valuesMap(const int id) const
{
  const QModelIndexList idxlst = match(index(0, 0), RIMR_Id, QVariant(id), 1, Qt::MatchExactly);

  if (!idxlst.isEmpty())
    return itemData(idxlst.at(0));

  return QMap<int, QVariant>();
}
