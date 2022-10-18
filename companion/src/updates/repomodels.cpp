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
  m_releaseChannel(-1),
  m_refreshRequired(true)
{
  setSortRole(UpdatesItemModel::IMDR_Date);
}

void ReleasesItemModel::setReleaseChannel(const int channel)
{
  if (m_releaseChannel != channel) {
    m_releaseChannel = channel;
    update();
  }
}

void ReleasesItemModel::parseMetaData(const int mdt, QJsonDocument * jsonDoc)
{
  json = jsonDoc;

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
  m_refreshRequired = false;
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

  setDynamicItemData(item);

  appendRow(item);
}

void ReleasesItemModel::setDynamicItemData(QStandardItem * item)
{
  item->setData(isReleaseAvailable(item), UpdatesItemModel::IMDR_Available);
}

void ReleasesItemModel::update()
{
  for (int i = 0; i < rowCount(); ++i)
    setDynamicItemData(item(i));
}

bool ReleasesItemModel::isReleaseAvailable(QStandardItem * item)
{
  if (item->data(IMDR_Prerelease).toBool()) {
    if (m_releaseChannel == ComponentData::RELEASE_CHANNEL_STABLE) {
      //qDebug() << "Release channel stable" << tagname << "is pre-release" ;
      return false;
    }
    else if (m_releaseChannel == ComponentData::RELEASE_CHANNEL_PRERELEASE &&
             item->data(IMDR_Tag).toString().toLower() == m_nightlyName) {
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
    RepoMetaData
*/

RepoMetaData::RepoMetaData(QObject * parent) :
  QObject(parent)
{

}

/*
    ReleasesMetaData
*/

ReleasesMetaData::ReleasesMetaData(QObject * parent) :
  RepoMetaData(parent),
  m_id(0)
{
  itemModel = new ReleasesItemModel();
  filteredItemModel = new ReleasesFilteredItemModel(itemModel);
}

void ReleasesMetaData::init(const QString repo, const QString nightly, const int settingsIndex, const int resultsPerPage)
{
  m_repo = repo;
  m_resultsPerPage = resultsPerPage;
  itemModel->setNightlyName(nightly);
  itemModel->setSettingsIndex(settingsIndex);
}

bool ReleasesMetaData::refreshRequired()
{
  return m_id == 0 ? true : itemModel->refreshRequired();
}

int ReleasesMetaData::getSetId()
{
  m_id = filteredItemModel->channelLatestId();
  return m_id;
}

int ReleasesMetaData::getSetId(int row)
{
  m_id = filteredItemModel->id(row);
  return m_id;
}

int ReleasesMetaData::getSetId(QVariant value, Qt::MatchFlags flags, int role)
{
  m_id = filteredItemModel->id(value, flags, role);
  return m_id;
}

QString ReleasesMetaData::name()
{
  if (m_id)
    return filteredItemModel->name(m_id);

  return "";
}

QString ReleasesMetaData::version()
{
  if (m_id)
    return filteredItemModel->version(m_id);

  return "0";
}

/*
    AssetsMetaData
*/

AssetsMetaData::AssetsMetaData(QObject * parent) :
  RepoMetaData(parent)
{
  itemModel = new AssetsItemModel();
  filteredItemModel = new AssetsFilteredItemModel(itemModel);
}

void AssetsMetaData::init(const QString repo, const int resultsPerPage)
{
  m_repo = repo;
  m_resultsPerPage = resultsPerPage;
}

int AssetsMetaData::getSetId(int row)
{
  m_id = filteredItemModel->id(row);
  return m_id;
}

int AssetsMetaData::getSetId(QVariant value, Qt::MatchFlags flags, int role)
{
  m_id = filteredItemModel->id(value, flags, role);
  return m_id;
}
