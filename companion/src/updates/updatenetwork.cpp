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

void UpdateNetwork::cleanup()
{
  if (m_timer.isActive())
    m_timer.stop();

  disconnect(&m_timer, &QTimer::timeout, this, &UpdateNetwork::updateProgress);

  if (m_eventLoop.isRunning())
   m_eventLoop.quit();
}

void UpdateNetwork::connectReplyCommon()
{
  connect(m_reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred), [=] (QNetworkReply::NetworkError code) {
    m_success = false;
    m_status->reportProgress(tr("Network error has occurred. Error code: %1").arg(code), QtCriticalMsg);
  });

  connect(m_reply, &QNetworkReply::sslErrors, [=]() {
    m_success = false;
    m_status->reportProgress(tr("Ssl library version: %1").arg(QSslSocket::sslLibraryVersionString()), QtCriticalMsg);
  });
}

void UpdateNetwork::convertBufferToJson(QJsonDocument * json)
{
  m_success = false;
  QJsonParseError res;

  *json = QJsonDocument::fromJson(*m_buffer, &res);

  if (res.error || json->isNull()) {
    m_status->reportProgress(tr("Unable to convert downloaded metadata to json. Error:%1\n%2").arg(res.error).arg(res.errorString()), QtCriticalMsg);
    return;
  }

  m_success = true;
}

void UpdateNetwork::download(const DownloadDataType type, const QString & url, const char * acceptHeader, const QString & filePath)
{
  m_action = tr("Downloading");

  if (!init(url))
    return;

  if (type == DDT_GitHub_SaveToFile || type == DDT_Build_SaveToFile) {
    m_file = new QFile(filePath);
    if (!m_file->open(QIODevice::WriteOnly)) {
      m_status->reportProgress(tr("Unable to open the download file %1 for writing. Error: %2").arg(filePath).arg(m_file->errorString()), QtCriticalMsg);
      return;
    }
  }
  else
    m_file = nullptr;

  if (type >= DDT_GitHub_First && type <= DDT_GitHub_Last) {
    m_request.setRawHeader(QByteArray("X-GitHub-Api-Version"), GH_API_VERSION);
    m_request.setRawHeader(QByteArray("Accept"), QByteArray(acceptHeader));
  }

  // returns a new QNetworkReply object
  m_reply = m_manager.get(m_request);

  // connect to the signals of the new object
  connectReplyCommon();

  connect(m_reply, &QNetworkReply::downloadProgress, [=](const qint64 bytesRead, const qint64 totalBytes) {
    m_status->setMaximum(totalBytes);
    m_status->setValue(bytesRead);
  });

  connect(m_reply, &QNetworkReply::readyRead, [=]() {
    if (type == DDT_GitHub_SaveToFile || type == DDT_Build_SaveToFile) {
      m_file->write(m_reply->readAll());
    }
    else {
      const QByteArray qba = m_reply->readAll();
      m_buffer->append(qba);
    }
  });

  connect(m_reply, &QNetworkReply::finished, [=]() { onGetFinished(m_reply, type); });

  connect(m_status, &UpdateStatus::cancelled, this, &UpdateNetwork::cancelDownload);

  m_timer.start();
  m_eventLoop.exec();
}

