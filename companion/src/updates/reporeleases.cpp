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

#include "reporeleases.h"
#include "appdata.h"
#include "helpers.h"

/*
    ReleasesItemModels
*/

//  static
const QString ReleasesItemModels::flagsToString(const int flags)
{
  QString str;

  for (int i = 0; i < 16; i++) {
    int flag = flags & (1 << i);
    if (flag) {
      if (!str.isEmpty())
        str.append(";");
      str.append(flagToString(flag));
    }
  }

  return str;
}

//  static
const QString ReleasesItemModels::flagToString(const int val)
{
  const QStringList strl = { "None", "Stable", "Pre-release", "Nightly" };
  return strl.value(val, CPN_STR_UNKNOWN_ITEM);
};

/*
    ReleasesRawItemModel
*/

ReleasesRawItemModel::ReleasesRawItemModel() :
  RepoRawItemModel("Raw Releases"),
  m_nightlyName("")
{
  setSortRole(RIMR_SortOrder);
}

void ReleasesRawItemModel::parseJsonObject(const QJsonObject & obj)
{
  QStandardItem * item = new QStandardItem();

  int flags = RELFLG_Stable;
  bool available = true;

  if (!obj.value("name").isUndefined()) {
    item->setText(obj.value("name").toString());
  }

  if (!obj.value("id").isUndefined()) {
    item->setData(obj.value("id").toInt(), RIMR_Id);
  }

  if (!obj.value("created_at").isUndefined()) {
    item->setData(QDateTime::fromString(obj.value("created_at").toString(), Qt::ISODate), RIMR_Date);
  }

  if (!obj.value("prerelease").isUndefined()) {
    if (obj.value("prerelease").toBool())
      flags = RELFLG_PreRelease;
  }

  if (!obj.value("tag_name").isUndefined()) {
    item->setData(obj.value("tag_name").toString(), RIMR_Tag);
    if (item->data(RIMR_Tag).toString().toLower() == m_nightlyName)
      flags = RELFLG_Nightly;
  }

  if (!obj.value("draft").isUndefined()) {
    if (obj.value("draft").toBool())
      available = false;
  }

  item->setData(available, RIMR_Available);
  item->setData(flags, RIMR_Flags);

  SemanticVersion sv;

  if (item->data(RIMR_Tag).toString().toLower() == m_nightlyName)
    sv.fromString("255.255.255");
  else
    sv.fromString(item->data(RIMR_Tag).toString());

  if (sv.isValid())
    item->setData(sv.toInt(), RIMR_SortOrder);
  else
    item->setData(item->data(RIMR_Date).toInt(), RIMR_SortOrder);

  appendRow(item);
}

void ReleasesRawItemModel::setNightlyName(const QString name)
{
  m_nightlyName = name.toLower();
}

/*
    ReleasesFilteredItemModel
*/

ReleasesFilteredItemModel::ReleasesFilteredItemModel(ReleasesRawItemModel * releasesRawItemModel) :
  RepoFilteredItemModel("Filtered Releases")
{
  setSourceModel(releasesRawItemModel);
  setSortRole(RIMR_Date);
}

const int ReleasesFilteredItemModel::channelLatestId() const
{
  return rowCount() > 0 ? id(0) : 0;
}

void ReleasesFilteredItemModel::setChannel(const int channel)
{
  int flags = 0;

  for (int i = 0; i <= channel; i++)
    flags |= (1 << (i + 1));

  setFilterFlags(flags);
}

const QString ReleasesFilteredItemModel::version(const int id) const
{
  return value(id, RIMR_Tag).toString();
}

/*
    RepoReleases
*/

RepoReleases::RepoReleases(QObject * parent, UpdateStatus * status, UpdateNetwork * network) :
  QObject(parent),
  RepoMetaData(status, network),
  m_rawItemModel(new ReleasesRawItemModel()),
  m_filteredItemModel(new ReleasesFilteredItemModel(m_rawItemModel))
{
  setModels(m_rawItemModel, m_filteredItemModel);
}

int RepoReleases::channelLatestId()
{
  return m_filteredItemModel->channelLatestId();
}

void RepoReleases::dumpItemModel(const QString modelName, const QAbstractItemModel * itemModel) const
{
  qDebug() << "Contents of model:" << modelName;

  if (itemModel) {
    for (int i = 0; i < itemModel->rowCount(); ++i) {
      qDebug().noquote() << "row:"           << i;
      qDebug().noquote() << "  name:"        << itemModel->data(itemModel->index(i, 0), Qt::DisplayRole).toString();
      qDebug().noquote() << "  id:"          << itemModel->data(itemModel->index(i, 0), RepoItemModelBase::RIMR_Id).toInt();
      qDebug().noquote() << "  date:"        << itemModel->data(itemModel->index(i, 0), RepoItemModelBase::RIMR_Date).toDateTime();
      qDebug().noquote() << "  version:"     << itemModel->data(itemModel->index(i, 0), RepoItemModelBase::RIMR_Tag).toString();
      qDebug().noquote() << "  flags:"       << ReleasesItemModels::flagsToString(itemModel->data(itemModel->index(i, 0), RepoItemModelBase::RIMR_Flags).toInt());
      qDebug().noquote() << "  avail:"       << itemModel->data(itemModel->index(i, 0), RepoItemModelBase::RIMR_Available).toBool();
    }
  }
}

void RepoReleases::init(const QString & repoPath,const QString & nightly, const int resultsPerPage)
{
  RepoMetaData::init(repoPath, resultsPerPage);
  m_rawItemModel->setNightlyName(nightly);
}

bool RepoReleases::retrieveMetaDataAll()
{
  return retrieveMetaData(RepoRawItemModel::MDT_Releases, urlReleases());
}

int RepoReleases::setId(const int id)
{
  if (this->id() != id) {
    RepoMetaData::setId(id);
    emit idChanged(this->id());
  }

  return this->id();
}

void RepoReleases::setChannel(const int channel)
{
  m_filteredItemModel->setChannel(channel);
  getSetId();
}

const QString RepoReleases::version() const
{
  return m_filteredItemModel->version(id());
}
