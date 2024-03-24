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

#include "repo.h"

Repo::Repo(QObject * parent, UpdateStatus * status, UpdateNetwork * network) :
  QObject(parent),
  m_status(status),
  m_network(network),
  m_releases(new RepoReleases(this, status, network)),
  m_assets(new RepoAssets(this, status, network))
{
  connect(m_releases, &RepoReleases::idChanged, [=](const int id) {
    m_assets->onReleaseIdChanged(id);
  });
}

Repo::~Repo()
{
  delete m_assets;
  delete m_releases;
}

bool Repo::getJson(const QString filename, QJsonDocument * json)
{
  m_network->downloadJsonContent(m_releases->urlContent(filename), json);

  if (!m_network->isSuccess()) {
    m_status->reportProgress("Unable to download json data", QtDebugMsg);
    return false;
  }

  return true;
}

void Repo::init(const QString & repoPath, const QString & nightly, const int resultsPerPage)
{
  m_path = repoPath;
  m_releases->init(repoPath, nightly, resultsPerPage);
  m_assets->init(repoPath, QString(), resultsPerPage);
}
