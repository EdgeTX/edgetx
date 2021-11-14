/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "otx.h"
#include "miniz.c"
#include <QFile>

#define MZ_ALLOCATION_SIZE    (32*1024)

bool OtxFormat::load(RadioData & radioData)
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
    qDebug() << tr("Error opening OTX archive %1").arg(filename);
    return false;
  }

  bool result = CategorizedStorageFormat::load(radioData);
  mz_zip_reader_end(&zip_archive);
  return result;
}

bool OtxFormat::write(const RadioData & radioData)
{
  qDebug() << "Saving to archive" << filename;

  memset(&zip_archive, 0, sizeof(zip_archive));
  if (!mz_zip_writer_init_heap(&zip_archive, 0, MZ_ALLOCATION_SIZE)) {
    setError(tr("Error initializing OTX archive writer"));
    return false;
  }

  bool result = CategorizedStorageFormat::write(radioData);
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
        setError(tr("Error creating OTX file %1:\n%2.").arg(filename).arg(file.errorString()));
        result = false;
      }
    }
    else {
      setError(tr("Error creating OTX archive"));
      result = false;
    }
  }

  mz_zip_writer_end(&zip_archive);
  return result;
}

bool OtxFormat::loadFile(QByteArray & filedata, const QString & filename)
{
  size_t size;
  void * data = mz_zip_reader_extract_file_to_heap(&zip_archive, qPrintable(filename), &size, 0);
  if (!data) {
    return false;
  }

  qDebug() << QString("Extracted file %1, size=%2").arg(filename).arg(size);
  filedata.clear();
  filedata.append((char *)data, size);
  mz_free(data);
  return true;
}

bool OtxFormat::writeFile(const QByteArray & filedata, const QString & filename)
{
  if (!mz_zip_writer_add_mem(&zip_archive, filename.toStdString().c_str(), filedata.data(), filedata.size(), MZ_DEFAULT_LEVEL)) {
    setError(tr("Error adding %1 to OTX archive").arg(filename));
    return false;
  }

  return true;
}
