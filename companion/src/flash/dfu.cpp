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

#include "dfu.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std::chrono_literals;

template <typename Duration>
void DFU::rebootAndRediscover(DfuDevice &device, uint32_t addr, const Slice<const uint8_t> &data, uint32_t reboot_addr, Duration timeout)
{
  qDebug() << "Rebooting into DFU...";
  device.reboot(addr, data, reboot_addr);
  auto start = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - start < timeout) {
    if (device.rediscover()) {
      return;
    }
  }
  qDebug() << "Timeout while reconnection to device";
}

const Vec<DfuDevice> DFU::findDevices()
{
  auto device_filter = DfuDeviceFilter::empty_filter();
  auto devices = device_filter->find_devices();

  if (devices.empty())
    qDebug() << "No DFU devices found";

  return devices;
}

QString DFU::printDevices(const Vec<DfuDevice> &devices)
{
  if (devices.empty())
    return tr("No DFU devices found");

  QString str;
  for (const auto &device : devices) {
    auto dev_info = device.device_info();
    str.append(QString("%1:%2 : %3 (%4)\n").arg(QString::number(dev_info.vendor_id, 16))
                                           .arg(QString::number(dev_info.product_id, 16))
                                           .arg(dev_info.product_string.c_str())
                                           .arg(QString::number(device.default_start_address(), 16)));
    for (const auto &interface : device.interfaces()) {
      str.append(QString("  %1:%2 : %3\n").arg(interface.interface())
                                          .arg(interface.alt_setting())
                                          .arg(interface.name().c_str()));
      for (const auto &segment : interface.segments()) {
        str.append(QString("    %1 -> %2\n").arg(QString::number(segment.start_addr, 16))
                                            .arg(QString::number(segment.end_addr, 16)));
      }
    }
  }

  return str;
}

void DFU::updateEraseStatus(size_t page, size_t pages)
{
  qDebug() << "Erasing page" << page << "of" << pages;
}

void DFU::updateDownloadStatus(size_t bytes, size_t total)
{
  int percent = (100 * bytes) / total;
  qDebug() << "Flashing" << percent << "%";
}

int DFU::writeRegion(const DfuDevice &device, uint32_t addr, const Slice<const uint8_t> &data)
{
  auto start_address = addr;
  auto end_address = start_address + data.size();

  auto ctx = device.start_download(start_address, end_address);
  auto erase_pages = ctx->get_erase_pages();
  auto pages = erase_pages.size();

  for (size_t i = 0; i < erase_pages.size(); i++) {
    updateEraseStatus(i + 1, pages);
    ctx->page_erase(erase_pages[i]);
  }

  size_t bytes_downloaded = 0;
  auto data_ptr = data.data();
  auto xfer_size = ctx->get_transfer_size();

  while (bytes_downloaded < data.size()) {
    auto single_xfer_size = std::min(uint32_t(xfer_size), uint32_t(data.size() - bytes_downloaded));
    bytes_downloaded += single_xfer_size;
    updateDownloadStatus(bytes_downloaded, data.size());
    ctx->download(addr, Slice<const uint8_t>(data_ptr, single_xfer_size));
    addr += single_xfer_size;
    data_ptr += single_xfer_size;
  }

  return 0;
}

int DFU::flashDevice(std::string filename)
{
  try {
    auto devices = findDevices();

    if (devices.empty()) {
      return -1;
    }

    qDebug() << printDevices(devices);

    auto file_size = std::filesystem::file_size(filename);

    std::vector<uint8_t> buffer(file_size);
    std::ifstream f(filename, std::ios::binary);
    f.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

    qDebug() << "Resetting state...";
    auto &device = devices[0];
    device.reset_state();

    Slice<const uint8_t> buffer_slice(buffer.data(), buffer.size());
    if (!is_uf2_payload(buffer_slice)) {
      auto addr = device.default_start_address();
      return writeRegion(device, addr, buffer_slice);
    }

    auto range_it = UF2RangeIterator::from_slice(buffer_slice);
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
    device.leave();

    return 0;

  } catch (const std::exception &err) {
    qDebug() << "Error:" << err.what();
    return 1;
  }
}
