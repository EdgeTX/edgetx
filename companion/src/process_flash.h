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

#ifndef _PROCESS_FLASH_H_
#define _PROCESS_FLASH_H_

#include <QObject>
#include <QString>
#include <QStringList>

class ProgressWidget;
class QProcess;
class QTimer;

class FlashProcess : public QObject
{
  Q_OBJECT

public:
  FlashProcess(const QString &cmd, const QStringList &args, ProgressWidget *progress);
  ~FlashProcess();
  bool run();

signals:
  void finished();

protected slots:
  void onStarted();
  void onReadyReadStandardOutput();
  void onReadyReadStandardError();
  void onFinished(int);
  void onKillTimerElapsed();

protected:
  void analyseStandardOutput(const QString &text);
  void analyseStandardError(const QString &text);
  void errorWizard();
  void addReadFuses();
  ProgressWidget * progress;
  const QString cmd;
  const QStringList args;
  QProcess * process;
  bool hasErrors;
  QString currStdoutLine;
  QString currStderrLine;
  unsigned int lfuse;
  unsigned int hfuse;
  unsigned int efuse;
  enum FlashPhase { READING, WRITING, VERIFYING };
  FlashPhase flashPhase;
#if !__GNUC__
  QTimer *killTimer;
#endif
};

#endif // _PROCESS_FLASH_H_
