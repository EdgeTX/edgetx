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

#pragma once

#include "miniz.h"

#include <QtCore>
#include <QString>

class ProgressWidget;

class MinizInterface : public QObject
{
  Q_OBJECT

  public:
    enum ProgressCalcMethod {
      PCM_COUNT,
      PCM_SIZE
    };

    MinizInterface(ProgressWidget * progress = nullptr, const ProgressCalcMethod progressMethod = PCM_COUNT, const int & logLevel = QtWarningMsg);
    ~MinizInterface();

    bool zipPathToFile(const QString & sourcePath, const QString & archiveFile, bool append = true);
    bool unzipArchiveToPath(const QString & archiveFile, const QString & destinationPath);

  public slots:
    void stop();

  private:
    ProgressWidget *progress;
    ProgressCalcMethod progressMethod;
    qint64 progressValue;
    int logLevel;
    QString path;
    QString archiveFile;
    bool stopping;

    mz_zip_archive zip_archive;

    bool addFileToArchive(const QString & path);
    bool createDirectory(const QString & path);
    void reportProgress(const QString & text, const int & type = QtInfoMsg, bool richText = false);
    bool isStopping() { return stopping; }
};
