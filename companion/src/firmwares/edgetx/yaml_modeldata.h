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
#include "modeldata.h"

#include <string>

extern const YamlLookupTable timerModeLut;
extern const YamlLookupTable trainerModeLut;

// extern const YamlLookupTable backlightModeLut;
// extern const YamlLookupTable bluetoothModeLut;
// extern const YamlLookupTable uartModeLut;
// extern const YamlLookupTable antennaModeLut;

namespace YAML {

// ENUM_CONVERTER(GeneralSettings::BeeperMode, beeperModeLut);

template <>
struct convert<TimerData> {
  // static YAML::Node encode(const GeneralSettings& rhs) {
  //       Node node;
  //       // TODO: convert struct into Node
  //       return node;
  // }

  static bool decode(const Node& node, TimerData& rhs)
  {
    // TODO: node["swtch"] >> rhs.swtch;
    node["mode"] >> timerModeLut >> rhs.mode;
    node["name"] >> rhs.name;
    node["minuteBeep"] >> rhs.minuteBeep;
    node["countdownBeep"] >> rhs.countdownBeep;
    node["start"] >> rhs.val;
    node["persistent"] >> rhs.persistent;
    node["countdownStart"] >> rhs.countdownStart;
    node["value"] >> rhs.pvalue;
    return true;
  }
};

template <>
struct convert<ModelData> {
  // static YAML::Node encode(const GeneralSettings& rhs) {
  //       Node node;
  //       // TODO: convert struct into Node
  //       return node;
  // }

  static bool decode(const Node& node, ModelData& rhs)
  {
    if (!node.IsMap()) return false;

    if (node["header"]) {
      const auto& header = node["header"];
      if (!header.IsMap()) return false;

      header["name"] >> rhs.name;
      header["bitmap"] >> rhs.bitmap;
      // TODO: modelId
    }

    if (node["timers"]) {
      const auto& timers = node["timers"];
      if (!timers.IsMap()) return false;
      timers >> rhs.timers;
    }

    node["noGlobalFunctions"] >> rhs.noGlobalFunctions;
    node["thrTrim"] >> rhs.thrTrim;
    node["trimInc"] >> rhs.trimInc;
    node["displayTrims"] >> rhs.trimsDisplay;
    node["disableThrottleWarning"] >> rhs.disableThrottleWarning;
    //node[] >> rhs.beepANACenter;
    node["extendedLimits"] >> rhs.extendedLimits;
    node["extendedTrims"] >> rhs.extendedTrims;
    node["throttleReversed"] >> rhs.throttleReversed;

    //flightModeData[]
    //mixData[]
    //limitData[]
    //inputNames[]
    //expoData[]
    //curves[]
    //logicalSw[]
    //customFn[]

    //swashRingData
    //thrTraceSrc
    //switchWarningStates
    //thrTrimSwitch
    //potsWarningMode
    //potsWarnEnabled[]
    //potsWarnPosition[]

    node["displayChecklist"] >> rhs.displayChecklist;

    //gvarData[]
    //mavlink ???
    //telemetryProtocol ???
    //frsky
    //rssiSource
    //rssiAlarms

    node["trainerMode"] >> trainerModeLut >> rhs.trainerMode;

    //moduleData[]
    //scriptData[]
    //sensorData[]

    node["toplcdTimer"] >> rhs.toplcdTimer;

    //customScreens
    //topBarData
    node["view"] >> rhs.view;
    node["modelRegistrationID"] >> rhs.registrationId;
    
    return true;
  }
};
}  // namespace YAML
