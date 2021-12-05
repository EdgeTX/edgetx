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

#include "yaml_modeldata.h"
#include "yaml_rawswitch.h"
#include "yaml_mixdata.h"
#include "yaml_rawsource.h"
#include "yaml_expodata.h"
#include "yaml_curvedata.h"
#include "yaml_moduledata.h"

#include "modeldata.h"
#include "output_data.h"
#include "eeprominterface.h"

#include <string>

static const YamlLookupTable timerModeLut = {
    {TimerData::TIMERMODE_OFF, "OFF"},
    {TimerData::TIMERMODE_ON, "ON"},
    {TimerData::TIMERMODE_START, "START"},
    {TimerData::TIMERMODE_THR, "THR"},
    {TimerData::TIMERMODE_THR_REL, "THR_REL"},
    {TimerData::TIMERMODE_THR_START, "THR_START"},
};

static const YamlLookupTable trainerModeLut = {
  {  TRAINER_MODE_OFF, "OFF"  },
  {  TRAINER_MODE_MASTER_JACK, "MASTER_TRAINER_JACK"  },
  {  TRAINER_MODE_SLAVE_JACK, "SLAVE"  },
  {  TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE, "MASTER_SBUS_EXT"  },
  {  TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE, "MASTER_CPPM_EXT"  },
  {  TRAINER_MODE_MASTER_BATTERY_COMPARTMENT, "MASTER_BATT_COMP"  },
  {  TRAINER_MODE_MASTER_BLUETOOTH, "MASTER_BT"  },
  {  TRAINER_MODE_SLAVE_BLUETOOTH, "SLAVE_BT"  },
  {  TRAINER_MODE_MULTI, "MASTER_MULTI"  },
};

struct YamlTrim {
  int mode = 0;
  int ref = 0;
  int value = 0;

  YamlTrim() = default;
  YamlTrim(int mode, int ref, int value):
    mode(mode), ref(ref), value(value)
  {}
};

struct YamlThrTrace {
  RawSource src;

  YamlThrTrace() = default;

  YamlThrTrace(unsigned int cpn_value)
  {
    if (cpn_value == 0) {
      src = RawSource(SOURCE_TYPE_STICK, 2/* throttle */);
      return;
    }
    cpn_value--;
    Boards board(getCurrentBoard());
    int pots = board.getCapability(Board::Pots);
    int sliders = board.getCapability(Board::Sliders);
    if (cpn_value < (unsigned int)(pots + sliders)) {
      src = RawSource(SOURCE_TYPE_STICK, 4/* sticks */ + cpn_value);
    }
    else {
      cpn_value -= pots + sliders;
      src = RawSource(SOURCE_TYPE_CH, cpn_value);
    }
  }

  unsigned int toCpn()
  {
    switch (src.type) {
      case SOURCE_TYPE_STICK:
        if (src.index == 2 /* throttle */) {
          return 0;
        } else {
          return src.index - 4/* sticks */ + 1;
        }
        break;
      case SOURCE_TYPE_CH: {
        Boards board(getCurrentBoard());
        int pots = board.getCapability(Board::Pots);
        int sliders = board.getCapability(Board::Sliders);
        return pots + sliders + src.index;
      } break;
      default:
        break;
    }
    return 0;
  }
};

