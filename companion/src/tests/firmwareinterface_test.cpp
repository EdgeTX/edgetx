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

#include "gtests.h"

#include "storage/firmwareinterface.h"

#include <QtEndian>

// Layout of an EdgeTX UF2 bootloader: vector table, ".version" and
// boot_version at the start of .text, and the UF2 info file further on
static QByteArray bootloaderImage(const QByteArray &bootVersion,
                                  const QByteArray &boardId)
{
  QByteArray data(0x298, '\x5a');
  data.append("BOOT10");
  data.append(bootVersion);
  data.append('\0');
  data.append(QByteArray(0x1000, '\xff'));
  data.append("EDGETX_UF2 FAT16   ");
  data.append('\0');
  if (!boardId.isNull()) {
    data.append("UF2 Bootloader 3.0.0\r\nBoard-ID: " + boardId +
                "\r\nDate: 2026-11-15\r\n");
    data.append('\0');
  }
  return data;
}

TEST(FirmwareInterface, parseBootloaderInfo)
{
  QString flavour, version;

  // both strings present
  EXPECT_TRUE(FirmwareInterface::parseBootloaderInfo(
      bootloaderImage("edgetx-tx15-pre-3.0.0-selfbuild (6e923f58)", "tx15"),
      flavour, version));
  EXPECT_EQ(flavour, "tx15");
  EXPECT_EQ(version, "pre-3.0.0-selfbuild (6e923f58)");

  // boot_version only
  EXPECT_TRUE(FirmwareInterface::parseBootloaderInfo(
      bootloaderImage("edgetx-tx16smk3-3.0.0 (abcdef12)", QByteArray()),
      flavour, version));
  EXPECT_EQ(flavour, "tx16smk3");
  EXPECT_EQ(version, "3.0.0 (abcdef12)");

  // Board-ID only (e.g. a bootloader with a different version string)
  EXPECT_TRUE(FirmwareInterface::parseBootloaderInfo(
      bootloaderImage("Factory bootloader", "gx10"), flavour, version));
  EXPECT_EQ(flavour, "gx10");
  EXPECT_TRUE(version.isEmpty());

  // Board-ID wins, a disagreeing boot_version gives no version
  EXPECT_TRUE(FirmwareInterface::parseBootloaderInfo(
      bootloaderImage("edgetx-t15-3.0.0 (abcdef12)", "t15pro"), flavour,
      version));
  EXPECT_EQ(flavour, "t15pro");
  EXPECT_TRUE(version.isEmpty());

  // no identification (e.g. STM32 ROM area or erased flash)
  EXPECT_FALSE(FirmwareInterface::parseBootloaderInfo(QByteArray(0x10000, '\xff'),
                                                      flavour, version));
  EXPECT_TRUE(flavour.isEmpty());
  EXPECT_TRUE(version.isEmpty());

  // garbage instead of a flavour
  EXPECT_FALSE(FirmwareInterface::parseBootloaderInfo(
      bootloaderImage("edgetx-\x01\x02-x", QByteArray()), flavour, version));
}

TEST(FirmwareInterface, encodeUF2RoundTrip)
{
  constexpr uint32_t addr = 0x90000000;

  // labelled version stamps, as in a colour LCD firmware
  QByteArray image(1000, '\0');
  image.append("FW\037\033: edgetx-tx15");
  image.append('\0');
  image.append("VERS\037\033: 3.0.0 (abcdef12)");
  image.append('\0');
  image.append(QByteArray(301, '\x42'));  // not a multiple of 256

  const QByteArray uf2 = FirmwareInterface::encodeUF2(addr, image);
  const qsizetype blocks = (image.size() + 255) / 256;
  ASSERT_EQ(uf2.size(), blocks * 512);

  for (qsizetype i = 0; i < blocks; i++) {
    const char *block = uf2.constData() + i * 512;
    EXPECT_EQ(qFromLittleEndian<uint32_t>(block + 12), addr + i * 256);
    EXPECT_EQ(qFromLittleEndian<uint32_t>(block + 20), uint32_t(i));
    EXPECT_EQ(qFromLittleEndian<uint32_t>(block + 24), uint32_t(blocks));
  }

  FirmwareInterface fw(uf2);
  EXPECT_TRUE(fw.isUF2());
  EXPECT_TRUE(fw.isValid());
  EXPECT_EQ(fw.getFlavour(), "tx15");
  EXPECT_EQ(fw.getVersion(), "3.0.0 (abcdef12)");
  EXPECT_EQ(fw.getFlash(), image);
  EXPECT_EQ(fw.typeFileExtn(), "uf2");
}
