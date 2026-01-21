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
#include "progresswidget.h"
#include "storage.h"
#include "helpers.h"
#include "firmwareinterface.h"

#include <QMessageBox>
#include <QFileDialog>

#include <chrono>
#include <stdexcept>

#define TR(msg) QCoreApplication::translate("RadioInterface", msg)

constexpr int  UF2_TRANSFER_SIZE            {4 * 1024};
constexpr char UF2_FIRMWARE_FILENAME[]      {"CURRENT.UF2"};
constexpr char UF2_INFO_FILENAME[]          {"INFO_UF2.TXT"};
//constexpr char UF2_INDEX_FILENAME[]         {"INDEX.HTM"};  // commented to stop compiler warning
constexpr char UF2_NEW_FIRMWARE_FILENAME[]  {"firmware.uf2"};

using namespace std::chrono_literals;

FirmwareReaderWorker::FirmwareReaderWorker(QObject *parent) :
  QThread(parent)
{
}

FirmwareReaderWorker::~FirmwareReaderWorker()
{
  //qDebug() << "FirmwareReaderWorker destructor called";
}

void FirmwareReaderWorker::run()
{
  emit statusChanged(tr("Reading..."));
  emit progressChanged(0, 100);
  isUf2DeviceFound() ? runUf2() : runDfu();
}

void FirmwareReaderWorker::runDfu()
{
  try {
    auto device_filter = DfuDeviceFilter::empty_filter();
    auto devices = device_filter->find_devices();

    if (devices.empty()) {
      throw std::runtime_error(tr("No DFU devices found").toStdString());
    } else if (devices.size() > 1) {
      throw std::runtime_error(tr("More than one DFU device found").toStdString());
    }

    emit newMessage(tr("Reset state"));
    auto &device = devices[0];
    device.reset_state();

    auto addr = device.default_start_address();
    auto ctx = device.start_upload(addr, 0);

    size_t total_length = ctx->get_length();
    size_t xfer_size = ctx->get_transfer_size();
    size_t bytes_uploaded = 0;

    emit newMessage("placeholder");  // anchor for updateMessage

    QByteArray data;
    while (bytes_uploaded < total_length) {
      auto rest_length = uint32_t(total_length - bytes_uploaded);
      auto single_xfer_size = std::min(uint32_t(xfer_size), rest_length);
      bytes_uploaded += single_xfer_size;

      emit progressChanged(bytes_uploaded, total_length);
      emit updateMessage(tr("Reading %1 of %2").arg(bytes_uploaded).arg(total_length));
      data.append(ctx->upload(single_xfer_size));
    }

    emit statusChanged(tr("Reading finished"));
    emit complete(data);

    // TODO: handle shouldStop

  } catch (const std::exception &e) {
    emit error(tr("DFU failed: %1").arg(e.what()));
  }
}

void FirmwareReaderWorker::runUf2()
{
  try {
    QString path = findMassStoragePath(UF2_FIRMWARE_FILENAME);
    QStorageInfo si(path);
    QFile fw(path);
    char buf[UF2_TRANSFER_SIZE];
    const int blockstotal = (fw.size() + UF2_TRANSFER_SIZE - 1) / UF2_TRANSFER_SIZE;

    QByteArray data;
    if (fw.open(QIODevice::ReadOnly)) {
      int blockcnt = 0;
      qint64 bytesread = 0;
      int read = fw.read(buf, UF2_TRANSFER_SIZE);
      if (read == 0 && fw.size() > 0) {
        throw std::runtime_error(tr("Error reading %1 (reason: no data read)")
                                 .arg(fw.fileName()).toStdString());
      } else if (read < 0) {
        throw std::runtime_error(tr("Error reading %1 (reason: %2)")
                                 .arg(fw.fileName())
                                 .arg(fw.errorString()).toStdString());
      }

      emit newMessage("placeholder");  // anchor for updateMessage

      do
      {
        bytesread += read;
        blockcnt++;
        data.append(buf, read);
        emit progressChanged(blockcnt, blockstotal);
        emit updateMessage(tr("Read block %1 of %2").arg(blockcnt).arg(blockstotal));
        read = fw.read(buf, UF2_TRANSFER_SIZE);
      } while (read > 0);

      if (read < 0) {
        data.clear();
        throw std::runtime_error(tr("Error reading %1 (reason: %2)")
                                 .arg(fw.fileName())
                                 .arg(fw.errorString()).toStdString());
      } else if (bytesread < fw.size()) {
        data.clear();
        throw std::runtime_error(tr("Error reading %1 (reason: read %2 of %3)")
                                 .arg(fw.fileName())
                                 .arg(bytesread)
                                 .arg(fw.size()).toStdString());
      }

    } else {
      throw std::runtime_error(tr("Cannot open %1 (reason: %2)")
                               .arg(fw.fileName())
                               .arg(fw.errorString()).toStdString());
    }

    fw.close();

    emit statusChanged(tr("Reading finished"));
    emit complete(data);

  } catch (const std::exception &e) {
    emit error(tr("UF2 failed: %1").arg(e.what()));
  }
}

