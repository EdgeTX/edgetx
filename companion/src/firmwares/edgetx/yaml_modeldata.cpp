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
#include "version.h"

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

static const YamlLookupTable jitterFilterLut = {
  {  0, "GLOBAL"  },
  {  1, "OFF"  },
  {  2, "ON"  },
};

struct YamlTrim {
  int mode = 0;
  int ref = 0;
  int value = 0;

  YamlTrim() = default;
  YamlTrim(int mode, int ref, int value):
    mode(mode), ref(ref), value(value)
  {}

  bool isEmpty() { return !mode && !ref && !value; }
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
        return 1 + pots + sliders + src.index;
      } break;
      default:
        break;
    }
    return 0;
  }
};

struct YamlPotsWarnEnabled {
  unsigned int value;
  const int cnt = CPN_MAX_POTS + CPN_MAX_SLIDERS; //  must not exceed 16 as radio stores in uint16_t

  #define MAXPOTWARNINGS  (CPN_MAX_POTS + CPN_MAX_SLIDERS)

  #if MAXPOTWARNINGS > 16
  #error Not enough space for pot and slider warnings
  #endif

  #undef MAXPOTWARNINGS

  YamlPotsWarnEnabled() = default;

  YamlPotsWarnEnabled(const bool * potsWarnEnabled)
  {
    value = 0;

    for (int i = 0; i < cnt; i++) {
      value |= (*(potsWarnEnabled + i)) << i;
    }
  }

  void toCpn(bool * potsWarnEnabled)
  {
    memset(potsWarnEnabled, 0, sizeof(bool) * cnt);

    for (int i = 0; i < cnt; i++) {
      *(potsWarnEnabled + i) = (bool)((value >> i) & 1);
    }
  }
};

//  modeldata: uint64_t switchWarningStates
//  Yaml switchWarningState: AuBuEuFuG-IuJu
struct YamlSwitchWarningState {

  static constexpr size_t MASK_LEN = 2;
  static constexpr size_t MASK = (1 << MASK_LEN) - 1;

  std::string src_str;
  unsigned int enabled;

  YamlSwitchWarningState() = default;

  YamlSwitchWarningState(uint64_t cpn_value, unsigned int switchWarningEnable)
    : enabled(~switchWarningEnable)
  {
    uint64_t states = cpn_value;

    std::stringstream ss;
    for (int i = 0; i < Boards::getCapability(getCurrentBoard(), Board::Switches); i++) {
      //TODO: exclude 2-pos toggle from switch warnings
      if (enabled & (1 << i)) {
        std::string tag = getCurrentFirmware()->getSwitchesTag(i);
        const char *sw = tag.data();

        if (tag.size() >= 2 && sw[0] == 'S') {
          ss << sw[1];
          switch(states & MASK) {
          case 0:
            ss << 'u';
            break;
          case 1:
            ss << '-';
            break;
          case 2:
            ss << 'd';
            break;
          }
        }
      }
      states >>= MASK_LEN;
    }

    ss >> src_str;
  }

  uint64_t toCpn()
  {
    uint64_t states = 0;
    enabled = 0;

    std::stringstream ss(src_str);
    while (!ss.eof()) {
      auto c = ss.get();
      if (c < 'A' && c > 'Z') {
        ss.ignore();
        continue;
      }

      std::string sw = std::string("S") + (char)c;
      int index = getCurrentFirmware()->getSwitchesIndex(sw.c_str());
      if (index < 0) {
        ss.ignore();
        continue;
      }

      auto s = ss.get();

      int value = 0;
      switch(s) {
      case 'u':
        value = 0;
        break;
      case '-':
        value = 1;
        break;
      case 'd':
        value = 2;
        break;
      default:
        continue;
      }

      states |= ((uint64_t)value << (index * MASK_LEN));
      enabled |= (1 << index);
    }

    return states;
  }
};

