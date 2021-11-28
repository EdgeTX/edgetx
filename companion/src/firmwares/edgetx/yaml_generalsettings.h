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

#include "yaml_ops.h"
#include "generalsettings.h"
#include "yaml_trainerdata.h"

namespace YAML {
  
  template<>
  struct convert<GeneralSettings> {

    // static YAML::Node encode(const GeneralSettings& rhs) {
    //       Node node;
    //       // TODO: convert struct into Node
    //       return node;
    // }

    static bool decode(const YAML::Node& node, GeneralSettings& rhs) {

      node["version"] >> rhs.version;
      node["versionBla"] >> rhs.version;
      // // calib
      node["currModel"] >> rhs.currModelIndex;
      // // filename
      node["contrast"] >> rhs.contrast;
      node["vBatWarn"] >> rhs.vBatWarn;
      node["txVoltageCalibration"] >> rhs.txVoltageCalibration;
      node["vBatMin"] >> rhs.vBatMin;
      node["vBatMax"] >> rhs.vBatMax;

      // TODO: lookup table
      //rhs.backlightMode = node["backlightMode"].as<std::string>(); // backlight_mode_all

      node["trainer"] >> rhs.trainer;
      node["view"] >> rhs.view;
      node["fai"] >> rhs.fai;
      node["disableMemoryWarning"] >> rhs.disableMemoryWarning;
      //node["beepMode"] >> rhs.beeperMode; // TODO: string mapping
      node["disableAlarmWarning"] >> rhs.disableAlarmWarning;
      node["disableRssiPoweroffAlarm"] >> rhs.disableRssiPoweroffAlarm;
      node["USBMode"] >> rhs.usbMode;
      node["jackMode"] >> rhs.jackMode;
      //node["hapticMode"] >> rhs.hapticMode; // TODO: strin mapping
      node["stickMode"] >> rhs.stickMode;
      node["timezone"] >> rhs.timezone;
      node["adjustRTC"] >> rhs.adjustRTC;
      node["inactivityTimer"] >> rhs.inactivityTimer;
      node["telemetryBaudrate"] >> rhs.telemetryBaudrate; // TODO: conversion???
      node["splashMode"] >> rhs.splashMode; // TODO: B&W only
      node["templateSetup"] >> rhs.templateSetup;
      node["hapticLength"] >> rhs.hapticLength; // TODO: conversion???
      node["speakerPitch"] >> rhs.speakerPitch; // TODO: conversion???
      node["hapticStrength"] >> rhs.hapticStrength; // TODO: conversion???
      node["beepLength"] >> rhs.beeperLength; // TODO: conversion???
      node["gpsFormat"] >> rhs.gpsFormat;
      node["speakerVolume"] >> rhs.speakerVolume; // TODO: conversion???
      node["backlightBright"] >> rhs.backlightBright;
      node["switchesDelay"] >> rhs.switchesDelay;
      node["globalTimer"] >> rhs.globalTimer;
      node["bluetoothName"] >> rhs.bluetoothName;
      node["bluetoothBaudrate"] >> rhs.bluetoothBaudrate;
      node["bluetoothMode"] >> rhs.bluetoothMode;
      node["countryCode"] >> rhs.countryCode;
      node["jitterFilter"] >> rhs.jitterFilter;
      node["disableRtcWarning"] >> rhs.rtcCheckDisable; // TODO: verify
      node["keysBacklight"] >> rhs.keysBacklight;
      node["imperial"] >> rhs.imperial;
      node["ttsLanguage"] >> rhs.ttsLanguage;
      node["beepVolume"] >> rhs.beepVolume; // TODO: conversion???
      node["wavVolume"] >> rhs.wavVolume;
      node["varioVolume"] >> rhs.varioVolume;
      node["varioPitch"] >> rhs.varioPitch;
      node["varioRange"] >> rhs.varioRange;
      node["varioRepeat"] >> rhs.varioRepeat;
      node["backgroundVolume"] >> rhs.backgroundVolume;
      //node["auxSerialMode"] >> rhs.auxSerialMode;
      //node["aux2SerialMode"] >> rhs.aux2SerialMode;
      //node["antennaMode"] >> rhs.antennaMode;
      node["backlightColor"] >> rhs.backlightColor;

      // if (node["customFn"]) {
      //   const YAML::Node& cfn = node["customFn"];
      //   for (int i = 0; i < CPN_MAX_SPECIAL_FUNCTIONS; i++) {
      //     cfn[i] >> rhs.customFn[i];
      //   }
      // }

      // TODO:
      // char switchName[CPN_MAX_SWITCHES][HARDWARE_NAME_LEN + 1];
      // unsigned int switchConfig[CPN_MAX_SWITCHES];
      // char stickName[CPN_MAX_STICKS][HARDWARE_NAME_LEN + 1];
      // char potName[CPN_MAX_KNOBS][HARDWARE_NAME_LEN + 1];
      // unsigned int potConfig[CPN_MAX_KNOBS];
      // char sliderName[CPN_MAX_SLIDERS][HARDWARE_NAME_LEN + 1];
      // unsigned int sliderConfig[CPN_MAX_SLIDERS];
      // RadioTheme::ThemeData themeData;

      node["ownerRegistrationID"] >> rhs.registrationId;

      return true;
    }
  };
}


