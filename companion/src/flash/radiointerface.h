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

#include "rs_dfu.h"

#include <QThread>
#include <QString>

#include <functional>

using SliceU8 = rust::Slice<const uint8_t>;
using ::rust::Vec;

struct Uf2DfuLayout;

class FirmwareReaderWorker : public QThread
{
  Q_OBJECT

  public:
    explicit FirmwareReaderWorker(QObject *parent = nullptr);
    ~FirmwareReaderWorker();

  protected:
    void run() override;

  signals:
    void newMessage(const QString &msg);
    void updateMessage(const QString &msg);
    void progressChanged(int value, int total);
    void statusChanged(const QString &status);
    void error(const QString &error);
    void complete(const QByteArray& data);

  private:
    void runDfu();
    void runUf2();
};

class FirmwareWriterWorker : public QThread
{
  Q_OBJECT

  private:
    QByteArray firmwareData;

  public:
    explicit FirmwareWriterWorker(const QByteArray &data, QObject *parent = nullptr);
    ~FirmwareWriterWorker();

  protected:
    void run() override;

  signals:
    void newMessage(const QString &msg);
    void updateMessage(const QString &msg);
    void progressChanged(int value, int total);
    void statusChanged(const QString &status);
    void error(const QString &error);
    void complete();
    // emitted with a blocking queued connection: the receiver saves the backup
    // and sets *saved, and the new firmware is only written if it succeeded
    void backupRead(const QByteArray &data, bool *saved);
    void newWarning(const QString &msg);

  public:
    // back up the firmware in the flash area written after the UF2 reboot
    // block, once the radio has restarted into the new bootloader's DFU.
    // The backup is a complete UF2 (old bootloader, reboot block, firmware).
    void setBackupAfterReboot(const Uf2DfuLayout &layout,
                              const QByteArray &oldBootloader);

  private:
    bool backupAfterReboot = false;
    uint32_t backupBootloaderAddress = 0;
    QByteArray backupBootloader;
    uint32_t backupRebootBlockAddress = 0;
    QByteArray backupRebootPayload;

    void writeUf2(DfuDevice &device, const SliceU8 &data);
    void backupFirmware(const DfuDevice &device, uint32_t addr);
    QByteArray readFirmwareImage(const DfuDevice &device, uint32_t addr);
    void writeRegion(const DfuDevice &device, uint32_t addr, const SliceU8 &data);
    void updateEraseStatus(size_t page, size_t pages);
    void updateDownloadStatus(size_t bytes, size_t total);
    void runDfu();
    void runUf2();

    template <typename Duration>
    void rebootAndRediscover(DfuDevice &device, uint32_t addr,
                                const SliceU8 &data, uint32_t reboot_addr,
                                Duration timeout);
};


class ProgressWidget;

//  TODO make a class
struct Uf2Info {
  QString version;
  QString board;
  QString date;
};

QString findMassStoragePath(const QString &filename, bool onlyPath = false, ProgressWidget *progress = nullptr);

bool readFirmware(const std::function<void(const QByteArray &)>& onComplete,
                  const std::function<void(const QString &)>& onError,
                  ProgressWidget *progress);

bool readFirmware(QByteArray &data, ProgressWidget *progress);
bool readFirmware(const QString &filename, ProgressWidget *progress);

bool writeFirmware(const QByteArray &data, ProgressWidget *progress);
// writes UF2 firmware over DFU and calls onBackup (in the GUI thread) with a
// UF2 backup of oldBootloader and the current firmware, read between the
// bootloader and firmware stages. The firmware is not written if onBackup
// returns false.
bool writeFirmware(const QByteArray &data, ProgressWidget *progress,
                   const Uf2DfuLayout &layout, const QByteArray &oldBootloader,
                   const std::function<bool(const QByteArray &)> &onBackup);
bool writeFirmware(const QString &filename, ProgressWidget *progress);

bool readSettings(const QString &filename, ProgressWidget *progress);
bool readSettingsSDCard(const QString &filename, ProgressWidget *progress, bool fromRadio = true);

bool writeSettings(const QString &filename, ProgressWidget *progress);

// UF2 file layout as flashed over DFU: the ranges before the reboot block are
// written by the STM32 ROM DFU, the device then restarts into the bootloader
// at bootloaderAddress, whose DFU writes the ranges after it
struct Uf2DfuLayout {
  bool hasReboot = false;
  uint32_t bootloaderAddress = 0;
  uint32_t bootloaderSize = 0;      // bytes written before the reboot block
  uint32_t rebootBlockAddress = 0;  // RAM address of the reboot command
  QByteArray rebootPayload;
  uint32_t firmwareAddress = 0;     // first range written after the reboot
};

Uf2DfuLayout getUf2DfuLayout(const QByteArray &uf2);

// read size bytes at addr from the (single) connected DFU device
bool readDfuMemory(uint32_t addr, uint32_t size, QByteArray &data, QString &error);

QString printDevicesInfo(const Vec<DfuDevice> &devices);
QString getDevicesInfo();
QString getFirmwareFilesFilter();
Uf2Info getUf2Info();
bool isDfuDeviceFound();
bool isUf2DeviceFound();
bool isRadioConnected();
bool writeFirmwareToFile(QWidget *parent, const QByteArray &data, ProgressWidget *progress, bool promptForFile = false);
QStringList dfuFileExtensions();
QStringList uf2FileExtensions();
QStringList firmwareFileExtensions();
