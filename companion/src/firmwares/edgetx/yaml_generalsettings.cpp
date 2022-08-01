/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "yaml_generalsettings.h"
#include "yaml_customfunctiondata.h"
#include "yaml_trainerdata.h"
#include "yaml_calibdata.h"
#include "yaml_switchconfig.h"
#include "yaml_moduledata.h"

#include "eeprominterface.h"
#include "edgetxinterface.h"
#include "version.h"

#include <QMessageBox>

const YamlLookupTable beeperModeLut = {
  {  GeneralSettings::BEEPER_QUIET, "mode_quiet" },
  {  GeneralSettings::BEEPER_ALARMS_ONLY, "mode_alarms" },
  {  GeneralSettings::BEEPER_NOKEYS, "mode_nokeys" },
  {  GeneralSettings::BEEPER_ALL, "mode_all"  },
};

enum BacklightMode {
  e_backlight_mode_off  = 0,
  e_backlight_mode_keys = 1,
  e_backlight_mode_sticks = 2,
  e_backlight_mode_all = e_backlight_mode_keys+e_backlight_mode_sticks,
  e_backlight_mode_on
};

const YamlLookupTable backlightModeLut = {
  {  e_backlight_mode_off, "backlight_mode_off"  },
  {  e_backlight_mode_keys, "backlight_mode_keys"  },
  {  e_backlight_mode_sticks, "backlight_mode_sticks"  },
  {  e_backlight_mode_all, "backlight_mode_all"  },
  {  e_backlight_mode_on, "backlight_mode_on"  },
};

const YamlLookupTable bluetoothModeLut = {
  {  GeneralSettings::BLUETOOTH_MODE_OFF, "OFF"  },
  {  GeneralSettings::BLUETOOTH_MODE_TELEMETRY, "TELEMETRY"  },
  {  GeneralSettings::BLUETOOTH_MODE_TRAINER, "TRAINER"  },
};

const YamlLookupTable oldUartModeLut = {
  {  GeneralSettings::AUX_SERIAL_OFF, "MODE_NONE"  },
  {  GeneralSettings::AUX_SERIAL_TELE_MIRROR, "MODE_TELEMETRY_MIRROR"  },
  {  GeneralSettings::AUX_SERIAL_TELE_IN, "MODE_TELEMETRY"  },
  {  GeneralSettings::AUX_SERIAL_SBUS_TRAINER, "MODE_SBUS_TRAINER"  },
  {  GeneralSettings::AUX_SERIAL_LUA, "MODE_LUA"  },
};

const YamlLookupTable serialPortLut = {
  {  GeneralSettings::SP_AUX1, "AUX1" },
  {  GeneralSettings::SP_AUX2, "AUX2" },
  {  GeneralSettings::SP_VCP, "VCP" },
};

const YamlLookupTable uartModeLut = {
  {  GeneralSettings::AUX_SERIAL_OFF, "NONE"  },
  {  GeneralSettings::AUX_SERIAL_TELE_MIRROR, "TELEMETRY_MIRROR"  },
  {  GeneralSettings::AUX_SERIAL_TELE_IN, "TELEMETRY_IN"  },
  {  GeneralSettings::AUX_SERIAL_SBUS_TRAINER, "SBUS_TRAINER"  },
  {  GeneralSettings::AUX_SERIAL_LUA, "LUA"  },
  {  GeneralSettings::AUX_SERIAL_CLI, "CLI"  },
  {  GeneralSettings::AUX_SERIAL_GPS, "GPS"  },
  {  GeneralSettings::AUX_SERIAL_DEBUG, "DEBUG"  },
};

const YamlLookupTable antennaModeLut = {
  {  GeneralSettings::ANTENNA_MODE_INTERNAL, "MODE_INTERNAL"  },
  {  GeneralSettings::ANTENNA_MODE_ASK, "MODE_ASK"  },
  {  GeneralSettings::ANTENNA_MODE_PER_MODEL, "MODE_PER_MODEL"  },
  {  GeneralSettings::ANTENNA_MODE_EXTERNAL, "MODE_EXTERNAL"  },
  {  GeneralSettings::ANTENNA_MODE_INTERNAL_EXTERNAL, "MODE_INTERNAL_EXTERNAL"  },
};

