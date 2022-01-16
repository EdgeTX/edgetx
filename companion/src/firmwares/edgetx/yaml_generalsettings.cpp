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

const YamlLookupTable uartModeLut = {
  {  GeneralSettings::AUX_SERIAL_OFF, "MODE_NONE"  },
  {  GeneralSettings::AUX_SERIAL_TELE_MIRROR, "MODE_TELEMETRY_MIRROR"  },
  {  GeneralSettings::AUX_SERIAL_TELE_IN, "MODE_TELEMETRY"  },
  {  GeneralSettings::AUX_SERIAL_SBUS_TRAINER, "MODE_SBUS_TRAINER"  },
  {  GeneralSettings::AUX_SERIAL_LUA, "MODE_LUA"  },
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

namespace YAML
{

ENUM_CONVERTER(GeneralSettings::BeeperMode, beeperModeLut);

Node convert<GeneralSettings>::encode(const GeneralSettings& rhs)
{
  Node node;

  std::string board = getCurrentFirmware()->getFlavour().toStdString();
  node["board"] = board;

  node["version"] = CPN_CURRENT_SETTINGS_VERSION;

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
  node["jackMode"] = rhs.jackMode;
  node["hapticMode"] = rhs.hapticMode;
  node["stickMode"] = rhs.stickMode;
  node["timezone"] = rhs.timezone;
  node["adjustRTC"] = (int)rhs.adjustRTC;
  node["inactivityTimer"] = rhs.inactivityTimer;
  node["telemetryBaudrate"] = rhs.telemetryBaudrate;  // TODO: conversion???
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
  node["jitterFilter"] = (int)rhs.jitterFilter;
  node["disableRtcWarning"] = (int)rhs.rtcCheckDisable;  // TODO: verify
  node["keysBacklight"] = (int)rhs.keysBacklight;
  node["imperial"] = rhs.imperial;
  node["ttsLanguage"] = rhs.ttsLanguage;
  node["beepVolume"] = rhs.beepVolume + 2;
  node["wavVolume"] = rhs.wavVolume + 2;
  node["varioVolume"] = rhs.varioVolume + 2;
  node["varioPitch"] = rhs.varioPitch * 10;
  node["varioRange"] = rhs.varioRange * 15;
  node["varioRepeat"] = rhs.varioRepeat;
  node["backgroundVolume"] = rhs.backgroundVolume + 2;
  node["auxSerialMode"] = uartModeLut << rhs.auxSerialMode;
  node["aux2SerialMode"] = uartModeLut << rhs.aux2SerialMode;
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

  node["version"] >> rhs.version;

  // Decoding uses profile firmare therefore all conversions are performed on the fly
  // So set board to firmware board
  // However we should alert users that conversions may have occured
  // If conversion should rename file with _converted suffix as done for bin
  // Need to pass back messages and flags like converted so they can be handled in a more suitable section

  rhs.variant = Board::BOARD_UNKNOWN;

  std::string flavour;
  node["board"] >> flavour;

  auto fw = getCurrentFirmware();
  auto msfw = Firmware::getFirmwareForFlavour(QString(flavour.c_str()));

  if (flavour.empty()) {
    qDebug() << "Warning: Settings file does not contain board flavour! Default firmware board will be used";
    flavour = msfw->getFlavour().toStdString();
  }

  qDebug() << "Settings version:" << rhs.version << "File flavour:" << flavour.c_str() << "Firmware flavour:" << fw->getFlavour();

  if (fw->getFlavour().toStdString() != flavour) {
    QString prmpt = QCoreApplication::translate("YamlGeneralSettings", "File board (%1) does not match current profile board (%2), models and settings will be converted where possible.\n\nDo you wish to continue?").arg(Boards::getBoardName(msfw->getBoard())).arg(Boards::getBoardName(fw->getBoard()));

    if (QMessageBox::question(NULL, CPN_STR_APP_NAME, prmpt, (QMessageBox::Yes | QMessageBox::No), QMessageBox::No) != QMessageBox::Yes) {
      //  TODO: this triggers an error in the calling code so we need a graceful way to handle
      return false;
    }
  }

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
  node["jackMode"] >> rhs.jackMode;
  node["hapticMode"] >> rhs.hapticMode;
  node["stickMode"] >> rhs.stickMode;
  node["timezone"] >> rhs.timezone;
  node["adjustRTC"] >> rhs.adjustRTC;
  node["inactivityTimer"] >> rhs.inactivityTimer;
  node["telemetryBaudrate"] >> rhs.telemetryBaudrate;  // TODO: conversion???

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
  node["jitterFilter"] >> rhs.jitterFilter;
  node["disableRtcWarning"] >> rhs.rtcCheckDisable;  // TODO: verify
  node["keysBacklight"] >> rhs.keysBacklight;
  node["imperial"] >> rhs.imperial;
  node["ttsLanguage"] >> rhs.ttsLanguage;
  node["beepVolume"] >> ioffset_int(rhs.beepVolume, 2);
  node["wavVolume"] >> ioffset_int(rhs.wavVolume, 2);
  node["varioVolume"] >> ioffset_int(rhs.varioVolume, 2);
  node["varioPitch"] >> ifactor<int>(rhs.varioPitch, 10);
  node["varioRange"] >> ifactor<int>(rhs.varioRange, 15);
  node["varioRepeat"] >> rhs.varioRepeat;
  node["backgroundVolume"] >> ioffset_int(rhs.backgroundVolume, 2);
  node["auxSerialMode"] >> uartModeLut >> rhs.auxSerialMode;
  node["aux2SerialMode"] >> uartModeLut >> rhs.aux2SerialMode;
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

  return true;
}
}  // namespace YAML
