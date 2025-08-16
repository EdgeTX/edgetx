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
#include "process_copy.h"
#include "progresswidget.h"
#include "storage.h"

#include "rs_dfu.h"

// #include "radionotfound.h"
// #include "helpers.h"

#include <QMessageBox>

#include <chrono>

#define TR(msg) QCoreApplication::translate("RadioInterface", msg)

using namespace std::chrono_literals;

using SliceU8 = rust::Slice<const uint8_t>;

bool readFirmware(const QString &filename, ProgressWidget *progress)
{
  bool result = false;

  QFile file(filename);
  if (file.exists() && !file.remove()) {
    QMessageBox::warning(
        NULL, CPN_STR_TTL_ERROR,
        TR("Could not delete temporary file: %1").arg(filename));
    return false;
  }

  // TODO: read firmware to file
  qDebug() << "TODO: read firmware to file";

  if (!QFileInfo(filename).exists()) {
    result = false;
  }

  return result;
}

void update_erase_status(ProgressWidget *progress, size_t page, size_t pages)
{
  progress->setValue(page);
  progress->addMessage(QString("Erasing page %1 of %2").arg(page).arg(pages));
  progress->setInfo(TR("Erasing page %1 of %2").arg(page).arg(pages));
}

void update_download_status(ProgressWidget *progress, size_t bytes, size_t total)
{
  progress->setValue(bytes);
  progress->addMessage(QString("Writing %1 of %2").arg(bytes).arg(total));
  progress->setInfo(TR("Writing %1 of %2").arg(bytes).arg(total));
}

void write_region(const DfuDevice &device, uint32_t addr, const SliceU8 &data,
                  ProgressWidget *progress)
{
  auto start_address = addr;
  auto end_address = start_address + data.size();

  auto ctx = device.start_download(start_address, end_address);
  auto erase_pages = ctx->get_erase_pages();
  auto pages = erase_pages.size();

  progress->setMaximum(pages);
  progress->setValue(0);

  for (size_t i = 0; i < pages; i++) {
    update_erase_status(progress, i + 1, pages);
    ctx->page_erase(erase_pages[i]);
  }

  size_t bytes_downloaded = 0;
  auto data_ptr = data.data();
  auto xfer_size = ctx->get_transfer_size();

  progress->setMaximum(data.size());
  progress->setValue(0);

  while (bytes_downloaded < data.size()) {
    auto single_xfer_size =
        std::min(uint32_t(xfer_size), uint32_t(data.size() - bytes_downloaded));
    bytes_downloaded += single_xfer_size;
    update_download_status(progress, bytes_downloaded, data.size());
    ctx->download(addr, SliceU8(data_ptr, single_xfer_size));
    addr += single_xfer_size;
    data_ptr += single_xfer_size;
  }
}

template <typename Duration>
void reboot_and_rediscover(DfuDevice &device, uint32_t addr,
                           const SliceU8 &data, uint32_t reboot_addr,
                           Duration timeout, ProgressWidget *progress)
{
  progress->addMessage(QString("Rebooting into DFU..."));
  progress->setInfo(TR("Rebooting into DFU..."));
  progress->setMaximum(2);
  progress->setValue(0);

  device.reboot(addr, data, reboot_addr);
  progress->setValue(1);

  progress->addMessage(QString("Waiting for device to reconnect..."));
  progress->setInfo(TR("Waiting for device to reconnect..."));

  auto start = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - start < timeout) {
    if (device.rediscover()) {
      progress->addMessage(QString("Device reconnected"));
      progress->setInfo(TR("Device reconnected"));
      progress->setValue(2);
      return;
    }
  }

  throw std::runtime_error("timeout while reconnecting to device");
}

