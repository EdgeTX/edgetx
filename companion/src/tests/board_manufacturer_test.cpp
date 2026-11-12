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

// Boards::getManufacturer() now reads the "manufacturer" field from each
// board's hw_defs JSON (via BoardJson) instead of a hand-maintained switch
// statement. This guards against the JSON entries and the switch it replaced
// silently drifting apart, e.g. a newly-added board being forgotten (as
// happened with BOARD_SENDUWING_H17 and BOARD_RADIOMASTER_GX15, both of
// which fell through to "???" under the old switch).

#include "gtests.h"

#include "constants.h"
#include "boards/board.h"
#include "firmwares/opentx/opentxinterface.h"

TEST(BoardManufacturer, NoRegisteredBoardIsUnknown)
{
  for (Firmware *firmware : Firmware::getRegisteredFirmwares()) {
    const QString manufacturer = firmware->getCapabilityStr(Capability::Manufacturer);
    EXPECT_FALSE(manufacturer.isEmpty()) << "board: " << firmware->getId().toStdString();
    EXPECT_NE(manufacturer, QString(CPN_STR_UNKNOWN_ITEM))
        << "board: " << firmware->getId().toStdString();
  }
}

TEST(BoardManufacturer, KnownBoardsMatchExpectedManufacturer)
{
  EXPECT_EQ(Board::getBoard("tx16s")->getCapabilityStr(Capability::Manufacturer), QString("RadioMaster"));
  EXPECT_EQ(Board::getBoard("gx15")->getCapabilityStr(Capability::Manufacturer), QString("RadioMaster"));
  EXPECT_EQ(Board::getBoard("nv14")->getCapabilityStr(Capability::Manufacturer), QString("Flysky"));
  EXPECT_EQ(Board::getBoard("v12")->getCapabilityStr(Capability::Manufacturer), QString("HelloRadioSky"));
  EXPECT_EQ(Board::getBoard("h17")->getCapabilityStr(Capability::Manufacturer), QString("Senduwing"));
  EXPECT_EQ(Board::getBoard("t16")->getCapabilityStr(Capability::Manufacturer), QString("Jumper"));
  EXPECT_EQ(Board::getBoard("commando14")->getCapabilityStr(Capability::Manufacturer), QString("iFlight"));
  EXPECT_EQ(Board::getBoard("x9e")->getCapabilityStr(Capability::Manufacturer), QString("FrSky"));
  EXPECT_EQ(Board::getBoard("lr3pro")->getCapabilityStr(Capability::Manufacturer), QString("BETAFPV"));
  EXPECT_EQ(Board::getBoard("f16")->getCapabilityStr(Capability::Manufacturer), QString("Fatfish"));
}