namespace YAML
{
Node convert<TimerData>::encode(const TimerData& rhs)
{
  Node node;
  node["swtch"] = rhs.swtch;
  node["mode"] = timerModeLut << rhs.mode;
  node["name"] = rhs.name;
  node["minuteBeep"] = (int)rhs.minuteBeep;
  node["countdownBeep"] = rhs.countdownBeep;
  node["start"] = rhs.val;
  node["persistent"] = rhs.persistent;
  node["countdownStart"] = rhs.countdownStart;
  node["value"] = rhs.pvalue;
  return node;
}

bool convert<TimerData>::decode(const Node& node, TimerData& rhs)
{
  node["swtch"] >> rhs.swtch;
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

template <>
struct convert<LimitData> {
  static Node encode(const LimitData& rhs)
  {
    Node node;
    node["min"] = rhs.min;
    node["max"] = rhs.max;
    node["revert"] = rhs.revert;
    node["offset"] = rhs.offset;
    node["ppmCenter"] = rhs.ppmCenter;
    node["symetrical"] = rhs.symetrical;
    node["failsafe"] = rhs.failsafe;
    node["name"] = rhs.name;
    node["curve"] = rhs.curve;
    return node;
  }

  static bool decode(const Node& node, LimitData& rhs)
  {
    node["min"] >> rhs.min;
    node["max"] >> rhs.max;
    node["revert"] >> rhs.revert;
    node["offset"] >> rhs.offset;
    node["ppmCenter"] >> rhs.ppmCenter;
    node["symetrical"] >> rhs.symetrical;
    node["failsafe"] >> rhs.failsafe;
    node["name"] >> rhs.name;
    node["curve"] >> rhs.curve;
    return true;
  }
};

template <>
struct convert<YamlTrim> {
  static Node encode(const YamlTrim& rhs)
  {
    Node node;
    node["value"] = rhs.value;
    if (rhs.mode < 0) {
      node["mode"] = -1;
    } else {
      node["mode"] = 2 * rhs.ref + rhs.mode;
    }
    return node;
  }
  static bool decode(const Node& node, YamlTrim& rhs)
  {
    node["value"] >> rhs.value;

    int trimMode = 0;
    node["mode"] >> trimMode;
    if (trimMode < 0) {
      rhs.mode = trimMode;
    } else {
      rhs.mode = trimMode % 2;
      rhs.ref = trimMode / 2;
    }
    return true;
  }
};

template <>
struct convert<FlightModeData> {
  static Node encode(const FlightModeData& rhs)
  {
    Node node;
    // TODO: use max number of trims for radio instead
    YamlTrim yt[CPN_MAX_TRIMS];
    for (int i=0; i<CPN_MAX_TRIMS; i++) {
      yt[i] = { rhs.trimMode[i], rhs.trimRef[i], rhs.trim[i] };
    }
    node["trim"] = convert_array<YamlTrim,CPN_MAX_TRIMS>::encode(yt);
    node["swtch"] = rhs.swtch;
    node["name"] = rhs.name;
    node["fadeIn"] = rhs.fadeIn;
    node["fadeOut"] = rhs.fadeOut;
    node["gvars"] = convert_array<int,CPN_MAX_GVARS>::encode(rhs.gvars);
    return node;
  }

  static bool decode(const Node& node, FlightModeData& rhs)
  {
    YamlTrim trims[CPN_MAX_TRIMS];
    node["trim"] >> trims;
    for (size_t i=0; i<CPN_MAX_TRIMS; i++) {
      rhs.trimMode[i] = trims[i].mode;
      rhs.trimRef[i] = trims[i].ref;
      rhs.trim[i] = trims[i].value;
    }
    node["swtch"] >> rhs.swtch;
    node["name"] >> rhs.name;
    node["fadeIn"] >> rhs.fadeIn;
    node["fadeOut"] >> rhs.fadeOut;
    node["gvars"] >> rhs.gvars;
    return true;
  }
};

Node convert<ModelData>::encode(const ModelData& rhs)
{
  Node node;
  node["header"]["name"] = rhs.name;
  node["header"]["bitmap"] = rhs.bitmap;
  // TODO: modelId

  for (int i=0; i<CPN_MAX_TIMERS; i++) {
    Node timer;
    timer = rhs.timers[i];
    node["timers"][std::to_string(i)] = timer;
  }

  node["noGlobalFunctions"] = (int)rhs.noGlobalFunctions;
  node["thrTrim"] = (int)rhs.thrTrim;
  node["trimInc"] = rhs.trimInc;
  node["displayTrims"] = rhs.trimsDisplay;
  node["disableThrottleWarning"] = (int)rhs.disableThrottleWarning;
  // node[] = rhs.beepANACenter;
  node["extendedLimits"] = (int)rhs.extendedLimits;
  node["extendedTrims"] = (int)rhs.extendedTrims;
  node["throttleReversed"] = (int)rhs.throttleReversed;

  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    Node fmNode;
    fmNode = rhs.flightModeData[i];
    node["flightModeData"].push_back(fmNode);
  }

  for (int i = 0; i < CPN_MAX_MIXERS; i++) {
    const MixData& mix = rhs.mixData[i];
    if (!mix.isEmpty()) {
      Node mixNode;
      mixNode = mix;
      node["mixData"].push_back(mixNode);
    }
  }

  for (int i = 0; i < CPN_MAX_CHNOUT; i++) {
    const LimitData& limit = rhs.limitData[i];
    if (!limit.isEmpty()) {
      Node limitNode;
      limitNode = limit;
      node["limitData"].push_back(limitNode);
    }
  }

  std::set<int> inputs;
  for (int i = 0; i < CPN_MAX_EXPOS; i++) {
    const ExpoData& expo = rhs.expoData[i];
    if (!expo.isEmpty()) {
      Node expoNode;
      expoNode = expo;
      node["expoData"].push_back(expoNode);
      inputs.insert(expo.chn);
    }
  }

  for (auto input : inputs) {
    node["inputNames"][std::to_string(input)]["val"] = rhs.inputNames[input];
  }

  for (int i = 0; i < CPN_MAX_CURVES; i++) {
    const CurveData& curve = rhs.curves[i];
    if (!curve.isEmpty()) {
      node["curves"][std::to_string(i)] = curve;
    }
  }

  YAML::Node points;
  YamlWriteCurvePoints(points, rhs.curves);
  if (points && (points.IsMap() || points.IsSequence())) {
    node["points"] = points;
  }

  // logicalSw[]
  // customFn[]
  // swashRingData

  YamlThrTrace thrTrace(rhs.thrTraceSrc);
  node["thrTraceSrc"] = thrTrace.src;

  // switchWarningStates
  // thrTrimSwitch
  // potsWarningMode
  // potsWarnEnabled[]
  // potsWarnPosition[]

  node["displayChecklist"] = (int)rhs.displayChecklist;

  // gvarData[]
  // mavlink ???
  // telemetryProtocol ???
  // frsky
  // rssiSource
  // rssiAlarms

  node["trainerMode"] = trainerModeLut << rhs.trainerMode;

  for (int i=0; i<CPN_MAX_MODULES; i++) {
    if (rhs.moduleData[i].protocol != PULSES_OFF) {
      node["moduleData"][std::to_string(i)] = rhs.moduleData[i];
    }
  }
  
  // scriptData[]
  // sensorData[]

  node["toplcdTimer"] = rhs.toplcdTimer;

  // customScreens
  // topBarData
  node["view"] = rhs.view;
  node["modelRegistrationID"] = rhs.registrationId;

  return node;
}

bool convert<ModelData>::decode(const Node& node, ModelData& rhs)
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
  // node[] >> rhs.beepANACenter;
  node["extendedLimits"] >> rhs.extendedLimits;
  node["extendedTrims"] >> rhs.extendedTrims;
  node["throttleReversed"] >> rhs.throttleReversed;