const YamlLookupTable internalModuleLut = {
  {  MODULE_TYPE_NONE, "TYPE_NONE"  },
  {  MODULE_TYPE_PPM, "TYPE_PPM"  },
  {  MODULE_TYPE_XJT_PXX1, "TYPE_XJT_PXX1"  },
  {  MODULE_TYPE_ISRM_PXX2, "TYPE_ISRM_PXX2"  },
  {  MODULE_TYPE_DSM2, "TYPE_DSM2"  },
  {  MODULE_TYPE_CROSSFIRE, "TYPE_CROSSFIRE"  },
  {  MODULE_TYPE_MULTIMODULE, "TYPE_MULTIMODULE"  },
  {  MODULE_TYPE_R9M_PXX1, "TYPE_R9M_PXX1"  },
  {  MODULE_TYPE_R9M_PXX2, "TYPE_R9M_PXX2"  },
  {  MODULE_TYPE_R9M_LITE_PXX1, "TYPE_R9M_LITE_PXX1"  },
  {  MODULE_TYPE_R9M_LITE_PXX2, "TYPE_R9M_LITE_PXX2"  },
  {  MODULE_TYPE_GHOST, "TYPE_GHOST"  },
  {  MODULE_TYPE_R9M_LITE_PRO_PXX2, "TYPE_R9M_LITE_PRO_PXX2"  },
  {  MODULE_TYPE_SBUS, "TYPE_SBUS"  },
  {  MODULE_TYPE_XJT_LITE_PXX2, "TYPE_XJT_LITE_PXX2"  },
  {  MODULE_TYPE_FLYSKY, "TYPE_FLYSKY"  },
};

YamlTelemetryBaudrate::YamlTelemetryBaudrate(
    const unsigned int* moduleBaudrate)
{
  if (Boards::getCapability(getCurrentFirmware()->getBoard(),
                            Board::SportMaxBaudRate) < 400000) {
    value = *moduleBaudrate;
  } else {
    value = (*moduleBaudrate + moduleBaudratesList.size() - 1) %
             moduleBaudratesList.size();
  }
}

void YamlTelemetryBaudrate::toCpn(unsigned int* moduleBaudrate,
                                  unsigned int variant)
{
  if (Boards::getCapability((Board::Type)variant, Board::SportMaxBaudRate) <
      400000) {
    *moduleBaudrate = value;
  } else {
    *moduleBaudrate = (value + 1) % moduleBaudratesList.size();
  }
}

