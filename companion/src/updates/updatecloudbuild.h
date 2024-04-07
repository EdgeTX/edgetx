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

    explicit UpdateCloudBuild(QWidget * parent);
    virtual ~UpdateCloudBuild();

  protected:
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
    QString m_jobStatus;
    QString m_radio;
    QString m_buildFlags;

    void cleanup();
    bool getStatus();
    bool isStatusInProgress();
    bool setAssetDownload();
    void waitForBuildFinish();
};
