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

#include <QtCore>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

constexpr char GITHUB_API_REPOS[]             {"https://api.github.com/repos"};
constexpr char GH_REPOS_EDGETX[]              {"https://api.github.com/repos/EdgeTX"};

constexpr char GH_ACCEPT_HEADER_BINARY[]      {"application/octet-stream"};
constexpr char GH_ACCEPT_HEADER_METADATA[]    {"application/vnd.github.v3+json"};
constexpr char GH_ACCEPT_HEADER_RAW[]         {"application/vnd.github.v3.raw"};

enum MetaDataType {
  MDT_Release,
  MDT_Releases,
  MDT_Asset,
  MDT_ReleaseAssets
};

QString metaDataTypeToString(MetaDataType val);

class UpdatesItemModel : public QStandardItemModel
{
  Q_OBJECT

  public:
    enum ItemModelDataRoles {
      IMDR_Id = Qt::UserRole,
      IMDR_Date,
      IMDR_Tag,
      IMDR_Prerelease,
      IMDR_Content,
      IMDR_Available,
      IMDR_Flags,
      IMDR_CopyFilter,
      IMDR_SubDirectory
    };
    Q_ENUM(ItemModelDataRoles)

    explicit UpdatesItemModel(QString name = "") :
      QStandardItemModel(nullptr),
      json(nullptr),
      m_name(name) {}

    virtual ~UpdatesItemModel() {}

  protected:
    QJsonDocument * json;

  private:
    QString m_name;
};

class ReleasesItemModel : public UpdatesItemModel
{
  Q_OBJECT

  friend class ReleasesMetaData;

  public:
    explicit ReleasesItemModel();
    virtual ~ReleasesItemModel() {}

    void setNightlyName(const QString name) { m_nightlyName = name.toLower(); }
    void setSettingsIndex(const int index) { m_settingsIdx = index; }
    void setReleaseChannel(const int channel);

    void parseMetaData(const int mdt, QJsonDocument * json);
    int settingsIndex() { return m_settingsIdx; }
    bool refreshRequired() { return m_refreshRequired; }

  private:
    int m_settingsIdx;
    QString m_nightlyName;
    int m_releaseChannel;
    bool m_refreshRequired;

    void setDynamicItemData(QStandardItem * item);
    void update();
    bool isReleaseAvailable(QStandardItem * item);

    void parseRelease();
    void parseReleases();
    void parseJsonObject(const QJsonObject & obj);
};

class AssetsItemModel : public UpdatesItemModel
{
  Q_OBJECT

  friend class AssetsMetaData;

  public:
    explicit AssetsItemModel();
    virtual ~AssetsItemModel() {}

    void parseMetaData(int mdt, QJsonDocument * json);

  private:
    void parseAsset();
    void parseReleaseAssets();
    void parseJsonObject(const QJsonObject & obj);
};

class UpdatesFilteredItemModel: public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    explicit UpdatesFilteredItemModel(UpdatesItemModel * sourceModel, QString name = "");
    virtual ~UpdatesFilteredItemModel() {};

    void dumpContents();
    QStringList list();

    int id(const int row);
    int id(const QVariant value, const Qt::MatchFlags flags = Qt::MatchExactly, const int role = Qt::DisplayRole);
    int rows() { return rowCount(); }

    QString date(const int id) { return metaDataValue(id, UpdatesItemModel::IMDR_Date).toString(); }
    QString name(const int id) { return metaDataValue(id, Qt::DisplayRole).toString(); }

  public slots:
    void setFilterFlags(const int flags);
    void setFilterPattern(const QString & pattern);

  signals:

  protected:
    int filterFlags;
    QRegularExpression filterText;

    bool filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const override;

    QMap<int, QVariant> itemMetaData(int id);
    QVariant metaDataValue(const int id, const int role);
    bool setMetaDataValue(const int id, const int role, const QVariant value);

  private:
    QString m_name;
};

class ReleasesFilteredItemModel: public UpdatesFilteredItemModel
{
    Q_OBJECT

  public:

  protected:
    friend class ReleasesMetaData;

    explicit ReleasesFilteredItemModel(UpdatesItemModel * sourceModel);
    virtual ~ReleasesFilteredItemModel() {};

    int channelLatestId();

    bool prerelease(const int id) { return metaDataValue(id, UpdatesItemModel::IMDR_Prerelease).toBool(); }
    QString version(const int id) { return metaDataValue(id, UpdatesItemModel::IMDR_Tag).toString(); }

  private:
};

class AssetsFilteredItemModel : public UpdatesFilteredItemModel
{
    Q_OBJECT

  public:

  protected:
    friend class AssetsMetaData;

    explicit AssetsFilteredItemModel(UpdatesItemModel * sourceModel);
    virtual ~AssetsFilteredItemModel() {};

