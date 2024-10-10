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

#include "yaml_generalsettings.h"
#include "yaml_customfunctiondata.h"
#include "yaml_trainerdata.h"
#include "yaml_calibdata.h"
#include "yaml_switchconfig.h"
#include "yaml_moduledata.h"

#include "eeprominterface.h"
#include "edgetxinterface.h"
#include "version.h"
#include "helpers.h"

#include <QMessageBox>
#include <QPushButton>

void YamlValidateNames(GeneralSettings& gs, Board::Type board)
{
  for (int i = 0; i < CPN_MAX_INPUTS; i++) {
    YamlValidateName(gs.inputConfig[i].name, board);
  }

  for (int i = 0; i < CPN_MAX_SWITCHES; i++) {
    YamlValidateName(gs.switchConfig[i].name, board);
  }
}

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
  e_backlight_mode_all = e_backlight_mode_keys + e_backlight_mode_sticks,
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
  {  GeneralSettings::AUX_SERIAL_SPACEMOUSE, "SPACEMOUSE"  },
  {  GeneralSettings::AUX_SERIAL_EXT_MODULE, "EXT_MODULE"  },
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
  {  MODULE_TYPE_FLYSKY_AFHDS2A, "TYPE_FLYSKY_AFHDS2A"  },
  {  MODULE_TYPE_FLYSKY_AFHDS3, "TYPE_FLYSKY_AFHDS3"  },
  {  MODULE_TYPE_LEMON_DSMP, "TYPE_LEMON_DSMP"  },
};

