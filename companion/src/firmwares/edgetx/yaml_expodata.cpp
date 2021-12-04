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

#include "yaml_expodata.h"
#include "yaml_mixdata.h"
#include "yaml_rawswitch.h"
#include "yaml_rawsource.h"
#include "curvereference.h"

namespace YAML
{
Node convert<ExpoData>::encode(const ExpoData& rhs)
{
  Node node;
  node["srcRaw"] = rhs.srcRaw;
  node["scale"] = rhs.scale;
  node["mode"] = rhs.mode;  // InputMode
  node["chn"] = rhs.chn;
  node["swtch"] = rhs.swtch;
  node["flightModes"] = YamlWriteFlightModes(rhs.flightModes);
  node["weight"] = YamlWriteMixWeight(rhs.weight);
  node["offset"] = rhs.offset;  // YamlWriteMixWeight
  node["curve"] = rhs.curve;
  node["carryTrim"] = rhs.carryTrim;
  node["name"] = rhs.name;
  return node;
}

bool convert<ExpoData>::decode(const Node& node, ExpoData& rhs)
{
  node["srcRaw"] >> rhs.srcRaw;
  node["scale"] >> rhs.scale;
  node["mode"] >> rhs.mode;  // InputMode
  node["chn"] >> rhs.chn;
  node["swtch"] >> rhs.swtch;
  if (node["flightModes"]) {
    rhs.flightModes = YamlReadFlightModes(node["flightModes"]);
  }
  if (node["weight"]) {
    rhs.weight = YamlReadMixWeight(node["weight"]);
  }
  if (node["offset"]) {
    rhs.offset = YamlReadMixWeight(node["offset"]);
  }
  node["curve"] >> rhs.curve;
  node["carryTrim"] >> rhs.carryTrim;
  node["name"] >> rhs.name;
  return true;
}
}  // namespace YAML