    bool setFlags(const int id, const int flags);
    bool setSubDirectory(const int id, const QString path);
    bool setCopyFilter(const int id, const QString filter);

    QString filename(int id) { return metaDataValue(id, Qt::DisplayRole).toString(); }

  private:
};

class RepoMetaData : public QObject
{
    Q_OBJECT
  public:
    explicit RepoMetaData(QObject * parent);
    virtual ~RepoMetaData() {};

    const QString urlReleases() { return QString("%1/releases").arg(m_repo); }
    const QString urlReleaseLatest() { return QString("%1/latest").arg(urlReleases()); }
    const QString urlContent(const QString & path) { return QString("%1/contents/%2").arg(m_repo).arg(path); }

  protected:
    QString m_repo;
    int m_resultsPerPage;
};

class ReleasesMetaData : public RepoMetaData
{
    Q_OBJECT
  public:
    explicit ReleasesMetaData(QObject * parent);
    virtual ~ReleasesMetaData() {};

    void init(const QString repo, const QString nightly, const int settingsIndex, const int resultsPerPage);

    bool refreshRequired();
    void setReleaseChannel(const int channel) { itemModel->setReleaseChannel(channel); }

    void setId(int id) { m_id = id; }
    int id() { return m_id; }

    int getSetId();
    int getSetId(int row);
    int getSetId(QVariant value, Qt::MatchFlags flags = Qt::MatchExactly, int role = Qt::DisplayRole);

    void parseMetaData(int mdt, QJsonDocument * jsonDoc) { itemModel->parseMetaData(mdt, jsonDoc); }

    int count() { return filteredItemModel->rows(); }
    QStringList list() { return filteredItemModel->list(); }
    void dumpContentsFiltered() { filteredItemModel->dumpContents(); }

    QString date() { return filteredItemModel->date(m_id); }
    QString name();
    bool prerelease() { return filteredItemModel->prerelease(m_id); }
    QString version();

    const QString urlRelease() { return QString("%1/%2").arg(urlReleases()).arg(m_id); }

  private:
    ReleasesItemModel *itemModel;
    ReleasesFilteredItemModel *filteredItemModel;
    int m_id;
};

class AssetsMetaData : public RepoMetaData
{
    Q_OBJECT
  public:
    explicit AssetsMetaData(QObject * parent);
    virtual ~AssetsMetaData() {};

    void init(const QString repo, const int resultsPerPage);

    void setId(int id) { m_id = id; }
    int id() { return m_id; }

    int getSetId(int row);
    int getSetId(QVariant value, Qt::MatchFlags flags = Qt::MatchExactly, int role = Qt::DisplayRole);

    void parseMetaData(int mdt, QJsonDocument * jsonDoc) { itemModel->parseMetaData(mdt, jsonDoc); }

    bool setFlags(int flags) { return filteredItemModel->setFlags(m_id, flags); }

    void setFilterFlags(int flags) { filteredItemModel->setFilterFlags(flags); }
    void setFilterPattern(const QString & pattern) { filteredItemModel->setFilterPattern(pattern); }

    bool setSubDirectory(const QString path) { return filteredItemModel->setSubDirectory(m_id, path); }
    bool setCopyFilter(const QString filter) { return filteredItemModel->setCopyFilter(m_id, filter); }

    int count() { return filteredItemModel->rows(); }
    QStringList list() { return filteredItemModel->list(); }
    void dumpContentsFiltered() { filteredItemModel->dumpContents(); }

    QString date() { return filteredItemModel->date(m_id); }
    QString name() { return filteredItemModel->name(m_id); }
    QString filename() { return filteredItemModel->metaDataValue(m_id, Qt::DisplayRole).toString(); }
    QString contentType() { return filteredItemModel->metaDataValue(m_id, UpdatesItemModel::IMDR_Content).toString(); }
    QString subDirectory() { return filteredItemModel->metaDataValue(m_id, UpdatesItemModel::IMDR_SubDirectory).toString(); }
    QString copyFilter() { return filteredItemModel->metaDataValue(m_id, UpdatesItemModel::IMDR_CopyFilter).toString(); }
    int flags() { return filteredItemModel->metaDataValue(m_id, UpdatesItemModel::IMDR_Flags).toInt(); }

    const QString urlReleaseAssets(int releaseId) {
                                   return QString("%1/%2/assets").arg(urlReleases()).arg(releaseId) % (m_resultsPerPage > -1 ?
                                   QString("\?per_page=%1").arg(m_resultsPerPage) : ""); }
    const QString urlAsset() { return QString("%1/assets/%2").arg(urlReleases()).arg(m_id); }

  private:
    AssetsItemModel *itemModel;
    AssetsFilteredItemModel *filteredItemModel;
    int m_id;
};