  node["flightModeData"] >> rhs.flightModeData;
  node["mixData"] >> rhs.mixData;
  node["limitData"] >> rhs.limitData;

  node["inputNames"] >> rhs.inputNames;
  node["expoData"] >> rhs.expoData;

  node["curves"] >> rhs.curves;
  YamlReadCurvePoints(node["points"], rhs.curves);

  // logicalSw[]
  // customFn[]

  // swashRingData

  YamlThrTrace thrTrace;
  node["thrTraceSrc"] >> thrTrace.src;
  rhs.thrTraceSrc = thrTrace.toCpn();

  // switchWarningStates
  // thrTrimSwitch
  // potsWarningMode
  // potsWarnEnabled[]
  // potsWarnPosition[]

  node["displayChecklist"] >> rhs.displayChecklist;

  // gvarData[]
  // mavlink ???
  // telemetryProtocol ???
  // frsky
  // rssiSource
  // rssiAlarms

  node["trainerMode"] >> trainerModeLut >> rhs.trainerMode;
  node["moduleData"] >> rhs.moduleData;
  // scriptData[]
  // sensorData[]

  node["toplcdTimer"] >> rhs.toplcdTimer;

  // customScreens
  // topBarData
  node["view"] >> rhs.view;
  node["modelRegistrationID"] >> rhs.registrationId;

  return true;
}

}  // namespace YAML
