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

#include "updateinterface.h"

#include <QEventLoop>
#include <QTimer>

class UpdateCloudBuild: public UpdateInterface
{
  Q_DECLARE_TR_FUNCTIONS(UpdateCloudBuild)

  public:
    enum ProcessStatus {
      STATUS_UNKNOWN,
      STATUS_WAITING,
      STATUS_IN_PROGRESS,
      STATUS_SUCCESS,
      STATUS_ERROR,
      STATUS_TIMEOUT,
      STATUS_CANCELLED
    };
    Q_ENUM(ProcessStatus)

    explicit UpdateCloudBuild(QWidget * parent);
    virtual ~UpdateCloudBuild();

  protected:
    int asyncInstall() override;
    void assetSettingsInit() override;
    bool buildFlaggedAsset(const int row) override;

  private slots:
    void cancel();
    void checkStatus();

  private:
    QJsonObject *m_objBody;
    QJsonDocument *m_docResp;
    QEventLoop m_eventLoop;
    QTimer m_timer;

    QString m_logDir;
    QString m_buildFlags;
    QTime m_buildStartTime;
    int m_jobStatus;
    QStringList m_profileOpts;
    QString m_radio;

    bool arrayExists(const QJsonObject & parent, const QString child);
    bool objectExists(const QJsonObject & parent, const QString child);
    bool stringExists(const QJsonObject & parent, const QString child);

    bool addBuildLanguage(const QString & flag, const QJsonArray & values, QJsonArray & buildFlags);
    bool addBuildFai(const QString & flag, const QJsonArray & values, QJsonArray & buildFlags);
    void addBuildFlag(QJsonArray & buildFlags, const QString & flag, const QString & val);
    void cleanup();
    bool getStatus();
    bool isStatusInProgress();
    bool setAssetDownload();
    void waitForBuildFinish();
    static QString statusToString(const int status);
    static int cloudStatusToInt(const QString status);
};
