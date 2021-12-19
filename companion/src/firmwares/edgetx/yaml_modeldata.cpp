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
#include "yaml_logicalswitchdata.h"
#include "yaml_customfunctiondata.h"
#include "yaml_sensordata.h"
#include "yaml_screendata.h"

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

static const YamlLookupTable swashTypeLut = {
  {  0, "TYPE_NONE"  },
  {  1, "TYPE_120"  },
  {  2, "TYPE_120X"  },
  {  3, "TYPE_140"  },
  {  4, "TYPE_90"  },
};

static const YamlLookupTable potsWarningModeLut = {
  {  0, "WARN_OFF"  },
  {  1, "WARN_MANUAL"  },
  {  2, "WARN_AUTO"  },
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

struct YamlPotsWarnEnabled {
  unsigned int value;

  YamlPotsWarnEnabled() = default;

  YamlPotsWarnEnabled(const bool * potsWarnEnabled)
  {
    value = 0;

    for (int i = 0; i < 8; i++) {
      value |= (*(potsWarnEnabled + i) << (7 - i));
    }
  }

  void toCpn(bool * potsWarnEnabled)
  {
    memset(potsWarnEnabled, 0, sizeof(bool) * (CPN_MAX_POTS + CPN_MAX_SLIDERS));

    for (int i = 1; i <= 8; i++) {
      *(potsWarnEnabled + (i - 1)) = (bool)((value >> (8 - i)) & 1);
    }
  }
};

//  modeldata: uint64_t switchWarningStates
//  Yaml switchWarningState: AuBuEuFuG-IuJu
struct YamlSwitchWarningState {
  std::string src_str;

  YamlSwitchWarningState() = default;

  YamlSwitchWarningState(uint64_t cpn_value, unsigned int switchWarningEnable)
  {
    uint64_t states = cpn_value;

    for (int i = 0; i < Boards::getCapability(getCurrentBoard(), Board::Switches); i++) {
      if (!(switchWarningEnable & (1 << i))) {
        std::string tag = getCurrentFirmware()->getSwitchesTag(i);
        const char *sw = tag.data();

        if (tag.size() >= 2 && sw[0] == 'S') {
          char val[3];
          val[0] = sw[1];

          uint64_t value = (i == 0 ? states & 0x3 : states & 0x1);

          if (value == 0)
            val[1] = 'u';
          else if (value == 1)
            val[1] = '-';
          else
            val[1] = 'd';
          val[2] = '\0';

          src_str.append(val);
        }
      }

      states >>= (i == 0 ? 2 : 1);
    }
  }

  uint64_t toCpn()
  {
    uint64_t states = 0;

    for (unsigned int i = 0; i < src_str.size() / 2; i++) {
      auto val = src_str.substr(i * 2, 2);
      if (val[0] >= 'A' && val[0] <= 'Z' && (
          val[1] == 'u' || val[1] == '-' || val[1] == 'd')) {

        int shift = 0;
        uint64_t mask;

        char sw[3];
        sw[0] = 'S';
        sw[1] = val[0];
        sw[2] = '\0';

        int index = getCurrentFirmware()->getSwitchesIndex(sw);

        if (index > -1) {

          if (IS_HORUS_OR_TARANIS(getCurrentFirmware()->getBoard())) {
            shift = index * 2;
            mask = 0x03ull << shift;
          }
          else {
            if (index == 0) {
              mask = 0x03;
            }
            else {
              shift = index + 1;
              mask = 0x01ull << shift;
            }
          }

          states &= ~mask;

          uint64_t value;

          if (val[1] == 'u')
            value = 0;
          else if (val[1] == '-')
            value = 1;
          else
            value = 2;

          if (value) {
            states |= ((uint64_t)value << shift);
          }
        }
      }
    }

    return states;
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
    node["trim"] = encode_array(yt);
    node["swtch"] = rhs.swtch;
    node["name"] = rhs.name;
    node["fadeIn"] = rhs.fadeIn;
    node["fadeOut"] = rhs.fadeOut;
    node["gvars"] = encode_array(rhs.gvars);
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

template <>
struct convert<SwashRingData> {
  static Node encode(const SwashRingData& rhs)
  {
    Node node;
    node["type"] = swashTypeLut << rhs.type;
    node["value"] = rhs.value;
    node["collectiveSource"] = rhs.collectiveSource;
    node["aileronSource"] = rhs.aileronSource;
    node["elevatorSource"] = rhs.elevatorSource;
    node["collectiveWeight"] = rhs.collectiveWeight;
    node["aileronWeight"] = rhs.aileronWeight;
    node["elevatorWeight"] = rhs.elevatorWeight;
    return node;
  }

  static bool decode(const Node& node, SwashRingData& rhs)
  {
    node["type"] >> swashTypeLut >> rhs.type;
    node["value"] >> rhs.value;
    node["collectiveSource"] >> rhs.collectiveSource;
    node["aileronSource"] >> rhs.aileronSource;
    node["elevatorSource"] >> rhs.elevatorSource;
    node["collectiveWeight"] >> rhs.collectiveWeight;
    node["aileronWeight"] >> rhs.aileronWeight;
    node["elevatorWeight"] >> rhs.elevatorWeight;
    return true;
  }
};

template <>
struct convert<GVarData> {
  static Node encode(const GVarData& rhs)
  {
    Node node;
    node["name"] = rhs.name;
    node["min"] = rhs.min;
    node["max"] = rhs.max;
    node["popup"] = rhs.popup;
    node["prec"] = rhs.prec;
    node["unit"] = rhs.unit;
    return node;
  }

  static bool decode(const Node& node, GVarData& rhs)
  {
    node["name"] >> rhs.name;
    node["min"] >> rhs.min;
    node["max"] >> rhs.max;
    node["popup"] >> rhs.popup;
    node["prec"] >> rhs.prec;
    node["unit"] >> rhs.unit;
    return true;
  }
};

struct YamlScriptDataInput { int i; };

template <>
struct convert<YamlScriptDataInput> {
  static Node encode(const YamlScriptDataInput& rhs)
  {
    Node node;
    node["u"]["value"] = rhs.i;
    return node;
  }
  static bool decode(const Node& node, YamlScriptDataInput& rhs)
  {
    node["u"]["value"] >> rhs.i;
    return true;
  }
};

template <>
struct convert<ScriptData> {
  static Node encode(const ScriptData& rhs)
  {
    Node node;
    node["file"] = rhs.filename;
    node["name"] = rhs.name;

    for (int i=0; i < 6; i++) {
      node["inputs"][std::to_string(i)]["u"]["value"] = (int)rhs.inputs[i];
    }

    return node;
  }

  static bool decode(const Node& node, ScriptData& rhs)
  {
    node["file"] >> rhs.filename;
    node["name"] >> rhs.name;

    YamlScriptDataInput inputs[CPN_MAX_SCRIPT_INPUTS];
    node["inputs"] >> inputs;

    for (int i=0; i < CPN_MAX_SCRIPT_INPUTS; i++) {
      rhs.inputs[i] = inputs[i].i;
    }

    return true;
  }
};

template <>
struct convert<RSSIAlarmData> {
  static Node encode(const RSSIAlarmData& rhs)
  {
    Node node;
    node["disabled"] = (int)rhs.disabled;
    node["warning"] = rhs.warning - 45;
    node["critical"] = rhs.critical - 42;
    return node;
  }

  static bool decode(const Node& node, RSSIAlarmData& rhs)
  {
    node["disabled"] >> rhs.disabled;
    node["warning"] >> rhs.warning;
    rhs.warning += 45;
    node["critical"] >> rhs.critical;
    rhs.critical += 42;
    return true;
  }
};

struct YamlTelemSource {
  unsigned int src = 0;
  YamlTelemSource() = default;
  YamlTelemSource(const unsigned int src) : src(src) {}
};

template <>
struct convert<YamlTelemSource> {
  static Node encode(const YamlTelemSource& rhs)
  {
    if (rhs.src == 0)
      return Node("none");
    return Node(rhs.src - 1);
  }

  static bool decode(const Node& node, YamlTelemSource& rhs)
  {
    if (node && node.IsScalar()) {
      std::string str = node.Scalar();
      if (str == "none") {
        rhs.src = 0;
      } else {
        rhs.src = std::stoi(str) + 1;
      }
    }
    return true;
  }
};

template <>
struct convert<FrSkyBarData> {
  static Node encode(const FrSkyBarData& rhs)
  {
    Node node;
    if (rhs.source.isSet()) {
      node["source"] = rhs.source;
      node["barMin"] = rhs.barMin;
      node["barMax"] = rhs.barMax;
    }
    return node;
  }

  static bool decode(const Node& node, FrSkyBarData& rhs)
  {
    node["source"] >> rhs.source;
    node["barMin"] >> rhs.barMin;
    node["barMax"] >> rhs.barMax;
    return true;
  }
};

template <>
struct convert<FrSkyLineData> {
  static Node encode(const FrSkyLineData& rhs)
  {
    Node node, sources;
    sources = encode_array(rhs.source);
    if (sources && sources.IsMap()) {
      node["sources"] = sources;
    }
    return node;
  }

  static bool decode(const Node& node, FrSkyLineData& rhs)
  {
    node["sources"] >> rhs.source;
    return true;
  }
};

static const YamlLookupTable screenTypeLut = {
  {TELEMETRY_SCREEN_NONE, "NONE"},
  {TELEMETRY_SCREEN_NUMBERS, "VALUES"},
  {TELEMETRY_SCREEN_BARS, "BARS"},
  {TELEMETRY_SCREEN_SCRIPT, "SCRIPT"},
};

template <>
struct convert<FrSkyScreenData> {
  static Node encode(const FrSkyScreenData& rhs)
  {
    Node node;
    node["type"] = LookupValue(screenTypeLut, rhs.type);
    switch(rhs.type) {
    case TELEMETRY_SCREEN_NONE:
      break;
    case TELEMETRY_SCREEN_NUMBERS:
      node["lines"] = encode_array(rhs.body.lines);
      break;
    case TELEMETRY_SCREEN_BARS:
      node["bars"] = encode_array(rhs.body.bars);
      break;
    case TELEMETRY_SCREEN_SCRIPT:
      node["script"]["file"] = rhs.body.script.filename;
      break;
    }
    return node;
  }

  static bool decode(const Node& node, FrSkyScreenData& rhs)
  {
    node["type"] >> screenTypeLut >> rhs.type;
    switch(rhs.type) {
    case TELEMETRY_SCREEN_NONE:
      return true;
    case TELEMETRY_SCREEN_NUMBERS:
      node["lines"] >> rhs.body.lines;
      break;
    case TELEMETRY_SCREEN_BARS:
      node["bars"] >> rhs.body.bars;
      break;
    case TELEMETRY_SCREEN_SCRIPT:
      if (node["script"]) {
        const auto& script = node["script"];
        if (script && script.IsMap()) {
          script["file"] >> rhs.body.script.filename;
        }
      }
      break;
    }
    return true;
  }
};

Node convert<ModelData>::encode(const ModelData& rhs)
{
  Node node;
  auto board = getCurrentBoard();

  Node header;
  header["name"] = rhs.name;
  header["bitmap"] = rhs.bitmap;

  for (int i=0; i<CPN_MAX_MODULES; i++) {
    if (rhs.moduleData[i].protocol != PULSES_OFF) {
      header["modelId"][std::to_string(i)] = rhs.moduleData[i].modelId;
    }
  }
  node["header"] = header;

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
  node["beepANACenter"] = rhs.beepANACenter;
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

  for (int i = 0; i < CPN_MAX_LOGICAL_SWITCHES; i++) {
    const LogicalSwitchData& ls = rhs.logicalSw[i];
    if (!ls.isEmpty()) {
      node["logicalSw"][std::to_string(i)] = ls;
    }
  }

  for (int i = 0; i < CPN_MAX_SPECIAL_FUNCTIONS; i++) {
    const CustomFunctionData& fn = rhs.customFn[i];
    if (!fn.isEmpty()) {
      node["customFn"][std::to_string(i)] = fn;
    }
  }

  if (getCurrentFirmware()->getCapability(Heli))
    node["swashR"] = rhs.swashRingData;

  YamlThrTrace thrTrace(rhs.thrTraceSrc);
  node["thrTraceSrc"] = thrTrace.src;

  YamlSwitchWarningState switchWarningState(rhs.switchWarningStates, rhs.switchWarningEnable);
  node["switchWarningState"] = switchWarningState.src_str;

  node["thrTrimSw"] = rhs.thrTrimSwitch;
  node["potsWarnMode"] = potsWarningModeLut << rhs.potsWarningMode;

  YamlPotsWarnEnabled potsWarnEnabled(&rhs.potsWarnEnabled[CPN_MAX_POTS + CPN_MAX_SLIDERS]);
  node["potsWarnEnabled"] = potsWarnEnabled.value;

  for (int i = 0; i < CPN_MAX_POTS + CPN_MAX_SLIDERS; i++) {
    if (rhs.potsWarnPosition[i] != 0)
      node["potsWarnPosition"][std::to_string(i)]["val"] = rhs.potsWarnPosition[i];
  }

  node["displayChecklist"] = (int)rhs.displayChecklist;

  for (int i = 0; i < CPN_MAX_GVARS; i++) {
    const GVarData& gv = rhs.gvarData[i];
    if (!gv.isEmpty()) {
      node["gvars"][std::to_string(i)] = gv;
    }
  }

  // mavlink ???

  node["telemetryProtocol"] = rhs.telemetryProtocol;

  if (!IS_FAMILY_HORUS_OR_T16(board)) {
    Node screens;
    for (int i=0; i<4; i++) {
      const auto& scr = rhs.frsky.screens[i];
      if (scr.type != TELEMETRY_SCREEN_NONE) {
        screens[std::to_string(i)] = rhs.frsky.screens[i];
      }
    }
    if (screens.IsMap()) {
      node["screens"] = screens;
    }
  }

  Node vario;
  vario["source"] = YamlTelemSource{rhs.frsky.varioSource};
  vario["centerSilent"] = (int)rhs.frsky.varioCenterSilent;
  vario["centerMax"] = rhs.frsky.varioCenterMax;
  vario["centerMin"] = rhs.frsky.varioCenterMin;
  vario["min"] = rhs.frsky.varioMin;
  vario["max"] = rhs.frsky.varioMax;

  node["varioData"] = vario;
  node["rssiSource"] = YamlTelemSource(rhs.rssiSource);

  if (IS_TARANIS_X9(getCurrentBoard())) {
    node["voltsSource"] = YamlTelemSource(rhs.frsky.voltsSource);
    node["altitudeSource"] = YamlTelemSource(rhs.frsky.altitudeSource);
  }

  node["rssiAlarms"] = rhs.rssiAlarms;
  node["trainerMode"] = trainerModeLut << rhs.trainerMode;

  for (int i=0; i<CPN_MAX_MODULES; i++) {
    if (rhs.moduleData[i].protocol != PULSES_OFF) {
      node["moduleData"][std::to_string(i)] = rhs.moduleData[i];
    }
  }

  for (int i=0; i<CPN_MAX_SCRIPTS; i++) {
    if (strlen(rhs.scriptData[i].filename) > 0) {
      node["scriptData"][std::to_string(i)] = rhs.scriptData[i];
    }
  }

  for (int i=0; i<CPN_MAX_SENSORS; i++) {
    if (!rhs.sensorData[i].isEmpty()) {
      node["telemetrySensors"][std::to_string(i)] = rhs.sensorData[i];
    }
  }

  if (IS_TARANIS_X9E(board)) {
    node["toplcdTimer"] = rhs.toplcdTimer;
  }

  if (IS_FAMILY_HORUS_OR_T16(getCurrentBoard())) {
    for (int i=0; i<MAX_CUSTOM_SCREENS; i++) {
      const auto& csd = rhs.customScreens.customScreenData[i];
      if (!csd.isEmpty()) {
        node["screenData"][std::to_string(i)] = csd;
      }
    }
    node["topbarData"] = rhs.topBarData;
    node["view"] = rhs.view;
  }

  node["modelRegistrationID"] = rhs.registrationId;

  if (getCurrentFirmware()->getCapability(FunctionSwitches)) {
    node["functionSwitchConfig"] = rhs.functionSwitchConfig;
    node["functionSwitchGroup"] = rhs.functionSwitchGroup;
    node["functionSwitchStartConfig"] = rhs.functionSwitchStartConfig;
    node["functionSwitchLogicalState"] = rhs.functionSwitchLogicalState;

    for (int i = 0; i < CPN_MAX_FUNCTION_SWITCHES; i++) {
      if (strlen(rhs.functionSwitchNames[i]) > 0) {
        node["functionSwitchNames"][std::to_string(i)] =
            rhs.functionSwitchNames[i];
      }
    }
  }

  return node;
}

bool convert<ModelData>::decode(const Node& node, ModelData& rhs)
{
  if (!node.IsMap()) return false;

  if (node["header"]) {
    const auto& header = node["header"];
    if (header.IsMap()) {
      header["name"] >> rhs.name;
      header["bitmap"] >> rhs.bitmap;

      unsigned int modelIds[CPN_MAX_MODULES];
      header["modelId"] >> modelIds;
      for (int i=0; i<CPN_MAX_MODULES; i++) {
        rhs.moduleData[i].modelId = modelIds[i];
      }
    }
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
  node["beepANACenter"] >> rhs.beepANACenter;
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

  node["logicalSw"] >> rhs.logicalSw;
  node["customFn"] >> rhs.customFn;

  if (node["swashR"]) {
    const auto& swashR = node["swashR"];
    if (!swashR.IsMap()) return false;
    swashR >> rhs.swashRingData;
  }

  YamlThrTrace thrTrace;
  node["thrTraceSrc"] >> thrTrace.src;
  rhs.thrTraceSrc = thrTrace.toCpn();

  YamlSwitchWarningState switchWarningState;
  node["switchWarningState"] >> switchWarningState.src_str;
  rhs.switchWarningStates = switchWarningState.toCpn();

  node["thrTrimSw"] >> rhs.thrTrimSwitch;
  node["potsWarnMode"] >> potsWarningModeLut >> rhs.potsWarningMode;

  YamlPotsWarnEnabled potsWarnEnabled;
  node["potsWarnEnabled"] >> potsWarnEnabled.value;
  potsWarnEnabled.toCpn(&rhs.potsWarnEnabled[CPN_MAX_POTS + CPN_MAX_SLIDERS]);

  node["potsWarnPosition"] >> rhs.potsWarnPosition;

  node["displayChecklist"] >> rhs.displayChecklist;

  node["gvars"] >> rhs.gvarData;

  // mavlink ???

  node["telemetryProtocol"] >> rhs.telemetryProtocol;

  // frsky
  if (node["screens"]) {
    node["screens"] >> rhs.frsky.screens;
  }

  if (node["varioData"]) {
    const auto& vario = node["varioData"];

    YamlTelemSource varioSrc;
    vario["source"] >> varioSrc;
    rhs.frsky.varioSource = varioSrc.src;

    vario["centerSilent"] >> rhs.frsky.varioCenterSilent;
    vario["centerMax"] >> rhs.frsky.varioCenterMax;
    vario["centerMin"] >> rhs.frsky.varioCenterMin;
    vario["min"] >> rhs.frsky.varioMin;
    vario["max"] >> rhs.frsky.varioMax;
  }

  if (node["rssiSource"]) {
    YamlTelemSource rssiSource;
    node["rssiSource"] >> rssiSource;
    rhs.rssiSource = rssiSource.src;
  }

  if (node["voltsSource"]) {
    YamlTelemSource voltsSource;
    node["voltsSource"] >> voltsSource;
    rhs.frsky.voltsSource = voltsSource.src;
  }

  if (node["altitudeSource"]) {
    YamlTelemSource altitudeSource;
    node["altitudeSource"] >> altitudeSource;
    rhs.frsky.altitudeSource = altitudeSource.src;
  }

  node["rssiAlarms"] >> rhs.rssiAlarms;

  node["trainerMode"] >> trainerModeLut >> rhs.trainerMode;
  node["moduleData"] >> rhs.moduleData;
  node["scriptData"] >> rhs.scriptData;
  node["telemetrySensors"] >> rhs.sensorData;

  node["toplcdTimer"] >> rhs.toplcdTimer;

  node["screenData"] >> rhs.customScreens.customScreenData;
  node["topbarData"] >> rhs.topBarData;

  node["view"] >> rhs.view;
  node["modelRegistrationID"] >> rhs.registrationId;

  node["functionSwitchConfig"] >> rhs.functionSwitchConfig;
  node["functionSwitchGroup"] >> rhs.functionSwitchGroup;
  node["functionSwitchStartConfig"] >> rhs.functionSwitchStartConfig;
  node["functionSwitchLogicalState"] >> rhs.functionSwitchLogicalState;
  node["functionSwitchNames"] >> rhs.functionSwitchNames;

  return true;
}

}  // namespace YAML
