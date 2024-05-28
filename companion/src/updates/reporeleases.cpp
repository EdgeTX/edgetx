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

#include "repotypes.h"
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

ReleasesRawItemModel::ReleasesRawItemModel(QObject * parentRepo) :
  RepoRawItemModel("Raw Releases"),
  m_parentRepo(parentRepo)
{
  setSortRole(RIMR_SortOrder);
}

void ReleasesRawItemModel::parseJsonObject(const QJsonObject & obj)
{
  //qDebug() << obj;
  RepoGitHub *rg = dynamic_cast<RepoGitHub*>(m_parentRepo);
  if (rg) {
    parseJsonObjectGitHub(obj);
  }
  else {
    RepoBuild *rb = dynamic_cast<RepoBuild*>(m_parentRepo);
    if (rb) {
      parseJsonObjectBuild(obj);
    }
  }
}

void ReleasesRawItemModel::parseJsonObjectBuild(const QJsonObject & obj)
{
  Repo *repo = static_cast<Repo*>(m_parentRepo);
  const QString fwRadio = getCurrentFirmware()->getFlavour();

  if (obj.value("releases").isObject()) {
    repo->setConfig(obj);
    const QJsonObject &releases = obj.value("releases").toObject();
    QStringList list = releases.keys();

    for (auto i = list.cbegin(), end = list.cend(); i != end; ++i) {
      QStandardItem * item = new QStandardItem();

      int flags = RELFLG_Stable;
      bool available = true;

      item->setText(*i);
      item->setData(*i, RIMR_Tag);

      if (item->data(RIMR_Tag).toString().toLower() == repo->nightly().toLower())
        item->setData((int)QDate::currentDate().toJulianDay(), RIMR_Id);
      else
        item->setData(SemanticVersion(*i).toInt(), RIMR_Id);

      item->setData(QDateTime::currentDateTimeUtc(), RIMR_Date);

      SemanticVersion sv;

      if (item->data(RIMR_Tag).toString().toLower() == repo->nightly().toLower()) {
        flags = RELFLG_Nightly;
        sv.fromString("255.255.255");
      }
      else
        sv.fromString(item->data(RIMR_Tag).toString());

      if (sv.isPreRelease())
        flags = RELFLG_PreRelease;

      if (sv.isValid())
        item->setData(sv.toInt(), RIMR_SortOrder);
      else
        item->setData(item->data(RIMR_Id).toInt(), RIMR_SortOrder);

      if (releases.value(*i).isObject()) {
        const QJsonObject &release = releases.value(*i).toObject();
        if (!release.value("exclude_targets").isUndefined()) {
          if (release.value("exclude_targets").isArray()) {
            const QJsonArray &excluded = release.value("exclude_targets").toArray();
            foreach (const QJsonValue &v, excluded) {
              if (v.isString() && v.toString() == fwRadio) {
                available = false;
              }
            }
          }
        }
      }

      item->setData(available, RIMR_Available);
      item->setData(flags, RIMR_Flags);

      appendRow(item);
    }
  }
}

void ReleasesRawItemModel::parseJsonObjectGitHub(const QJsonObject & obj)
{
  Repo *repo = static_cast<Repo*>(m_parentRepo);
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
    if (item->data(RIMR_Tag).toString().toLower() == repo->nightly().toLower())
      flags = RELFLG_Nightly;
  }

  if (!obj.value("draft").isUndefined()) {
    if (obj.value("draft").toBool())
      available = false;
  }

  item->setData(available, RIMR_Available);
  item->setData(flags, RIMR_Flags);

  SemanticVersion sv;

  if (item->data(RIMR_Tag).toString().toLower() == repo->nightly().toLower())
    sv.fromString("255.255.255");
  else
    sv.fromString(item->data(RIMR_Tag).toString());

  if (sv.isValid())
    item->setData(sv.toInt(), RIMR_SortOrder);
  else
    item->setData(item->data(RIMR_Date).toInt(), RIMR_SortOrder);

  appendRow(item);
}

/*
    ReleasesFilteredItemModel
*/

ReleasesFilteredItemModel::ReleasesFilteredItemModel(QObject * parentRepo, ReleasesRawItemModel * releasesRawItemModel) :
  RepoFilteredItemModel("Filtered Releases"),
  m_parentRepo(parentRepo)
{
  setSourceModel(releasesRawItemModel);
  setSortRole(RIMR_SortOrder);
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

RepoReleases::RepoReleases(QObject * parentRepo, UpdateStatus * status, UpdateNetwork * network) :
  QObject(parentRepo),
  RepoMetaData(status, network),
  m_rawItemModel(new ReleasesRawItemModel(parentRepo)),
  m_filteredItemModel(new ReleasesFilteredItemModel(parentRepo, m_rawItemModel)),
  m_parentRepo(parentRepo)
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

bool RepoReleases::retrieveMetaDataAll()
{
  bool res = false;
  Repo *repo = dynamic_cast<Repo*>(m_parentRepo);
  if (repo) {
    res = retrieveMetaData(repo->releasesMetaDataType(), repo->urlReleases());
    m_rawItemModel->sort(0, Qt::DescendingOrder);
  }

  return res;
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
