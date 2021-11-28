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

typedef std::pair<int, std::string> YamlLookupTableElmt;
typedef std::vector<YamlLookupTableElmt> YamlLookupTable;

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

namespace YAML {

  ENUM_CONVERTER(GeneralSettings::BeeperMode, beeperModeLut);

  template<>
  struct convert<GeneralSettings> {

    // static YAML::Node encode(const GeneralSettings& rhs) {
    //       Node node;
    //       // TODO: convert struct into Node
    //       return node;
    // }

    static bool decode(const Node& node, GeneralSettings& rhs) {

      node["version"] >> rhs.version;
      node["versionBla"] >> rhs.version;
      // // calib
      node["currModel"] >> rhs.currModelIndex;
      // // filename
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
      node["bluetoothMode"] >> bluetoothModeLut >> rhs.bluetoothMode;
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
      node["auxSerialMode"] >> uartModeLut >> rhs.auxSerialMode;
      node["aux2SerialMode"] >> uartModeLut >> rhs.aux2SerialMode;
      node["antennaMode"] >> antennaModeLut >> rhs.antennaMode;
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


