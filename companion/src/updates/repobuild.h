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

#pragma once

#include "repo.h"

class RepoBuild : public Repo
{
    Q_OBJECT

  public:
    explicit RepoBuild(QObject * parent, UpdateStatus * status, UpdateNetwork * network,
                       const QString & path, const QString & nightly, const int resultsPerPage);
    virtual ~RepoBuild();

    const QString urlAsset(const int assetId) const;
    const QString urlAssets(const int releaseId) const { return ""; }
    const QString urlContent(const QString & filename) const { return ""; }
    const QString urlJobs() const;
    const QString urlReleases() const;
    const QString urlStatus() const;

    const UpdateNetwork::DownloadDataType assetDownloadDataType() const override { return UpdateNetwork::DDT_Build_SaveToFile; }
    const UpdateNetwork::DownloadDataType assetContentDataType() const override { return UpdateNetwork::DDT_Unknown; }

    const RepoRawItemModel::MetaDataType assetMetaDataType() const override { return RepoRawItemModel::MDT_Build_Asset; }
    const RepoRawItemModel::MetaDataType assetsMetaDataType() const override { return RepoRawItemModel::MDT_Build_Assets; }
    const RepoRawItemModel::MetaDataType releasesMetaDataType() const override { return RepoRawItemModel::MDT_Build_Releases; }

  private:
};
