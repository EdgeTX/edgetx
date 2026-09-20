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

// Flex switches (FL1/FL2) are not listed in the hwdefs json, so Companion adds
// them itself (BoardJson::afterLoadFixups). The set of boards must match the
// firmware targets that define FLEXSW (MAX_FLEX_SWITCHES); a board missing
// here loses FL1/FL2 and everything indexed after them.

#include "gtests.h"

#include "firmwares/eeprominterface.h"
#include "firmwares/boardjson.h"
#include "firmwares/boards.h"

namespace {

struct FlexCase {
  const char* flavour;
  int stdSwitches;  // switches in the hwdefs json, including function switches
};

class FlexSwitchesBoard : public ::testing::TestWithParam<FlexCase>
{
 protected:
  void SetUp() override
  {
    Firmware::setCurrentVariant(Firmware::getFirmwareForFlavour(GetParam().flavour));
    ASSERT_NE(getCurrentFirmware(), nullptr)
        << GetParam().flavour << " firmware must be registered for this test";
  }
};

}  // namespace

// Firmware indexes flex switches straight after the physical switches
// (switchGetMaxSwitches()), which is what the simulator and YAML rely on.
TEST_P(FlexSwitchesBoard, FlexSwitchesFollowPhysicalSwitches)
{
  const Board::Type board = getCurrentBoard();
  const int n = GetParam().stdSwitches;

  EXPECT_EQ(Boards::getCapability(board, Board::FlexSwitches), 2);
  EXPECT_EQ(Boards::getSwitchYamlIndex(QStringLiteral("FL1"), BoardJson::YLT_REF, board), n);
  EXPECT_EQ(Boards::getSwitchYamlIndex(QStringLiteral("FL2"), BoardJson::YLT_REF, board), n + 1);
}

INSTANTIATE_TEST_SUITE_P(
    FlexBoards, FlexSwitchesBoard,
    ::testing::Values(FlexCase{"tx16s", 10}, FlexCase{"tx16smk3", 16},
                      FlexCase{"f16", 8}, FlexCase{"v16", 10}, FlexCase{"mt12", 4}),
    [](const ::testing::TestParamInfo<FlexCase>& i) { return std::string(i.param.flavour); });

// Radios whose firmware has MAX_FLEX_SWITCHES == 0 (no FLEXSW) must not gain
// any, even though most of them have flex inputs.
TEST(FlexSwitchesAbsent, BoardsWithoutFlexSwitches)
{
  for (const char* flavour : {"x10", "t16", "t18", "st16", "pl18"}) {
    Firmware* fw = Firmware::getFirmwareForFlavour(flavour);
    ASSERT_NE(fw, nullptr) << flavour << " firmware must be registered for this test";
    Firmware::setCurrentVariant(fw);
    EXPECT_EQ(Boards::getCapability(getCurrentBoard(), Board::FlexSwitches), 0) << flavour;
  }
}
