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

/*
    AssetsRawItemModel
*/

AssetsRawItemModel::AssetsRawItemModel(QObject * parentRepo) :
  RepoRawItemModel("Raw Assets"),
  m_parentRepo(parentRepo)
{

}

void AssetsRawItemModel::parseJsonObject(const QJsonObject & obj)
{
  //qDebug() << obj;
  RepoGitHub *rg = dynamic_cast<RepoGitHub*>(m_parentRepo);
  if (rg) {
    parseJsonObjectGitHub(obj);
  }
  else {
    RepoBuild *rb = dynamic_cast<RepoBuild*>(m_parentRepo);
    if (rb) {
      parseJsonObjectBuild();
    }
  }
}

void AssetsRawItemModel::parseJsonObjectBuild()
{
  Repo *repo = static_cast<Repo*>(m_parentRepo);

  if (repo->config()->value("targets").isObject()) {
    const QJsonObject &targets = repo->config()->value("targets").toObject();
    QStringList list = targets.keys();
    int id = 0;

    for (auto i = list.cbegin(), end = list.cend(); i != end; ++i) {
      QStandardItem * item = new QStandardItem();

      item->setText(*i);
      item->setData(++id, RIMR_Id);
      item->setData(true, RIMR_Available);
      item->setData(0, RIMR_Flags);

      appendRow(item);
    }
  }
}

void AssetsRawItemModel::parseJsonObjectGitHub(const QJsonObject & obj)
{
  QStandardItem * item = new QStandardItem();

  if (!obj.value("name").isUndefined()) {
    item->setText(obj.value("name").toString());
    item->setData(obj.value("name").toString(), RIMR_DownloadName);
  }

  if (!obj.value("id").isUndefined()) {
    item->setData(obj.value("id").toInt(), RIMR_Id);
  }

  if (!obj.value("content_type").isUndefined()) {
    item->setData(obj.value("content_type").toString(), RIMR_Content);
  }

  if (!obj.value("updated_at").isUndefined()) {
    item->setData(QDateTime::fromString(obj.value("updated_at").toString(), Qt::ISODate), RIMR_Date);
  }

  item->setData(true, RIMR_Available);
  item->setData(0, RIMR_Flags);

  appendRow(item);
}

bool AssetsRawItemModel::setDownloadUrl(const int id, const QString url)
{
  return setValue(id, RIMR_DownloadUrl, QVariant(url));
}

bool AssetsRawItemModel::setDownloadName(const int id, const QString name)
{
  return setValue(id, RIMR_DownloadName, QVariant(name));
}

bool AssetsRawItemModel::resetFlags()
{
  for (int i = 0; i < rowCount(); i++) {
    item(i)->setData(0, RIMR_Flags);
  }

  return true;
}

/*
    AssetsFilteredItemModel
*/

AssetsFilteredItemModel::AssetsFilteredItemModel(QObject * parentRepo, AssetsRawItemModel * assetsRawItemModel) :
  RepoFilteredItemModel("Filtered Assets"),
  m_parentRepo(parentRepo)
{
  setSourceModel(assetsRawItemModel);
}

const QString AssetsFilteredItemModel::contentType(const int id) const
{
  return value(id, RIMR_Content).toString();
}

const QString AssetsFilteredItemModel::copyFilter(const int id) const
{
  return value(id, RIMR_CopyFilter).toString();
}

const QString AssetsFilteredItemModel::downloadUrl(const int id) const
{
  return value(id, RIMR_DownloadUrl).toString();
}

const QString AssetsFilteredItemModel::downloadName(const int id) const
{
  return value(id, RIMR_DownloadName).toString();
}

bool AssetsFilteredItemModel::setCopyFilter(const int id, const QString filter)
{
  return setValue(id, RIMR_CopyFilter, QVariant(filter));
}

bool AssetsFilteredItemModel::setSubDirectory(const int id, const QString path)
{
  return setValue(id, RIMR_SubDirectory, QVariant(path));
}

const QString AssetsFilteredItemModel::subDirectory(const int id) const
{
  return value(id, RIMR_SubDirectory).toString();
}

/*
    RepoAssets
*/

RepoAssets::RepoAssets(QObject * parentRepo, UpdateStatus * status, UpdateNetwork * network) :
  QObject(parentRepo),
  RepoMetaData(status, network),
  m_rawItemModel(new AssetsRawItemModel(parentRepo)),
  m_filteredItemModel(new AssetsFilteredItemModel(parentRepo, m_rawItemModel)),
  m_parentRepo(parentRepo)

{
  setModels(m_rawItemModel, m_filteredItemModel);
}

bool RepoAssets::build(const int row)
{
  status()->reportProgress(tr("Generic asset build not implemented"), QtDebugMsg);
  return true;
}

const QString RepoAssets::contentType() const
{
  return m_filteredItemModel->contentType(id());
}

