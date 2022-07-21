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

#include "repomodels.h"
#include "constants.h"
#include "progresswidget.h"

#include <QtCore>
#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

class UpdateParameters : public QWidget
{
    Q_OBJECT

  public:
    enum UpdateFilterType {
      UFT_None,
      UFT_Exact,
      UFT_Startswith,
      UFT_Endswith,
      UFT_Contains,
      UFT_Expression,
    };

    static QStringList updateFilterTypeList();

    static QString updateFilterTypeToString(UpdateFilterType uft);

    struct AssetParams{
      int flags;
      UpdateFilterType filterType;
      QString filter;
      int maxExpected;
      QString destSubDir;
      UpdateFilterType copyFilterType;
      QString copyFilter;
    };

    struct GeneralParams{
      int flags;
      QString currentRelease;
      QString updateRelease;
      QString downloadDir;
      bool decompressUseDwnld;
      QString decompressDir;
      bool updateUseSD;
      QString updateDir;
      QVector<AssetParams> assets;
    };

    GeneralParams data;

    UpdateParameters(QWidget * parent);
    virtual ~UpdateParameters() {}

    AssetParams & addAsset();
    static QString buildFilterPattern(const UpdateFilterType filterType, const QString & filter);

    UpdateParameters& operator=(const UpdateParameters& source);
};

class UpdateInterface : public QWidget
{
  Q_OBJECT

  public:

    enum DownloadDataType {
      DDT_Binary,
      DDT_Content,
      DDT_SaveToFile,
      DDT_MetaData,
    };
    Q_ENUM(DownloadDataType)

    enum UpdateFlags {
      UPDFLG_None            = 0,
      UPDFLG_Update          = 1 << 1,
      UPDFLG_Locked          = 1 << 2,
      UPDFLG_Preparation     = 1 << 3,
      UPDFLG_Download        = 1 << 4,
      UPDFLG_DelDownload     = 1 << 5,
      UPDFLG_Decompress      = 1 << 6,
      UPDFLG_DelDecompress   = 1 << 7,
      UPDFLG_CopyDest        = 1 << 8,
      UPDFLG_CopyFiles       = 1 << 9,
      UPDFLG_CopyStructure   = 1 << 10,
      UPDFLG_Housekeeping    = 1 << 11,
      UPDFLG_AsyncInstall    = 1 << 12,
      UPDFLG_Common          = UPDFLG_Preparation | UPDFLG_Download | UPDFLG_Decompress | UPDFLG_CopyDest | UPDFLG_Housekeeping,
    };
    Q_ENUM(UpdateFlags)

    explicit UpdateInterface(QWidget * parent);
    virtual ~UpdateInterface();

    const QString getName() const { return name; }

  protected:
    friend class UpdateFactories;

    ReleasesMetaData *releases;
    AssetsMetaData *assets;
    UpdateParameters *dfltParams;
    UpdateParameters *runParams;
    ProgressWidget *progress;

    QString name;
    int settingsIdx;
    int resultsPerPage;
    QString downloadDir;
    QString decompressDir;
    QString updateDir;
    QString fwFlavour;
    QString language;
    int logLevel;

    virtual bool update();
    virtual bool preparation();
    virtual bool flagAssets();
    virtual bool download();
    virtual bool decompress();
    virtual bool copyToDestination();
    virtual bool copyAsset();
    virtual bool housekeeping();
    virtual bool asyncInstall();

    void setName(QString name);
    void setRepo(QString repo) { releases->setRepo(repo); assets->setRepo(repo); }
    void setResultsPerPage(int cnt) { resultsPerPage = cnt; }
    void setReleasesNightlyName(QString name) { releases->setNightlyName(name); }

    void initParamFolders(UpdateParameters * params);
    const UpdateParameters * const getDefaultParams() { return dfltParams; }
    UpdateParameters * getRunParams() { return runParams; }
    void resetRunEnvironment();
    void setRunUpdate() { runParams->data.flags |= UPDFLG_Update; }

    const QStringList getReleases();

    virtual const bool isUpdateAvailable();

    virtual bool autoUpdate(ProgressWidget * progress = nullptr);
    virtual bool manualUpdate(ProgressWidget * progress = nullptr);

    virtual const QString currentVersion();

    void clearRelease();

