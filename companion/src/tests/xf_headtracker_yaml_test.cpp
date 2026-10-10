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

// The YAML tags must match the firmware's (MIXSRC_XFHT_* in yaml_datastructs),
// or a model using the headtracker loses its sources when Companion saves it.

#include "firmwares/edgetx/yaml_rawsource.h"
#include "firmwares/eeprominterface.h"
#include "firmwares/generalsettings.h"
#include "gtests.h"

namespace
{

class XfHeadtracker : public ::testing::Test
{
 protected:
  void SetUp() override
  {
    Firmware::setCurrentVariant(Firmware::getFirmwareForFlavour("tx16s"));
    ASSERT_NE(getCurrentFirmware(), nullptr);
  }
};

}  // namespace

TEST_F(XfHeadtracker, MixSrcEncodeDecodeRoundTrips)
{
  const char* tags[] = {"XFHT_ROLL", "XFHT_PITCH", "XFHT_YAW"};
  for (int i = 1; i <= CPN_MAX_XFHT; i++) {
    const RawSource src(SOURCE_TYPE_XFHT, i);
    EXPECT_EQ(YamlRawSourceEncode(src), tags[i - 1]);

    RawSource back = YamlRawSourceDecode(tags[i - 1]);
    EXPECT_EQ(back.type, SOURCE_TYPE_XFHT);
    EXPECT_EQ(back.index, i);
  }

  const RawSource inverted(SOURCE_TYPE_XFHT, -3);
  EXPECT_EQ(YamlRawSourceEncode(inverted), "!XFHT_YAW");
  EXPECT_EQ(YamlRawSourceDecode("!XFHT_YAW").index, -3);

  EXPECT_EQ(YamlRawSourceEncode(RawSource(SOURCE_TYPE_XFHT, CPN_MAX_XFHT + 1)),
            "NONE");
}

TEST_F(XfHeadtracker, MixSrcNames)
{
  EXPECT_EQ(RawSource(SOURCE_TYPE_XFHT, 1).toString(), "htR");
  EXPECT_EQ(RawSource(SOURCE_TYPE_XFHT, 2).toString(), "htP");
  EXPECT_EQ(RawSource(SOURCE_TYPE_XFHT, 3).toString(), "htY");
}

TEST_F(XfHeadtracker, MixSrcAvailableOnlyWithAuxPortSet)
{
  Board::Type board = getCurrentBoard();
  const RawSource roll(SOURCE_TYPE_XFHT, 1);

  EXPECT_FALSE(roll.isAvailable(nullptr, nullptr, board));

  GeneralSettings gs;
  EXPECT_FALSE(roll.isAvailable(nullptr, &gs, board));

  gs.serialPort[GeneralSettings::SP_AUX2] =
      GeneralSettings::AUX_SERIAL_XF_HEADTRACKER;
  EXPECT_TRUE(roll.isAvailable(nullptr, &gs, board));
  EXPECT_FALSE(RawSource(SOURCE_TYPE_XFHT, CPN_MAX_XFHT + 1)
                   .isAvailable(nullptr, &gs, board));
}