//  static
QString UpdateNetwork::downloadDataTypeToString(const DownloadDataType val)
{
  switch ((int)val) {
    case DDT_GitHub_Binary:
      return "GitHub Binary";
    case DDT_GitHub_Content:
      return "GitHub Content";
    case DDT_GitHub_MetaData:
      return "GitHub Meta Data";
    case DDT_GitHub_Raw:
      return "GitHub Raw";
    case DDT_GitHub_SaveToFile:
      return "GitHub Save to file";
    case DDT_Build_MetaData:
      return "Build Meta Data";
    case DDT_Build_SaveToFile:
      return "Build Save to file";
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

void UpdateNetwork::downloadJson(const QString & url, QJsonDocument * json)
{
  downloadToBuffer(DDT_GitHub_Content, url);
  if (m_success)
    convertBufferToJson(json);
}

void UpdateNetwork::downloadJsonAsset(const QString & url, QJsonDocument * json)
{
  downloadToBuffer(DDT_GitHub_Content, url);
  if (m_success)
    convertBufferToJson(json);
}

void UpdateNetwork::downloadJsonContent(const QString & url, QJsonDocument * json)
{
  downloadToBuffer(DDT_GitHub_Raw, url);
  if (m_success)
    convertBufferToJson(json);
}

void UpdateNetwork::downloadMetaData(const DownloadDataType type, const QString & url, QJsonDocument * json)
{
  downloadToBuffer(type, url);
  if (m_success)
    convertBufferToJson(json);
}

void UpdateNetwork::downloadToBuffer(const DownloadDataType type, const QString & url)
{
  m_success = false;
  download(type,
           url,
           type == DDT_GitHub_MetaData ? GH_ACCEPT_HEADER_METADATA :
                                         type == DDT_GitHub_Raw ? GH_ACCEPT_HEADER_RAW : GH_ACCEPT_HEADER_CONTENT,
           QString());
}

void UpdateNetwork::downloadToFile(const DownloadDataType type, const QString & url, const QString & filePath)
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
        m_status->reportProgress(tr("Download cancelled by user"), QtWarningMsg);
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

  download(type,
           url,
           type == DDT_GitHub_SaveToFile ? GH_ACCEPT_HEADER_CONTENT : NULL,
           fi.absoluteFilePath());
}

QByteArray * UpdateNetwork::getDownloadBuffer()
{
  return m_buffer;
}

bool UpdateNetwork::init(const QString & urlStr)
{
  m_status->setValue(0);
  m_status->setMaximum(100);

  m_buffer->clear();
  m_success = false;
  m_request = QNetworkRequest();

  if (m_reply) {
    delete m_reply;
    m_reply = nullptr;
  }

  m_timer.setInterval(1000);
  connect(&m_timer, &QTimer::timeout, this, &UpdateNetwork::updateProgress);

  m_url.setUrl(urlStr);

  if (!m_url.isValid()) {
    m_status->reportProgress(tr("Invalid URL: %1").arg(urlStr), QtCriticalMsg);
    return false;
  }
  else
    m_status->reportProgress(tr("URL: %1").arg(urlStr), QtDebugMsg);

  m_request.setUrl(m_url);
  m_request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
  m_request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
  return true;
}

const bool UpdateNetwork::isSuccess()
{
  return m_success;
}

void UpdateNetwork::onGetFinished(QNetworkReply * reply, DownloadDataType type)
{
  cleanup();
  disconnect(m_status, &UpdateStatus::cancelled, this, &UpdateNetwork::cancelDownload);

  m_status->setValue(m_status->maximum());

  if (type == DDT_GitHub_SaveToFile || type == DDT_Build_SaveToFile) {
    m_file->flush();
    m_file->close();
  }

  if (m_reply->error()) {
    m_success = false;
    m_status->reportProgress(tr("Unable to download %1. GET error:%2\n%3").arg(downloadDataTypeToString(type)).arg(m_reply->error()).arg(m_reply->errorString()), QtCriticalMsg);

    if (type == DDT_GitHub_SaveToFile || type == DDT_Build_SaveToFile)
      m_file->remove();
  }
  else
    m_success = true;

  if (type == DDT_GitHub_SaveToFile || type == DDT_Build_SaveToFile) {
    delete m_file;
    m_file = nullptr;
  }

  if (m_reply) {
    delete m_reply;
    m_reply = nullptr;
  }
}

void UpdateNetwork::onPostFinished()
{
  cleanup();

  if (m_reply->error()) {
    m_success = false;
    m_status->reportProgress(tr("POST error: %2\n%3").arg(m_reply->error()).arg(m_reply->errorString()), QtCriticalMsg);
  }

  if (m_reply) {
    delete m_reply;
    m_reply = nullptr;
  }

  m_success = true;
}

void UpdateNetwork::post(const QString & action, const QString & url, QJsonDocument * data)
{
  m_action = action;

  if (!init(url))
    return;

  m_status->setMaximum(0);  // put progress bar into wait mode as cannot guess how long request will run

  m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  // returns a new QNetworkReply object
  m_reply = m_manager.post(m_request, data->toJson(QJsonDocument::Compact));

  // connect to the signals of the new object
  connectReplyCommon();

  connect(m_reply, &QNetworkReply::readyRead, [=]() {
    const QByteArray qba = m_reply->readAll();
    m_buffer->append(qba);
  });

  connect(m_reply, &QNetworkReply::finished, this, &UpdateNetwork::onPostFinished);

  m_timer.start();
  m_eventLoop.exec();
}

bool UpdateNetwork::saveBufferToFile(const QString & filePath)
{
  QFile f(QDir::toNativeSeparators(filePath));

  if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    m_status->reportProgress(tr("Failed to open %1 for writing").arg(QDir::toNativeSeparators(filePath)), QtWarningMsg);
    return false;
  }

  QTextStream out(&f);
  out << *m_buffer;
  f.close();

  return true;
}

bool UpdateNetwork::saveJsonDocToFile(QJsonDocument * json, const QString & filePath)
{
  QFile f(QDir::toNativeSeparators(filePath));

  if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    m_status->reportProgress(tr("Failed to open %1 for writing").arg(QDir::toNativeSeparators(filePath)), QtWarningMsg);
    return false;
  }

  QTextStream out(&f);
  out << json->toJson();
  f.close();

  return true;
}

bool UpdateNetwork::saveJsonObjToFile(QJsonObject & obj, const QString & filePath)
{
  saveJsonDocToFile(new QJsonDocument(obj), filePath);
  return true;
}

void UpdateNetwork::submitRequest(const QString & action, const QString & url, QJsonDocument * data, QJsonDocument * response)
{
  post(action, url, data);
  if (m_success)
    convertBufferToJson(response);
}

void UpdateNetwork::cancelDownload()
{
  m_status->reportProgress(tr("Download cancelled"), QtWarningMsg);
  m_reply->abort();
}

void UpdateNetwork::updateProgress()
{
  m_status->reportProgress(QString("%1...").arg(m_action), QtDebugMsg);
}