namespace YAML
{

ENUM_CONVERTER(GeneralSettings::BeeperMode, beeperModeLut);

Node convert<GeneralSettings>::encode(const GeneralSettings& rhs)
{
  Node node;

  node["semver"] = VERSION;

  std::string board = getCurrentFirmware()->getFlavour().toStdString();
  node["board"] = board;

  YamlCalibData calib(rhs.calibMid, rhs.calibSpanNeg, rhs.calibSpanPos);
  node["calib"] = calib;

  node["currModel"] = rhs.currModelIndex;
  node["currModelFilename"] = patchFilenameToYaml(rhs.currModelFilename);
  node["contrast"] = rhs.contrast;
  node["vBatWarn"] = rhs.vBatWarn;
  node["txVoltageCalibration"] = rhs.txVoltageCalibration;

  node["vBatMin"] = rhs.vBatMin + 90;
  node["vBatMax"] = rhs.vBatMax + 120;

  node["backlightMode"] = backlightModeLut << std::abs(rhs.backlightMode);
  node["trainer"] = rhs.trainer;
  node["view"] = rhs.view;
  node["fai"] = (int)rhs.fai;
  node["disableMemoryWarning"] = (int)rhs.disableMemoryWarning;
  node["beepMode"] = rhs.beeperMode;
  node["disableAlarmWarning"] = (int)rhs.disableAlarmWarning;
  node["disableRssiPoweroffAlarm"] = (int)rhs.disableRssiPoweroffAlarm;
  node["USBMode"] = rhs.usbMode;
  node["stickDeadZone"] = rhs.stickDeadZone;
  node["jackMode"] = rhs.jackMode;
  node["hapticMode"] = rhs.hapticMode;
  node["stickMode"] = rhs.stickMode;
  node["timezone"] = rhs.timezone;
  node["adjustRTC"] = (int)rhs.adjustRTC;
  node["inactivityTimer"] = rhs.inactivityTimer;

  YamlTelemetryBaudrate internalModuleBaudrate(&rhs.internalModuleBaudrate);
  node["internalModuleBaudrate"] = internalModuleBaudrate.value;

  node["internalModule"] = LookupValue(internalModuleLut, rhs.internalModule);
  node["splashMode"] = rhs.splashMode;                // TODO: B&W only
  node["lightAutoOff"] = rhs.backlightDelay;
  node["templateSetup"] = rhs.templateSetup;
  node["hapticLength"] = rhs.hapticLength + 2;
  node["speakerPitch"] = rhs.speakerPitch * 15;
  node["hapticStrength"] = rhs.hapticStrength + 2;
  node["beepLength"] = rhs.beeperLength + 2;
  node["gpsFormat"] = rhs.gpsFormat;
  node["speakerVolume"] = rhs.speakerVolume + 12;
  node["backlightBright"] = rhs.backlightBright;
  node["blOffBright"] = rhs.backlightOffBright;
  node["switchesDelay"] = rhs.switchesDelay;
  node["globalTimer"] = rhs.globalTimer;
  node["bluetoothName"] = rhs.bluetoothName;
  node["bluetoothBaudrate"] = rhs.bluetoothBaudrate;
  node["bluetoothMode"] = bluetoothModeLut << rhs.bluetoothMode;
  node["countryCode"] = rhs.countryCode;
  node["noJitterFilter"] = (int)rhs.noJitterFilter;
  node["disableRtcWarning"] = (int)rhs.rtcCheckDisable;  // TODO: verify
  node["keysBacklight"] = (int)rhs.keysBacklight;
  node["rotEncMode"] = (int)rhs.rotEncMode;
  node["imperial"] = rhs.imperial;
  node["ttsLanguage"] = rhs.ttsLanguage;
  node["beepVolume"] = rhs.beepVolume + 2;
  node["wavVolume"] = rhs.wavVolume + 2;
  node["varioVolume"] = rhs.varioVolume + 2;
  node["varioPitch"] = rhs.varioPitch * 10;
  node["varioRange"] = rhs.varioRange * 15;
  node["varioRepeat"] = rhs.varioRepeat;
  node["backgroundVolume"] = rhs.backgroundVolume + 2;

  Node serialPort;
  for (int i = 0; i < GeneralSettings::SP_COUNT; i++) {
    if (rhs.serialPort[i] != GeneralSettings::AUX_SERIAL_OFF || rhs.serialPower[i]) {
      Node mode = uartModeLut << rhs.serialPort[i];
      serialPort[LookupValue(serialPortLut, i)]["mode"] = mode;
      serialPort[LookupValue(serialPortLut, i)]["power"] = (int)rhs.serialPower[i];
    }
  }
  if (serialPort && serialPort.IsMap())
    node["serialPort"] = serialPort;

  node["antennaMode"] = antennaModeLut << rhs.antennaMode;
  node["backlightColor"] = rhs.backlightColor;
  node["pwrOnSpeed"] = rhs.pwrOnSpeed;
  node["pwrOffSpeed"] = rhs.pwrOffSpeed;

  for (int i = 0; i < CPN_MAX_SPECIAL_FUNCTIONS; i++) {
    const CustomFunctionData& fn = rhs.customFn[i];
    if (!fn.isEmpty()) {
      node["customFn"][std::to_string(i)] = fn;
    }
  }

  Node sticksConfig;
  sticksConfig = YamlStickConfig(rhs.stickName);
  if (sticksConfig && sticksConfig.IsMap()) {
    node["sticksConfig"] = sticksConfig;
  }

  Node switchConfig;
  switchConfig = YamlSwitchConfig(rhs.switchName, rhs.switchConfig);
  if (switchConfig && switchConfig.IsMap()) {
    node["switchConfig"] = switchConfig;
  }

  Node potsConfig;
  potsConfig = YamlPotConfig(rhs.potName, rhs.potConfig);
  if (potsConfig && potsConfig.IsMap()) {
    node["potsConfig"] = potsConfig;
  }

  Node slidersConfig;
  slidersConfig = YamlSliderConfig(rhs.sliderName, rhs.sliderConfig);
  if (slidersConfig && slidersConfig.IsMap()) {
    node["slidersConfig"] = slidersConfig;
  }

  // Color lcd theme settings are not used in EdgeTx
  // RadioTheme::ThemeData themeData;

  node["ownerRegistrationID"] = rhs.registrationId;

  // Gyro (for now only xlites)
  node["gyroMax"] = rhs.gyroMax;
  node["gyroOffset"] = rhs.gyroOffset;

  // OneBit sampling (X9D only?)
  node["uartSampleMode"] = rhs.uartSampleMode;

  return node;
}

bool convert<GeneralSettings>::decode(const Node& node, GeneralSettings& rhs)
{
  if (!node.IsMap()) return false;

  // for (const auto& n : node) {
  //   qDebug() << QString::fromStdString(n.first.Scalar());
  // }

  node["semver"] >> rhs.semver;
  rhs.version = CPN_CURRENT_SETTINGS_VERSION; // depreciated in EdgeTX however data conversions use

  // Decoding uses profile firmare therefore all conversions are performed on the fly
  // So set board to firmware board
  // However we should alert users that conversions may have occured
  // If conversion should rename file with _converted suffix as done for bin
  // Need to pass back messages and flags like converted so they can be handled in a more suitable section

  //  Note: This is part of an interim workaround to ensure critical settings are set to profile board defaults
  //  TODO: remove and let board conversion occur as for eeprom and profile
  //        this will require yaml import of general and model settings to be refactored to not use current firmware
  bool needsConversion = false;
  //

  rhs.variant = Board::BOARD_UNKNOWN;

  std::string flavour;
  node["board"] >> flavour;

  auto fw = getCurrentFirmware();

  qDebug() << "Settings version:" << rhs.semver << "File flavour:" << flavour.c_str() << "Firmware flavour:" << fw->getFlavour();

  if (flavour.empty()) {
    QString prmpt = QCoreApplication::translate("YamlGeneralSettings", "Warning: Radio settings file is missing the board entry!\n\nCurrent firmware profile board will be used.\n\nDo you wish to continue?");
    if (QMessageBox::question(NULL, CPN_STR_APP_NAME, prmpt, (QMessageBox::Yes | QMessageBox::No), QMessageBox::No) != QMessageBox::Yes) {
      //  TODO: this triggers an error in the calling code so we need a graceful way to handle
      return false;
    }
    flavour = fw->getFlavour().toStdString();
    needsConversion = true;
  }
  else if (fw->getFlavour().toStdString() != flavour) {
    auto msfw = Firmware::getFirmwareForFlavour(QString(flavour.c_str()));

    QString prmpt = QCoreApplication::translate("YamlGeneralSettings", "Settings file board (%1) does not match current profile board (%2).\n\nDo you wish to continue?").arg(Boards::getBoardName(msfw->getBoard())).arg(Boards::getBoardName(fw->getBoard()));

    if (QMessageBox::question(NULL, CPN_STR_APP_NAME, prmpt, (QMessageBox::Yes | QMessageBox::No), QMessageBox::No) != QMessageBox::Yes) {
      //  TODO: this triggers an error in the calling code so we need a graceful way to handle
      return false;
    }
    needsConversion = true;
  }

  //  TODO: do not override here
  rhs.variant = fw->getBoard();

  YamlCalibData calib;
  node["calib"] >> calib;
  calib.copy(rhs.calibMid, rhs.calibSpanNeg, rhs.calibSpanPos);

  node["currModel"] >> rhs.currModelIndex;
  node["currModelFilename"] >> rhs.currModelFilename;
  node["contrast"] >> rhs.contrast;
  node["vBatWarn"] >> rhs.vBatWarn;
  node["txVoltageCalibration"] >> rhs.txVoltageCalibration;

  node["vBatMin"] >> ioffset_int(rhs.vBatMin, 90);
  node["vBatMax"] >> ioffset_int(rhs.vBatMax, 120);

  node["backlightMode"] >> backlightModeLut >> rhs.backlightMode;
  node["trainer"] >> rhs.trainer;
  node["view"] >> rhs.view;
  node["fai"] >> rhs.fai;
  node["disableMemoryWarning"] >> rhs.disableMemoryWarning;
  node["beepMode"] >> rhs.beeperMode;
  node["disableAlarmWarning"] >> rhs.disableAlarmWarning;
  node["disableRssiPoweroffAlarm"] >> rhs.disableRssiPoweroffAlarm;
  node["USBMode"] >> rhs.usbMode;
  node["stickDeadZone"] >> rhs.stickDeadZone;
  node["jackMode"] >> rhs.jackMode;
  node["hapticMode"] >> rhs.hapticMode;
  node["stickMode"] >> rhs.stickMode;
  node["timezone"] >> rhs.timezone;
  node["adjustRTC"] >> rhs.adjustRTC;
  node["inactivityTimer"] >> rhs.inactivityTimer;

  YamlTelemetryBaudrate internalModuleBaudrate;
  if (node["telemetryBaudrate"]) {
    node["telemetryBaudrate"] >> internalModuleBaudrate.value;
  } else {
    node["internalModuleBaudrate"] >> internalModuleBaudrate.value;
  }
  internalModuleBaudrate.toCpn(&rhs.internalModuleBaudrate, rhs.variant);

  if (node["internalModule"]) {
    node["internalModule"] >> internalModuleLut >> rhs.internalModule;
  } else {
    rhs.internalModule = Boards::getDefaultInternalModules(fw->getBoard());
  }

  node["splashMode"] >> rhs.splashMode;                // TODO: B&W only
  node["lightAutoOff"] >> rhs.backlightDelay;
  node["templateSetup"] >> rhs.templateSetup;
  node["hapticLength"] >> ioffset_int(rhs.hapticLength, 2);
  node["speakerPitch"] >> ifactor<unsigned int>(rhs.speakerPitch, 15);
  node["hapticStrength"] >> ioffset_int(rhs.hapticStrength, 2);
  node["beepLength"] >> ioffset_int(rhs.beeperLength, 2);
  node["gpsFormat"] >> rhs.gpsFormat;
  node["speakerVolume"] >> ioffset_int(rhs.speakerVolume, 12);
  node["backlightBright"] >> rhs.backlightBright;
  node["blOffBright"] >> rhs.backlightOffBright;
  node["switchesDelay"] >> rhs.switchesDelay;
  node["globalTimer"] >> rhs.globalTimer;
  node["bluetoothName"] >> rhs.bluetoothName;
  node["bluetoothBaudrate"] >> rhs.bluetoothBaudrate;
  node["bluetoothMode"] >> bluetoothModeLut >> rhs.bluetoothMode;
  node["countryCode"] >> rhs.countryCode;
  node["jitterFilter"] >> rhs.noJitterFilter;   // PR1363 : read old name and
  node["noJitterFilter"] >> rhs.noJitterFilter; // new, but don't write old
  node["disableRtcWarning"] >> rhs.rtcCheckDisable;  // TODO: verify
  node["keysBacklight"] >> rhs.keysBacklight;
  node["rotEncDirection"] >> rhs.rotEncMode;    // PR2045: read old name and
  node["rotEncMode"] >> rhs.rotEncMode;         // new, but don't write old
  node["imperial"] >> rhs.imperial;
  node["ttsLanguage"] >> rhs.ttsLanguage;
  node["beepVolume"] >> ioffset_int(rhs.beepVolume, 2);
  node["wavVolume"] >> ioffset_int(rhs.wavVolume, 2);
  node["varioVolume"] >> ioffset_int(rhs.varioVolume, 2);
  node["varioPitch"] >> ifactor<int>(rhs.varioPitch, 10);
  node["varioRange"] >> ifactor<int>(rhs.varioRange, 15);
  node["varioRepeat"] >> rhs.varioRepeat;
  node["backgroundVolume"] >> ioffset_int(rhs.backgroundVolume, 2);

  //  depreciated v2.7 replaced by serialPort
  if (node["auxSerialMode"]) {
    node["auxSerialMode"] >> oldUartModeLut >>
        rhs.serialPort[GeneralSettings::SP_AUX1];
  }

  //  depreciated v2.7 replaced by serialPort
  if (node["aux2SerialMode"]) {
    node["aux2SerialMode"] >> oldUartModeLut >>
        rhs.serialPort[GeneralSettings::SP_AUX2];
  }

  if (node["serialPort"]) {
    Node serialPort = node["serialPort"];
    if (serialPort.IsMap()) {
      for (const auto& port : serialPort) {
        YAML::Node port_nr = port.first >> serialPortLut;
        if (port_nr) {
          int p = port_nr.as<int>();
          if (p >= 0 && p < GeneralSettings::SP_COUNT && port.second.IsMap()) {
            port.second["mode"] >> uartModeLut >> rhs.serialPort[p];
            //  introduced v2.8
            Node port_pwr = port.second["power"];
            if (port_pwr && port_pwr.IsScalar()) {
              try {
                int pwr = port_pwr.as<int>();
                if (pwr == 0 || pwr == 1)
                  rhs.serialPower[p] = pwr;
              } catch(...) {}
            }
          }
        }
      }
    }
  }

  node["antennaMode"] >> antennaModeLut >> rhs.antennaMode;
  node["backlightColor"] >> rhs.backlightColor;
  node["pwrOnSpeed"] >> rhs.pwrOnSpeed;
  node["pwrOffSpeed"] >> rhs.pwrOffSpeed;

  node["customFn"] >> rhs.customFn;

  YamlStickConfig stickConfig;
  node["sticksConfig"] >> stickConfig;
  stickConfig.copy(rhs.stickName);

  YamlSwitchConfig switchConfig;
  node["switchConfig"] >> switchConfig;
  switchConfig.copy(rhs.switchName, rhs.switchConfig);

  YamlPotConfig potsConfig;
  node["potsConfig"] >> potsConfig;
  potsConfig.copy(rhs.potName, rhs.potConfig);

  YamlSliderConfig slidersConfig;
  node["slidersConfig"] >> slidersConfig;
  slidersConfig.copy(rhs.sliderName, rhs.sliderConfig);

  // Color lcd theme settings are not used in EdgeTx
  // RadioTheme::ThemeData themeData;

  node["ownerRegistrationID"] >> rhs.registrationId;

  // Gyro (for now only xlites)
  node["gyroMax"] >> rhs.gyroMax;
  node["gyroOffset"] >> rhs.gyroOffset;

  // OneBit sampling (X9D only?)
  node["uartSampleMode"] >> rhs.uartSampleMode;

  //  override critical settings after import
  //  TODO: for consistency move up call stack to use existing eeprom and profile conversions
  if (needsConversion)
    rhs.init();

  return true;
}
}  // namespace YAML