static const YamlLookupTable hatsModeLut = {
  {  GeneralSettings::HATSMODE_TRIMS_ONLY, "TRIMS_ONLY"  },
  {  GeneralSettings::HATSMODE_KEYS_ONLY, "KEYS_ONLY"  },
  {  GeneralSettings::HATSMODE_SWITCHABLE, "SWITCHABLE"  },
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

  auto fw = getCurrentFirmware();
  auto board = fw->getBoard();

  bool hasColorLcd = Boards::getCapability(board, Board::HasColorLcd);

  node["semver"] = VERSION;

  std::string strboard = fw->getFlavour().toStdString();
  node["board"] = strboard;

  if (rhs.manuallyEdited)
    node["manuallyEdited"] = (int)rhs.manuallyEdited;

  YamlCalibData calib(rhs.inputConfig);
  node["calib"] = calib;

  node["currModelFilename"] = patchFilenameToYaml(rhs.currModelFilename);
  node["vBatWarn"] = rhs.vBatWarn;
  node["txVoltageCalibration"] = rhs.txVoltageCalibration;

  node["vBatMin"] = rhs.vBatMin + 90;
  node["vBatMax"] = rhs.vBatMax + 120;

  if (!Boards::getCapability(board, Board::HasColorLcd)) {
    node["backlightColor"] = rhs.backlightColor;
    node["contrast"] = rhs.contrast;
    node["currModel"] = rhs.currModelIndex;
  }

  node["backlightMode"] = backlightModeLut << std::abs(rhs.backlightMode);
  node["trainer"] = rhs.trainer;
  node["PPM_Multiplier"] = rhs.PPM_Multiplier;
  node["view"] = rhs.view;
  node["fai"] = (int)rhs.fai;
  node["disableMemoryWarning"] = (int)rhs.disableMemoryWarning;
  node["beepMode"] = rhs.beeperMode;
  node["alarmsFlash"] = (int)rhs.alarmsFlash;
  node["disableAlarmWarning"] = (int)rhs.disableAlarmWarning;
  node["disableRssiPoweroffAlarm"] = (int)rhs.disableRssiPoweroffAlarm;
  node["disableTrainerPoweroffAlarm"] = (int)rhs.disableTrainerPoweroffAlarm;
  node["USBMode"] = rhs.usbMode;
  node["hatsMode"] = hatsModeLut << rhs.hatsMode;
  node["stickDeadZone"] = rhs.stickDeadZone;
  node["jackMode"] = rhs.jackMode;
  node["hapticMode"] = rhs.hapticMode;
  node["stickMode"] = rhs.stickMode;
  node["timezone"] = rhs.timezone;
  node["timezoneMinutes"] = rhs.timezoneMinutes;
  node["adjustRTC"] = (int)rhs.adjustRTC;
  node["inactivityTimer"] = rhs.inactivityTimer;

  YamlTelemetryBaudrate internalModuleBaudrate(&rhs.internalModuleBaudrate);
  node["internalModuleBaudrate"] = internalModuleBaudrate.value;

  node["internalModule"] = LookupValue(internalModuleLut, rhs.internalModule);
  node["splashMode"] = rhs.splashMode;
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
  node["audioMuteEnable"] = (int)rhs.muteIfNoSound;
  node["keysBacklight"] = (int)rhs.keysBacklight;
  node["rotEncMode"] = (int)rhs.rotEncMode;
  node["imperial"] = rhs.imperial;
  node["ppmunit"] = rhs.ppmunit;
  node["ttsLanguage"] = rhs.ttsLanguage;
  node["beepVolume"] = rhs.beepVolume + 2;
  node["wavVolume"] = rhs.wavVolume + 2;
  node["varioVolume"] = rhs.varioVolume + 2;
  node["varioPitch"] = rhs.varioPitch * 10;
  node["varioRange"] = rhs.varioRange * 15;
  node["varioRepeat"] = rhs.varioRepeat;
  node["backgroundVolume"] = rhs.backgroundVolume + 2;
  node["dontPlayHello"] = (int)rhs.dontPlayHello;
  if (hasColorLcd) {
    node["modelQuickSelect"] = (int)rhs.modelQuickSelect;
    node["modelSelectLayout"] = rhs.modelSelectLayout;
    node["labelSingleSelect"] = rhs.labelSingleSelect;
    node["labelMultiMode"] = rhs.labelMultiMode;
    node["favMultiMode"] = rhs.favMultiMode;
  } else if (Boards::getCapability(board, Board::LcdWidth) == 128) {
    node["invertLCD"] = (int)rhs.invertLCD;
  }

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
  node["pwrOnSpeed"] = rhs.pwrOnSpeed;
  node["pwrOffSpeed"] = rhs.pwrOffSpeed;
  node["pwrOffIfInactive"] = rhs.pwrOffIfInactive; // Power off after inactivity
  node["disablePwrOnOffHaptic"] = (int)rhs.disablePwrOnOffHaptic;

  for (int i = 0; i < CPN_MAX_SPECIAL_FUNCTIONS; i++) {
    const CustomFunctionData& fn = rhs.customFn[i];
    if (!fn.isEmpty()) {
      node["customFn"][std::to_string(i)] = fn;
    }
  }

  Node sticksConfig;
  sticksConfig = YamlStickConfig(rhs.inputConfig);
  if (sticksConfig && sticksConfig.IsMap()) {
    node["sticksConfig"] = sticksConfig;
  }

  Node potsConfig;
  potsConfig = YamlPotConfig(rhs.inputConfig);
  if (potsConfig && potsConfig.IsMap()) {
    node["potsConfig"] = potsConfig;
  }

  Node switchConfig;
  switchConfig = YamlSwitchConfig(rhs.switchConfig);
  if (switchConfig && switchConfig.IsMap()) {
    node["switchConfig"] = switchConfig;
  }

  Node flexSwitches;
  flexSwitches = YamlSwitchesFlex(rhs.switchConfig);
  if (flexSwitches && flexSwitches.IsMap()) {
    node["flexSwitches"] = flexSwitches;
  }

  node["ownerRegistrationID"] = rhs.registrationId;

  if (Boards::getCapability(board, Board::HasIMU)) {
    node["imuMax"] = rhs.imuMax;
    node["imuOffset"] = rhs.imuOffset;
  }

  // OneBit sampling (X9D only?)
  node["uartSampleMode"] = rhs.uartSampleMode;

  if (hasColorLcd)
    node["selectedTheme"] = rhs.selectedTheme;

  // Radio level tabs control (global settings)
  if (hasColorLcd)
    node["radioThemesDisabled"] = (int)rhs.radioThemesDisabled;

  node["radioGFDisabled"] = (int)rhs.radioGFDisabled;
  node["radioTrainerDisabled"] = (int)rhs.radioTrainerDisabled;
  // Model level tabs control (global setting)
  node["modelHeliDisabled"] = (int)rhs.modelHeliDisabled;
  node["modelFMDisabled"] = (int)rhs.modelFMDisabled;
  node["modelCurvesDisabled"] = (int)rhs.modelCurvesDisabled;
  node["modelGVDisabled"] = (int)rhs.modelGVDisabled;
  node["modelLSDisabled"] = (int)rhs.modelLSDisabled;
  node["modelSFDisabled"] = (int)rhs.modelSFDisabled;
  node["modelCustomScriptsDisabled"] = (int)rhs.modelCustomScriptsDisabled;
  node["modelTelemetryDisabled"] = (int)rhs.modelTelemetryDisabled;

  return node;
}