FirmwareWriterWorker::FirmwareWriterWorker(const QByteArray &data, QObject *parent) :
  QThread(parent),
  firmwareData(data)
{
}

FirmwareWriterWorker::~FirmwareWriterWorker()
{
  //qDebug() << "FirmwareWriterWorker destructor called";
}

void FirmwareWriterWorker::run()
{
  emit statusChanged(tr("Initialise"));
  emit progressChanged(0, 100);
  isUf2DeviceFound() ? runUf2() : runDfu();
}

void FirmwareWriterWorker::runUf2()
{
  try {
    QString path = findMassStoragePath(UF2_FIRMWARE_FILENAME, true);
    if (path.isEmpty()) {
      throw std::runtime_error(tr("Cannot find USB device containing %1")
                               .arg(UF2_FIRMWARE_FILENAME).toStdString());
    }

    QStorageInfo si(path);
    if (si.bytesAvailable() < firmwareData.size()) {
      throw std::runtime_error(tr("Insufficient free space on %1 to write new firmware")
                               .arg(QDir::toNativeSeparators(path)).toStdString());
    }

    QFile newfw(Helpers::concatPath(path, UF2_NEW_FIRMWARE_FILENAME));
    int blockstotal = (firmwareData.size() + UF2_TRANSFER_SIZE - 1) / UF2_TRANSFER_SIZE;

    if (firmwareData.size() == 0)
      throw std::runtime_error(tr("No data to write to new firmware file").toStdString());

    emit statusChanged(tr("Writing..."));

    if (newfw.open(QIODevice::ReadWrite)) {
      int blockcnt = 0;
      qint64 byteswritten = 0;
      QByteArray ba = firmwareData.mid(blockcnt * UF2_TRANSFER_SIZE, UF2_TRANSFER_SIZE);

      emit newMessage("placeholder");  // anchor for updateMessage

      do
      {
        qint64 written = newfw.write(ba, ba.size());
        if (written < 0) {
          throw std::runtime_error(tr("Error writing %1 (reason: %2)")
                                   .arg(QDir::toNativeSeparators(newfw.fileName()))
                                   .arg(newfw.errorString()).toStdString());
        } else if (written < ba.size()) {
          throw std::runtime_error(tr("Error writing block to %1 (reason: bytes written %2 of %3)")
                                      .arg(QDir::toNativeSeparators(newfw.fileName()))
                                      .arg(written)
                                      .arg(ba.size()).toStdString());
        }

        newfw.flush();
        byteswritten += written;
        blockcnt++;
        emit progressChanged(blockcnt, blockstotal);
        emit updateMessage(tr("Writing block %1 of %2").arg(blockcnt).arg(blockstotal));

        ba.clear(); // just to be on the safe side
        ba = firmwareData.mid(blockcnt * UF2_TRANSFER_SIZE, UF2_TRANSFER_SIZE);
      } while (ba.size() > 0);

      if (byteswritten < firmwareData.size()) {
        throw std::runtime_error(tr("Error writing %1 (reason: written %2 of %3)")
                                 .arg(QDir::toNativeSeparators(newfw.fileName()))
                                 .arg(byteswritten)
                                 .arg(firmwareData.size()).toStdString());
      }

    } else {
      throw std::runtime_error(tr("Cannot open %1 (reason: %2)")
                               .arg(QDir::toNativeSeparators(newfw.fileName()))
                               .arg(newfw.errorString()).toStdString());
    }

    newfw.close();
    emit statusChanged(tr("Writing finished"));
    emit complete();

  } catch (const std::exception &e) {
    emit error(tr("UF2 failed: %1").arg(e.what()));
  }
}