template <>
void operator >> (const YAML::Node& node, FlightModeData& value)
{
  if (node && !node.IsNull()) {
    YAML::convert<FlightModeData>::decode(node, value);
  }
}

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
  node["showElapsed"] = rhs.showElapsed;
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
  node["showElapsed"] >> rhs.showElapsed;
  return true;
}

static int32_t YamlReadLimitValue(const YAML::Node& node, int32_t shift = 0)
{
  std::string val = node.as<std::string>();
  if ((val.size() >= 4)
      && (val[0] == '-')
      && (val[1] == 'G')
      && (val[2] == 'V')
      && (val[3] >= '1')
      && (val[3] <= '9')) {

      return -10000 - std::stoi(val.substr(3));
  }

  if ((val.size() >= 3)
      && (val[0] == 'G')
      && (val[1] == 'V')
      && (val[2] >= '1')
      && (val[2] <= '9')) {

    return 10000 + std::stoi(val.substr(2));
  }

  try {
    return std::stoi(val) + shift;
  } catch(...) {
    throw YAML::TypedBadConversion<int>(node.Mark());
  }
}

static std::string YamlWriteLimitValue(int32_t sval, int32_t shift = 0)
{
  if (sval < -10000) {
    int n = -sval - 10000;
    return std::string("-GV") + std::to_string(n);
  } else if (sval > 10000) {
    int n = sval - 10000;
    return std::string("GV") + std::to_string(n);
  }

  return std::to_string(sval - shift);
}

template <>
struct convert<LimitData> {
  static Node encode(const LimitData& rhs)
  {
    Node node;
    node["min"] = YamlWriteLimitValue(rhs.min, -1000);
    node["max"] = YamlWriteLimitValue(rhs.max, 1000);
    node["revert"] = (int)rhs.revert;
    node["offset"] = YamlWriteLimitValue(rhs.offset);
    node["ppmCenter"] = rhs.ppmCenter;
    node["symetrical"] = (int)rhs.symetrical;
    node["name"] = rhs.name;
    node["curve"] = rhs.curve.value;
    return node;
  }

  static bool decode(const Node& node, LimitData& rhs)
  {
    if (node["min"]) {
      rhs.min = YamlReadLimitValue(node["min"], -1000);
    }
    if (node["max"]) {
      rhs.max = YamlReadLimitValue(node["max"], 1000);
    }
    if (node["offset"]) {
      rhs.offset = YamlReadLimitValue(node["offset"]);
    }
    node["revert"] >> rhs.revert;
    node["ppmCenter"] >> rhs.ppmCenter;
    node["symetrical"] >> rhs.symetrical;
    node["name"] >> rhs.name;
    node["curve"] >> rhs.curve.value;
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
      node["mode"] = (1 << 5) - 1;
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
    if (trimMode == (1 << 5) - 1) {
      rhs.mode = -1;
    } else {
      rhs.mode = trimMode % 2;
      rhs.ref = trimMode / 2;
    }
    return true;
  }
};

Node EncodeFMData(const FlightModeData& rhs, int phaseIdx)
{
    Node node;

    size_t n_trims = Boards::getCapability(getCurrentBoard(), Board::NumTrims);
    YamlTrim yt[n_trims];

    Node trims;
    for (size_t i=0; i<n_trims; i++) {
      yt[i] = { rhs.trimMode[i], rhs.trimRef[i], rhs.trim[i] };
      if (!yt[i].isEmpty()) {
        trims[std::to_string(i)] = yt[i];
      }
    }
    if (trims && trims.IsMap()) {
      node["trim"] = trims;
    }

    if (phaseIdx > 0) {
      node["swtch"] = rhs.swtch;
    }
    node["name"] = rhs.name;
    node["fadeIn"] = rhs.fadeIn;
    node["fadeOut"] = rhs.fadeOut;

    Node gvars;
    for (size_t i=0; i<CPN_MAX_GVARS; i++) {
      if (!rhs.isGVarEmpty(phaseIdx, i)) {
        gvars[std::to_string(i)]["val"] = rhs.gvars[i];
      }
    }
    if (gvars && gvars.IsMap()) {
      node["gvars"] = gvars;
    }

    return node;
}

