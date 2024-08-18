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

#include "minizinterface.h"
#include "progresswidget.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QDebug>
#include <QWidget>
#include <QApplication>

MinizInterface::MinizInterface(ProgressWidget * progress, const ProgressCalcMethod progressMethod, const int & logLevel) :
  QObject(nullptr),
  progress(progress),
  progressMethod(progressMethod),
  progressValue(0),
  logLevel(logLevel),
  path(""),
  archiveFile(""),
  stopping(false)
{
  reportProgress(tr("Progress calculation method: file %1").arg(progressMethod == PCM_COUNT ? tr("count") : tr("size")), QtDebugMsg);
}

MinizInterface::~MinizInterface()
{

}

bool MinizInterface::zipPathToFile(const QString & sourcePath, const QString & archiveFile, bool append)
{
  if (progress)
    progress->setInfo(tr("Compressing %1 to %2 with append %3").arg(sourcePath, archiveFile, append ? "true" : "false"));
  else
    qDebug() << QString("Compressing %1 to %2 with append %3").arg(sourcePath, archiveFile, append ? "true" : "false");

  reportProgress(tr("Miniz version: %1").arg(MZ_VERSION), QtDebugMsg);

  progressValue = 0;
  path = sourcePath;
  this->archiveFile = archiveFile;
  mz_zip_zero_struct(&zip_archive);

  QFileInfo afi(archiveFile);

  if (afi.exists() && !append)
    reportProgress(tr("Existing archive will be overwritten"), QtCriticalMsg);

  if (afi.exists() && append) {
    if (!mz_zip_reader_init_file(&zip_archive, qPrintable(archiveFile), 0)) {
      reportProgress(tr("Unable to open existing archive"), QtFatalMsg);
      return false;
    }
    else
      reportProgress(tr("Existing archive opened"), QtDebugMsg);

    if (!mz_zip_writer_init_from_reader(&zip_archive, qPrintable(archiveFile))) {
      reportProgress(tr("Unable to write to existing archive"), QtFatalMsg);
      return false;
    }
    else
      reportProgress(tr("Existing archive initialised"), QtDebugMsg);
  }
  else {
    if (!mz_zip_writer_init_file(&zip_archive, qPrintable(archiveFile), 0)) {
      reportProgress(tr("Failure to initialise archive"), QtFatalMsg);
      return false;
    }
    else
      reportProgress(tr("Archive initialised"), QtDebugMsg);
  }

  QFileInfo pfi(path);

  int fileCount = 0;

  if (pfi.isDir()) {
    if (progressMethod == PCM_COUNT) {
      reportProgress(tr("Calculating number of items to archive"), QtDebugMsg);

      QDirIterator itcnt(path, QDirIterator::Subdirectories);

      while (itcnt.hasNext()) {
        fileCount++;
      }
    }

    if (progress) {
      if (progressMethod == PCM_COUNT)
        progress->setMaximum(fileCount);
      else
        progress->setMaximum(pfi.size());
    }

    fileCount = 0;
    QDirIterator it(path, QDirIterator::Subdirectories);

    while (it.hasNext()) {
      it.next();
      if (!addFileToArchive(it.filePath()))
        return false;
    }
  }
  else {
    fileCount = 1;

    if (progress) {
      if (progressMethod == PCM_COUNT)
        progress->setMaximum(fileCount);
      else
        progress->setMaximum(pfi.size());
    }

    if (!addFileToArchive(path))
      return false;
  }

  if (!mz_zip_writer_finalize_archive(&zip_archive)) {
    mz_zip_writer_end(&zip_archive);
    reportProgress(tr("Failure to finalise archive"), QtFatalMsg);
    return false;
  }

  mz_zip_writer_end(&zip_archive);

  if (progress)
    progress->setValue(progress->maximum());

  reportProgress(tr("Compress complete"));

  return true;
}

bool MinizInterface::addFileToArchive(const QString & path)
{
  if (!mz_zip_writer_add_file(&zip_archive, qPrintable(archiveFile), qPrintable(path), NULL, 0, MZ_DEFAULT_COMPRESSION)) {
    mz_zip_writer_end(&zip_archive);
    reportProgress(tr("Failure to add %1").arg(path), QtFatalMsg);
    return false;
  }

  if (progressMethod == PCM_COUNT)
    ++progressValue;
  else
    progressValue += QFileInfo(path).size();

  if (progress)
    progress->setValue(progressValue);

  reportProgress(tr("Added file: %1").arg(path), QtDebugMsg);

  return true;
}

