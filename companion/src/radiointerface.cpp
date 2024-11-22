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

#include "radiointerface.h"
#include "appdata.h"
#include "eeprominterface.h"
#include "process_flash.h"
#include "radionotfound.h"
#include "burnconfigdialog.h"
#include "helpers.h"
#include "process_copy.h"
#include "storage.h"
#include "progresswidget.h"

QString getRadioInterfaceCmd()
{
  burnConfigDialog bcd;
  Board::Type board = getCurrentBoard();
  if (IS_STM32(board)) {
    return bcd.getDFU();
  }
  else
    return bcd.getSAMBA();
}

QStringList getDfuArgs(const QString & cmd, const QString & filename)
{
  QStringList args;
  burnConfigDialog bcd;
  args << bcd.getDFUArgs();
  if (!filename.endsWith(".dfu"))
    args << "--dfuse-address" << "0x08000000";
  if (cmd == "-U")
    args.last().append(":" % QString::number(Boards::getFlashSize(getCurrentBoard())));
  args << "--device" << "0483:df11";
  args << cmd % filename;
  return args;
}

QStringList getSambaArgs(const QString & tcl)
{
  QStringList result;

  QString tclFilename = generateProcessUniqueTempFileName("temp.tcl");
  if (QFile::exists(tclFilename)) {
    qunlink(tclFilename);
  }
  QFile tclFile(tclFilename);
  if (!tclFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(NULL, CPN_STR_TTL_ERROR,
                         QCoreApplication::translate("RadioInterface", "Cannot write file %1:\n%2.").arg(tclFilename).arg(tclFile.errorString()));
    return result;
  }

  QTextStream outputStream(&tclFile);
  outputStream << tcl;

  burnConfigDialog bcd;
  result << bcd.getSambaPort() << bcd.getArmMCU() << tclFilename ;
  return result;
}

QStringList getReadEEpromCmd(const QString & filename)
{
  QStringList result;
  if (!IS_STM32(getCurrentBoard())) {
    result = getSambaArgs(QString("SERIALFLASH::Init 0\n") + "receive_file {SerialFlash AT25} \"" + filename + "\" 0x0 0x80000 0\n");
  }
  return result;
}

QStringList getWriteEEpromCmd(const QString & filename)
{
  Board::Type board = getCurrentBoard();
  if (IS_STM32(board)) {
    // impossible
    return QStringList();
  }
  else {
    return getSambaArgs(QString("SERIALFLASH::Init 0\n") + "send_file {SerialFlash AT25} \"" + filename + "\" 0x0 0\n");
  }
}

QStringList getWriteFirmwareArgs(const QString & filename)
{
  Board::Type board = getCurrentBoard();
  if (IS_STM32(board)) {
    return getDfuArgs("-D", filename);
  }
  else if (board == Board::BOARD_SKY9X) {
    return getSambaArgs(QString("send_file {Flash} \"") + filename + "\" 0x400000 0\n" + "FLASH::ScriptGPNMV 2\n");
  }
  else {
    return getSambaArgs(QString("send_file {Flash} \"") + filename + "\" 0x400000 0\n" + "FLASH::ScriptGPNMV 2\n");
  }
}

QStringList getReadFirmwareArgs(const QString & filename)
{
  Board::Type board = getCurrentBoard();
  if (IS_STM32(board)) {
    return getDfuArgs("-U", filename);
  }
  else if (board == Board::BOARD_SKY9X) {
    return getSambaArgs(QString("receive_file {Flash} \"") + filename + "\" 0x400000 0x40000 0\n");
  }
  else {
    return getSambaArgs(QString("receive_file {Flash} \"") + filename + "\" 0x400000 0x80000 0\n");
  }
}

bool readFirmware(const QString & filename, ProgressWidget * progress)
{
  bool result = false;

  QFile file(filename);
  if (file.exists() && !file.remove()) {
    QMessageBox::warning(NULL, CPN_STR_TTL_ERROR,
                         QCoreApplication::translate("RadioInterface", "Could not delete temporary file: %1").arg(filename));
    return false;
  }

  QString path = findMassstoragePath("FIRMWARE.BIN");
  if (!path.isEmpty()) {
    qDebug() << "readFirmware: reading" << path << "into" << filename;
    CopyProcess copyProcess(path, filename, progress);
    result = copyProcess.run();
  }

  if (!result) {
    qDebug() << "readFirmware: reading" << filename << "with" << getRadioInterfaceCmd() << getReadFirmwareArgs(filename);
    FlashProcess flashProcess(getRadioInterfaceCmd(), getReadFirmwareArgs(filename), progress);
    result = flashProcess.run();
  }

  if (!QFileInfo(filename).exists()) {
    result = false;
  }

  return result;
}

bool writeFirmware(const QString & filename, ProgressWidget * progress)
{
  QString path = findMassstoragePath("FIRMWARE.BIN");
  if (!path.isEmpty()) {
    qDebug() << "writeFirmware: writing" << path << "from" << filename;
    CopyProcess copyProcess(filename, path, progress);
    return copyProcess.run();
  }

  qDebug() << "writeFirmware: writing" << filename << "with" << getRadioInterfaceCmd() << getWriteFirmwareArgs(filename);
  FlashProcess flashProcess(getRadioInterfaceCmd(), getWriteFirmwareArgs(filename), progress);
  return flashProcess.run();
}

