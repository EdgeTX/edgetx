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

#include "process_sync.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QMutexLocker>

#define SYNC_MAX_ERRORS       50  // give up after this many errors per destination

// a flood of log messages can make the UI unresponsive so we'll introduce a dynamic sleep period based on log frequency (values in [us])
#define PAUSE_FACTOR          60UL
#define PAUSE_RECOVERY        (PAUSE_FACTOR / 3 * 2)
#define PAUSE_MINTM           100UL
#define PAUSE_MAXTM           75000UL

#if (QT_VERSION < QT_VERSION_CHECK(5, 5, 0))
  #define QtInfoMsg    QtMsgType(4)
#endif

#define PRINT_CREATE(str)     emitProgressMessage((str), QtInfoMsg)
#define PRINT_REPLACE(str)    emitProgressMessage((str), QtWarningMsg)
//#define PRINT_DELETE(str)   emitProgressMessage((str), QtCriticalMsg)  // unused
#define PRINT_ERROR(str)      emitProgressMessage((str), QtFatalMsg)
#define PRINT_SKIP(str)       emitProgressMessage((str), QtDebugMsg)
#define PRINT_INFO(str)       emit progressMessage((str))                // this is always emitted regardless of logLevel option
#define PRINT_SEP()           PRINT_INFO(QString(70, '='))

#ifdef Q_OS_WIN
  extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
  #define FILTER_RE_SYNTX     QRegExp::Wildcard
#else
  #define FILTER_RE_SYNTX     QRegExp::WildcardUnix
#endif