bool convert<GeneralSettings>::decode(const Node& node, GeneralSettings& rhs)
{
  if (!node.IsMap()) return false;

  // for (const auto& n : node) {
  //   qDebug() << QString::fromStdString(n.first.Scalar());
  // }

  radioSettingsVersion = SemanticVersion();

  if (node["semver"]) {
    node["semver"] >> rhs.semver;
    if (SemanticVersion().isValid(rhs.semver)) {
      radioSettingsVersion = SemanticVersion(QString(rhs.semver));
    }
    else {
      qDebug() << "Invalid settings version:" << rhs.semver;
      memset(rhs.semver, 0, sizeof(rhs.semver));
    }
  }

  qDebug() << "Settings version:" << radioSettingsVersion.toString();

  if (radioSettingsVersion > SemanticVersion(VERSION)) {
    QString prmpt = QCoreApplication::translate("YamlGeneralSettings", "Warning: File version %1 is not supported by this version of Companion!\n\nModel and radio settings may be corrupted if you continue.");
    prmpt = prmpt.arg(radioSettingsVersion.toString());
    QMessageBox msgBox;
    msgBox.setWindowTitle(QCoreApplication::translate("YamlGeneralSettings", "Read Radio Settings"));
    msgBox.setText(prmpt);
    msgBox.setIcon(QMessageBox::Warning);
    QPushButton *pbAccept = new QPushButton(CPN_STR_TTL_ACCEPT);
    QPushButton *pbDecline = new QPushButton(CPN_STR_TTL_DECLINE);
    msgBox.addButton(pbAccept, QMessageBox::AcceptRole);
    msgBox.addButton(pbDecline, QMessageBox::RejectRole);
    msgBox.setDefaultButton(pbDecline);
    msgBox.exec();
    if (msgBox.clickedButton() == pbDecline)
      return false;
  }

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
  calib.copy(rhs.inputConfig);

  node["currModel"] >> rhs.currModelIndex;
  node["currModelFilename"] >> rhs.currModelFilename;
  node["contrast"] >> rhs.contrast;
  node["vBatWarn"] >> rhs.vBatWarn;
  node["txVoltageCalibration"] >> rhs.txVoltageCalibration;

  node["vBatMin"] >> ioffset_int(rhs.vBatMin, 90);
  node["vBatMax"] >> ioffset_int(rhs.vBatMax, 120);

  node["backlightMode"] >> backlightModeLut >> rhs.backlightMode;
  node["trainer"] >> rhs.trainer;
  node["PPM_Multiplier"] >> rhs.PPM_Multiplier;
  node["view"] >> rhs.view;
  node["fai"] >> rhs.fai;
  node["disableMemoryWarning"] >> rhs.disableMemoryWarning;
  node["beepMode"] >> rhs.beeperMode;
  node["alarmsFlash"] >> rhs.alarmsFlash;
  node["disableAlarmWarning"] >> rhs.disableAlarmWarning;
  node["disableRssiPoweroffAlarm"] >> rhs.disableRssiPoweroffAlarm;
  node["disableTrainerPoweroffAlarm"] >> rhs.disableTrainerPoweroffAlarm;
  node["USBMode"] >> rhs.usbMode;
  node["hatsMode"] >> hatsModeLut >> rhs.hatsMode;
  node["stickDeadZone"] >> rhs.stickDeadZone;
  node["jackMode"] >> rhs.jackMode;
  node["hapticMode"] >> rhs.hapticMode;
  node["stickMode"] >> rhs.stickMode;
  node["timezone"] >> rhs.timezone;
  node["timezoneMinutes"] >> rhs.timezoneMinutes;
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

  node["splashMode"] >> rhs.splashMode;
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
  node["audioMuteEnable"] >> rhs.muteIfNoSound;
  node["keysBacklight"] >> rhs.keysBacklight;
  node["rotEncDirection"] >> rhs.rotEncMode;    // PR2045: read old name and
  node["rotEncMode"] >> rhs.rotEncMode;         // new, but don't write old
  node["imperial"] >> rhs.imperial;
  node["ppmunit"] >> rhs.ppmunit;
  node["ttsLanguage"] >> rhs.ttsLanguage;
  node["beepVolume"] >> ioffset_int(rhs.beepVolume, 2);
  node["wavVolume"] >> ioffset_int(rhs.wavVolume, 2);
  node["varioVolume"] >> ioffset_int(rhs.varioVolume, 2);
  node["varioPitch"] >> ifactor<int>(rhs.varioPitch, 10);
  node["varioRange"] >> ifactor<int>(rhs.varioRange, 15);
  node["varioRepeat"] >> rhs.varioRepeat;
  node["backgroundVolume"] >> ioffset_int(rhs.backgroundVolume, 2);
  node["modelQuickSelect"] >> rhs.modelQuickSelect;
  node["dontPlayHello"] >> rhs.dontPlayHello;
  node["invertLCD"] >> rhs.invertLCD;

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
  node["pwrOffIfInactive"] >> rhs.pwrOffIfInactive;   // Power off after inactivity
  node["disablePwrOnOffHaptic"] >> rhs.disablePwrOnOffHaptic;

  if (node["customFn"]) {
    // decode common for radio GF and model SF and conversion test assumes decoding a model
    modelSettingsVersion = radioSettingsVersion;
    node["customFn"] >> rhs.customFn;
  }

  // the GeneralSettings struct is initialised to hardware definition defaults which is fine for new settings
  // however when parsing saved settings set all inputs to None and override with parsed values
  // thus any inputs not parsed will be None rather than the default
  for (int i = 0; i < CPN_MAX_INPUTS; i++) {
    rhs.inputConfig[i].flexType = (Board::FlexType)Board::FLEX_NONE;
  }

  if (node["sticksConfig"]) {
    YamlStickConfig stickConfig;
    node["sticksConfig"] >> stickConfig;
    // merge
    stickConfig.copy(rhs.inputConfig);
  }

  if (node["potsConfig"]) {
    YamlPotConfig potsConfig;
    node["potsConfig"] >> potsConfig;
    // merge
    potsConfig.copy(rhs.inputConfig);
  }

  // for parsing pre v2.10 config - this is not encoded
  if (node["slidersConfig"]) {
    YamlSliderConfig slidersConfig;
    node["slidersConfig"] >> slidersConfig;
    // merge
    slidersConfig.copy(rhs.inputConfig);
  }

  // the GeneralSettings struct is initialised to hardware definition defaults which is fine for new settings
  // however when parsing saved settings set all switches to None and override with parsed values
  // thus any switches not parsed will be None rather than the default
  for (int i = 0; i < CPN_MAX_SWITCHES; i++) {
    rhs.switchConfig[i].type = Board::SWITCH_NOT_AVAILABLE;
  }

  if (node["switchConfig"]) {
    YamlSwitchConfig switchConfig;
    node["switchConfig"] >> switchConfig;
    switchConfig.copy(rhs.switchConfig);
  }

  // MUST be parsed after switchConfig
  if (node["flexSwitches"]) {
    YamlSwitchesFlex flexSwitches;
    node["flexSwitches"] >> flexSwitches;
    // merge
    flexSwitches.copy(rhs.switchConfig);
  }

  node["ownerRegistrationID"] >> rhs.registrationId;

  // depreciated in 2.11
  node["gyroMax"] >> rhs.imuMax;
  node["gyroOffset"] >> rhs.imuOffset;

  node["imuMax"] >> rhs.imuMax;
  node["imuOffset"] >> rhs.imuOffset;

  // OneBit sampling (X9D only?)
  node["uartSampleMode"] >> rhs.uartSampleMode;

  node["selectedTheme"] >> rhs.selectedTheme;

  // Radio level tabs control (global settings)
  node["radioThemesDisabled"] >> rhs.radioThemesDisabled;
  node["radioGFDisabled"] >> rhs.radioGFDisabled;
  node["radioTrainerDisabled"] >> rhs.radioTrainerDisabled;
  // Model level tabs control (global setting)
  node["modelHeliDisabled"] >> rhs.modelHeliDisabled;
  node["modelFMDisabled"] >> rhs.modelFMDisabled;
  node["modelCurvesDisabled"] >> rhs.modelCurvesDisabled;
  node["modelGVDisabled"] >> rhs.modelGVDisabled;
  node["modelLSDisabled"] >> rhs.modelLSDisabled;
  node["modelSFDisabled"] >> rhs.modelSFDisabled;
  node["modelCustomScriptsDisabled"] >> rhs.modelCustomScriptsDisabled;
  node["modelTelemetryDisabled"] >> rhs.modelTelemetryDisabled;

  node["modelSelectLayout"] >> rhs.modelSelectLayout;
  node["labelSingleSelect"] >> rhs.labelSingleSelect;
  node["labelMultiMode"] >> rhs.labelMultiMode;
  node["favMultiMode"] >> rhs.favMultiMode;

  //  override critical settings after import
  //  TODO: for consistency move up call stack to use existing eeprom and profile conversions
  if (needsConversion)
    rhs.init();

  // perform integrity checks and fix-ups
  YamlValidateNames(rhs, fw->getBoard());
  rhs.validateFlexSwitches();

  return true;
}
}  // namespace YAML