const QString RepoAssets::copyFilter() const
{
  return m_filteredItemModel->copyFilter(id());
}

bool RepoAssets::downloadToBuffer(const int id)
{
  if (id > 0)
    setId(id);

  Repo *repo = static_cast<RepoGitHub*>(m_parentRepo);
  network()->downloadToBuffer(repo->assetDownloadDataType(), repo->urlAsset(this->id()));
  return network()->isSuccess();
}

bool RepoAssets::downloadToFile(const int row, QString & downloadDir)
{
  Repo *repo = static_cast<Repo*>(m_parentRepo);
  getSetId(row);
  QString fmt = QString("%1/A%2/%3").arg(downloadDir).arg(id()).arg(downloadName());
  QFileInfo f(fmt);
  network()->downloadToFile(repo->assetDownloadDataType(), repo->urlAsset(id()), f.absoluteFilePath());
  return network()->isSuccess();
}

const QString RepoAssets::downloadUrl() const
{
  return m_filteredItemModel->downloadUrl(id());
}

const QString RepoAssets::downloadUrl(const int row)
{
  getSetId(row);
  return downloadUrl();
}

const QString RepoAssets::downloadName() const
{
  return m_filteredItemModel->downloadName(id());
}

const QString RepoAssets::downloadName(const int row)
{
  getSetId(row);
  return downloadName();
}

void RepoAssets::dumpItemModel(const QString modelName, const QAbstractItemModel * itemModel) const
{
  qDebug() << "Contents of model:" << modelName;

  if (itemModel) {
    for (int i = 0; i < itemModel->rowCount(); ++i) {
      qDebug().noquote() << "row:"            << i;
      qDebug().noquote() << "  name:"         << itemModel->data(itemModel->index(i, 0), Qt::DisplayRole).toString();
      qDebug().noquote() << "  id:"           << itemModel->data(itemModel->index(i, 0), RepoItemModelBase::RIMR_Id).toInt();
      qDebug().noquote() << "  date:"         << itemModel->data(itemModel->index(i, 0), RepoItemModelBase::RIMR_Date).toDateTime();
      qDebug().noquote() << "  content:"      << itemModel->data(itemModel->index(i, 0), RepoItemModelBase::RIMR_Content).toString();
      qDebug().noquote() << "  copy filter:"  << itemModel->data(itemModel->index(i, 0), RepoItemModelBase::RIMR_CopyFilter).toString();
      qDebug().noquote() << "  sub dir:"      << itemModel->data(itemModel->index(i, 0), RepoItemModelBase::RIMR_SubDirectory).toString();
      qDebug().noquote() << "  flags:"        << itemModel->data(itemModel->index(i, 0), RepoItemModelBase::RIMR_Flags).toInt();
      qDebug().noquote() << "  avail:"        << itemModel->data(itemModel->index(i, 0), RepoItemModelBase::RIMR_Available).toBool();
    }
  }
}

bool RepoAssets::getJson(QJsonDocument * json)
{
  Repo *repo = static_cast<Repo*>(m_parentRepo);
  network()->downloadJsonAsset(repo->urlAsset(id()), json);

  if (!network()->isSuccess()) {
    status()->reportProgress("Unable to download json data", QtDebugMsg);
    return false;
  }

  return true;
}

bool RepoAssets::getJson(const QString assetName, QJsonDocument * json)
{
  if (!getSetId(assetName))
    return false;

  return getJson(json);
}

const QString RepoAssets::name() const
{
  return m_filteredItemModel->name(id());
}

const QString RepoAssets::name(const int row)
{
  getSetId(row);
  return name();
}

void RepoAssets::onReleaseIdChanged(const int id)
{
  m_releaseId = id;
  invalidate();
}

bool RepoAssets::retrieveMetaDataAll()
{
  Repo *repo = static_cast<Repo*>(m_parentRepo);
  return retrieveMetaData(repo->assetsMetaDataType(), repo->urlAssets(m_releaseId));
}

bool RepoAssets::retrieveMetaDataOne(const int id)
{
  setId(id);

  Repo *repo = static_cast<Repo*>(m_parentRepo);
  return retrieveMetaData(repo->assetMetaDataType(), repo->urlAsset(id));
}

bool RepoAssets::setCopyFilter(const QString filter)
{
  return m_filteredItemModel->setCopyFilter(id(), filter);
}

bool RepoAssets::setSubDirectory(const QString path)
{
  return m_filteredItemModel->setSubDirectory(id(), path);
}

const QString RepoAssets::subDirectory() const
{
  return m_filteredItemModel->subDirectory(id());
}

bool RepoAssets::setDownloadUrl(const QString url)
{
  return m_rawItemModel->setDownloadUrl(id(), url);
}

bool RepoAssets::setDownloadName(const QString name)
{
  return m_rawItemModel->setDownloadName(id(), name);
}

bool RepoAssets::resetFlags()
{
  return m_rawItemModel->resetFlags();
}