SyncProcess::SyncProcess(const SyncProcess::SyncOptions & options) :
  m_options(options),
  m_pauseTime(PAUSE_MINTM),
  stopping(false)
{
  qRegisterMetaType<SyncProcess::SyncStatus>();

  if (m_options.compareType == OVERWR_ALWAYS && (m_options.direction == SYNC_A2B_B2A || m_options.direction == SYNC_B2A_A2B))
    m_options.compareType = OVERWR_IF_DIFF;

  m_dirFilters = QDir::Filters(m_options.dirFilterFlags);
  if (!(m_dirFilters & QDir::Dirs))
    m_dirFilters &= ~(QDir::AllDirs);

  if (!m_options.includeFilter.isEmpty() && m_options.includeFilter != "*")
    m_dirIteratorFilters = m_options.includeFilter.split(',', Qt::SkipEmptyParts);

  if (!m_options.excludeFilter.isEmpty()) {
    for (const QString & f : m_options.excludeFilter.split(',', Qt::SkipEmptyParts))
      m_excludeFilters.append(QRegExp(f, ((m_dirFilters & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive), FILTER_RE_SYNTX));
  }

  if (m_options.flags & OPT_DRY_RUN)
    testRunStr = tr("[TEST RUN] ");

  //qDebug() << m_options;
#ifdef Q_OS_WIN
  qt_ntfs_permission_lookup++;  // global enable NTFS permissions checking
#endif
}

SyncProcess::~SyncProcess()
{
#ifdef Q_OS_WIN
  qt_ntfs_permission_lookup--;  // global revert NTFS permissions checking
#endif
}

void SyncProcess::stop()
{
  QWriteLocker locker(&stopReqMutex);
  stopping = true;
}

bool SyncProcess::isStopRequsted()
{
  QReadLocker locker(&stopReqMutex);
  return stopping;
}

void SyncProcess::run()
{
  const QString folderA = (m_options.direction == SYNC_B2A_A2B ? m_options.folderB : m_options.folderA);
  const QString folderB = (m_options.direction == SYNC_B2A_A2B ? m_options.folderA : m_options.folderB);
  const SyncDirection direction = (m_options.direction == SYNC_B2A_A2B ? SYNC_A2B_B2A : SyncDirection(m_options.direction));
  const QString gathering = tr("Gathering file information for %1...");
  const QString noFiles = tr("No files found in %1");
  int count = 0;

  m_stat.clear();
  m_startTime = QDateTime::currentDateTime();

  emit started();
  emit fileCountChanged(0);
  emit statusUpdate(m_stat);

  if (direction == SYNC_A2B_B2A || direction == SYNC_A2B) {
    emit statusMessage(gathering.arg(folderA));
    count = getFilesCount(folderA);

    if (isStopRequsted())
      goto endrun;

    if (count) {
      m_stat.count = count;
      if (m_options.direction == SYNC_A2B_B2A)
        count *= 2;  // assume this direction is only 50% of total, exact will be calculated later
      emit fileCountChanged(count);
      updateDir(folderA, folderB);
      if (isStopRequsted())
        goto endrun;
    }
    else {
      PRINT_INFO(noFiles.arg(folderA));
      PRINT_SEP();
    }
  }

  if (direction == SYNC_A2B_B2A || direction == SYNC_B2A) {
    emit statusMessage(gathering.arg(folderB));
    count = getFilesCount(folderB);

    if (isStopRequsted())
      goto endrun;

    m_stat.count += count;
    emit fileCountChanged(m_stat.count);

    if (count) {
      updateDir(folderB, folderA);
    }
    else {
      PRINT_INFO(noFiles.arg(folderB));
      PRINT_SEP();
    }
  }

  if (!m_stat.count) {
    emit statusMessage(tr("Synchronization failed, nothing found to copy."), QtWarningMsg);
    emit finished();
    return;
  }

  endrun:
  finish();
}

void SyncProcess::finish()
{
  const lldiv_t elapsed = lldiv(m_startTime.secsTo(QDateTime::currentDateTime()), 60);
  QString endStr = testRunStr;
  if (m_stat.index < m_stat.count)
    endStr.append(tr("Synchronization aborted at %1 of %2 files.").arg(m_stat.index).arg(m_stat.count));
  else
    endStr.append(tr("Synchronization finished with %1 files in %2m %3s.").arg(m_stat.count).arg(elapsed.quot).arg(elapsed.rem));
  emit statusMessage(endStr);
  emit finished();
}

SyncProcess::FileFilterResult SyncProcess::fileFilter(const QFileInfo & fileInfo)
{
  // Windows Junctions (mount points) are not detected as links (QTBUG-45344), but that's OK since they're really "hard links."
  const bool chkDirLnk = ((m_dirFilters & QDir::NoSymLinks) && !(m_dirFilters & QDir::AllDirs)) || ((m_options.flags & OPT_SKIP_DIR_LINKS) && fileInfo.isDir());
  if ((chkDirLnk || ((m_dirFilters & QDir::NoSymLinks) && fileInfo.isFile())) && QFileInfo(fileInfo.absoluteFilePath()).isSymLink())  // MUST create a new QFileInfo here (QTBUG-69001)
    return FILE_LINK_IGNORE;

  if (m_options.maxFileSize > 0 && fileInfo.isFile() && fileInfo.size() > m_options.maxFileSize)
    return FILE_OVERSIZE;

  if (!m_excludeFilters.isEmpty() && (!(m_dirFilters & QDir::AllDirs) || fileInfo.isFile())) {
    for (QVector<QRegExp>::const_iterator it = m_excludeFilters.constBegin(), end = m_excludeFilters.constEnd(); it != end; ++it) {
      if (QRegExp(*it).exactMatch(fileInfo.fileName()))
        return FILE_EXCLUDE;
    }
  }

  return FILE_ALLOW;
}

QFileInfoList SyncProcess::dirInfoList(const QString & directory)
{
  QDir::Filters flt = m_dirFilters;
  if (!(flt & QDir::Dirs) && (m_options.flags & OPT_RECURSIVE))
    flt |= ((m_options.dirFilterFlags & QDir::AllDirs) ? QDir::AllDirs : QDir::Dirs);
  return QDir(directory).entryInfoList(m_dirIteratorFilters, flt, QDir::Name | QDir::DirsLast);
}

void SyncProcess::pushDirEntries(const QFileInfo & fileInfo, QMutableListIterator<QFileInfo> & it)
{
  if ((m_options.flags & OPT_RECURSIVE) && fileInfo.isDir()) {
    for (const QFileInfo &fi : dirInfoList(fileInfo.absoluteFilePath()))
      it.insert(fi);
  }
}

int SyncProcess::getFilesCount(const QString & directory)
{
  if (!QFile::exists(directory))
    return 0;

  int result = 0;
  QFileInfoList infoList = dirInfoList(directory);
  QMutableListIterator<QFileInfo> it(infoList);
  it.toBack();
  while (it.hasPrevious() && !isStopRequsted()) {
    const QFileInfo fi(it.previous());
    it.remove();
    if (fileFilter(fi) == FILE_ALLOW) {
      pushDirEntries(fi, it);
      if (fi.isFile()) {
        result++;
      }
    }
    QApplication::processEvents();
  }
  return result;
}

void SyncProcess::updateDir(const QString & source, const QString & destination)
{
  SyncStatus pStat = m_stat;
  const QDir srcDir(source), dstDir(destination);
  FileFilterResult ffr;
  emit statusMessage(testRunStr % tr("Synchronizing: %1\n    To: %2").arg(source, destination));
  PRINT_INFO(testRunStr % tr("Starting synchronization:\n  %1 -> %2\n").arg(source, destination));

  QFileInfoList infoList = dirInfoList(source);
  QMutableListIterator<QFileInfo> it(infoList);
  it.toBack();
  while (it.hasPrevious() && !isStopRequsted()) {
    const QFileInfo fi(it.previous());
    it.remove();
    if ((ffr = fileFilter(fi)) == FILE_ALLOW) {
      pushDirEntries(fi, it);
      if ((m_dirFilters & QDir::Dirs) || fi.isFile()) {
        updateEntry(fi.filePath(), srcDir, dstDir);
        if (fi.isFile())
          ++m_stat.index;
        emit statusUpdate(m_stat);
        if (m_stat.errored - pStat.errored > SYNC_MAX_ERRORS) {
          PRINT_ERROR(tr("\nToo many errors, giving up."));
          break;
        }
      }
    }
    else if (m_options.logLevel == QtDebugMsg) {
      switch (ffr) {
        case FILE_OVERSIZE:
          PRINT_SKIP(tr("Skipping large file: %1 (%2KB)").arg(fi.fileName()).arg(int(fi.size() / 1024)));
          break;
        case FILE_EXCLUDE:
          PRINT_SKIP(tr("Skipping filtered file: %1").arg(fi.fileName()));
          break;
        case FILE_LINK_IGNORE:
          PRINT_SKIP(tr("Skipping linked file: %1").arg(fi.fileName()));
          break;
        default:
          break;
      }
      // don't count as skipped because these weren't included in the total file count to begin with
    }
    // throttle if needed
    m_pauseTime = qMax(m_pauseTime - PAUSE_RECOVERY, PAUSE_MINTM);
    pause();
  }

  QString endStr = "\n" % testRunStr;
  if (isStopRequsted())
    endStr.append(tr("Aborted synchronization of:"));
  else
    endStr.append(tr("Finished synchronizing:"));
  endStr.append(QString("\n  %1 -> %2\n  ").arg(source, destination));
  endStr.append(tr("Created: %1; Updated: %2; Skipped: %3; Errors: %4;").arg(m_stat.created-pStat.created).arg(m_stat.updated-pStat.updated).arg(m_stat.skipped-pStat.skipped).arg(m_stat.errored-pStat.errored));
  PRINT_INFO(endStr);
  PRINT_SEP();
}

bool SyncProcess::updateEntry(const QString & entry, const QDir & source, const QDir & destination)
{
  const QString srcPath = QDir::toNativeSeparators(source.absoluteFilePath(entry));
  const QString destPath = QDir::toNativeSeparators(destination.absoluteFilePath(source.relativeFilePath(entry)));
  const QFileInfo sourceInfo(srcPath);
  const QFileInfo destInfo(destPath);
  static QString lastMkPath;

  // check if this is a directory OR if we're copying a file with a path which doesn't exist yet.
  if (sourceInfo.isDir() || !destInfo.absoluteDir().exists()) {
    const QString mkPath = sourceInfo.isDir() ? destPath : QDir::toNativeSeparators(destInfo.absolutePath());
    if (!destination.exists(mkPath)) {
      if (mkPath == lastMkPath) {
        // we've already tried, and apparently failed, to create this folder... bail out but log as error.
        if (!(m_options.flags & OPT_DRY_RUN)) {
          ++m_stat.errored;
          return false;
        }
      }
      else {
        lastMkPath = mkPath;
        PRINT_CREATE(tr("Creating directory: %1").arg(mkPath));
        if (!(m_options.flags & OPT_DRY_RUN) && !destination.mkpath(mkPath)) {
          PRINT_ERROR(tr("Could not create directory: %1").arg(mkPath));
            ++m_stat.errored;
          return false;
        }
      }
    }
    else if (m_dirFilters & QDir::Dirs) {
      PRINT_SKIP(tr("Directory exists: %1").arg(mkPath));
    }
    if (sourceInfo.isDir())
      return true;
  }

  //qDebug() << destPath;
  QFile sourceFile(srcPath);
  QFile destinationFile(destPath);
  const bool destExists = destInfo.exists();
  bool checkDate = (m_options.compareType == OVERWR_NEWER_IF_DIFF || m_options.compareType == OVERWR_NEWER_ALWAYS);
  bool checkContent = (m_options.compareType == OVERWR_NEWER_IF_DIFF || m_options.compareType == OVERWR_IF_DIFF);
  bool existed = false;

  if (destExists && checkDate) {
    const QDate cmprDate = QDate::currentDate();
    if (sourceInfo.lastModified().date() > cmprDate || destInfo.lastModified().date() > cmprDate) {
      PRINT_ERROR(tr("At least one of the file modification dates is in the future, error on: %1").arg(srcPath));
      ++m_stat.errored;
      return false;
    }
    if (sourceInfo.lastModified() <= destInfo.lastModified()) {
      PRINT_SKIP(tr("Skipping older file: %1").arg(srcPath));
      ++m_stat.skipped;
      return true;
    }
    checkDate = false;
  }

  if (destExists && checkContent) {
    if (!sourceFile.open(QFile::ReadOnly)) {
      PRINT_ERROR(tr("Could not open source file '%1': %2").arg(srcPath, sourceFile.errorString()));
      ++m_stat.errored;
      return false;
    }
    if (!destinationFile.open(QFile::ReadOnly)) {
      PRINT_ERROR(tr("Could not open destination file '%1': %2").arg(destPath, destinationFile.errorString()));
      ++m_stat.errored;
      return false;
    }

    const bool skip = QCryptographicHash::hash(sourceFile.readAll(), QCryptographicHash::Md5) == QCryptographicHash::hash(destinationFile.readAll(), QCryptographicHash::Md5);
    sourceFile.close();
    destinationFile.close();
    if (skip) {
      PRINT_SKIP(tr("Skipping identical file: %1").arg(srcPath));
      ++m_stat.skipped;
      return true;
    }
    checkContent = false;
  }

  if (!destExists || (!checkDate && !checkContent)) {
    if (destInfo.exists()) {
      existed = true;
      PRINT_REPLACE(tr("Replacing file: %1").arg(destPath));
      if (!(m_options.flags & OPT_DRY_RUN) && !destinationFile.remove()) {
        PRINT_ERROR(tr("Could not delete destination file '%1': %2").arg(destPath, destinationFile.errorString()));
        ++m_stat.errored;
        return false;
      }
    }
    else {
      PRINT_CREATE(tr("Creating file: %1").arg(destPath));
    }
    if (!(m_options.flags & OPT_DRY_RUN) && !sourceFile.copy(destPath)) {
      PRINT_ERROR(tr("Copy failed: '%1' to '%2': %3").arg(srcPath, destPath, sourceFile.errorString()));
      ++m_stat.errored;
      return false;
    }

    if (existed)
      ++m_stat.updated;
    else
      ++m_stat.created;
  }

  return true;
}

void SyncProcess::pause()
{
  QElapsedTimer tim;
  const qint64 exp = m_pauseTime * 1000;
  tim.start();
  while (tim.nsecsElapsed() < exp && !isStopRequsted())
    QApplication::processEvents();
}

void SyncProcess::emitProgressMessage(const QString & text, int type)
{
  if (m_options.logLevel == QtDebugMsg || (m_options.logLevel == QtInfoMsg && type > QtDebugMsg) || (type < QtInfoMsg && type >= m_options.logLevel)) {
    emit progressMessage(text, type);
    m_pauseTime = qMin(m_pauseTime + PAUSE_FACTOR, PAUSE_MAXTM);
  }
}
