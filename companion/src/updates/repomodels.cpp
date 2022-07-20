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

#include "repomodels.h"
#include "appdata.h"

#include <QStandardItem>
#include <QJsonDocument>

QString metaDataTypeToString(MetaDataType val)
{
  switch ((int)val) {
    case MDT_Release:
      return "Release";
    case MDT_Releases:
      return "Releases";
    case MDT_Asset:
      return "Asset";
    case MDT_ReleaseAssets:
      return "Release Assets";
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

/*
    ReleasesItemModel
*/

ReleasesItemModel::ReleasesItemModel() :
  UpdatesItemModel("Releases"),
  m_settingsIdx(-1),
  m_nightlyName(""),
  m_releaseChannel(-1)
{
  setSortRole(UpdatesItemModel::IMDR_Date);
}

bool ReleasesItemModel::releaseChannelChanged()
{
  if (m_settingsIdx > -1 && m_releaseChannel != (int)g.component[m_settingsIdx].releaseChannel())
    return true;
  else
    return false;
}

void ReleasesItemModel::parseMetaData(const int mdt, QJsonDocument * jsonDoc)
{
  json = jsonDoc;

  if (m_settingsIdx > -1)
    m_releaseChannel = g.component[m_settingsIdx].releaseChannel();

  switch (mdt) {
    case MDT_Release:
      parseRelease();
      break;
    case MDT_Releases:
      parseReleases();
      break;
  }
}

void ReleasesItemModel::parseRelease()
{
  clear();

  if (json->isObject()) {
    const QJsonObject &obj = json->object();
    parseJsonObject(obj);
  }
}

void ReleasesItemModel::parseReleases()
{
  clear();

  if (json->isArray()) {
    const QJsonArray &array = json->array();
    foreach (const QJsonValue &v, array)
    {
      if (v.isObject()) {
        const QJsonObject &obj = v.toObject();
        parseJsonObject(obj);
      }
    }
  }

  sort(0, Qt::DescendingOrder);
}

void ReleasesItemModel::parseJsonObject(const QJsonObject & obj)
{
  QStandardItem * item = new QStandardItem();

  QString tag = QString();
  bool prerelease = false;

  if (!obj.value("name").isUndefined()) {
    item->setText(obj.value("name").toString());
  }

  if (!obj.value("id").isUndefined()) {
    item->setData(obj.value("id").toInt(), UpdatesItemModel::IMDR_Id);
  }

  if (!obj.value("tag_name").isUndefined()) {
    tag = obj.value("tag_name").toString();
    item->setData(tag, UpdatesItemModel::IMDR_Tag);
  }

  if (!obj.value("prerelease").isUndefined()) {
    prerelease = obj.value("prerelease").toBool();
    item->setData(prerelease, UpdatesItemModel::IMDR_Prerelease);
  }

  if (!obj.value("created_at").isUndefined()) {
    item->setData(QDateTime::fromString(obj.value("created_at").toString(), Qt::ISODate), UpdatesItemModel::IMDR_Date);
  }

  item->setData(isReleaseAvailable(tag, prerelease), UpdatesItemModel::IMDR_Available);

  appendRow(item);
}

bool ReleasesItemModel::isReleaseAvailable(const QString tagname, const bool prerelease)
{
  if (prerelease) {
    if (m_releaseChannel == ComponentData::RELEASE_CHANNEL_STABLE) {
      //qDebug() << "Release channel stable" << tagname << "is pre-release" ;
      return false;
    }
    else if (m_releaseChannel == ComponentData::RELEASE_CHANNEL_PRERELEASE && tagname.toLower() == m_nightlyName) {
      //qDebug() << "Release channel pre-release" << tagname << "is nightly" ;
      return false;
    }
  }
  //qDebug() << "Release available:" << tagname;
  return true;
}

/*
    AssetsItemModel
*/

AssetsItemModel::AssetsItemModel() :
  UpdatesItemModel("Assets")
{

}

void AssetsItemModel::parseMetaData(int mdt, QJsonDocument * jsonDoc)
{
  json = jsonDoc;

  switch (mdt) {
    case MDT_Asset:
      parseAsset();
      break;
    case MDT_ReleaseAssets:
      parseReleaseAssets();
      break;
  }
}

void AssetsItemModel::parseAsset()
{
  clear();

  if (json->isObject()) {
    const QJsonObject &obj = json->object();
    parseJsonObject(obj);
  }
}

void AssetsItemModel::parseReleaseAssets()
{
  clear();

  if (json->isArray()) {
    const QJsonArray &array = json->array();
    foreach (const QJsonValue &v, array)
    {
      if (v.isObject()) {
        const QJsonObject &obj = v.toObject();
        parseJsonObject(obj);
      }
    }
  }

  sort(0);
}

void AssetsItemModel::parseJsonObject(const QJsonObject & obj)
{
  QStandardItem * item = new QStandardItem();

  if (!obj.value("name").isUndefined()) {
    item->setText(obj.value("name").toString());
  }

  if (!obj.value("id").isUndefined()) {
    item->setData(obj.value("id").toInt(), UpdatesItemModel::IMDR_Id);
  }

  if (!obj.value("content_type").isUndefined()) {
    item->setData(obj.value("content_type").toString(), UpdatesItemModel::IMDR_Content);
  }

  if (!obj.value("updated_at").isUndefined()) {
    item->setData(QDateTime::fromString(obj.value("updated_at").toString(), Qt::ISODate), UpdatesItemModel::IMDR_Date);
  }

  item->setData(true, UpdatesItemModel::IMDR_Available);

  appendRow(item);
}

/*
    UpdatesFilteredItemModel
*/

UpdatesFilteredItemModel::UpdatesFilteredItemModel(UpdatesItemModel * sourceModel, QString name) :
  QSortFilterProxyModel(nullptr),
  filterFlags(0),
  m_name(name)
{
  filterText.setPattern("");
  filterText.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
  setFilterRole(UpdatesItemModel::IMDR_Available);
  setFilterKeyColumn(0);
  setDynamicSortFilter(true);
  setSourceModel(sourceModel);
}

void UpdatesFilteredItemModel::setFilterFlags(const int flags)
{
  if (filterFlags != flags) {
    filterFlags = flags;
    filterText.setPattern("");
    invalidateFilter();
  }
}

void UpdatesFilteredItemModel::setFilterPattern(const QString & pattern)
{
  if (filterText.pattern() != pattern) {
    filterText.setPattern(pattern);
    filterFlags = 0;
    invalidateFilter();
  }
}

bool UpdatesFilteredItemModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
  const QModelIndex & srcIdx = sourceModel()->index(sourceRow, 0, sourceParent);

  if (!srcIdx.isValid() || !sourceModel()->data(srcIdx, filterRole()).toBool())
    return false;

  if (!filterFlags && filterText.pattern().isEmpty())
    return true;

  if (filterFlags) {
    bool ok;
    const int flags = sourceModel()->data(srcIdx, UpdatesItemModel::IMDR_Flags).toInt(&ok);

    return (ok && (!flags || (filterFlags & flags)));
  }
  else {
    return sourceModel()->data(srcIdx, Qt::DisplayRole).toString().contains(filterText);
  }
}

void UpdatesFilteredItemModel::dumpContents()
{
  qDebug() << "Contents of" << m_name;

  for (int i = 0; i < rowCount(); ++i) {
    qDebug() << "row:"   << i
             << "text:"  << data(index(i, 0), Qt::DisplayRole).toString()
             << "id:"    << data(index(i, 0), UpdatesItemModel::IMDR_Id).toInt()
             << "date:"  << data(index(i, 0), UpdatesItemModel::IMDR_Date).toDateTime()
             << "tag:"  << data(index(i, 0), UpdatesItemModel::IMDR_Tag).toString()
             << "avail:" << data(index(i, 0), UpdatesItemModel::IMDR_Available).toBool()
             << "flags:" << data(index(i, 0), UpdatesItemModel::IMDR_Flags).toInt()
             << "content:" << data(index(i, 0), UpdatesItemModel::IMDR_Content).toString()
             << "prerelease:"  << data(index(i, 0), UpdatesItemModel::IMDR_Prerelease).toBool();
  }
}

QMap<int, QVariant> UpdatesFilteredItemModel::itemMetaData(const int id)
{
  const QModelIndexList idxlst = match(index(0, 0), UpdatesItemModel::IMDR_Id, QVariant(id), 1, Qt::MatchExactly);

  if (!idxlst.isEmpty())
    return itemData(idxlst.at(0));

  return QMap<int, QVariant>();
}

QVariant UpdatesFilteredItemModel::metaDataValue(const int id, const int role)
{
  //qDebug() << "id:" << id << "role:";

  const QModelIndexList idxlst = match(index(0, 0), UpdatesItemModel::IMDR_Id, QVariant(id), 1, Qt::MatchExactly);

  if (!idxlst.isEmpty())
    return data(idxlst.at(0), role);

  return QVariant();
}

bool UpdatesFilteredItemModel::setMetaDataValue(const int id, const int role, const QVariant value)
{
  //qDebug() << "id:" << id << "role:" << role << "value:" << value;

  const QModelIndexList idxlst = match(index(0, 0), UpdatesItemModel::IMDR_Id, QVariant(id), 1, Qt::MatchExactly);

  if (!idxlst.isEmpty())
    return setData(idxlst.at(0), value, role);

  return false;
}

int UpdatesFilteredItemModel::id(const int row)
{
  const QModelIndex & idx = index(row, 0);

  if (!idx.isValid())
    return 0;

  bool ok;
  const int id = data(idx, UpdatesItemModel::IMDR_Id).toInt(&ok);

  if (ok)
    return id;

  return 0;
}

int UpdatesFilteredItemModel::id(const QVariant value, const Qt::MatchFlags flags, const int role)
{
  const QModelIndexList idxlst = match(index(0, 0), role, value, 1, flags);

  if (!idxlst.isEmpty()) {
    bool ok;
    const int id = data(idxlst.at(0), UpdatesItemModel::IMDR_Id).toInt(&ok);

    if (ok)
      return id;
  }

  return 0;
}

QStringList UpdatesFilteredItemModel::list()
{
  QStringList strl;

  for (int i = 0; i < rowCount(); i++)
    strl << data(index(i, 0)).toString();

  return strl;
}


/*
    ReleasesFilteredItemModel
*/

ReleasesFilteredItemModel::ReleasesFilteredItemModel(UpdatesItemModel * sourceModel) :
  UpdatesFilteredItemModel(sourceModel, "Filtered Releases")
{
  setSortRole(UpdatesItemModel::IMDR_Date);
}

int ReleasesFilteredItemModel::channelLatestId()
{
  return rows() > 0 ? id(0) : 0; //  assumes model sorted descending date
}

/*
    AssetsFilteredItemModel
*/

AssetsFilteredItemModel::AssetsFilteredItemModel(UpdatesItemModel * sourceModel) :
  UpdatesFilteredItemModel(sourceModel, "Filtered Assets")
{

}

bool AssetsFilteredItemModel::setFlags(const int id, const int flags)
{
  return setMetaDataValue(id, UpdatesItemModel::IMDR_Flags, QVariant(flags));
}

bool AssetsFilteredItemModel::setSubDirectory(const int id, const QString path)
{
  return setMetaDataValue(id, UpdatesItemModel::IMDR_SubDirectory, QVariant(path));
}

bool AssetsFilteredItemModel::setCopyFilter(const int id, const QString filter)
{
  return setMetaDataValue(id, UpdatesItemModel::IMDR_CopyFilter, QVariant(filter));
}

/*
    ReleasesMetaData
*/

ReleasesMetaData::ReleasesMetaData(QWidget * parent) :
  QWidget(parent),
  m_repo(""),
  m_id(0)
{
  itemModel = new ReleasesItemModel();
  filteredItemModel = new ReleasesFilteredItemModel(itemModel);
}

bool ReleasesMetaData::refreshRequired()
{
  if (m_id != 0)
    return itemModel->releaseChannelChanged();

  return true;
}

/*
    AssetsMetaData
*/

AssetsMetaData::AssetsMetaData(QWidget * parent) :
  QWidget(parent)
{
  itemModel = new AssetsItemModel();
  filteredItemModel = new AssetsFilteredItemModel(itemModel);
}