    virtual const QString currentRelease();
    virtual const QString updateRelease();
    virtual const bool isLatestRelease();
    virtual const bool isLatestVersion(const QString & current, const QString & latest);
    virtual const QString latestRelease();

    void getRadioProfileSettings();

    bool repoReleasesMetaData();
    bool repoReleaseAssetsMetaData();

    bool downloadReleasesMetaData();
    bool downloadReleaseLatestMetaData();
    bool downloadReleaseMetaData(const int releaseId);

    bool downloadReleaseAssetsMetaData(const int releaseId);
    bool downloadAssetMetaData(const int assetId);
    bool getSetAssets(const int flags, const UpdateParameters::UpdateFilterType assetFilterType, const QString & assetsFilter,
                      const int maxAssetsExpected, const QString & subDirectory,
                      const UpdateParameters::UpdateFilterType copyFilterType, const QString & copyFilter);
    bool downloadAsset(int row);
    bool downloadFlaggedAssets();
    bool decompressAsset(int row);
    bool decompressFlaggedAssets();
    bool copyFlaggedAssets();
    bool copyStructure();
    bool copyFiles();

    bool downloadTextFileToBuffer(const QString & path);
    void downloadFileToBuffer(const QString & url);
    QByteArray * getDownloadBuffer() { return buffer; }

    bool convertDownloadToJson(QJsonDocument * json);

    bool decompressArchive(const QString & archivePath, const QString & destPath);
    bool saveReleaseSettings();
    void reportProgress(const QString & text, const int type = QtInfoMsg);
    void progressMessage(const QString & text);
    void criticalMsg(const QString & msg);
    static QString downloadDataTypeToString(DownloadDataType val);
    static QString updateFlagsToString(UpdateFlags val);

  private slots:
    void onDownloadFinished(QNetworkReply * reply, DownloadDataType ddt, int subtype);

  private:
    QNetworkAccessManager manager;
    QNetworkRequest request;
    QNetworkReply *reply;
    QByteArray *buffer;
    QFile *file;
    QUrl url;

    bool downloadSuccess;

    static QString semanticVersion(QString version);

    int getSettingsIdx();
    void setSettingsIdx();
    bool setRunFolders();
    bool checkCreateDirectory(const QString & dirSetting, const UpdateFlags flag);

    void downloadMetaData(const MetaDataType mdt, const QString & url);
    void download(const DownloadDataType type, const int subtype, const QString & urlStr, const char * header, const QString & filePath);
    void downloadBinaryToFile(const QString & url, const QString & filename);

    void parseMetaData(int mdt);
};

class UpdateFactoryInterface
{
  public:
    explicit UpdateFactoryInterface() {}
    virtual ~UpdateFactoryInterface() {}
    virtual UpdateInterface * instance() = 0;
    virtual QString name() = 0;
};

template <class T>
class UpdateFactory : public UpdateFactoryInterface
{
  public:
    explicit UpdateFactory(QWidget * parent) :
      UpdateFactoryInterface(),
      m_instance(new T(parent)) {}

    virtual ~UpdateFactory() {}

    virtual UpdateInterface * instance() { return m_instance; }
    virtual QString name() { return m_instance->getName(); }

  private:
    UpdateInterface *m_instance;
};

class UpdateFactories : public QWidget
{
  Q_OBJECT

  public:
    explicit UpdateFactories(QWidget * parent = nullptr);
    virtual ~UpdateFactories();

    void registerUpdateFactory(UpdateFactoryInterface * factory);
    void registerUpdateFactories();
    void unregisterUpdateFactories();

    const UpdateParameters * const getDefaultParams(const QString & name);
    UpdateParameters * const getRunParams(const QString & name);
    void resetRunEnvironment(const QString & name);
    void resetAllRunEnvironments();
    void setRunUpdate(const QString & name);
    const QMap<QString, int> sortedComponentsList();

    void clearRelease(const QString & name);
    const QString currentRelease(const QString & name);
    const QString updateRelease(const QString & name);
    const bool isLatestRelease(const QString & name);
    const QString latestRelease(const QString & name);
    const QStringList releases(const QString & name);

    bool autoUpdate(ProgressWidget * progress = nullptr);
    bool manualUpdate(ProgressWidget * progress = nullptr);
    const bool updatesAvailable();

  private:
    QVector<UpdateFactoryInterface *> registeredUpdateFactories;
};
