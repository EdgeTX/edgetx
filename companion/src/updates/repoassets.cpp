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

#include "repoassets.h"

/*
    AssetsRawItemModel
*/

AssetsRawItemModel::AssetsRawItemModel() :
  RepoRawItemModel("Raw Assets")
{

}

void AssetsRawItemModel::parseJsonObject(const QJsonObject & obj)
{
  QStandardItem * item = new QStandardItem();

  if (!obj.value("name").isUndefined()) {
    item->setText(obj.value("name").toString());
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

/*
    AssetsFilteredItemModel
*/

AssetsFilteredItemModel::AssetsFilteredItemModel(AssetsRawItemModel * assetsRawItemModel) :
  RepoFilteredItemModel("Filtered Assets")
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

RepoAssets::RepoAssets(QObject * parent, UpdateStatus * status, UpdateNetwork * network) :
  QObject(parent),
  RepoMetaData(status, network),
  m_rawItemModel(new AssetsRawItemModel()),
  m_filteredItemModel(new AssetsFilteredItemModel(m_rawItemModel))
{
  setModels(m_rawItemModel, m_filteredItemModel);
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
  //progressMessage(tr("Download asset %1").arg(id));
  if (id > 0)
    setId(id);
  network()->downloadToBuffer(UpdateNetwork::DDT_Content, urlAsset(this->id()));
  return network()->isSuccess();
}

bool RepoAssets::downloadToFile(const int row, QString & downloadDir)
{
  getSetId(row);
  QFileInfo f(QString("%1/A%2/%3").arg(downloadDir).arg(id()).arg(name()));
  network()->downloadToFile(urlAsset(id()), f.absoluteFilePath());
  return network()->isSuccess();
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
  network()->downloadJsonAsset(urlAsset(id()), json);

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

void RepoAssets::init(const QString & repoPath,const QString & nightly, const int resultsPerPage)
{
  RepoMetaData::init(repoPath, resultsPerPage);
}

void RepoAssets::onReleaseIdChanged(const int id)
{
  m_releaseId = id;
  invalidate();
}

bool RepoAssets::retrieveMetaDataAll()
{
  return retrieveMetaData(RepoRawItemModel::MDT_Assets, urlAssets(m_releaseId));
}

bool RepoAssets::retrieveMetaDataOne(const int id)
{
  setId(id);
  return retrieveMetaData(RepoRawItemModel::MDT_Asset, urlAsset(id));
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
