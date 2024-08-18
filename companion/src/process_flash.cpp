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

#include "process_flash.h"
#include "progresswidget.h"
#include "eeprominterface.h"
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QEventLoop>

#if defined _MSC_VER || !defined __GNUC__
#include <Windows.h>
#include <WinBase.h>
#include <tlhelp32.h>
#define sleep(x) Sleep(x*1000)
#else
#include <unistd.h>
#endif

FlashProcess::FlashProcess(const QString & cmd, const QStringList & args, ProgressWidget * progress):
  progress(progress),
  cmd(cmd),
  args(args),
  process(new QProcess(this)),
  hasErrors(false),
  lfuse(0),
  hfuse(0),
  efuse(0),
  flashPhase(READING)
#if !__GNUC__
  , killTimer(nullptr)
#endif
{
  connect(process, SIGNAL(started()),this, SLOT(onStarted()));
  connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStandardOutput()));
  connect(process, SIGNAL(readyReadStandardError()), this, SLOT(onReadyReadStandardError()));
  connect(process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
}

FlashProcess::~FlashProcess()
{
#if !__GNUC__
  delete killTimer;
#endif
}

bool FlashProcess::run()
{
  if (!QFile::exists(cmd)) {
    QMessageBox::critical(nullptr, CPN_STR_APP_NAME, tr("Executable %1 not found").arg(cmd));
    return false;
  }

#if !__GNUC__
  if (cmd.toLower().contains("sam-ba")) {
    killTimer = new QTimer(this);
    connect(killTimer, SIGNAL(timeout()), this, SLOT(onKillTimerElapsed()));
    killTimer->start(2000);
  }
#endif

  QEventLoop loop;
  connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
  process->start(cmd, args);
  loop.exec();

  return true;
}

void FlashProcess::onStarted()
{
  progress->lock(true);
  progress->addText(cmd + " " + args.join(" "));
  progress->addSeparator();
}

#if !__GNUC__
bool killProcessByName(const char *szProcessToKill)
{
  HANDLE hProcessSnap;
  HANDLE hProcess;
  PROCESSENTRY32 pe32;

  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  // Takes a snapshot of all the processes

  if (hProcessSnap == INVALID_HANDLE_VALUE) {
    return false;
  }

  pe32.dwSize = sizeof(PROCESSENTRY32);

  if (!Process32First(hProcessSnap, &pe32)) {
    CloseHandle(hProcessSnap);
    return false;
  }

  do {
    if (!strcmp(pe32.szExeFile,szProcessToKill)) {    //  checks if process at current position has the name of to be killed app
      hProcess = OpenProcess(PROCESS_TERMINATE,0, pe32.th32ProcessID);  // gets handle to process
      TerminateProcess(hProcess, 0);   // Terminate process by handle
      CloseHandle(hProcess);  // close the handle
    }
  } while (Process32Next(hProcessSnap, &pe32));  // gets next member of snapshot

  CloseHandle(hProcessSnap);  // closes the snapshot handle
  return true;
}
#endif

void FlashProcess::onKillTimerElapsed()
{
#if !__GNUC__
  // trick to accelerate SAM-BA startup
  killProcessByName("tasklist.exe");
#endif
}

void FlashProcess::analyseStandardOutput(const QString & text)
{
  currStdoutLine.append(text);

  if (currStdoutLine.contains("size = ")) {
    int pos = currStdoutLine.lastIndexOf("size = ");
    QString temp = currStdoutLine.mid(pos+7);
    pos = temp.lastIndexOf("\n");
    int size = temp.left(pos).toInt();
    progress->setMaximum(size / 2048);
  }

  if (currStdoutLine.contains("\n")) {
    int nlPos = currStdoutLine.lastIndexOf("\n");
    currStdoutLine = currStdoutLine.mid(nlPos+1);
  }

  if (!currStdoutLine.isEmpty()) {
    if (currStdoutLine.at(0) == QChar('.')) {
      int pos = currStdoutLine.lastIndexOf(".");
      progress->setValue(pos);
    }
    else if (currStdoutLine.startsWith("Starting upload: [")) {
      int pos = (currStdoutLine.lastIndexOf("#") - 19) * 100 / 256;
      progress->setValue(pos);
    }
  }

  if (text.contains("Complete ")) {
#if !__GNUC__
    delete killTimer;
    killTimer = nullptr;
#endif
    int start = text.indexOf("Complete ");
    int end = text.indexOf("%");
    if (start > 0) {
      start += 9;
      int value = text.mid(start, end - start).toInt();
      progress->setValue(value);
    }
  }

  if (text.contains("Upload\t[") || text.contains("Download\t[")) {
    int end = text.indexOf("%");
    if (end > 0) {
      int value = text.mid(end - 3, 3).toInt();
      progress->setValue(value);
    }
  }

  if (text.contains(":010000")) {
    // contains fuse info
    QStringList stl = text.split(":01000000");
    foreach (QString t, stl) {
      bool ok = false;
      if (!lfuse)        lfuse = t.left(2).toInt(&ok, 16);
      if (!hfuse && !ok) hfuse = t.left(2).toInt(&ok, 16);
      if (!efuse && !ok) efuse = t.left(2).toInt(&ok, 16);
    }
  }

  if (text.contains("-E-")) {
    hasErrors = true;
  }

  if (text.contains("Cannot open device")) {
    hasErrors = true;
  }
}

