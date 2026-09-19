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
#include "firmwares/boards.h"
#include "firmwares/eeprominterface.h"

TEST(BoardManufacturer, NoRegisteredBoardIsUnknown)
{
  for (Firmware *firmware : Firmware::getRegisteredFirmwares()) {
    const QString manufacturer = Boards::getManufacturer(firmware->getBoard());
    EXPECT_FALSE(manufacturer.isEmpty()) << "board: " << firmware->getId().toStdString();
    EXPECT_NE(manufacturer, QString(CPN_STR_UNKNOWN_ITEM))
        << "board: " << firmware->getId().toStdString();
  }
}

TEST(BoardManufacturer, KnownBoardsMatchExpectedManufacturer)
{
  EXPECT_EQ(Boards::getManufacturer(Board::BOARD_RADIOMASTER_TX16S), QString("RadioMaster"));
  EXPECT_EQ(Boards::getManufacturer(Board::BOARD_RADIOMASTER_GX15), QString("RadioMaster"));
  EXPECT_EQ(Boards::getManufacturer(Board::BOARD_FLYSKY_NV14), QString("Flysky"));
  EXPECT_EQ(Boards::getManufacturer(Board::BOARD_HELLORADIOSKY_V12), QString("HelloRadioSky"));
  EXPECT_EQ(Boards::getManufacturer(Board::BOARD_SENDUWING_H17), QString("Senduwing"));
  EXPECT_EQ(Boards::getManufacturer(Board::BOARD_JUMPER_T16), QString("Jumper"));
  EXPECT_EQ(Boards::getManufacturer(Board::BOARD_IFLIGHT_COMMANDO14), QString("iFlight"));
  EXPECT_EQ(Boards::getManufacturer(Board::BOARD_TARANIS_X9E), QString("FrSky"));
  EXPECT_EQ(Boards::getManufacturer(Board::BOARD_BETAFPV_LR3PRO), QString("BETAFPV"));
  EXPECT_EQ(Boards::getManufacturer(Board::BOARD_FATFISH_F16), QString("Fatfish"));
}
