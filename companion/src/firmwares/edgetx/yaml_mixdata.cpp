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

static const YamlLookupTable mixMultiplexLut = {
  {  MLTPX_ADD, "ADD"  },
  {  MLTPX_MUL, "MUL"  },
  {  MLTPX_REP, "REPL"  },
};

#define GVAR_SMALL 128
#define CPN_GV1 1024

int32_t YamlReadMixWeight(const YAML::Node& node)
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
    return std::stoi(val);
  } catch(...) {
    throw YAML::TypedBadConversion<int>(node.Mark());
  }
}

std::string YamlWriteMixWeight(int32_t sval)
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

uint32_t YamlReadFlightModes(const YAML::Node& node)
{
  std::string fm_str = node.as<std::string>();
  uint32_t bits = 0;
  uint32_t mask = 1;

  for (uint32_t i = 0; i < fm_str.size(); i++) {
    if (fm_str[i] == '1') bits |= mask;
    mask <<= 1;
  }
  return bits;
}

std::string YamlWriteFlightModes(uint32_t val)
{
  std::string fm_str;
  // TODO: constant for number of flight modes
  for (int i=0; i<9; i++) {
    uint32_t bit = (val >> i) & 1;
    fm_str += bit ? "1" : "0";
  }
  return fm_str;
}

static const YamlLookupTable dummy = {};

namespace YAML
{
ENUM_CONVERTER(MltpxValue, mixMultiplexLut);

Node convert<CurveReference>::encode(const CurveReference& rhs)
{
  Node node;
  node["type"] = (int)rhs.type;
  node["value"] = YamlWriteMixWeight(rhs.value);
  return node;
}

bool convert<CurveReference>::decode(const Node& node, CurveReference& rhs)
{
  // no symbolic value defined in radio
  int type = 0;
  node["type"] >> type;
  rhs.type = (CurveReference::CurveRefType)type;
  if (node["value"]) {
    rhs.value = YamlReadMixWeight(node["value"]);
  }
  return true;
}

Node convert<MixData>::encode(const MixData& rhs)
{
  Node node;
  node["destCh"] = rhs.destCh - 1;
  node["srcRaw"] = rhs.srcRaw;
  node["weight"] = YamlWriteMixWeight(rhs.weight);
  node["swtch"] = rhs.swtch;
  node["curve"] = rhs.curve;
  node["delayUp"] = rhs.delayUp;
  node["delayDown"] = rhs.delayDown;
  node["speedUp"] = rhs.speedUp;
  node["speedDown"] = rhs.speedDown;
  node["carryTrim"] = rhs.carryTrim;
  node["mltpx"] = rhs.mltpx;
  node["mixWarn"] = rhs.mixWarn;
  node["flightModes"] = YamlWriteFlightModes(rhs.flightModes);
  node["offset"] = YamlWriteMixWeight(rhs.sOffset);
  node["name"] = rhs.name;
  return node;
}

bool convert<MixData>::decode(const Node& node, MixData& rhs)
{
  node["destCh"] >> ioffset_int((int&)rhs.destCh, -1);
  node["srcRaw"] >> rhs.srcRaw;
  if (node["weight"]) {
    rhs.weight = YamlReadMixWeight(node["weight"]);
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
    rhs.flightModes = YamlReadFlightModes(node["flightModes"]);
  }
  if (node["offset"]) {
    rhs.sOffset = YamlReadMixWeight(node["offset"]);
  }
  node["name"] >> rhs.name;
  return true;
}
}
