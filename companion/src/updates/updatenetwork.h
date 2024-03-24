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
      DDT_Binary,
      DDT_Content,
      DDT_SaveToFile,
      DDT_MetaData,
      DDT_Raw,
    };
    Q_ENUM(DownloadDataType)

    explicit UpdateNetwork(QObject * parent, UpdateStatus * status);
    virtual ~UpdateNetwork();

    void convertBufferToJson(QJsonDocument * json);
    void downloadMetaData(const QString & url, QJsonDocument * json);
    void downloadJson(const QString & url, QJsonDocument * json);
    void downloadJsonAsset(const QString & url, QJsonDocument * json);
    void downloadJsonContent(const QString & url, QJsonDocument * json);
    void downloadToFile(const QString & url, const QString & filePath);
    void downloadToBuffer(const DownloadDataType type, const QString & url);
    void download(const DownloadDataType type, const QString & urlStr, const char * header, const QString & filePath);
    QByteArray * getDownloadBuffer();
    const bool isSuccess();

    static QString downloadDataTypeToString(const DownloadDataType val);

  private slots:
    void onDownloadFinished(QNetworkReply * reply, DownloadDataType ddt);

  private:
    UpdateStatus *m_status;
    QNetworkAccessManager m_manager;
    QNetworkRequest m_request;
    QNetworkReply *m_reply;
    QByteArray *m_buffer;
    QFile *m_file;
    QUrl m_url;
    bool m_success;

};
