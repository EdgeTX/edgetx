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

#include "sdcard.h"
#include <QFile>
#include <QDir>

bool SdcardFormat::write(const RadioData & radioData)
{
  QDir dir(filename);
  dir.mkdir("RADIO");
  dir.mkdir("MODELS");
  return LabelsStorageFormat::write(radioData);
}

bool SdcardFormat::loadFile(QByteArray & filedata, const QString & filename)
{
  QString path = this->filename + "/" + filename;
  QFile file(path);
  if (!file.open(QFile::ReadOnly)) {
    setError(tr("Error opening file %1:\n%2.").arg(path).arg(file.errorString()));
    return false;
  }
  filedata = file.readAll();
  qDebug() << "File" << path << "read, size:" << filedata.size();
  return true;
}

bool SdcardFormat::writeFile(const QByteArray & data, const QString & filename)
{
  QString path = this->filename + "/" + filename;
  QFile file(path);
  if (!file.open(QFile::WriteOnly)) {
    setError(tr("Error opening file %1 in write mode:\n%2.").arg(path).arg(file.errorString()));
    return false;
  }
  file.write(data.data(), data.size());
  file.close();
  qDebug() << "File" << path << "written, size:" << data.size();
  return true;
}

bool SdcardFormat::getFileList(std::list<std::string>& filelist)
{
  QDir dir(filename);
  if (!dir.cd("MODELS")) return false;

  QStringList ql = dir.entryList();
  for (const auto& str : ql) {
    filelist.push_back("MODELS/" + str.toStdString());
  }
  return true;
}

bool SdcardFormat::deleteFile(const QString & filename)
{
  QString path = this->filename + "/" + filename;

  if (!QFile::remove(path)) {
    setError(tr("Error deleting file %1").arg(path));
    return false;
  }

  qDebug() << "File" << path << "deleted";
  return true;
}

bool SdcardStorageFactory::probe(const QString & path)
{
  return QDir(path).exists();
}
