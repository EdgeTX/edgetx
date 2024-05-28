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

#include "repobuild.h"

RepoBuild::RepoBuild(QObject * parent, UpdateStatus * status, UpdateNetwork * network,
                     const QString & path, const QString & nightly, const int resultsPerPage) :
  Repo(parent, status, network, path, nightly, resultsPerPage)
{
}

RepoBuild::~RepoBuild()
{
}

const QString RepoBuild::urlAsset(const int assetId) const
{
  return QString(assets()->downloadUrl());
}

const QString RepoBuild::urlJobs() const
{
  return QString("%1/jobs").arg(path());
}

const QString RepoBuild::urlReleases() const
{
  return QString("%1/targets").arg(path());
}

const QString RepoBuild::urlStatus() const
{
  return QString("%1/status").arg(path());
}