bool writeFirmware(const QString &filename, ProgressWidget *progress)
{
  QFile f(filename);
  if (!f.open(QIODeviceBase::ReadOnly)) {
    QMessageBox::critical(
        NULL, CPN_STR_TTL_ERROR,
        TR("Error opening file %1:\n%2.").arg(filename).arg(f.errorString()));
    return false;
  }

  auto data = f.readAll();
  f.close();

  try {
    auto device_filter = DfuDeviceFilter::empty_filter();
    auto devices = device_filter->find_devices();

    if (devices.empty()) {
      qDebug() << "Error: No DFU device";
      QMessageBox::critical(NULL, CPN_STR_TTL_ERROR, TR("No DFU devices"));
      return -1;
    }

    progress->addMessage("Resetting state...");
    progress->setInfo(TR("Resetting state..."));

    auto &device = devices[0];
    device.reset_state();

    SliceU8 buffer_slice((const uint8_t *)data.constData(), data.size());
    if (!is_uf2_payload(buffer_slice)) {
      auto addr = device.default_start_address();
      write_region(device, addr, buffer_slice, progress);
      return true;
    }

    auto range_it = UF2RangeIterator::from_slice(buffer_slice);
    auto addr_range = UF2AddressRange::new_empty();

    while (range_it->next(*addr_range)) {
      auto addr = addr_range->start_address();
      const auto &payload = addr_range->payload();

      uint32_t reboot_address = 0;
      if (addr_range->reboot_address(reboot_address)) {
        reboot_and_rediscover(device, addr, payload, reboot_address, 30s, progress);
      } else {
        write_region(device, addr, payload, progress);
      }
    }

    device.leave();
    return true;

  } catch (const std::exception& err) {
    qDebug() << "Error:" << err.what();
    return false;
  }
}

bool readSettings(const QString &filename, ProgressWidget *progress)
{
  QFile file(filename);
  if (file.exists() && !file.remove()) {
    QMessageBox::warning(
        NULL, CPN_STR_TTL_ERROR,
        TR("Could not delete temporary file: %1").arg(filename));
    return false;
  }

  return readSettingsSDCard(filename, progress);
}

bool readSettingsSDCard(const QString &filename, ProgressWidget *progress,
                        bool fromRadio)
{
  QString radioPath;

  if (fromRadio) {
    radioPath = findMassstoragePath("RADIO", true, progress);
    qDebug() << "Searching for SD card, found" << radioPath;
  } else {
    radioPath = g.currentProfile().sdPath();
    if (!QFile::exists(radioPath % "/RADIO")) radioPath.clear();
  }

  if (radioPath.isEmpty()) {
    QMessageBox::critical(progress, CPN_STR_TTL_ERROR,
                          TR("Unable to find SD card!"));
    return false;
  }

  RadioData radioData;
  Storage inputStorage(radioPath);
  if (!inputStorage.load(radioData)) {
    QString errorMsg = inputStorage.error();
    if (errorMsg.isEmpty()) {
      errorMsg = TR("Failed to read Models and Settings from") % radioPath;
    }
    QMessageBox::critical(progress, CPN_STR_TTL_ERROR, errorMsg);
    return false;
  }
  Storage outputStorage(filename);
  if (!outputStorage.write(radioData)) {
    QMessageBox::critical(
        progress, CPN_STR_TTL_ERROR,
        TR("Failed to write Models and Setting file") % " " % filename);
    return false;
  }

  return QFileInfo(filename).exists();
}

QString findMassstoragePath(const QString &filename, bool onlyPath,
                            ProgressWidget *progress)
{
  QString foundPath;
  QString foundProbefile;
  int found = 0;

  // Linux: "vfat"; macOS: "msdos" or "lifs"; Win: "FAT32"
  QRegularExpression fstypeRe("^(v?fat|msdos|lifs)",
                              QRegularExpression::CaseInsensitiveOption);

  foreach (const QStorageInfo &si, QStorageInfo::mountedVolumes()) {
    if (!si.isReady() || si.isReadOnly() ||
        !QString(si.fileSystemType()).contains(fstypeRe))
      continue;

    QString temppath = si.rootPath();
    QString probefile = temppath % "/" % filename;
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
    QMessageBox::critical(progress, CPN_STR_TTL_ERROR,
                          filename % " " % TR("found in multiple locations"));
  }

  return QString();
}
