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

#ifndef PROCESS_SYNC_H
#define PROCESS_SYNC_H

#include <QObject>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QReadWriteLock>
#include <QRegExp>
#include <QVector>

class SyncProcess : public QObject
{
    Q_OBJECT

  public:
    enum SyncDirection {
      SYNC_A2B_B2A = 0,
      SYNC_B2A_A2B,
      SYNC_A2B,
      SYNC_B2A
    };
    Q_ENUM(SyncDirection)

    enum SyncCompareType {
      OVERWR_NEWER_IF_DIFF = 0,
      OVERWR_NEWER_ALWAYS,
      OVERWR_IF_DIFF,
      OVERWR_ALWAYS
    };
    Q_ENUM(SyncCompareType)

    enum SyncOptionFlag {
      OPT_NONE            = 0,
      OPT_DRY_RUN         = 0x01,
      OPT_SKIP_EMPTY_DIR  = 0x02,
      OPT_RECURSIVE       = 0x04,
      OPT_SKIP_DIR_LINKS  = 0x08
    };
    Q_DECLARE_FLAGS(SyncOptionFlags, SyncOptionFlag)
    Q_FLAG(SyncOptionFlags)

    // TODO: perhaps move this to AppData/saved settings once that refactoring is merged.
    struct SyncOptions {
        explicit SyncOptions() { reset(); }

        QString folderA;
        QString folderB;
        QString excludeFilter;   // CSV list of glob pattern(s)
        QString includeFilter;   // CSV list of glob pattern(s)
        qint64 maxFileSize;      // Bytes
        int direction;           // SyncDirection
        int compareType;         // SyncCompareType
        int flags;               // SyncOptionFlags
        int dirFilterFlags;      // QDir::Filters
        int logLevel;            // QtMsgType (see ProgressWidget::addMessage())
        int sessionId;           // Last run radio profile

        void reset()
        {
          excludeFilter = QStringLiteral(".*,System Volume*");
          includeFilter.clear();
          direction = SYNC_A2B_B2A;
          compareType = OVERWR_NEWER_IF_DIFF;
          maxFileSize = 2 * 1024 * 1024;
          logLevel = QtWarningMsg;
          flags = OPT_RECURSIVE;
          dirFilterFlags = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot;
          sessionId = -1;
        }

        friend inline QDebug operator<<(QDebug debug, const SyncOptions &o) {
          return debug << "Incl:" << o.includeFilter << "Excl:" << o.excludeFilter << SyncDirection(o.direction) << SyncCompareType(o.compareType) << SyncOptionFlags(o.flags) << QDir::Filters(o.dirFilterFlags);
        }
    };

    struct SyncStatus {
        int index;
        int count;
        int created;
        int updated;
        int skipped;
        int errored;
        void clear() { memset(this, 0, sizeof(SyncStatus)); }
    };

    explicit SyncProcess(const SyncOptions & options);
    ~SyncProcess();

  public slots:
    void run();
    void stop();

  signals:
    void started();
    void finished();
    void fileCountChanged(int count);
    void statusUpdate(const SyncProcess::SyncStatus & status);
    void statusMessage(const QString & text, const int & type = QtInfoMsg);
    void progressMessage(const QString & text, const int & type = QtInfoMsg, bool richText = false);

  protected:
    enum FileFilterResult { FILE_ALLOW, FILE_OVERSIZE, FILE_EXCLUDE, FILE_LINK_IGNORE };

    bool isStopRequsted();
    void finish();
    FileFilterResult fileFilter(const QFileInfo & fileInfo);
    QFileInfoList dirInfoList(const QString & directory);
    int getFilesCount(const QString & directory);
    void updateDir(const QString & source, const QString & destination);
    void pushDirEntries(const QFileInfo & fi, QMutableListIterator<QFileInfo> &it);
    bool updateEntry(const QString & entry, const QDir & source, const QDir & destination);
    void pause();
    void emitProgressMessage(const QString &text, int type);

    SyncOptions m_options;
    SyncStatus m_stat;
    QReadWriteLock stopReqMutex;
    QString testRunStr;
    QVector<QRegExp> m_excludeFilters;
    QStringList m_dirIteratorFilters;
    QDir::Filters m_dirFilters;
    QDateTime m_startTime;
    unsigned long m_pauseTime;
    bool stopping;
};

Q_DECLARE_METATYPE(SyncProcess::SyncOptions)
Q_DECLARE_TYPEINFO(SyncProcess::SyncOptions, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(SyncProcess::SyncStatus)
Q_DECLARE_TYPEINFO(SyncProcess::SyncStatus, Q_PRIMITIVE_TYPE);

#endif // PROCESS_SYNC_H
