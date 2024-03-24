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

#include "updatenetwork.h"
#include "constants.h"

#include <QMessageBox>
#include <QtNetwork/QNetworkProxyFactory>

constexpr char GH_API_VERSION[]               {"2022-11-28"};
constexpr char GH_ACCEPT_HEADER_CONTENT[]     {"application/octet-stream"};
constexpr char GH_ACCEPT_HEADER_METADATA[]    {"application/vnd.github+json"};
constexpr char GH_ACCEPT_HEADER_RAW[]         {"application/vnd.github.raw+json"};

UpdateNetwork::UpdateNetwork(QObject * parent, UpdateStatus * status) :
  QObject(parent),
  m_status(status),
  m_reply(nullptr),
  m_buffer(new QByteArray()),
  m_file(nullptr)
{
  QNetworkProxyFactory::setUseSystemConfiguration(true);
}

UpdateNetwork::~UpdateNetwork()
{
  delete m_buffer;
  if (m_reply)
    delete m_reply;
  if (m_file)
    delete m_file;
}

//  static
QString UpdateNetwork::downloadDataTypeToString(const DownloadDataType val)
{
  switch ((int)val) {
    case DDT_Binary:
      return "Binary";
    case DDT_Content:
      return "Content";
    case DDT_MetaData:
      return "Meta Data";
    case DDT_Raw:
      return "Raw";
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

void UpdateNetwork::downloadMetaData(const QString & url, QJsonDocument * json)
{
  downloadToBuffer(DDT_MetaData, url);
  if (m_success)
    convertBufferToJson(json);
}

void UpdateNetwork::downloadJson(const QString & url, QJsonDocument * json)
{
  downloadToBuffer(DDT_Content, url);
  if (m_success)
    convertBufferToJson(json);
}

void UpdateNetwork::downloadJsonAsset(const QString & url, QJsonDocument * json)
{
  downloadToBuffer(DDT_Content, url);
  if (m_success)
    convertBufferToJson(json);
}

void UpdateNetwork::downloadJsonContent(const QString & url, QJsonDocument * json)
{
  downloadToBuffer(DDT_Raw, url);
  if (m_success)
    convertBufferToJson(json);
}

void UpdateNetwork::downloadToBuffer(const DownloadDataType type, const QString & url)
{
  m_success = false;
  download(type,
           url,
           type == DDT_MetaData ? GH_ACCEPT_HEADER_METADATA :
                                  type == DDT_Raw ? GH_ACCEPT_HEADER_RAW : GH_ACCEPT_HEADER_CONTENT,
           QString());
}

void UpdateNetwork::downloadToFile(const QString & url, const QString & filePath)
{
  m_success = false;

  QFileInfo fi(filePath);

  m_status->progressMessage(tr("Downloading: %1").arg(fi.fileName()));
  m_status->reportProgress(tr("Download: %1").arg(fi.fileName()));

  if (fi.exists()) {
    m_status->reportProgress(tr("File exists: %1").arg(fi.fileName()), QtDebugMsg);
    if (m_status) {
      int reply = QMessageBox::question(m_status->progress(), CPN_STR_APP_NAME, tr("File %1 exists. Download again?").arg(fi.fileName()),
                                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No);
      if (reply == QMessageBox::Cancel) {
        return;
      }
      else if (reply == QMessageBox::No) {
        m_success = true;
        return;
      }
      else {
        ;
      }
    }
  }
  else if (!QDir().mkpath(fi.path())) {
    m_status->reportProgress(tr("Failed to create directory %1!").arg(fi.path()), QtCriticalMsg);
    return;
  }

  download(DDT_SaveToFile, url, GH_ACCEPT_HEADER_CONTENT, fi.absoluteFilePath());
}

void UpdateNetwork::download(const DownloadDataType type, const QString & urlStr, const char * header, const QString & filePath)
{
  m_status->setValue(0);
  m_status->setMaximum(100);

  m_buffer->clear();
  m_success = false;

  if (type == DDT_SaveToFile) {
    m_file = new QFile(filePath);
    if (!m_file->open(QIODevice::WriteOnly)) {
      m_status->criticalMsg(tr("Unable to open the download file %1 for writing. Error: %2").arg(filePath).arg(m_file->errorString()));
      return;
    }
  }
  else
    m_file = nullptr;

  m_url.setUrl(urlStr);

  if (!m_url.isValid()) {
    m_status->criticalMsg(tr("Invalid URL: %1").arg(urlStr));
    return;
  }
  else
    m_status->reportProgress(tr("URL: %1").arg(urlStr), QtDebugMsg);

  m_request.setUrl(m_url);
  m_request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
  m_request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

  m_request.setRawHeader(QByteArray("X-GitHub-Api-Version"), GH_API_VERSION);
  m_request.setRawHeader(QByteArray("Accept"), QByteArray(header));

  m_reply = m_manager.get(m_request);

  connect(m_reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred), [=] (QNetworkReply::NetworkError code) {
    //  leave it to the finished slot to deal with error condition
    m_status->criticalMsg(tr("Network error has occurred. Error code: %1").arg(code));
  });

  connect(m_reply, &QNetworkReply::sslErrors, [=]() {
    m_status->reportProgress(tr("Ssl library version: %1").arg(QSslSocket::sslLibraryVersionString()), QtDebugMsg);
  });

  connect(m_reply, &QNetworkReply::readyRead, [=]() {
    if (type == DDT_SaveToFile) {
      m_file->write(m_reply->readAll());
    }
    else {
      const QByteArray qba = m_reply->readAll();
      m_buffer->append(qba);
    }
  });

  connect(m_reply, &QNetworkReply::finished, [&]() {
    onDownloadFinished(m_reply, type);
  });

  connect(m_reply, &QNetworkReply::downloadProgress, [=](const qint64 bytesRead, const qint64 totalBytes) {
    m_status->setMaximum(totalBytes);
    m_status->setValue(bytesRead);
  });

  while (!m_reply->isFinished()) {
    qApp->processEvents();
  }

  return;
}

QByteArray * UpdateNetwork::getDownloadBuffer()
{
  return m_buffer;
}

const bool UpdateNetwork::isSuccess()
{
  return m_success;
}

void UpdateNetwork::onDownloadFinished(QNetworkReply * reply, DownloadDataType type)
{
  m_status->setValue(m_status->maximum());

  if (type == DDT_SaveToFile) {
    m_file->flush();
    m_file->close();
  }

  if (m_reply->error()) {
    m_success = false;
    m_status->criticalMsg(tr("Unable to download %1. Error:%2\n%3").arg(downloadDataTypeToString(type)).arg(m_reply->error()).arg(m_reply->errorString()));

    if (type == DDT_SaveToFile)
      m_file->remove();
  }
  else
    m_success = true;

  if (type == DDT_SaveToFile) {
    delete m_file;
    m_file = nullptr;
  }
}

void UpdateNetwork::convertBufferToJson(QJsonDocument * json)
{
  m_success = false;
  QJsonParseError res;

  *json = QJsonDocument::fromJson(*m_buffer, &res);

  if (res.error || json->isNull()) {
    m_status->criticalMsg(tr("Unable to convert downloaded metadata to json. Error:%1\n%2").arg(res.error).arg(res.errorString()));
    return;
  }

  m_success = true;
}
