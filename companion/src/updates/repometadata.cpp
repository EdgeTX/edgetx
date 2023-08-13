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

#include "repometadata.h"

RepoMetaData::RepoMetaData(UpdateStatus * status, UpdateNetwork * network) :
  m_status(status),
  m_network(network),
  m_rawItemModel(nullptr),
  m_filteredItemModel(nullptr),
  m_id(0)
{

}

const int RepoMetaData::count() const
{
  return m_filteredItemModel->count();
}

const QString RepoMetaData::date() const
{
  return m_filteredItemModel->date(id());
}

void RepoMetaData::dumpRawItemModel() const
{
  dumpItemModel(m_rawItemModel->modelName(), m_rawItemModel);
}

void RepoMetaData::dumpFilteredItemModel() const
{
  dumpItemModel(m_filteredItemModel->modelName(), m_filteredItemModel);
}

const int RepoMetaData::flags() const
{
  return m_filteredItemModel->getFlags(id());
}

int RepoMetaData::getSetId()
{
  return getSetId(0);
}

int RepoMetaData::getSetId(const int row)
{
  return setId(m_filteredItemModel->id(row));
}

int RepoMetaData::getSetId(QVariant value, Qt::MatchFlags flags, int role)
{
  return setId(m_filteredItemModel->id(value, flags, role));
}

const int RepoMetaData::id() const
{
  return m_id;
}

void RepoMetaData::init(const QString & repoPath, const int resultsPerPage)
{
  m_repoPath = repoPath;
  m_resultsPerPage = resultsPerPage;
}

void RepoMetaData::invalidate()
{
  setId(0);
  m_rawItemModel->invalidate();
}

const int RepoMetaData::isEmpty() const
{
  return m_filteredItemModel->isEmpty();
}

const bool RepoMetaData::isIdValid() const
{
  return m_id > 0;
}

const QStringList RepoMetaData::list() const
{
  return m_filteredItemModel->list();
}

const QString RepoMetaData::name() const
{
  return m_filteredItemModel->name(id());
}

UpdateNetwork* const RepoMetaData::network() const
{
  return m_network;
}

const QString RepoMetaData::repoPath() const
{
  return m_repoPath;
}

const int RepoMetaData::resultsPerPage() const
{
  return m_resultsPerPage;
}

bool RepoMetaData::retrieveMetaData(const int mdt, const QString url)
{
  if (!m_rawItemModel->isRefreshRequired())
    return true;

  QJsonDocument *json = new QJsonDocument();

  network()->downloadMetaData(url, json);

  if (!network()->isSuccess()) {
    status()->reportProgress("Unable to download release asset meta data", QtDebugMsg);
    invalidate();
    return false;
  }

  m_rawItemModel->parseMetaData(mdt, json);

  // set current id to first row as most methods assume a current id has been set
  getSetId();

  delete json;

  return true;
}

bool RepoMetaData::setFlags(int flags)
{
  return m_filteredItemModel->setFlags(id(), flags);
}

void RepoMetaData::setFilterFlags(int flags)
{
  m_filteredItemModel->setFilterFlags(flags);
}

void RepoMetaData::setFilterPattern(const QString & pattern)
{
  m_filteredItemModel->setFilterPattern(pattern);
}

int RepoMetaData::setId(const int id)
{
  m_id = id;
  return m_id;
}

void RepoMetaData::setModels(RepoRawItemModel * repoRawItemModel, RepoFilteredItemModel * repoFilteredItemModel)
{
  m_rawItemModel = repoRawItemModel;
  m_filteredItemModel = repoFilteredItemModel;
  invalidate();
}

UpdateStatus* const RepoMetaData::status() const
{
  return m_status;
}

const QString RepoMetaData::urlAsset(const int assetId) const
{
  return QString("%1/assets/%2").arg(urlReleases()).arg(assetId);
}

const QString RepoMetaData::urlAssets(const int releaseId) const
{
  return QString("%1/%2/assets").arg(urlReleases()).arg(releaseId) % (m_resultsPerPage > -1 ?
    QString("\?per_page=%1").arg(m_resultsPerPage) : "");
}

const QString RepoMetaData::urlContent(const QString & filename) const
{
  return QString("%1/contents/%2").arg(m_repoPath).arg(filename);
}

const QString RepoMetaData::urlReleases() const
{
  return QString("%1/releases").arg(m_repoPath);
}
