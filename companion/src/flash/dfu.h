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

#include <QtCore>

using ::rust::Slice;
using ::rust::Vec;

class DFU : public QObject
{
  Q_OBJECT

  public:
    explicit DFU(QObject * parent = nullptr) : QObject(parent) {}
    virtual ~DFU() {}

    template <typename Duration>
    static void rebootAndRediscover(DfuDevice &device, uint32_t addr, const Slice<const uint8_t> &data, uint32_t reboot_addr, Duration timeout);

    static const Vec<DfuDevice> findDevices();
    static int flashDevice(std::string filename);
    static QString printDevices(const Vec<DfuDevice> &devices);
    static void updateDownloadStatus(size_t bytes, size_t total);
    static void updateEraseStatus(size_t page, size_t pages);
    static int writeRegion(const DfuDevice &device, uint32_t addr, const Slice<const uint8_t> &data);
};