void FlashProcess::analyseStandardError(const QString &text)
{
  currStderrLine.append(text);
  if (currStderrLine.contains("#")) {
    QString avrflashPhase = currStderrLine.left(1).toLower();
    if (avrflashPhase == "w") {
      flashPhase = WRITING;
      progress->setInfo(tr("Writing..."));
      progress->setValue(2 * currStderrLine.count("#"));
    }
    else if (avrflashPhase == "r") {
      if (flashPhase == READING) {
        progress->setInfo(tr("Reading..."));
      }
      else {
        flashPhase = VERIFYING;
        progress->setInfo(tr("Verifying..."));
      }
      progress->setValue(2 * currStderrLine.count("#"));
    }
  }

  if (currStderrLine.contains("\n")) {
    int nlPos = currStderrLine.lastIndexOf("\n");
    currStderrLine = currStderrLine.mid(nlPos+1);
  }

  if ((text.contains("-E-") && !text.contains("-E- No receive file name")) ||
       text.contains("No DFU capable USB device found")) {
    hasErrors = true;
  }
}

void FlashProcess::onReadyReadStandardOutput()
{
  QString text = QString(process->readAllStandardOutput());
  progress->addText(text);
  analyseStandardOutput(text);
}

void FlashProcess::onReadyReadStandardError()
{
  QString text = QString(process->readAllStandardError());
  progress->addText(text);
  analyseStandardError(text);
}

void FlashProcess::errorWizard()
{
  QString output = progress->getText();

  if (output.contains("avrdude: Expected signature for")) { // wrong signature
    int pos = output.indexOf("avrdude: Device signature = ");
    bool fwexist = false;
    QString DeviceStr = tr("unknown");
    QString FwStr = "";

    if (pos > 0) {
      QString DeviceId = output.mid(pos+28, 8);
      if (DeviceId=="0x1e9602") {
        DeviceStr = "Atmega 64";
        FwStr="\n" + tr("ie: OpenTX for 9X board or OpenTX for 9XR board");
        fwexist = true;
      }
      else if (DeviceId=="0x1e9702") {
        DeviceStr = "Atmega 128";
        FwStr="\n" + tr("ie: OpenTX for M128 / 9X board or OpenTX for 9XR board with M128 chip");
        fwexist = true;
      }
      else if (DeviceId=="0x1e9703") {
        DeviceStr = "Atmega 1280";
      }
      else if (DeviceId=="0x1e9704") {
        DeviceStr = "Atmega 1281";
      }
      else if (DeviceId=="0x1e9801") {
        DeviceStr = "Atmega 2560";
        FwStr="\n" + tr("ie: OpenTX for Gruvin9X  board");
        fwexist = true;
      }
      else if (DeviceId=="0x1e9802") {
        DeviceStr = "Atmega 2561";
      }
    }
    if (!fwexist) {
      QMessageBox::warning(nullptr, "Companion - Tip of the day", tr("Your radio uses a %1 CPU!!!\n\nPlease check advanced burn options to set the correct cpu type.").arg(DeviceStr));
    }
    else {
      Firmware *firmware = getCurrentFirmware();
      QMessageBox::warning(nullptr, "Companion - Tip of the day", tr("Your radio uses a %1 CPU!!!\n\nPlease select an appropriate firmware type to program it.").arg(DeviceStr)+FwStr+tr("\nYou are currently using:\n %1").arg(firmware->getName()));
    }
  }
  else if (output.contains("No DFU capable USB device found")){
    QMessageBox::warning(nullptr, "Companion - Tip of the day", tr("Your radio does not seem connected to USB or the driver is not initialized!!!."));
  }
}

void FlashProcess::onFinished(int code=0)
{
  progress->addSeparator();
  if (code==1 && cmd.toLower().contains("sam-ba")) {
    code = 0;
  }
  if (code) {
    progress->setInfo(tr("Flashing done (exit code = %1)").arg(code));
    if (cmd.toLower().contains("avrdude") || cmd.toLower().contains("dfu")) {
      errorWizard();
    }
  }
  else if (hasErrors) {
    progress->setInfo(tr("Flashing done with errors"));
  }
  if (lfuse || hfuse || efuse) {
    addReadFuses();
  }
  progress->setValue(progress->maximum());
  progress->lock(false);
  emit finished();
}

void FlashProcess::addReadFuses()
{
  progress->addSeparator();
  progress->addText(tr("FUSES: Low=%1 High=%2 Ext=%3").arg(lfuse, 2, 16, QChar('0')).arg(hfuse, 2, 16, QChar('0')).arg(efuse, 2, 16, QChar('0')));
  progress->addSeparator();
}
