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

#include "repodatamodels.h"
#include "repometadata.h"

class ReleasesItemModels
{
  public:
    explicit ReleasesItemModels() {}
    virtual ~ReleasesItemModels() {}

    enum ReleaseFlag {
      RELFLG_None        = 0,
      RELFLG_Stable      = 1 << 1,
      RELFLG_PreRelease  = 1 << 2,
      RELFLG_Nightly     = 1 << 3,
    };

    static const QString flagsToString(const int flags);
    static const QString flagToString(const int flag);
};

class ReleasesRawItemModel : public RepoRawItemModel, public ReleasesItemModels
{
  Q_OBJECT

  public:
    explicit ReleasesRawItemModel(QObject * parentRepo);
    virtual ~ReleasesRawItemModel() {}

  protected:
    friend class RepoReleases;

    virtual void parseJsonObject(const QJsonObject & obj);

  private:
    QObject *m_parentRepo;  // cannot use Repo class from repo.h as compile error due to cyclic headers

    void parseJsonObjectGitHub(const QJsonObject & obj);
    void parseJsonObjectBuild(const QJsonObject & obj);
};

class ReleasesFilteredItemModel: public RepoFilteredItemModel, public ReleasesItemModels
{
    Q_OBJECT

  public:
    explicit ReleasesFilteredItemModel(QObject * parentRepo, ReleasesRawItemModel * releasesRawItemModel);
    virtual ~ReleasesFilteredItemModel() {}

  protected:
    friend class RepoReleases;

    const int channelLatestId() const;
    void setChannel(const int channel);
    const QString version(const int id) const;

  private:
    QObject *m_parentRepo;  // cannot use Repo class from repo.h as compile error due to cyclic headers
};

class RepoReleases : public QObject, public RepoMetaData
{
    Q_OBJECT

  public:
    explicit RepoReleases(QObject * parentRepo, UpdateStatus * status, UpdateNetwork * network);
    virtual ~RepoReleases() {}

    int setId(const int id) override;
    bool retrieveMetaDataAll() override;
    bool retrieveMetaDataOne(const int id) override { return false; } // not implemented

    int channelLatestId();
    void setChannel(const int channel);
    const QString version() const;

  signals:
    void idChanged(int id);

  private:
    ReleasesRawItemModel* const m_rawItemModel;
    ReleasesFilteredItemModel* const m_filteredItemModel;
    QObject *m_parentRepo;  // cannot use Repo class from repo.h as compile error due to cyclic headers

    void dumpItemModel(const QString modelName, const QAbstractItemModel * itemModel) const override;
};
