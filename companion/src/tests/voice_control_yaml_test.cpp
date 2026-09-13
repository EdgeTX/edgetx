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

// CI1302 voice-control (VGR/VFL) source and switch coverage for the
// HelloRadioSky V16 board. Companion reimplements the firmware's YAML
// grammar independently, so these are the only guard against VGR/VFL
// silently decoding to NONE and getting lost on save.

#include "gtests.h"

#include <QByteArray>

#include "firmwares/eeprominterface.h"
#include "firmwares/edgetx/edgetxinterface.h"
#include "firmwares/edgetx/yaml_rawsource.h"
#include "firmwares/edgetx/yaml_rawswitch.h"
#include "firmwares/boardjson.h"
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

class VoiceControlV16 : public ::testing::Test
{
 protected:
  void SetUp() override
  {
    Firmware::setCurrentVariant(Firmware::getFirmwareForFlavour("v16"));
    ASSERT_NE(getCurrentFirmware(), nullptr)
        << "v16 (HelloRadioSky V16) firmware must be registered for this test";
    ASSERT_TRUE(IS_HELLORADIOSKY_V16(getCurrentBoard()));
  }
};

class VoiceControlOtherBoard : public ::testing::Test
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

// VGR/VFL reuse the SOURCE_TYPE_SPECIAL RESERVED1/RESERVED2 slots; yaml tags
// must match the firmware's exactly ("VGR"/"VFL").
TEST_F(VoiceControlV16, MixSrcEncodeDecodeRoundTrips)
{
  const RawSource vgr(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_RESERVED1);
  const RawSource vfl(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_RESERVED2);

  EXPECT_EQ(YamlRawSourceEncode(vgr), "VGR");
  EXPECT_EQ(YamlRawSourceEncode(vfl), "VFL");

  RawSource vgrBack = YamlRawSourceDecode("VGR");
  EXPECT_EQ(vgrBack.type, SOURCE_TYPE_SPECIAL);
  EXPECT_EQ(vgrBack.index, SOURCE_TYPE_SPECIAL_RESERVED1);

  RawSource vflBack = YamlRawSourceDecode("VFL");
  EXPECT_EQ(vflBack.type, SOURCE_TYPE_SPECIAL);
  EXPECT_EQ(vflBack.index, SOURCE_TYPE_SPECIAL_RESERVED2);
}

// VGR/VFL are offered on the V16 board; RESERVED3/4 stay excluded.
TEST_F(VoiceControlV16, MixSrcAvailableOnlyForVoiceReservedSlots)
{
  Board::Type board = getCurrentBoard();
  const RawSource vgr(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_RESERVED1);
  const RawSource vfl(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_RESERVED2);
  const RawSource reserved3(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_RESERVED3);
  const RawSource reserved4(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_RESERVED4);

  EXPECT_TRUE(vgr.isAvailable(nullptr, nullptr, board));
  EXPECT_TRUE(vfl.isAvailable(nullptr, nullptr, board));
  EXPECT_FALSE(reserved3.isAvailable(nullptr, nullptr, board));
  EXPECT_FALSE(reserved4.isAvailable(nullptr, nullptr, board));
}

// VGR/VFL must not leak into other radios' source pickers.
TEST_F(VoiceControlOtherBoard, MixSrcNotAvailableOnOtherBoards)
{
  Board::Type board = getCurrentBoard();
  ASSERT_FALSE(IS_HELLORADIOSKY_V16(board));

  const RawSource vgr(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_RESERVED1);
  const RawSource vfl(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_RESERVED2);

  EXPECT_FALSE(vgr.isAvailable(nullptr, nullptr, board));
  EXPECT_FALSE(vfl.isAvailable(nullptr, nullptr, board));
}

// Look the base switch index up rather than hardcoding it, since it depends
// on how many physical switches precede VGR/VFL in the table.
TEST_F(VoiceControlV16, SwitchEncodeDecodeRoundTrips)
{
  Board::Type board = getCurrentBoard();

  for (const QString& tag : {QStringLiteral("VGR"), QStringLiteral("VFL")}) {
    int swIdx = Boards::getSwitchYamlIndex(tag, BoardJson::YLT_REF, board);
    ASSERT_GE(swIdx, 0) << tag.toStdString() << " switch not registered for this board";

    for (int pos = 0; pos < 3; pos++) {
      RawSwitch rs(SWITCH_TYPE_SWITCH, swIdx * 3 + pos + 1);

      std::string expected = QString(tag + QString::number(pos)).toStdString();
      EXPECT_EQ(YamlRawSwitchEncode(rs), expected);

      RawSwitch back = YamlRawSwitchDecode(expected);
      EXPECT_TRUE(back == rs) << "round-trip mismatch for " << expected;
    }
  }
}

// A model using VGR as a mix source and VFL2 as its switch must survive a
// full YAML round-trip unchanged.
TEST_F(VoiceControlV16, ModelUsingVoiceSourceAndSwitchRoundTrips)
{
  Board::Type board = getCurrentBoard();
  int vflIdx = Boards::getSwitchYamlIndex(QStringLiteral("VFL"), BoardJson::YLT_REF, board);
  ASSERT_GE(vflIdx, 0);

  ModelData m;
  m.clear();
  m.used = true;
  m.mixData[0].destCh = 1;
  m.mixData[0].weight = 100;
  m.mixData[0].srcRaw = RawSource(SOURCE_TYPE_SPECIAL, SOURCE_TYPE_SPECIAL_RESERVED1);       // VGR
  m.mixData[0].swtch  = RawSwitch(SWITCH_TYPE_SWITCH, vflIdx * 3 + 2 + 1);                    // VFL2

  QByteArray y1;
  ModelData m2 = roundTrip(m, y1);

  EXPECT_EQ(m2.mixData[0].destCh, m.mixData[0].destCh);
  EXPECT_EQ(m2.mixData[0].srcRaw.type,  m.mixData[0].srcRaw.type);
  EXPECT_EQ(m2.mixData[0].srcRaw.index, m.mixData[0].srcRaw.index);
  EXPECT_TRUE(m2.mixData[0].swtch == m.mixData[0].swtch)
      << "VFL2 switch was lost/changed on round-trip";

  // Fixed point: re-serialising the reloaded model must not lose it either.
  QByteArray y2;
  ModelData m3 = roundTrip(m2, y2);
  EXPECT_EQ(m3.mixData[0].srcRaw.index, m2.mixData[0].srcRaw.index);
  EXPECT_TRUE(m3.mixData[0].swtch == m2.mixData[0].swtch);
}