void FirmwareWriterWorker::runDfu()
{
  try {
    auto device_filter = DfuDeviceFilter::empty_filter();
    auto devices = device_filter->find_devices();

    if (devices.empty()) {
      throw std::runtime_error(tr("No DFU devices found").toStdString());
    } else if (devices.size() > 1) {
      throw std::runtime_error(tr("More than one DFU device found").toStdString());
    }

    emit newMessage(tr("Reset state"));
    auto &device = devices[0];
    device.reset_state();

    SliceU8 buffer_slice((const uint8_t *)firmwareData.constData(),
                         firmwareData.size());

    if (!is_uf2_payload(buffer_slice)) {
      auto addr = device.default_start_address();
      writeRegion(device, addr, buffer_slice);
    } else {
      writeUf2(device, buffer_slice);
    }

    device.leave();

    emit statusChanged(tr("Writing finished"));
    emit complete();

    // TODO: handle shouldStop

  } catch (const std::exception &e) {
    emit error(tr("DFU failed: %1").arg(e.what()));
  }
}

void FirmwareWriterWorker::writeUf2(DfuDevice &device, const SliceU8 &data)
{
  auto range_it = UF2RangeIterator::from_slice(data);
  auto addr_range = UF2AddressRange::new_empty();

  while (range_it->next(*addr_range)) {
    auto addr = addr_range->start_address();
    const auto &payload = addr_range->payload();

    uint32_t reboot_address = 0;

    if (addr_range->reboot_address(reboot_address)) {
      rebootAndRediscover(device, addr, payload, reboot_address, 30s);
    } else {
      writeRegion(device, addr, payload);
    }
  }
}

void FirmwareWriterWorker::writeRegion(const DfuDevice &device, uint32_t addr,
                                       const SliceU8 &data)
{
  emit statusChanged(tr("Erasing..."));

  auto start_address = addr;
  auto end_address = start_address + data.size() - 1;

  auto ctx = device.start_download(start_address, end_address);
  auto erase_pages = ctx->get_erase_pages();
  auto pages = erase_pages.size();

  emit newMessage("placeholder");  // anchor for updateMessage

  for (size_t i = 0; i < pages; i++) {
    updateEraseStatus(i + 1, pages);
    ctx->page_erase(erase_pages[i]);
  }

  emit statusChanged(tr("Writing..."));

  size_t bytes_downloaded = 0;
  auto data_ptr = data.data();
  auto xfer_size = ctx->get_transfer_size();

  emit newMessage("placeholder");  // anchor for updateMessage

  while (bytes_downloaded < data.size()) {
    auto single_xfer_size =
        std::min(uint32_t(xfer_size), uint32_t(data.size() - bytes_downloaded));
    bytes_downloaded += single_xfer_size;
    updateDownloadStatus(bytes_downloaded, data.size());
    ctx->download(addr, SliceU8(data_ptr, single_xfer_size));
    addr += single_xfer_size;
    data_ptr += single_xfer_size;
  }
}

void FirmwareWriterWorker::updateEraseStatus(size_t page, size_t pages)
{
  emit progressChanged(page, pages);
  emit updateMessage(tr("Erasing page %1 of %2").arg(page).arg(pages));
}

void FirmwareWriterWorker::updateDownloadStatus(size_t bytes, size_t total)
{
  emit progressChanged(bytes, total);
  emit updateMessage(tr("Writing %1 of %2").arg(bytes).arg(total));
}

template <typename Duration>
void FirmwareWriterWorker::rebootAndRediscover(DfuDevice &device, uint32_t addr,
                                               const SliceU8 &data,
                                               uint32_t reboot_addr,
                                               Duration timeout)
{
  emit newMessage(tr("Rebooting into DFU..."));
  emit progressChanged(0, 2);

  device.reboot(addr, data, reboot_addr);
  emit progressChanged(1, 2);

  emit newMessage(tr("Waiting for device to reconnect..."));

  auto start = std::chrono::steady_clock::now();

  while (std::chrono::steady_clock::now() - start < timeout) {
    if (device.rediscover()) {
      emit newMessage(tr("Device reconnected"));
      emit progressChanged(2, 2);
      return;
    }
  }

  throw std::runtime_error(
      tr("Timeout while reconnecting to device").toStdString());
}

