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

class AssetsRawItemModel : public RepoRawItemModel
{
  Q_OBJECT

  public:
    explicit AssetsRawItemModel(QObject * parentRepo);
    virtual ~AssetsRawItemModel() {}

  protected:
    friend class RepoAssets;

    virtual void parseJsonObject(const QJsonObject & obj);

    bool setDownloadUrl(const int id, const QString url);
    bool setDownloadName(const int id, const QString name);
    bool resetFlags();

  private:
    QObject *m_parentRepo;  // cannot use Repo class from repo.h as compile error due to cyclic headers

    void parseJsonObjectGitHub(const QJsonObject & obj);
    void parseJsonObjectBuild();
};

class AssetsFilteredItemModel : public RepoFilteredItemModel
{
    Q_OBJECT

  public:
    explicit AssetsFilteredItemModel(QObject * parentRepo, AssetsRawItemModel * assetsRawItemModel);
    virtual ~AssetsFilteredItemModel() {};

  protected:
    friend class RepoAssets;

    const QString contentType(const int id) const;
    const QString copyFilter(const int id) const;
    bool setCopyFilter(const int id, const QString filter);
    bool setSubDirectory(const int id, const QString path);
    const QString subDirectory(const int id) const;
    const QString downloadUrl(const int id) const;
    const QString downloadName(const int id) const;

  private:
    QObject *m_parentRepo;  // cannot use Repo class from repo.h as compile error due to cyclic headers
};

class RepoAssets : public QObject, public RepoMetaData
{
    Q_OBJECT

  public slots:
    void onReleaseIdChanged(int id);

  public:
    explicit RepoAssets(QObject * parent, UpdateStatus * status, UpdateNetwork * network);
    virtual ~RepoAssets() {};

    virtual bool retrieveMetaDataAll();
    virtual bool retrieveMetaDataOne(const int id);

    bool build(const int row);
    const QString contentType() const;
    const QString copyFilter() const;
    bool downloadToBuffer(const int id = 0);
    bool downloadToFile(const int row, QString & downloadDir);
    bool getJson(QJsonDocument * json);
    bool getJson(const QString assetName, QJsonDocument * json);
    const QString name() const;
    const QString name(const int row);

    bool setCopyFilter(const QString filter);

    bool setSubDirectory(const QString path);
    const QString subDirectory() const;

    bool setDownloadUrl(const QString url);
    const QString downloadUrl() const;
    const QString downloadUrl(const int row);

    bool setDownloadName(const QString name);
    const QString downloadName() const;
    const QString downloadName(const int row);

    bool resetFlags();

  signals:
    void idChanged(int id);

  private:
    AssetsRawItemModel* const m_rawItemModel;
    AssetsFilteredItemModel* const m_filteredItemModel;
    QObject *m_parentRepo;  // cannot use Repo class from repo.h as compile error due to cyclic headers
    int m_releaseId;

    virtual void dumpItemModel(const QString modelName, const QAbstractItemModel * itemModel) const;
};
