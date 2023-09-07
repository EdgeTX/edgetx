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

#include "constants.h"
#include "appdata.h"
#include "updatestatus.h"
#include "updatenetwork.h"
#include "repo.h"
#include "updateparameters.h"

#include <QtCore>
#include <QWidget>

class UpdateInterface : public QWidget
{
    Q_OBJECT

  public:

    //  The CID is used as a key to the application settings therefore
    //  ids must be explicit so no renumbering if components removed
    //  new components must be added to the end of the enum
    enum ComponentIdentity {
      CID_Unknown         = -1,
      CID_SDCard          = 0,
      CID_Firmware        = 1,
      CID_Sounds          = 2,
      CID_Themes          = 3,
      CID_MultiProtocol   = 4,
      CID_Companion       = 5,
    };
    Q_ENUM(ComponentIdentity)

    enum UpdateFlags {
      UPDFLG_None            = 0,
      UPDFLG_Update          = 1 << 1,
      UPDFLG_Locked          = 1 << 2,
      UPDFLG_Preparation     = 1 << 3,
      UPDFLG_Download        = 1 << 4,
      UPDFLG_Decompress      = 1 << 5,
      UPDFLG_CopyDest        = 1 << 6,
      UPDFLG_CopyFiles       = 1 << 7,
      UPDFLG_CopyStructure   = 1 << 8,
      UPDFLG_Housekeeping    = 1 << 9,
      UPDFLG_AsyncInstall    = 1 << 10,
      UPDFLG_DelDownloads    = 1 << 11,
      UPDFLG_DelDecompress   = 1 << 12,
      UPDFLG_Common_Asset    = UPDFLG_Download | UPDFLG_Decompress | UPDFLG_CopyDest,
      UPDFLG_Common          = UPDFLG_Common_Asset | UPDFLG_Preparation | UPDFLG_Housekeeping,
    };
    Q_ENUM(UpdateFlags)

    explicit UpdateInterface(QWidget * parent, ComponentIdentity id, QString name);
    virtual ~UpdateInterface();

    virtual void assetSettingsSave();
    virtual const bool isReleaseLatest();
    virtual const bool isUpdateAvailable();
    virtual const bool isVersionLatest(const QString & current, const QString & latest);
    virtual const QString releaseCurrent();
    virtual const QString releaseLatest();
    virtual const QString releaseUpdate();
    virtual bool update(ProgressWidget * progress = nullptr);
    virtual const QString versionCurrent();

    const int id() const;
    const bool isUpdateable() const;
    const QString name() const;
    UpdateParameters* const params() const;
    void releaseClear();
    const QStringList releaseList();
    void resetEnvironment();
    void setReleaseChannel(const int channel);
    void setRunUpdate();

  protected:
    virtual void assetSettingsInit() = 0;
    virtual void assetSettingsLoad();
    virtual bool asyncInstall();
    virtual bool copyAsset();
    virtual bool copyToDestination();
    virtual bool decompress();
    virtual bool download();
    virtual bool flagAssets();
    virtual bool housekeeping();
    virtual bool preparation();

    bool copyFiles();
    bool copyFlaggedAssets();
    bool copyStructure();
    bool decompressAsset(int row);
    const QString decompressDir() const;
    bool decompressFlaggedAssets();
    const QString downloadDir() const;
    bool downloadFlaggedAssets();
    bool filterAssets(const UpdateParameters::AssetParams & ap);
    void init(QString repo, QString nightly = "", int resultsPerPage = -1);
    const bool isSettingsIndexValid() const;
    UpdateNetwork* const network() const;
    Repo* const repo() const;
    bool retrieveAssetsJsonFile(const QString & assetName, QJsonDocument * json);
    bool retrieveRepoJsonFile(const QString & filename, QJsonDocument * json);
    bool setFilteredAssets(const UpdateParameters::AssetParams & ap);
    void setFirmwareFlavour();
    void setLanguage();
    void setParamFolders();
    void setReleaseId(QString name);
    UpdateStatus* const status() const;
    const QString updateDir() const;

    static QStringList versionToStringList(QString version);
    static const QString updateFlagsToString(const int flags);
    static const QString updateFlagToString(const int flag);

  private:
    const ComponentIdentity m_id;
    const QString m_name;
    UpdateParameters* const m_params;
    UpdateStatus* const m_status;
    UpdateNetwork* const m_network;
    Repo* const m_repo;
    QString m_downloadDir;
    QString m_decompressDir;
    QString m_updateDir;

    void appSettingsInit();
    bool checkCreateDirectory(const QString & dirSetting, const UpdateFlags flag);
    bool decompressArchive(const QString & archivePath, const QString & destPath);
    bool releaseSettingsSave();
    bool setRunFolders();
};
