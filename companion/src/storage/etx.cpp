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

#include "etx.h"
#include "helpers.h"

#include <QFile>

#define MZ_ALLOCATION_SIZE    (32*1024)

bool EtxFormat::load(RadioData & radioData)
{
  QFile file(filename);

  if (!file.open(QFile::ReadOnly)) {
    setError(tr("Error opening file %1:\n%2.").arg(filename).arg(file.errorString()));
    return false;
  }

  QByteArray archiveContents = file.readAll();

  qDebug() << "File" << filename << "read, size:" << archiveContents.size();

  // open zip file
  memset(&zip_archive, 0, sizeof(zip_archive));
  if (!mz_zip_reader_init_mem(&zip_archive, archiveContents.data(), archiveContents.size(), 0)) {
    qDebug() << tr("Error opening EdgeTX archive %1").arg(filename);
    return false;
  }

  bool result = LabelsStorageFormat::load(radioData);
  mz_zip_reader_end(&zip_archive);
  return result;
}

bool EtxFormat::write(RadioData & radioData)
{
  qDebug() << "Saving to archive" << filename;

  memset(&zip_archive, 0, sizeof(zip_archive));
  if (!mz_zip_writer_init_heap(&zip_archive, 0, MZ_ALLOCATION_SIZE)) {
    setError(tr("Error initializing EdgeTX archive writer"));
    return false;
  }

  bool result = LabelsStorageFormat::write(radioData);
  if (result) {
    // finalize archive and get contents
    char * archiveContents;
    size_t archiveSize;
    if (mz_zip_writer_finalize_heap_archive(&zip_archive, (void **)&archiveContents, &archiveSize)) {
      qDebug() << "Archive size" << archiveSize;
      // write contents to file
      QFile file(filename);
      if (file.open(QIODevice::WriteOnly)) {
        qint64 len = file.write(archiveContents, archiveSize);
        if (len != (qint64)archiveSize) {
          setError(tr("Error writing file %1:\n%2.").arg(filename).arg(file.errorString()));
          result = false;
        }
      }
      else {
        setError(tr("Error creating EdgeTX file %1:\n%2.").arg(filename).arg(file.errorString()));
        result = false;
      }
    }
    else {
      setError(tr("Error creating EdgeTX archive"));
      result = false;
    }
  }

  mz_zip_writer_end(&zip_archive);
  return result;
}

bool EtxFormat::loadFile(QByteArray & filedata, const QString & filename, bool optional)
{
  size_t size;
  void * data = mz_zip_reader_extract_file_to_heap(&zip_archive, qPrintable(filename), &size, 0);

  if (!data) return optional;

  qDebug() << QString("Extracted file %1, size=%2").arg(filename).arg(size);
  filedata.clear();
  filedata.append((char *)data, size);
  mz_free(data);
  return true;
}

bool EtxFormat::loadImageFile(const QString & filename, bool optional)
{
  QString srcpath("IMAGES/" % filename);
  size_t size;
  void * data = mz_zip_reader_extract_file_to_heap(&zip_archive, qPrintable(srcpath), &size, 0);

  if (!data) return optional;

  QByteArray ba;
  ba.append((char *)data, size);
  mz_free(data);

  QFile imgfile(Helpers::getImagesCacheDir() % "/" % filename);

  if (imgfile.open(QIODevice::WriteOnly)) {
    if (imgfile.write(ba) != (qint64)size) {
      qDebug() << "Image data written does not match expected";
      return false;
    }

    imgfile.close();
  } else {
    setError(tr("Unable to write %1 to cache").arg(filename));
    return false;
  }

  qDebug() << QString("Extracted file %1, size=%2").arg(srcpath).arg(size);
  return true;
}

bool EtxFormat::writeFile(const QByteArray & filedata, const QString & filename)
{
  if (!mz_zip_writer_add_mem(&zip_archive, filename.toStdString().c_str(), filedata.data(), filedata.size(), MZ_DEFAULT_LEVEL)) {
    setError(tr("Error adding %1 to archive").arg(filename));
    return false;
  }

  qDebug() << "File" << filename << "written, size:" << filedata.size();
  return true;
}

bool EtxFormat::writeImageFile(const QString & filename)
{
  QString srcpath(Helpers::getImagePath(filename));

  if (!srcpath.isEmpty()) {
    QByteArray ba;
    QFile imgfile(srcpath);

    if (imgfile.open(QIODevice::ReadOnly)) {
      ba.append(imgfile.readAll());
      imgfile.close();
    } else {
      setError(tr("Unable to open image %1").arg(filename));
      return false;
    }

    QString destpath("IMAGES/" % filename);

    if (!mz_zip_writer_add_mem(&zip_archive, destpath.toStdString().c_str(), ba.data(), ba.size(), MZ_DEFAULT_LEVEL)) {
      setError(tr("Error adding file %1").arg(destpath));
      return false;
    }

    qDebug() << "File" << destpath << "written, size:" << ba.size();
    return true;
  }

  return false;
}

bool EtxFormat::getFileList(std::list<std::string>& filelist)
{
  int count = (int)mz_zip_reader_get_num_files(&zip_archive);
  if (count == 0) return false;

  mz_zip_archive_file_stat file_stat;
  for (int i=0; i<count; i++) {
    if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) continue;
    if (mz_zip_reader_is_file_a_directory(&zip_archive, i)) continue;
    filelist.push_back(file_stat.m_filename);
  }

  return true;
}
