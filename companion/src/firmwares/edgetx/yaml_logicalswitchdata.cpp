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

#include "yaml_logicalswitchdata.h"
#include "yaml_rawswitch.h"
#include "yaml_rawsource.h"

static const YamlLookupTable funcLut = {
    {LS_FN_OFF, "FUNC_NONE"},
    {LS_FN_VEQUAL, "FUNC_VEQUAL"},
    {LS_FN_VALMOSTEQUAL, "FUNC_VALMOSTEQUAL"},
    {LS_FN_VPOS, "FUNC_VPOS"},
    {LS_FN_VNEG, "FUNC_VNEG"},
    {LS_FN_APOS, "FUNC_APOS"},
    {LS_FN_ANEG, "FUNC_ANEG"},
    {LS_FN_AND, "FUNC_AND"},
    {LS_FN_OR, "FUNC_OR"},
    {LS_FN_XOR, "FUNC_XOR"},
    {LS_FN_EDGE, "FUNC_EDGE"},
    {LS_FN_EQUAL, "FUNC_EQUAL"},
    {LS_FN_GREATER, "FUNC_GREATER"},
    {LS_FN_LESS, "FUNC_LESS"},
    {LS_FN_DPOS, "FUNC_DIFFEGREATER"},
    {LS_FN_DAPOS, "FUNC_ADIFFEGREATER"},
    {LS_FN_TIMER, "FUNC_TIMER"},
    {LS_FN_STICKY, "FUNC_STICKY"},
};

static int timerValue2lsw(uint32_t t)
{
  if (t < 20) {
    return t - 129;
  } else if (t < 600) {
    return t / 5 - 113;
  } else {
    return t / 10 - 53;
  }
}
// val = [-129,-110] => [0,19]     (step  1)
// val = [-109,6]    => [20,595]   (step  5)
// val = [7,122]     => [600,1750] (step 10)
//
static int lswTimerValue(int val)
{
  return (val < -109 ? 129+val : (val < 7 ? (113+val)*5 : (53+val)*10));
}

namespace YAML
{
Node convert<LogicalSwitchData>::encode(const LogicalSwitchData& rhs)
{
  Node node;
  node["func"] = funcLut << rhs.func;

  std::string def;

  switch (rhs.getFunctionFamily()) {
  case LS_FAMILY_VBOOL:
  case LS_FAMILY_STICKY: {
    def += YamlRawSwitchEncode(RawSwitch(rhs.val1));
    def += ",";
    def += YamlRawSwitchEncode(RawSwitch(rhs.val2));
  } break;

  case LS_FAMILY_EDGE: {
    def += YamlRawSwitchEncode(RawSwitch(rhs.val1));
    def += ",";
    def += std::to_string(lswTimerValue(rhs.val2));
    def += ",";
    if (rhs.val3 < 0) {
      def += '<';
    } else if (rhs.val3 == 0) {
      def += '-';
    } else {
      def += std::to_string(lswTimerValue(rhs.val2 + rhs.val3));
    }
  } break;

  case LS_FAMILY_VCOMP: {
    def += YamlRawSourceEncode(RawSource(rhs.val1));
    def += ",";
    def += YamlRawSourceEncode(RawSource(rhs.val2));
  } break;

  case LS_FAMILY_TIMER: {
    def += std::to_string(lswTimerValue(rhs.val1));
    def += ",";
    def += std::to_string(lswTimerValue(rhs.val2));
  } break;

  default: {
    def += YamlRawSourceEncode(RawSource(rhs.val1));
    def += ",";
    def += std::to_string(rhs.val2);
  } break;
  }

  node["def"] = def;
  node["delay"] = rhs.delay;
  node["duration"] = rhs.duration;
  node["andsw"] = YamlRawSwitchEncode(RawSwitch(rhs.andsw));
  node["lsPersist"] = (int)rhs.lsPersist;
  node["lsState"] = (int)rhs.lsState;

  return node;
}

bool convert<LogicalSwitchData>::decode(const Node& node,
                                        LogicalSwitchData& rhs)
{
  node["func"] >> funcLut >> rhs.func;

  std::string def_str;
  node["def"] >> def_str;
  std::stringstream def(def_str);

  switch (rhs.getFunctionFamily()) {
  case LS_FAMILY_VBOOL:
  case LS_FAMILY_STICKY: {
    std::string sw_str;
    getline(def, sw_str, ',');
    rhs.val1 = YamlRawSwitchDecode(sw_str).toValue();
    getline(def, sw_str);
    rhs.val2 = YamlRawSwitchDecode(sw_str).toValue();
  } break;

  case LS_FAMILY_EDGE: {
    std::string sw_str;
    getline(def, sw_str, ',');
    rhs.val1 = YamlRawSwitchDecode(sw_str).toValue();
    int v2=0, v3=0;
    def >> v2;
    rhs.val2 = timerValue2lsw(v2);
    def.ignore();
    if (def.peek() == '<') {
      rhs.val3 = -1;
    } else if (def.peek() == '-') {
      rhs.val3 = 0;
    } else {
      def >> v3;
      v3 = timerValue2lsw(v3);
      rhs.val3 = v3 - rhs.val2;
    }
  } break;

  case LS_FAMILY_VCOMP: {
    std::string src_str;
    getline(def, src_str, ',');
    if (def_str.size() >= 4 && def_str.substr(0, 4) == "lua(") {
      std::string tmp_str;
      getline(def, tmp_str, ',');
      src_str += ("," + tmp_str);
    }
    rhs.val1 = YamlRawSourceDecode(src_str).toValue();
    getline(def, src_str);
    rhs.val2 = YamlRawSourceDecode(src_str).toValue();
  } break;

  case LS_FAMILY_TIMER: {
    int v1=0, v2=0;
    def >> v1;
    rhs.val1 = timerValue2lsw(v1);
    def.ignore();
    def >> v2;
    rhs.val2 = timerValue2lsw(v2);
  } break;

  default: {
    std::string src_str;
    getline(def, src_str, ',');
    if (def_str.size() >= 4 && def_str.substr(0, 4) == "lua(") {
      std::string tmp_str;
      getline(def, tmp_str, ',');
      src_str += ("," + tmp_str);
    }
    rhs.val1 = YamlRawSourceDecode(src_str).toValue();
    def >> rhs.val2;
  } break;
  }

  node["delay"] >> rhs.delay;
  node["duration"] >> rhs.duration;

  RawSwitch andsw;
  node["andsw"] >> andsw;
  rhs.andsw = andsw.toValue();

  node["lsPersist"] >> rhs.lsPersist;
  node["lsState"] >> rhs.lsState;

  return true;
}
}  // namespace YAML
