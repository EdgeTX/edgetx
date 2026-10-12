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

// Battery sensor current/power source coverage for the HelloRadioSky V15
// board. Companion reimplements the firmware's YAML grammar independently,
// so these guard against TX_BAT_CURRENT/TX_BAT_POWER silently decoding to
// NONE and getting lost on save.

#include "gtests.h"

#include <QByteArray>

#include "firmwares/eeprominterface.h"
#include "firmwares/edgetx/edgetxinterface.h"
#include "firmwares/edgetx/yaml_rawsource.h"
#include "firmwares/mixdata.h"

namespace {

ModelData roundTrip(const ModelData& in, QByteArray& yamlOut)
{
  writeModelToYaml(in, yamlOut);
  ModelData out;
  out.clear();
  loadModelFromYaml(out, yamlOut);
  return out;
}

class BatterySensorV15 : public ::testing::Test
{
 protected:
  void SetUp() override
  {
    Firmware::setCurrentVariant(Firmware::getFirmwareForFlavour("v15"));
    ASSERT_NE(getCurrentFirmware(), nullptr)
        << "v15 (HelloRadioSky V15) firmware must be registered for this test";
    ASSERT_TRUE(IS_HELLORADIOSKY_V15(getCurrentBoard()));
  }
};

class BatterySensorOtherBoard : public ::testing::Test
{
 protected:
  void SetUp() override
  {
    Firmware::setCurrentVariant(Firmware::getFirmwareForFlavour("tx16s"));
    ASSERT_NE(getCurrentFirmware(), nullptr)
        << "tx16s firmware must be registered for this test";
  }
};

}  // namespace

// The yaml tags must match the firmware's MixSources names exactly.
TEST_F(BatterySensorV15, MixSrcEncodeDecodeRoundTrips)
{
  const RawSource cur(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_TX_BAT_CURRENT);
  const RawSource pwr(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_TX_BAT_POWER);

  EXPECT_EQ(YamlRawSourceEncode(cur), "TX_BAT_CURRENT");
  EXPECT_EQ(YamlRawSourceEncode(pwr), "TX_BAT_POWER");

  RawSource curBack = YamlRawSourceDecode("TX_BAT_CURRENT");
  EXPECT_EQ(curBack.type, SOURCE_TYPE_SPECIAL);
  EXPECT_EQ(curBack.index, SOURCE_TYPE_SPECIAL_TX_BAT_CURRENT);

  RawSource pwrBack = YamlRawSourceDecode("TX_BAT_POWER");
  EXPECT_EQ(pwrBack.type, SOURCE_TYPE_SPECIAL);
  EXPECT_EQ(pwrBack.index, SOURCE_TYPE_SPECIAL_TX_BAT_POWER);
}

TEST_F(BatterySensorV15, MixSrcAvailable)
{
  Board::Type board = getCurrentBoard();
  const RawSource cur(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_TX_BAT_CURRENT);
  const RawSource pwr(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_TX_BAT_POWER);
  const RawSource reserved1(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_RESERVED1);

  EXPECT_TRUE(cur.isAvailable(nullptr, nullptr, board));
  EXPECT_TRUE(pwr.isAvailable(nullptr, nullptr, board));
  EXPECT_FALSE(reserved1.isAvailable(nullptr, nullptr, board));
}

TEST_F(BatterySensorV15, ModelUsingBatterySourceRoundTrips)
{
  ModelData m;
  m.clear();
  m.used = true;
  m.mixData[0].destCh = 1;
  m.mixData[0].weight = 100;
  m.mixData[0].srcRaw = RawSource(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_TX_BAT_CURRENT);
  m.mixData[1].destCh = 2;
  m.mixData[1].weight = 100;
  m.mixData[1].srcRaw = RawSource(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_TX_BAT_POWER);

  QByteArray y1;
  ModelData m2 = roundTrip(m, y1);

  for (int i = 0; i < 2; i++) {
    EXPECT_EQ(m2.mixData[i].srcRaw.type, m.mixData[i].srcRaw.type);
    EXPECT_EQ(m2.mixData[i].srcRaw.index, m.mixData[i].srcRaw.index);
  }
}

// The battery sources must not leak into other radios' source pickers.
TEST_F(BatterySensorOtherBoard, MixSrcNotAvailableOnOtherBoards)
{
  Board::Type board = getCurrentBoard();
  ASSERT_FALSE(IS_HELLORADIOSKY_V15(board));

  const RawSource cur(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_TX_BAT_CURRENT);
  const RawSource pwr(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_TX_BAT_POWER);

  EXPECT_FALSE(cur.isAvailable(nullptr, nullptr, board));
  EXPECT_FALSE(pwr.isAvailable(nullptr, nullptr, board));
}