bool readSettings(const QString & filename, ProgressWidget * progress)
{
  Board::Type board = getCurrentBoard();

  QFile file(filename);
  if (file.exists() && !file.remove()) {
    QMessageBox::warning(NULL, CPN_STR_TTL_ERROR,
                         QCoreApplication::translate("RadioInterface", "Could not delete temporary file: %1").arg(filename));
    return false;
  }

  if (Boards::getCapability(board, Board::HasSDCard))
    return readSettingsSDCard(filename, progress);
  else
    return readSettingsEeprom(filename, progress);
}

bool readSettingsSDCard(const QString & filename, ProgressWidget * progress)
{
  QString radioPath = findMassstoragePath("RADIO", true, progress);
  qDebug() << "Searching for SD card, found" << radioPath;
  if (radioPath.isEmpty()) {
    QMessageBox::critical(progress, CPN_STR_TTL_ERROR,
                          QCoreApplication::translate("RadioInterface", "Unable to find radio SD card!"));
    return false;
  }

  // if radio.yml it is a converted radio
  // if radio.bin unconverted pre 2.6
  // if neither then unconverted eeprom or empty
  if (!QFile::exists(radioPath % "/RADIO/radio.yml") && !QFile::exists(radioPath % "/RADIO/radio.bin"))
    return readSettingsEeprom(filename, progress);

  RadioData radioData;
  Storage inputStorage(radioPath);
  if (!inputStorage.load(radioData)) {
    QString errorMsg = inputStorage.error();
    if (errorMsg.isEmpty()) {
      errorMsg = QCoreApplication::translate("RadioInterface", "Failed to read Models and Settings from") % radioPath;
    }
    QMessageBox::critical(progress, CPN_STR_TTL_ERROR, errorMsg);
    return false;
  }
  Storage outputStorage(filename);
  if (!outputStorage.write(radioData)) {
    QMessageBox::critical(progress, CPN_STR_TTL_ERROR, QCoreApplication::translate("RadioInterface", "Failed to write Models and Setting file") % " " % filename);
    return false;
  }

  if (getCurrentBoard() == Board::BOARD_JUMPER_T16 && inputStorage.getBoard() == Board::BOARD_X10) {
    if (displayT16ImportWarning() == false)
      return false;
  }

  return QFileInfo(filename).exists();
}


bool readSettingsEeprom(const QString & filename, ProgressWidget * progress)
{
  Board::Type board = getCurrentBoard();

  QString path = findMassstoragePath("EEPROM.BIN");
  if (path.isEmpty()) {
    // On previous OpenTX we called the EEPROM file "TARANIS.BIN" :(
    path = findMassstoragePath("TARANIS.BIN");
  }
  if (path.isEmpty()) {
    // Mike's bootloader calls the EEPROM file "ERSKY9X.BIN" :(
    path = findMassstoragePath("ERSKY9X.BIN");
  }
  if (path.isEmpty()) {
    RadioNotFoundDialog dialog(progress);
    dialog.exec();
    return false;
  }
  CopyProcess copyProcess(path, filename, progress);
  if (!copyProcess.run()) {
    return false;
  }


  if (!IS_STM32(board)) {
    FlashProcess flashProcess(getRadioInterfaceCmd(), getReadEEpromCmd(filename), progress);
    if (!flashProcess.run()) {
      return false;
    }
  }

  return QFileInfo(filename).exists();
}

bool writeSettings(const QString & filename, ProgressWidget * progress)
{
  Board::Type board = getCurrentBoard();

  QString path = findMassstoragePath("EEPROM.BIN");
  if (path.isEmpty()) {
    // On previous OpenTX we called the EEPROM file "TARANIS.BIN" :(
    path = findMassstoragePath("TARANIS.BIN");
  }
  if (path.isEmpty()) {
    // Mike's bootloader calls the EEPROM file "ERSKY9X.BIN" :(
    path = findMassstoragePath("ERSKY9X.BIN");
  }
  if (!path.isEmpty()) {
    CopyProcess copyProcess(filename, path, progress);
    return copyProcess.run();
  }

  if (!IS_TARANIS(board)) {
    FlashProcess flashProcess(getRadioInterfaceCmd(), getWriteEEpromCmd(filename), progress);
    return flashProcess.run();
  }

  RadioNotFoundDialog dialog(progress);
  dialog.exec();

  return false;
}

QString findMassstoragePath(const QString & filename, bool onlyPath, ProgressWidget *progress)
{
  QString temppath;
  QString probefile;
  int found = 0;

  QRegularExpression fstypeRe("^(v?fat|msdos|lifs)", QRegularExpression::CaseInsensitiveOption);  // Linux: "vfat"; macOS: "msdos" or "lifs"; Win: "FAT32"

  foreach(const QStorageInfo & si, QStorageInfo::mountedVolumes()) {
    //qDebug() << si.rootPath() << si.name() << si.device() << si.displayName() << si.fileSystemType() << si.isReady() << si.bytesTotal() << si.blockSize();
    if (!si.isReady() || si.isReadOnly() || !QString(si.fileSystemType()).contains(fstypeRe))
      continue;
    temppath = si.rootPath();
    probefile = temppath % "/" % filename;
    qDebug() << "Searching for" << probefile;
    if (QFile::exists(probefile)) {
      found++;
      qDebug() << probefile << "found";
    }
  }

  if (found == 1)
    return onlyPath ? temppath : probefile;
  else if (found > 1) {
    QMessageBox::critical(progress, CPN_STR_TTL_ERROR, filename % " " % QCoreApplication::translate("RadioInterface", "found in multiple locations"));
  }

  return QString();
}
