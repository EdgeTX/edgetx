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

#ifndef _PROCESS_COPY_H_
#define _PROCESS_COPY_H_

#include <QObject>
#include <QString>
#include <QStringList>

class ProgressWidget;

class CopyProcess : public QObject
{
  Q_OBJECT

public:
  CopyProcess(const QString &source, const QString &destination, ProgressWidget *progress);
  bool run();

signals:
  void finished();

protected slots:
  void onTimer();

protected:
  ProgressWidget *progress;
  const QString source;
  const QString destination;
  bool result;
};

#endif // _PROCESS_COPY_H_