template <typename Worker>
void connectProgress(Worker *worker, ProgressWidget *progress)
{
  // lock the progress dialog for as long as the thread is running
  progress->connect(worker, &QThread::started, progress,
                    [progress]() { progress->lock(true); });

  progress->connect(worker, &QThread::finished, progress,
                    [progress]() { progress->lock(false); });

  // delete worker once the thread is finished
  progress->connect(worker, &QThread::finished, worker, &QObject::deleteLater);

  // progress and status handling
  progress->connect(worker, &Worker::progressChanged, progress,
                    [progress](int val, int max) {
                      if (progress->maximum() != max)
                        progress->setMaximum(max);
                      progress->setValue(val);
                    });

  progress->connect(worker, &Worker::statusChanged, progress,
                    [progress](const QString& status) {
                      progress->updateInfoAndMessages(status);
                    });

  progress->connect(worker, &Worker::newMessage, progress,
                    [progress](const QString& msg) {
                      progress->addMessage(msg);
                    });

  progress->connect(worker, &Worker::updateMessage, progress,
                    [progress](const QString& msg) {
                      progress->updateLastMessage(msg);
                    });

  progress->connect(worker, &Worker::error, progress,
                    [progress](const QString& status) {
                      progress->addMessage(status, QtFatalMsg);
                      progress->setInfo(status);
                    });
}

bool readFirmware(const std::function<void(const QByteArray &)>& onComplete,
                  const std::function<void(const QString &)>& onError,
                  ProgressWidget *progress)
{
  auto worker = std::make_unique<FirmwareReaderWorker>();
  connectProgress(worker.get(), progress);

  // connect closures
  progress->connect(worker.get(), &FirmwareReaderWorker::complete, progress, onComplete);
  progress->connect(worker.get(), &FirmwareReaderWorker::error, progress, onError);

  // now start it!
  worker.release()->start();
  return true;
}

bool readFirmware(const std::function<void(const QByteArray &)>& onComplete,
                  ProgressWidget *progress)
{
  auto worker = std::make_unique<FirmwareReaderWorker>(progress);
  connectProgress(worker.get(), progress);

  // write data to file on success
  progress->connect(worker.get(), &FirmwareReaderWorker::complete, progress, onComplete);

  // now start it!
  worker.release()->start();
  return true;
}

bool readFirmware(const QString &filename, ProgressWidget *progress)
{
  QFile::remove(filename);
  return readFirmware(
      [filename](const QByteArray &data) {
        QFile file(filename);
        if (!file.open(QFile::WriteOnly)) {
          //qDebug() << "Unable to write to " << filename;
        } else {
          file.write(data);
          file.close();
          //qDebug() << "Firmware written to " << filename;
        }
      },
      progress);
}

bool readFirmware(QByteArray &data, ProgressWidget *progress)
{
  return readFirmware([&data](const QByteArray &_data) { data = _data; },
                      progress);
}

bool writeFirmware(const QString &filename, ProgressWidget *progress)
{
  QFile f(filename);
  if (!f.open(QIODeviceBase::ReadOnly)) {
    QMessageBox::critical(nullptr, TR("Write Firmware"),
                          TR("Cannot open file '%1': %2")
                          .arg(QDir::toNativeSeparators(filename))
                          .arg(f.errorString()));
  }

  auto data = f.readAll();
  f.close();

  return writeFirmware(data, progress);
}

bool writeFirmware(const QByteArray &data, ProgressWidget *progress)
{
  auto worker = std::make_unique<FirmwareWriterWorker>(data, progress);
  connectProgress(worker.get(), progress);

  // now start it!
  worker.release()->start();
  return true;
}

QString printDevicesInfo(const Vec<DfuDevice> &devices)
{
  QString ret;
  for (const auto &device : devices) {
    auto dev_info = device.device_info();
    ret.append(QString("%1:%2: %3 (%4)\n").arg(QString::number(dev_info.vendor_id, 16))
                                          .arg(QString::number(dev_info.product_id, 16))
                                          .arg(dev_info.product_string.c_str())
                                          .arg(QString::number(device.default_start_address(), 16)));
  }

  return ret;
}

QString getDevicesInfo()
{
  try {
    auto device_filter = DfuDeviceFilter::empty_filter();
    auto devices = device_filter->find_devices();
    if (devices.empty()) {
      return QString(TR("No DFU devices found"));
    }

    return printDevicesInfo(devices);

  } catch (const std::exception& e) {
    return TR("Error: %1").arg(e.what());
  }

  return QString();
}

bool readSettings(const QString &filename, ProgressWidget *progress)
{
  QFile file(filename);
  if (file.exists() && !file.remove()) {
    QMessageBox::warning(nullptr, CPN_STR_TTL_ERROR,
                         TR("Could not delete temporary file: %1")
                         .arg(QDir::toNativeSeparators(filename)));
    return false;
  }

  return readSettingsSDCard(filename, progress);
}

