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

#include "downloaddialog.h"
#include "ui_downloaddialog.h"
#include "constants.h"
#include "helpers.h"

#include <QTime>

DownloadDialog::DownloadDialog(QWidget *parent, QString src, QString tgt, QString contentType, QString title):
  QDialog(parent),
  ui(new Ui::DownloadDialog),
  reply(nullptr),
  file(nullptr),
  aborted(false)
{
    ui->setupUi(this);
    setWindowIcon(CompanionIcon("download.png"));
    ui->progressBar->setValue(0);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);

    if (tgt.isEmpty()) {
      setWindowTitle(src);
      return;  // just show wait dialog.
    }

    setWindowTitle(windowTitle() % title);

    file = new QFile(tgt);
    if (!file->open(QIODevice::WriteOnly)) {
      QMessageBox::critical(this, CPN_STR_APP_NAME, tr("Unable to open the download file %1 for writing.\nError: %2").arg(tgt).arg(file->errorString()));
      QTimer::singleShot(0, this, SLOT(fileError()));
    }
    else {
      url.setUrl(src);
      qDebug() << "url:" << url.url();
      request.setUrl(url);
      request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
      request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

      request.setRawHeader(QByteArray("Accept"), contentType.toUtf8());

      reply = qnam.get(request);
      connect(reply, SIGNAL(finished()), this, SLOT(httpFinished()));
      connect(reply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));
      connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateDataReadProgress(qint64, qint64)));
    }
}

DownloadDialog::~DownloadDialog()
{
  delete ui;
  delete file;
}

void DownloadDialog::reject()
{
  if (reply && reply->isRunning()) {
    aborted = true;
    reply->abort();  // this will call QNetworkReply::finished()
    return;
  }

  QDialog::reject();
}

void DownloadDialog::httpFinished()
{
  file->flush();
  file->close();

  const bool ok = !(reply->error() || aborted);

  if (!ok) {
    file->remove();
    if (!aborted)
      QMessageBox::information(this, CPN_STR_APP_NAME, tr("Download failed: %1.").arg(reply->errorString()));
  }

  reply->deleteLater();
  reply = nullptr;
  file->deleteLater();
  file = nullptr;

  if (ok)
    accept();
  else
    reject();
}

void DownloadDialog::httpReadyRead()
{
  if (file) {
    file->write(reply->readAll());
  }
}

void DownloadDialog::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
  ui->progressBar->setMaximum(totalBytes);
  ui->progressBar->setValue(bytesRead);
}

void DownloadDialog::fileError()
{
  delete file;
  file = nullptr;
  reject();
}
