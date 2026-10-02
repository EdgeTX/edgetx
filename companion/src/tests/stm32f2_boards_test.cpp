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

// EdgeTX 2.11 was the last release for STM32F2 radios. Companion treats their
// documents as view-only, using Board::IsF2, which is read from the "cpu_type"
// of each board's (frozen) hw_defs JSON. These tests guard that mapping, so an
// F4 radio can't become read-only, nor an F2 radio writable, by accident.

#include "gtests.h"

#include "firmwares/boards.h"
#include "firmwares/eeprominterface.h"

#include <set>

TEST(Stm32F2Boards, F2BoardsAreDetected)
{
  for (Board::Type board : { Board::BOARD_TARANIS_X9D, Board::BOARD_TARANIS_X9DP,
                             Board::BOARD_TARANIS_X7, Board::BOARD_TARANIS_XLITE,
                             Board::BOARD_TARANIS_XLITES, Board::BOARD_TARANIS_X9LITE,
                             Board::BOARD_TARANIS_X9LITES, Board::BOARD_JUMPER_T12,
                             Board::BOARD_JUMPER_TLITE, Board::BOARD_JUMPER_TPRO,
                             Board::BOARD_BETAFPV_LR3PRO, Board::BOARD_RADIOMASTER_T8,
                             Board::BOARD_RADIOMASTER_TX12 }) {
    EXPECT_TRUE(Boards::getCapability(board, Board::IsF2))
        << "board: " << Boards::getBoardName(board).toStdString();
  }
}

TEST(Stm32F2Boards, F4SiblingsAreNotF2)
{
  for (Board::Type board : { Board::BOARD_TARANIS_X9DP_2019, Board::BOARD_TARANIS_X7_ACCESS,
                             Board::BOARD_TARANIS_X9E, Board::BOARD_JUMPER_T12MAX,
                             Board::BOARD_JUMPER_TPROV2, Board::BOARD_RADIOMASTER_TX12_MK2,
                             Board::BOARD_RADIOMASTER_TX16S }) {
    EXPECT_FALSE(Boards::getCapability(board, Board::IsF2))
        << "board: " << Boards::getBoardName(board).toStdString();
  }
}

TEST(Stm32F2Boards, OnlyKnownFirmwaresAreF2)
{
  const std::set<std::string> expected = {
    "edgetx-lr3pro", "edgetx-t12",    "edgetx-t8",      "edgetx-tlite", "edgetx-tpro",
    "edgetx-tx12",   "edgetx-x7",     "edgetx-x9d",     "edgetx-x9d+",  "edgetx-x9lite",
    "edgetx-x9lites", "edgetx-xlite", "edgetx-xlites",
  };

  std::set<std::string> found;
  for (Firmware *firmware : Firmware::getRegisteredFirmwares()) {
    if (Boards::getCapability(firmware->getBoard(), Board::IsF2))
      found.insert(firmware->getId().toStdString());
  }

  EXPECT_EQ(found, expected);
}
