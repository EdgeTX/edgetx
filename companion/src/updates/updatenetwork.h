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

#include <QtCore>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>

class UpdateNetwork : public QObject
{
    Q_OBJECT

  public:

    enum DownloadDataType {
      DDT_Unknown,
      DDT_GitHub_Binary,
      DDT_GitHub_First = DDT_GitHub_Binary,
      DDT_GitHub_Content,
      DDT_GitHub_MetaData,
      DDT_GitHub_Raw,
      DDT_GitHub_SaveToFile,
      DDT_GitHub_Last = DDT_GitHub_SaveToFile,
      DDT_Build_MetaData,
      DDT_Build_First = DDT_Build_MetaData,
      DDT_Build_SaveToFile,
      DDT_Build_Last = DDT_Build_SaveToFile,
    };
    Q_ENUM(DownloadDataType)

    explicit UpdateNetwork(QObject * parent, UpdateStatus * status);
    virtual ~UpdateNetwork();

    void convertBufferToJson(QJsonDocument * json);
    void downloadMetaData(const DownloadDataType type, const QString & url, QJsonDocument * json);
    void downloadJson(const QString & url, QJsonDocument * json);
    void downloadJsonAsset(const QString & url, QJsonDocument * json);
    void downloadJsonContent(const QString & url, QJsonDocument * json);
    void downloadToFile(const DownloadDataType type, const QString & url, const QString & filePath);
    void downloadToBuffer(const DownloadDataType type, const QString & url);
    void download(const DownloadDataType type, const QString & url, const char * acceptHeader, const QString & filePath);
    QByteArray * getDownloadBuffer();
    const bool isSuccess();
    void post(const QString & action, const QString & url, QJsonDocument * json);
    bool saveBufferToFile(const QString & filePath);
    bool saveJsonDocToFile(QJsonDocument * json, const QString & filePath);
    bool saveJsonObjToFile(QJsonObject & obj, const QString & filePath);
    void submitRequest(const QString & action, const QString & url, QJsonDocument * data, QJsonDocument * response);

    static QString downloadDataTypeToString(const DownloadDataType val);

  private slots:
    void onGetFinished(QNetworkReply * reply, DownloadDataType ddt);
    void onPostFinished();
    void updateProgress();
    void cancelDownload();

  private:
    UpdateStatus *m_status;
    QNetworkAccessManager m_manager;
    QNetworkRequest m_request;
    QNetworkReply *m_reply;
    QByteArray *m_buffer;
    QFile *m_file;
    QUrl m_url;
    bool m_success;
    QEventLoop m_eventLoop;
    QTimer m_timer;
    QString m_action;

    bool init(const QString & url);
    void connectReplyCommon();
    void cleanup();
};