bool MinizInterface::unzipArchiveToPath(const QString & archiveFile, const QString & destinationPath)
{
  QFileInfo file(archiveFile);

  if (progress)
    progress->setInfo(tr("Decompressing %1").arg(file.fileName()));
  else
    qDebug() << QString("Decompressing %1 to %2").arg(archiveFile, destinationPath);

  reportProgress(tr("Miniz version: %1").arg(MZ_VERSION), QtDebugMsg);

  progressValue = 0;
  path = destinationPath;
  this->archiveFile = archiveFile;
  mz_zip_zero_struct(&zip_archive);

  if (!mz_zip_reader_init_file(&zip_archive, qPrintable(archiveFile), 0)) {
    reportProgress(tr("File does not appear to be a compressed archive"), QtFatalMsg);
    return false;
  }

  int fileCount = (int)mz_zip_reader_get_num_files(&zip_archive);

  if (fileCount == 0) {
    mz_zip_reader_end(&zip_archive);
    reportProgress(tr("Compressed archive does not contain any files"), QtFatalMsg);
    return false;
  }

  if (progress) {
    if (progressMethod == PCM_COUNT)
      progress->setMaximum(fileCount);
    else
      progress->setMaximum(QFileInfo(archiveFile).size());
  }

  mz_zip_archive_file_stat file_stat;

  if (!mz_zip_reader_file_stat(&zip_archive, 0, &file_stat)) {
    mz_zip_reader_end(&zip_archive);
    reportProgress(tr("File status error"), QtFatalMsg);
    return false;
  }

  for (int i = 0; i < fileCount; i++) {
    if (isStopping()) {
      mz_zip_reader_end(&zip_archive);
      reportProgress(tr("Decompression interrupted"), QtWarningMsg);
      return false;
    }

    if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
      mz_zip_reader_end(&zip_archive);
      reportProgress(tr("Unable to obtain file status for index: %1").arg(i), QtFatalMsg);
      return false;
    }

    QString destPath = path % "/" % QString(file_stat.m_filename);

    if (mz_zip_reader_is_file_a_directory(&zip_archive, i)) {
      if (!createDirectory(destPath)) {
        mz_zip_reader_end(&zip_archive);
        return false;
      }
    }
    else {
      QFileInfo dfi(destPath);
      if (!createDirectory(dfi.absolutePath())) {
        mz_zip_reader_end(&zip_archive);
        return false;
      }

      if (!mz_zip_reader_extract_to_file(&zip_archive, i, qPrintable(destPath), 0)) {
        mz_zip_reader_end(&zip_archive);
        reportProgress(tr("Failed to extract %1 to %2").arg(dfi.fileName(), dfi.absolutePath()), QtFatalMsg);
        return false;
      }
      else {
        if (dfi.size() != (qint64)file_stat.m_uncomp_size) {
          reportProgress(tr("File %1 extracted size %2 does not match original %3").arg(file_stat.m_filename).arg(dfi.size()).arg((qint64)file_stat.m_uncomp_size), QtFatalMsg);
            // return false;  disable until testing complete as there maybe some adjustment factor or even OS strangeness
        }
      }

      if (progressMethod == PCM_COUNT)
        ++progressValue;
      else
        progressValue += file_stat.m_comp_size;

      if (progress)
        progress->setValue(progressValue);

      reportProgress(tr("Extracted file: %1").arg(file_stat.m_filename), QtDebugMsg);
      // to assist in responsiveness and allow stop signal to be received and processed
      QApplication::processEvents();
    }
  }

  mz_zip_reader_end(&zip_archive);

  if (progress)
    progress->setValue(progress->maximum());

  reportProgress(tr("Decompress complete"), QtDebugMsg);

  return true;
}

bool MinizInterface::createDirectory(const QString & path)
{
  if (!QDir(path).exists()) {
    if (!QDir().mkpath(path)) {
      reportProgress(tr("Failed to create directory: %1").arg(path), QtFatalMsg);
      return false;
    }
    else
      reportProgress(tr("Created directory: %1").arg(path), QtDebugMsg);
  }

  return true;
}

void MinizInterface::reportProgress(const QString & text, const int & type, bool richText)
{
  if (type >= logLevel) {
    if (progress)
      progress->addMessage(text, type);
    else
      qDebug() << text;
  }
}

void MinizInterface::stop()
{
  stopping = true;
}
