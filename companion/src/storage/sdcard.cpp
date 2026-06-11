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
#include "helpers.h"

#include <QFile>
#include <QDir>

bool SdcardFormat::write(RadioData & radioData)
{
  QDir dir(filename);
  dir.mkdir("RADIO");
  dir.mkdir("MODELS");
  dir.mkdir("IMAGES");
  return LabelsStorageFormat::write(radioData);
}

bool SdcardFormat::loadFile(QByteArray & filedata, const QString & filename, bool optional)
{
  filedata.clear(); // in case buffer is being reused
  QString path = this->filename + "/" + filename;
  QFile file(path);

  if (!file.exists()) return optional;

  if (!file.open(QFile::ReadOnly)) {
    fatalMsg(tr("Error opening file %1:\n%2.").arg(filename).arg(file.errorString()));
    return false;
  }

  filedata = file.readAll();

  if (filedata.isEmpty()) {
    fatalMsg(tr("File empty: %1").arg(filename));
    return false;
  }

  statusMsg(tr("File loaded: %1").arg(filename));
  return true;
}

bool SdcardFormat::loadImageFile(const QString & filename, bool optional)
{
  QString srcpath = this->filename + "/IMAGES/" + filename;
  QFile file(srcpath);

  if (!file.exists()) return optional;

  if (QDir(Helpers::getImagesCacheDir()).exists()) {
    QString destpath(Helpers::getImagesCacheDir() % "/" % filename);
    QFile destfile(destpath);

    // QFile::copy fails if destination file exists
    if (destfile.exists()) {
      if (destfile.remove()) {
        //qDebug() << "Deleted file from image cache" << filename;
      } else {
        fatalMsg(tr("Unable to delete cached").arg(filename));
        return false;
      }
    }

    if (file.copy(Helpers::getImagesCacheDir() % "/" % filename)) {
      // qDebug() << "Cached image" << filename;
    } else {
      fatalMsg(tr("Failed to cache image: %1").arg(filename));
      return false;
    }
  } else {
    fatalMsg(tr("No application image cache"));
    return false;
  }

  return true;
}

bool SdcardFormat::writeFile(const QByteArray & data, const QString & filename)
{
  QString path = this->filename + "/" + filename;
  QFile file(path);

  if (!file.open(QFile::WriteOnly)) {
    fatalMsg(tr("Error opening file %1 in write mode:\n%2.").arg(path).arg(file.errorString()));
    return false;
  }

  qint64 writesz = file.write(data.data(), data.size());
  file.close();

  if (writesz != data.size()) {
    fatalMsg(tr("Error writing file %1: wrote %2 of %3 bytes").arg(path).arg(writesz).arg(data.size()));
    return false;
  }

  statusMsg(tr("File written: %1").arg(path));
  return true;
}

bool SdcardFormat::writeImageFile(const QString & filename)
{
  QString srcpath(Helpers::getImagePath(filename));

  if (!srcpath.isEmpty()) {
    QFileInfo srcinfo(srcpath);
    QString destpath(this->filename + "/IMAGES/" + filename);
    QFileInfo destinfo(destpath);

    if (destinfo.exists()) {
      // for performance reasons avoid overwriting the same image
      if (destinfo.size() == srcinfo.size()) {
        statusMsg(tr("Duplicate image file: %1 skipped").arg(filename));
        return true;
      }
      // Size differs, remove the existing file before copying
      if (!QFile::remove(destpath)) {
        fatalMsg(tr("Error removing existing image file: %1").arg(filename));
        return false;
      }
    }

    if (!QFile(srcpath).copy(destpath)) {
      fatalMsg(tr("Error writing image file: %1").arg(filename));
      return false;
    }

    statusMsg(tr("Image written: %1").arg(filename));
    return true;
  }

  fatalMsg(tr("No application image cache"));
  return false;
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
    fatalMsg(tr("Error deleting file: %1").arg(path));
    return false;
  }

  statusMsg(tr("Deleted file: %1").arg(path));
  return true;
}

/*

  class SdcardStorageFactory
*/

bool SdcardStorageFactory::probe(const QString & path)
{
  return QDir(path).exists();
}
