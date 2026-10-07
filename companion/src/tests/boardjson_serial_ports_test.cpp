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

// The AUX serial port capabilities come from each board's hw_defs JSON
// ("has_aux_serial", "has_aux2_serial" and their "_pwr" variants), which
// mirror what the firmware build actually defines (AUX_SERIAL_USART,
// AUX2_SERIAL_USART, AUX_SERIAL_PWR_GPIO, AUX2_SERIAL_PWR_GPIO). These spot
// checks cover cases the previous family-based guesses got wrong.

#include "gtests.h"

#include "firmwares/boards.h"
#include "firmwares/eeprominterface.h"

struct SerialPortCaps {
  Board::Type board;
  bool aux1;
  bool aux1Power;
  bool aux2;
  bool aux2Power;
};

TEST(BoardSerialPorts, MatchFirmware)
{
  const SerialPortCaps expected[] = {
    // AUX1 only, no power control
    {Board::BOARD_HELLORADIOSKY_V12, true, false, false, false},
    // AUX1 unswitched, AUX2 switched
    {Board::BOARD_JUMPER_T16, true, false, true, true},
    // both ports switched
    {Board::BOARD_RADIOMASTER_TX16S, true, true, true, true},
    // AUX1 switched, no AUX2
    {Board::BOARD_JUMPER_T15H7, true, true, false, false},
    // both ports, no power control
    {Board::BOARD_HORUS_X12S, true, false, true, false},
    // no AUX ports at all
    {Board::BOARD_JUMPER_T15PRO, false, false, false, false},
    {Board::BOARD_X10_EXPRESS, false, false, false, false},
    {Board::BOARD_TARANIS_X7, false, false, false, false},
    // AUX1 the old rules missed
    {Board::BOARD_FLYSKY_NV14, true, false, false, false},
  };

  for (const auto& e : expected) {
    const std::string name = Boards::getBoardName(e.board).toStdString();
    EXPECT_EQ((bool)Boards::getCapability(e.board, Board::HasAuxSerialMode), e.aux1) << name;
    EXPECT_EQ((bool)Boards::getCapability(e.board, Board::HasAuxSerialPower), e.aux1Power) << name;
    EXPECT_EQ((bool)Boards::getCapability(e.board, Board::HasAux2SerialMode), e.aux2) << name;
    EXPECT_EQ((bool)Boards::getCapability(e.board, Board::HasAux2SerialPower), e.aux2Power) << name;
  }
}

static bool hasOption(const Firmware * firmware, const char * name)
{
  for (const auto & group : firmware->getFirmwareBase()->optionGroups()) {
    for (const auto & opt : group) {
      if (QString(opt.name) == name)
        return true;
    }
  }
  return false;
}

// The "bluetooth" and "nogps" build options offered in the radio profile must
// match what the firmware can actually be built with, as recorded in hw_defs
TEST(BoardSerialPorts, BuildOptionsMatchFirmware)
{
  for (Firmware * firmware : Firmware::getRegisteredFirmwares()) {
    const Board::Type board = firmware->getBoard();
    const std::string id = firmware->getId().toStdString();
    EXPECT_EQ(hasOption(firmware, "bluetooth"),
              (bool)Boards::getCapability(board, Board::BluetoothOptional)) << id;
    EXPECT_EQ(hasOption(firmware, "nogps"),
              (bool)Boards::getCapability(board, Board::HasInternalGPS)) << id;
  }
}

struct VariantCaps {
  const char * id;
  bool bluetooth;
  bool aux1;
  bool aux2;
  bool gps;
};

TEST(BoardSerialPorts, VariantOptions)
{
  const VariantCaps expected[] = {
    {"edgetx-tx16s", false, true, true, true},
    // Bluetooth takes AUX2, GPS still possible on AUX1
    {"edgetx-tx16s-bluetooth", true, true, false, true},
    {"edgetx-tx16s-nogps", false, true, true, false},
    // Bluetooth takes AUX1, the only port, so no GPS either
    {"edgetx-v16-bluetooth", true, false, false, false},
    // Bluetooth always built in, on its own UART
    {"edgetx-x12s", true, true, true, true},
    {"edgetx-v12", false, true, false, false},
  };

  for (const auto & e : expected) {
    Firmware * firmware = Firmware::getFirmwareForId(e.id);
    ASSERT_NE(firmware, nullptr) << e.id;
    EXPECT_EQ((bool)firmware->getCapability(BluetoothEnabled), e.bluetooth) << e.id;
    EXPECT_EQ((bool)firmware->getCapability(AuxSerialAvailable), e.aux1) << e.id;
    EXPECT_EQ((bool)firmware->getCapability(Aux2SerialAvailable), e.aux2) << e.id;
    EXPECT_EQ((bool)firmware->getCapability(InternalGPSAvailable), e.gps) << e.id;
  }
}