bool convert<FlightModeData>::decode(const Node& node,
                                     FlightModeData& rhs)
{
  YamlTrim trims[CPN_MAX_TRIMS];
  node["trim"] >> trims;
  for (size_t i = 0; i < CPN_MAX_TRIMS; i++) {
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
    node["popup"] = (int)rhs.popup;
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

template <>
struct convert<ScriptData> {
  static Node encode(const ScriptData& rhs)
  {
    Node node;
    node["file"] = rhs.filename;
    node["name"] = rhs.name;

    for (int i=0; i < 6; i++) {
      if (rhs.inputs[i] != 0)
        node["inputs"][std::to_string(i)]["u"]["value"] = (int)rhs.inputs[i];
    }

    return node;
  }

  static bool decode(const Node& node, ScriptData& rhs)
  {
    node["file"] >> rhs.filename;
    node["name"] >> rhs.name;

    for (int i=0; i < CPN_MAX_SCRIPT_INPUTS; i++) {
      if (node["inputs"][std::to_string(i)]) {
        if (node["inputs"][std::to_string(i)]["u"]["value"]) {
          node["inputs"][std::to_string(i)]["u"]["value"] >> rhs.inputs[i];
        }
      }
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
        try {
          rhs.src = std::stoi(str) + 1;
        } catch(...) {}
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
    for (int i = 0; i < getCurrentFirmware()->getCapability(TelemetryCustomScreensFieldsPerLine); i++) {
      if (rhs.source[i].isSet()) {
        sources[std::to_string(i)]["val"] = rhs.source[i];
      }
    }
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
    if (rhs.type != TELEMETRY_SCREEN_NONE) {
      Node cfg;
      switch(rhs.type) {
      case TELEMETRY_SCREEN_NUMBERS: {
        Node lines;
        for (int i = 0; i < getCurrentFirmware()->getCapability(TelemetryCustomScreensLines); i++) {
          lines[std::to_string(i)] = rhs.body.lines[i];
        }
        cfg["lines"] = lines;
        } break;
      case TELEMETRY_SCREEN_BARS: {
        Node bars;
        for (int i = 0; i < getCurrentFirmware()->getCapability(TelemetryCustomScreensBars); i++) {
          bars[std::to_string(i)] = rhs.body.bars[i];
        }
        cfg["bars"] = bars;
        } break;
      case TELEMETRY_SCREEN_SCRIPT:
        cfg["script"]["file"] = rhs.body.script.filename;
        break;
      }
      if (cfg && cfg.IsMap()) {
        node["u"] = cfg;
      }
    }
    return node;
  }

  static bool decode(const Node& node, FrSkyScreenData& rhs)
  {
    node["type"] >> screenTypeLut >> rhs.type;
    if (rhs.type == TELEMETRY_SCREEN_NONE) return true;

    if (node["u"]) {  // radio outputs generic u to indicate union field
      const auto& cfg = node["u"];
      if (cfg && cfg.IsMap()) {
        switch(rhs.type) {
        case TELEMETRY_SCREEN_NUMBERS:
          if (cfg["lines"]) {
            const auto& lines = cfg["lines"];
            if (lines && lines.IsMap()) {
              lines >> rhs.body.lines;
            }
          }
          break;
        case TELEMETRY_SCREEN_BARS:
          if (cfg["bars"]) {
            const auto& bars = cfg["bars"];
            if (bars && bars.IsMap()) {
              bars >> rhs.body.bars;
            }
          }
          break;
        case TELEMETRY_SCREEN_SCRIPT:
          if (cfg["script"]) {
            const auto& script = cfg["script"];
            if (script && script.IsMap()) {
              script["file"] >> rhs.body.script.filename;
            }
          }
          break;
        }
      }
    }
    return true;
  }
};

Node convert<ModelData>::encode(const ModelData& rhs)
{
  Node node;
  auto board = getCurrentBoard();

  node["semver"] = VERSION;

  Node header;
  header["name"] = rhs.name;
  header["bitmap"] = rhs.bitmap;

  for (int i=0; i<CPN_MAX_MODULES; i++) {
    if (rhs.moduleData[i].protocol != PULSES_OFF) {
      header["modelId"][std::to_string(i)]["val"] = rhs.moduleData[i].modelId;
    }
  }
  node["header"] = header;

  Node timers;
  for (int i=0; i<CPN_MAX_TIMERS; i++) {
    if (!rhs.timers[i].isEmpty()) {
      timers[std::to_string(i)] = rhs.timers[i];
    }
  }
  if (timers && timers.IsMap()) {
    node["timers"] = timers;
  }

  node["noGlobalFunctions"] = (int)rhs.noGlobalFunctions;
  node["thrTrim"] = (int)rhs.thrTrim;
  node["trimInc"] = rhs.trimInc;
  node["displayTrims"] = rhs.trimsDisplay;
  node["ignoreSensorIds"] = (int)rhs.frsky.ignoreSensorIds;
  node["disableThrottleWarning"] = (int)rhs.disableThrottleWarning;
  node["enableCustomThrottleWarning"] = (int)rhs.enableCustomThrottleWarning;
  node["customThrottleWarningPosition"] = (int)rhs.customThrottleWarningPosition;
  node["beepANACenter"] = rhs.beepANACenter;
  node["extendedLimits"] = (int)rhs.extendedLimits;
  node["extendedTrims"] = (int)rhs.extendedTrims;
  node["throttleReversed"] = (int)rhs.throttleReversed;

  for (int i = 0; i < CPN_MAX_FLIGHT_MODES; i++) {
    if (!rhs.flightModeData[i].isEmpty(i)) {
      node["flightModeData"][std::to_string(i)] =
          EncodeFMData(rhs.flightModeData[i], i);
    }
  }

  for (int i = 0; i < CPN_MAX_MIXERS; i++) {
    const MixData& mix = rhs.mixData[i];
    if (!mix.isEmpty()) {
      node["mixData"].push_back(Node(mix));
    }
  }

  for (int i = 0; i < CPN_MAX_CHNOUT; i++) {
    const LimitData& limit = rhs.limitData[i];
    if (!limit.isEmpty()) {
      node["limitData"][std::to_string(i)] = limit;
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
    if (rhs.inputNames[input][0]) {
      node["inputNames"][std::to_string(input)]["val"] = rhs.inputNames[input];
    }
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
  node["jitterFilter"] = jitterFilterLut << rhs.jitterFilter;

  YamlPotsWarnEnabled potsWarnEnabled(&rhs.potsWarnEnabled[0]);
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
  vario["source"] = YamlTelemSource(rhs.frsky.varioSource);
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

  for (int i=0; i<CPN_MAX_MODULES; i++) {
    if (rhs.moduleData[i].protocol != PULSES_OFF) {
      node["moduleData"][std::to_string(i)] = rhs.moduleData[i];
    }
  }

  for (int i=0; i<CPN_MAX_CHNOUT; i++) {
    if (rhs.limitData[i].failsafe != 0) {
      node["failsafeChannels"][std::to_string(i)]["val"] = rhs.limitData[i].failsafe;
    }
  }

  node["trainerData"]["mode"] = trainerModeLut << rhs.trainerMode;
  node["trainerData"]["channelsStart"] = rhs.moduleData[2].channelsStart;
  node["trainerData"]["channelsCount"] = (rhs.moduleData[2].channelsCount - 8);
  node["trainerData"]["frameLength"] = rhs.moduleData[2].ppm.frameLength;
  node["trainerData"]["delay"] = ((rhs.moduleData[2].ppm.delay - 300) / 50);
  node["trainerData"]["pulsePol"] = (int)rhs.moduleData[2].ppm.pulsePol;

  for (int i=0; i<CPN_MAX_SCRIPTS; i++) {
    if (strlen(rhs.scriptData[i].filename) > 0) {
      node["scriptsData"][std::to_string(i)] = rhs.scriptData[i];
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
    Node topbarData;
    topbarData = rhs.topBarData;
    if (topbarData && topbarData.IsMap()) {
      node["topbarData"] = topbarData;
    }
    node["view"] = rhs.view;
  }

  node["modelRegistrationID"] = rhs.registrationId;

  if (Boards::getCapability(getCurrentBoard(), Board::FunctionSwitches)) {
    node["functionSwitchConfig"] = rhs.functionSwitchConfig;
    node["functionSwitchGroup"] = rhs.functionSwitchGroup;
    node["functionSwitchStartConfig"] = rhs.functionSwitchStartConfig;
    node["functionSwitchLogicalState"] = rhs.functionSwitchLogicalState;

    for (int i = 0; i < CPN_MAX_FUNCTION_SWITCHES; i++) {
      if (strlen(rhs.functionSwitchNames[i]) > 0) {
        node["switchNames"][std::to_string(i)]["val"] = rhs.functionSwitchNames[i];
      }
    }
  }

  return node;
}

bool convert<ModelData>::decode(const Node& node, ModelData& rhs)
{
  if (!node.IsMap()) return false;

  unsigned int modelIds[CPN_MAX_MODULES];
  memset(modelIds, 0, sizeof(modelIds));

  node["semver"] >> rhs.semver;

  if (node["header"]) {
    const auto& header = node["header"];
    if (header.IsMap()) {
      header["name"] >> rhs.name;
      header["bitmap"] >> rhs.bitmap;
      header["modelId"] >> modelIds;
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
  node["ignoreSensorIds"] >> rhs.frsky.ignoreSensorIds;
  node["disableThrottleWarning"] >> rhs.disableThrottleWarning;
  node["enableCustomThrottleWarning"] >> rhs.enableCustomThrottleWarning;
  node["customThrottleWarningPosition"] >> rhs.customThrottleWarningPosition;
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
  if (node["points"])
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
  rhs.switchWarningEnable = ~switchWarningState.enabled;

  node["thrTrimSw"] >> rhs.thrTrimSwitch;
  node["potsWarnMode"] >> potsWarningModeLut >> rhs.potsWarningMode;
  node["jitterFilter"] >> jitterFilterLut >> rhs.jitterFilter;

  YamlPotsWarnEnabled potsWarnEnabled;
  node["potsWarnEnabled"] >> potsWarnEnabled.value;
  potsWarnEnabled.toCpn(&rhs.potsWarnEnabled[0]);

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

  node["moduleData"] >> rhs.moduleData;
  for (int i=0; i<CPN_MAX_MODULES; i++) {
    rhs.moduleData[i].modelId = modelIds[i];
  }

  if (node["failsafeChannels"]) {
    int failsafeChans[CPN_MAX_CHNOUT];
    memset(failsafeChans, 0, sizeof(failsafeChans));
    node["failsafeChannels"] >> failsafeChans;
    for (int i=0; i<CPN_MAX_CHNOUT; i++) {
      rhs.limitData[i].failsafe = failsafeChans[i];
    }
  }

  if (node["trainerData"]) {
    const auto& trainer = node["trainerData"];
    if (!trainer.IsMap()) return false;
    trainer["mode"] >> trainerModeLut >> rhs.trainerMode;
    trainer["channelsStart"] >> rhs.moduleData[2].channelsStart;
    trainer["channelsCount"] >> rhs.moduleData[2].channelsCount;
    rhs.moduleData[2].channelsCount += 8;
    trainer["frameLength"] >> rhs.moduleData[2].ppm.frameLength;
    trainer["delay"] >> rhs.moduleData[2].ppm.delay;
    rhs.moduleData[2].ppm.delay = 300 + 50 * rhs.moduleData[2].ppm.delay;
    trainer["pulsePol"] >> rhs.moduleData[2].ppm.pulsePol;
  }

  node["scriptsData"] >> rhs.scriptData;
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
  node["switchNames"] >> rhs.functionSwitchNames;
  return true;
}

}  // namespace YAML
