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

#include "updatestatus.h"
#include "updatenetwork.h"
#include "reporeleases.h"
#include "repoassets.h"

#include <QtCore>

constexpr char GH_API_REPOS[]         {"https://api.github.com/repos"};
constexpr char GH_API_REPOS_EDGETX[]  {"https://api.github.com/repos/EdgeTX"};

class Repo : public QObject
{
    Q_OBJECT

  public:
    explicit Repo(QObject * parent, UpdateStatus * status, UpdateNetwork * network);
    virtual ~Repo();

    RepoAssets* const assets() const { return m_assets; }
    bool getJson(const QString filename, QJsonDocument * json);
    void init(const QString & repoPath, const QString & nightly, const int resultsPerPage);
    const QString path() const { return m_path; }
    RepoReleases* const releases() const { return m_releases; }

  private:
    UpdateStatus* const m_status;
    UpdateNetwork* const m_network;
    RepoReleases* const m_releases;
    RepoAssets* const m_assets;
    QString m_path;
};
