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

#include "yaml_mixdata.h"
#include "yaml_rawswitch.h"
#include "yaml_rawsource.h"
#include "curvereference.h"

// static const YamlLookupTable timerModeLut = {
//     {TimerData::TIMERMODE_OFF, "OFF"},
//     {TimerData::TIMERMODE_ON, "ON"},
//     {TimerData::TIMERMODE_START, "START"},
//     {TimerData::TIMERMODE_THR, "THR"},
//     {TimerData::TIMERMODE_THR_REL, "THR_REL"},
//     {TimerData::TIMERMODE_THR_START, "THR_START"},
// };

// static const YamlLookupTable trainerModeLut = {
//   {  TRAINER_MODE_OFF, "OFF"  },
//   {  TRAINER_MODE_MASTER_JACK, "MASTER_TRAINER_JACK"  },
//   {  TRAINER_MODE_SLAVE_JACK, "SLAVE"  },
//   {  TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE, "MASTER_SBUS_EXT"  },
//   {  TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE, "MASTER_CPPM_EXT"  },
//   {  TRAINER_MODE_MASTER_BATTERY_COMPARTMENT, "MASTER_BATT_COMP"  },
//   {  TRAINER_MODE_MASTER_BLUETOOTH, "MASTER_BT"  },
//   {  TRAINER_MODE_SLAVE_BLUETOOTH, "SLAVE_BT"  },
//   {  TRAINER_MODE_MULTI, "MASTER_MULTI"  },
// };

static const YamlLookupTable mixMultiplexLut = {
  {  MLTPX_ADD, "ADD"  },
  {  MLTPX_MUL, "MUL"  },
  {  MLTPX_REP, "REPL"  },
};

#define GVAR_SMALL 128
#define CPN_GV1 1024

static int32_t readMixWeight(const std::string& val)
{
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

  return std::stoi(val);
}

static std::string writeMixWeight(int32_t sval)
{
  if (sval < -10000) {
    int n = -sval - 10000;
    return std::string("-GV") + std::to_string(n);
  } else if (sval > 10000) {
    int n = sval - 10000;
    return std::string("GV") + std::to_string(n);
  }

  return std::to_string(sval);
}

static const YamlLookupTable dummy = {};

namespace YAML
{
ENUM_CONVERTER(MltpxValue, mixMultiplexLut);

Node convert<CurveReference>::encode(const CurveReference& rhs)
{
  Node node;
  node["type"] = (int)rhs.type;
  node["value"] = rhs.value;
  return node;
}

bool convert<CurveReference>::decode(const Node& node, CurveReference& rhs)
{
  // no symbolic value defined in radio
  int type = 0;
  node["type"] >> type;
  rhs.type = (CurveReference::CurveRefType)type;

  node["value"] >> rhs.value;
  return true;
}

Node convert<MixData>::encode(const MixData& rhs)
{
  Node node;
  node["destCh"] = rhs.destCh - 1;
  node["srcRaw"] = rhs.srcRaw;
  node["weight"] = writeMixWeight(rhs.weight);
  node["swtch"] = rhs.swtch;
  node["curve"] = rhs.curve;
  node["delayUp"] = rhs.delayUp;
  node["delayDown"] = rhs.delayDown;
  node["speedUp"] = rhs.speedUp;
  node["speedDown"] = rhs.speedDown;
  node["carryTrim"] = rhs.carryTrim;
  node["mltpx"] = rhs.mltpx;
  node["mixWarn"] = rhs.mixWarn;

  // TODO: constant for number of flight modes
  std::string fm_str;
  for (int i=0; i<9; i++) {
    uint32_t bit = (rhs.flightModes >> i) & 1;
    fm_str += bit ? "1" : "0";
  }
  node["flightModes"] = fm_str;

  node["offset"] = rhs.sOffset; // TODO: use writeMixWeight()
  node["name"] = rhs.name;
  return node;
}

bool convert<MixData>::decode(const Node& node, MixData& rhs)
{
  node["destCh"] >> ioffset_int((int&)rhs.destCh, -1);
  node["srcRaw"] >> rhs.srcRaw;

  std::string weight_str;
  node["weight"] >> weight_str;
  if (!weight_str.empty()) {
    rhs.weight = readMixWeight(weight_str);
  }
  node["swtch"] >> rhs.swtch;
  node["curve"] >> rhs.curve;
  node["delayUp"] >> rhs.delayUp;
  node["delayDown"] >> rhs.delayDown;
  node["speedUp"] >> rhs.speedUp;
  node["speedDown"] >> rhs.speedDown;
  node["carryTrim"] >> rhs.carryTrim;
  node["mltpx"] >> rhs.mltpx;
  node["mixWarn"] >> rhs.mixWarn;

  if (node["flightModes"]) {
      std::string fm_str;
      node["flightModes"] >> fm_str;

      uint32_t bits = 0;
      uint32_t mask = 1;

      for (uint32_t i = 0; i < fm_str.size(); i++) {
          if (fm_str[i] == '1') bits |= mask;
          mask <<= 1;
      }

      rhs.flightModes = bits;
  }

  node["offset"] >> rhs.sOffset; // TODO: use readMixWeight()
  node["name"] >> rhs.name;
  return true;
}
}
