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

#include "repogithub.h"

RepoGitHub::RepoGitHub(QObject * parent, UpdateStatus * status, UpdateNetwork * network,
                       const QString & path, const QString & nightly, const int resultsPerPage) :
  Repo(parent, status, network, path, nightly, resultsPerPage)
{
}

RepoGitHub::~RepoGitHub()
{
}

const QString RepoGitHub::urlAsset(const int assetId) const
{
  return QString("%1/assets/%2").arg(urlReleases()).arg(assetId);
}

const QString RepoGitHub::urlAssets(const int releaseId) const
{
  return QString("%1/%2/assets").arg(urlReleases()).arg(releaseId) % (resultsPerPage() > -1 ?
    QString("\?per_page=%1").arg(resultsPerPage()) : "");
}

const QString RepoGitHub::urlContent(const QString & filename) const
{
  return QString("%1/contents/%2").arg(path()).arg(filename);
}

const QString RepoGitHub::urlReleases() const
{
  return QString("%1/releases").arg(path());
}
