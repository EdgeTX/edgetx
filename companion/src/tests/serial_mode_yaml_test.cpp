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

// Serial port modes are stored by name. A name Companion does not know
// decodes to OFF, and is then silently lost when the settings are saved, so
// every mode the firmware can write must be known here.

#include "gtests.h"

#include <QByteArray>

#include "firmwares/eeprominterface.h"
#include "firmwares/edgetx/edgetxinterface.h"
#include "firmwares/generalsettings.h"
#include "firmwares/modeldata.h"

namespace {

void setFirmware(const char* flavour)
{
  Firmware::setCurrentVariant(Firmware::getFirmwareForFlavour(flavour));
  ASSERT_NE(getCurrentFirmware(), nullptr) << flavour << " firmware must be registered";
}

}  // namespace

TEST(SerialModeYaml, trainerModesRoundTrip)
{
  setFirmware("tx15");

  GeneralSettings in;
  in.serialPort[GeneralSettings::SP_AUX1] = GeneralSettings::AUX_SERIAL_SBUS_TRAINER_INV;
  in.serialPort[GeneralSettings::SP_VCP] = GeneralSettings::AUX_SERIAL_CRSF_TRAINER;

  QByteArray yaml;
  ASSERT_TRUE(writeRadioSettingsToYaml(in, yaml));
  EXPECT_TRUE(yaml.contains("SBUS_TRAINER_INV")) << yaml.constData();
  EXPECT_TRUE(yaml.contains("CRSF_TRAINER")) << yaml.constData();

  GeneralSettings out;
  ASSERT_TRUE(loadRadioSettingsFromYaml(out, yaml));
  EXPECT_EQ(out.serialPort[GeneralSettings::SP_AUX1],
            (unsigned)GeneralSettings::AUX_SERIAL_SBUS_TRAINER_INV);
  EXPECT_EQ(out.serialPort[GeneralSettings::SP_VCP],
            (unsigned)GeneralSettings::AUX_SERIAL_CRSF_TRAINER);
}

// Master/Serial needs an SBUS trainer port. The inverted variant only counts
// where the firmware offers it, i.e. not on F4 (see serialGetSbusTrainerPort()).
TEST(SerialModeYaml, masterSerialAcceptsInvertedSbusOffF4Only)
{
  GeneralSettings gs;
  gs.serialPort[GeneralSettings::SP_AUX1] = GeneralSettings::AUX_SERIAL_SBUS_TRAINER_INV;
  ModelData model;

  setFirmware("tx15");  // STM32H7
  EXPECT_TRUE(model.isTrainerModeAvailable(gs, getCurrentFirmware(), TRAINER_MODE_MASTER_SERIAL));

  setFirmware("tx16s");  // STM32F4
  EXPECT_FALSE(model.isTrainerModeAvailable(gs, getCurrentFirmware(), TRAINER_MODE_MASTER_SERIAL));

  gs.serialPort[GeneralSettings::SP_AUX1] = GeneralSettings::AUX_SERIAL_SBUS_TRAINER;
  EXPECT_TRUE(model.isTrainerModeAvailable(gs, getCurrentFirmware(), TRAINER_MODE_MASTER_SERIAL));
}