bool readSettingsSDCard(const QString &filename, ProgressWidget *progress,
                        bool fromRadio)
{
  QString radioPath;
  if (fromRadio) {
    radioPath = findMassStoragePath("RADIO", true, progress);
    qDebug() << "Searching for SD card, found" << radioPath;
  } else {
    radioPath = g.currentProfile().sdPath();
    if (!QFile::exists(radioPath % "/RADIO"))
      radioPath.clear();
  }

  if (radioPath.isEmpty()) {
    QMessageBox::critical(progress, CPN_STR_TTL_ERROR, TR("Unable to find SD card!"));
    return false;
  }

  RadioData radioData;
  Storage inputStorage(radioPath);

  if (!inputStorage.load(radioData)) {
    QString errorMsg = inputStorage.error();
    if (errorMsg.isEmpty())
      errorMsg = TR("Failed to read Models and Settings from")
                    % QDir::toNativeSeparators(radioPath);

    QMessageBox::critical(progress, CPN_STR_TTL_ERROR, errorMsg);
    return false;
  }

  Storage outputStorage(filename);
  if (!outputStorage.write(radioData)) {
    QMessageBox::critical(progress, CPN_STR_TTL_ERROR,
                          TR("Failed to write Models and Setting file") % " "
                          % QDir::toNativeSeparators(filename));
    return false;
  }

  return QFileInfo(filename).exists();
}

QString findMassStoragePath(const QString &filename, bool onlyPath, ProgressWidget *progress)
{
  QString foundPath;
  QString foundProbefile;
  int found = 0;
  // Linux: "vfat"; macOS: "msdos" or "lifs"; Win: "FAT32"
  QRegularExpression fstypeRe("^(v?fat|msdos|lifs)",
                              QRegularExpression::CaseInsensitiveOption);

  foreach (const QStorageInfo &si, QStorageInfo::mountedVolumes()) {
    if (!si.isReady() || si.isReadOnly() || !QString(si.fileSystemType()).contains(fstypeRe))
      continue;

/*     qDebug() << "device:" << si.device()
             << "type:" << si.fileSystemType()
             << "block size:" << si.blockSize()
             << "capacity:" << si.bytesTotal() / 1000 / 1000 << "MB"
             << "available:" << si.bytesFree() / 1000/ 1000 << "MB"
             << "root path:" << si.rootPath();
 */
    QString temppath = si.rootPath();
    QString probefile = Helpers::concatPath(temppath, filename);
    qDebug() << "Searching for" << probefile;
    if (QFile::exists(probefile)) {
      found++;
      foundPath = temppath;
      foundProbefile = probefile;
      qDebug() << probefile << "found";
    }
  }

  if (found == 1) {
    return onlyPath ? foundPath : foundProbefile;
  } else if (found > 1) {
    QMessageBox::critical(progress, CPN_STR_TTL_ERROR, QDir::toNativeSeparators(filename)
                          % " " % TR("found in multiple locations"));
  }

  return QString();
}

QString getFirmwareFilesFilter()
{
  return isUf2DeviceFound() ? QString(UF2_FILES_FILTER) : QString(FIRMWARE_FILES_FILTER);
}

Uf2Info getUf2Info()
{
  Uf2Info info;
  QString path = findMassStoragePath(UF2_INFO_FILENAME);
  if (path.isEmpty())
    return info;

  QFile file(path);
  if (!file.open(QFile::ReadOnly)) {
    QMessageBox::critical(nullptr, CPN_STR_TTL_ERROR,
                          TR("Error opening file %1:\n%2.")
                          .arg(QDir::toNativeSeparators(path)).arg(file.errorString()));
    return info;
  }

  QByteArray filedata;
  const char *version_label = "UF2 Bootloader";
  const char *board_label = "Board-ID:";
  const char *date_label = "Date:";

  filedata = file.readLine();

  do
  {
    if (filedata.startsWith(version_label))
      info.version = filedata.mid(QString(version_label).size()).trimmed();
    else if (filedata.startsWith(board_label))
      info.board = filedata.mid(QString(board_label).size()).trimmed();
    else if (filedata.startsWith(date_label))
      info.date = filedata.mid(QString(date_label).size()).trimmed();

    filedata = file.readLine();

  } while (!filedata.isEmpty());

  file.close();

  qDebug() << "UF2 information - version:" << info.version
           << "board:" << info.board
           << "date:" << info.date;

  return info;
}

bool isUf2DeviceFound()
{
  return !findMassStoragePath("INFO_UF2.TXT", false).isEmpty();
}

bool isDfuDeviceFound()
{
  try {
    auto device_filter = DfuDeviceFilter::empty_filter();
    auto devices = device_filter->find_devices();
    if (devices.empty()) {
      return false;
    }
  } catch (const std::exception& e) {
    return false;
  }

  return true;
}

bool isRadioConnected()
{
  return isUf2DeviceFound() || isDfuDeviceFound();
}

bool writeFirmwareToFile(QWidget *parent, const QByteArray &data, ProgressWidget *progress, bool promptForFile)
{
  progress->updateInfoAndMessages(TR("Writing..."));
  progress->addMessage(TR("Initialise"));

  FirmwareInterface fw(data);
  if (!fw.isValid()) {
    QString errMsg(TR("Firmware is not valid"));
    progress->addMessage(errMsg, QtFatalMsg);
    progress->setInfo(errMsg);
    return false;
  }

  QString backupDir;

  if (g.currentProfile().getVariantFromType() == fw.getFlavour()) {
    backupDir = g.currentProfile().pBackupDir();
  }
  else {
    // try to find a profile but there could be more than one profile for same variant
    // if more than one take safer approach and use global
    progress->addMessage(TR("Searching profiles for firmware type..."));
    int cnt = 0;
    int profId = 0;
    QMap<int, QString> map;
    map = g.getActiveProfiles();
    if (map.size() > 0) {
      QMapIterator<int, QString> i(map);
      while (i.hasNext()) {
          i.next();
          if (g.getProfile(i.key()).getVariantFromType() == fw.getFlavour()) {
            progress->addMessage(TR("Firmware type match found in profile %1 %2")
                                  .arg(QString::number(i.key()))
                                  .arg(i.value()));
            profId = i.key();
            cnt++;
          }
      }

      progress->addMessage(TR("%1 matches found. Using profile %2 %3")
                            .arg(QString::number(cnt))
                            .arg(QString::number(profId))
                            .arg(g.getProfile(profId).name()));

      if (cnt > 0)
        backupDir = g.getProfile(profId).pBackupDir();
    }
  }

  if (backupDir.isEmpty())
    backupDir = g.backupDir();

  // include time in file name as there could be multiple backups in a day
  QString filePath = QString("%1/fw-%2-%3-%4.%5")
                      .arg(backupDir.replace("\\", "/"))  // for Windows
                      .arg(fw.getFlavour())
                      .arg(QDate(QDate::currentDate()).toString("yyyyMMdd"))
                      .arg(QTime(QTime::currentTime()).toString("HHmmss"))
                      .arg(fw.typeFileExtn());

  if (promptForFile) {
      filePath = QFileDialog::getSaveFileName(parent, TR("Firmware File"),
                                              filePath, getFirmwareFilesFilter());

    if (filePath.isEmpty()) {
      return false;
    }
  } else if (backupDir.isEmpty()) {
    progress->addMessage(TR("Backup directory not configured for profile or application"),
                         QtFatalMsg);
    return false;
  } else if (!QFileInfo::exists(backupDir)) {
    progress->addMessage(TR("Configured backup directory does not exist: %1")
                         .arg(QDir::toNativeSeparators(backupDir)), QtFatalMsg);
    return false;
  }

  QFile f(filePath);
  if (!f.open(QIODevice::ReadWrite)) {
    progress->addMessage(TR("Error opening: %1 (reason: %2)")
                         .arg(QDir::toNativeSeparators(f.fileName()))
                         .arg(f.errorString()), QtFatalMsg);
    progress->setInfo(TR("Writing failed"));
    return false;
  }

  if (f.write(data) <= 0) {
    progress->addMessage(TR("Error writing: %1 (reason: %2)")
                         .arg(QDir::toNativeSeparators(f.fileName()))
                         .arg(f.errorString()), QtFatalMsg);
    progress->setInfo(TR("Writing failed"));
    f.close();
    return false;
  } else {
    progress->updateInfoAndMessages(TR("Writing finished"));
  }

  f.close();
  return true;
}

QStringList dfuFileExtensions()
{
  return QStringList() << "bin" << "uf2";
}

QStringList uf2FileExtensions()
{
  return QStringList() << "uf2";
}

QStringList firmwareFileExtensions()
{
  return dfuFileExtensions();
}
