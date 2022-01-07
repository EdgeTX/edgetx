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

#include "yaml_rawsource.h"
#include "eeprominterface.h"

std::string YamlRawSourceEncode(const RawSource& rhs)
{
  std::string src_str;
  switch (rhs.type) {
    case SOURCE_TYPE_VIRTUAL_INPUT:
      src_str += "I" + std::to_string(rhs.index);
      break;
    case SOURCE_TYPE_LUA_OUTPUT:
      src_str += "lua(";
      src_str += std::to_string(rhs.index / 16);
      src_str += ",";
      src_str += std::to_string(rhs.index % 16);
      src_str += ")";
      break;
    case SOURCE_TYPE_STICK:
      src_str = getCurrentFirmware()->getAnalogInputTag(rhs.index);
      break;
    case SOURCE_TYPE_TRIM:
      src_str = getCurrentFirmware()->getTrimSourcesTag(rhs.index);
      break;
    case SOURCE_TYPE_MAX:
      src_str += "MAX";
      break;
    case SOURCE_TYPE_SWITCH:
      src_str += getCurrentFirmware()->getSwitchesTag(rhs.index);
      break;
    case SOURCE_TYPE_CUSTOM_SWITCH:
      src_str += "ls(";
      src_str += std::to_string(rhs.index);
      src_str += ")";
      break;
    case SOURCE_TYPE_CYC:
      src_str = getCurrentFirmware()->getRawSourceCyclicTag(rhs.index);
      break;
    case SOURCE_TYPE_PPM:
      src_str += "tr(";
      src_str += std::to_string(rhs.index);
      src_str += ")";
      break;
    case SOURCE_TYPE_CH:
      src_str += "ch(";
      src_str += std::to_string(rhs.index);
      src_str += ")";
      break;
    case SOURCE_TYPE_GVAR:
      src_str += "gv(";
      src_str += std::to_string(rhs.index);
      src_str += ")";
      break;
    case SOURCE_TYPE_SPECIAL:
      src_str = getCurrentFirmware()->getRawSourceSpecialTypesTag(rhs.index);
      break;
    case SOURCE_TYPE_TELEMETRY:
      src_str = "tele(";
      switch (rhs.index % 3) {
        case 0:
          break;
        case 1:
          src_str += '-';
          break;
        case 2:
          src_str += '+';
          break;
      }
      src_str += std::to_string(rhs.index / 3);
      src_str += ")";
      break;
    default:
      src_str = "NONE";
      break;
  }
  return src_str;
}

RawSource YamlRawSourceDecode(const std::string& src_str)
{
  RawSource rhs;
  const char* val = src_str.data();
  size_t val_len = src_str.size();

  if (val_len > 1 && val[0] == 'I'
      && (val[1] >= '0') && (val[1] <= '9')) {

    int idx = std::stoi(src_str.substr(1));
    if (idx < CPN_MAX_INPUTS) {
      rhs = RawSource(SOURCE_TYPE_VIRTUAL_INPUT, idx);
    }
  } else if (val_len >= 2
             && val[0] == 'S'
             && val[1] >= 'A'
             && val[1] <= 'Z') {

    int idx = getCurrentFirmware()->getSwitchesIndex(src_str.c_str());
    if (idx >= 0 && idx < CPN_MAX_SWITCHES)
      rhs = RawSource(SOURCE_TYPE_SWITCH, idx);

  } else if (val_len > 4 &&
             val[0] == 'l' &&
             val[1] == 'u' &&
             val[2] == 'a' &&
             val[3] == '(') {

    std::stringstream src(src_str.substr(4));
    int script = 0, output = 0;                   //  TODO: check rename outputs to inputs???
    src >> script;
    src.ignore();
    src >> output;
    if (script < CPN_MAX_SCRIPTS && output < CPN_MAX_SCRIPT_INPUTS)
      rhs = RawSource(SOURCE_TYPE_LUA_OUTPUT, script * 16 + output);

  } else if (val_len > 3 &&
             val[0] == 'l' &&
             val[1] == 's' &&
             val[2] == '(') {

    std::stringstream src(src_str.substr(3));
    int ls = 0;
    src >> ls;
    if (ls < CPN_MAX_LOGICAL_SWITCHES)
      rhs = RawSource(SOURCE_TYPE_CUSTOM_SWITCH, ls);

  } else if (val_len > 3 &&
             val[0] == 't' &&
             val[1] == 'r' &&
             val[2] == '(') {

    std::stringstream src(src_str.substr(3));
    int tr = 0;
    src >> tr;
    if (tr < getCurrentFirmware()->getCapability(TrainerInputs))
      rhs = RawSource(SOURCE_TYPE_PPM, tr);

  } else if (val_len > 3 &&
             val[0] == 'c' &&
             val[1] == 'h' &&
             val[2] == '(') {

    std::stringstream src(src_str.substr(3));
    int ch = 0;
    src >> ch;
    if (ch < CPN_MAX_CHNOUT)
      rhs = RawSource(SOURCE_TYPE_CH, ch);

  } else if (val_len > 3 &&
             val[0] == 'g' &&
             val[1] == 'v' &&
             val[2] == '(') {

    std::stringstream src(src_str.substr(3));
    int gv = 0;
    src >> gv;
    if (gv < CPN_MAX_GVARS)
      rhs = RawSource(SOURCE_TYPE_GVAR, gv);

  } else if (val_len > 5 &&
             val[0] == 't' &&
             val[1] == 'e' &&
             val[2] == 'l' &&
             val[3] == 'e' &&
             val[4] == '(') {

    std::stringstream src(src_str.substr(5));

    // parse sign
    uint8_t sign = 0;
    char c = src.peek();
    if (c == '-') {
      sign = 1;
      src.ignore();
    } else if (c == '+') {
      sign = 2;
      src.ignore();
    }

    int sensor = 0;
    src >> sensor;
    if (sensor < CPN_MAX_SENSORS)
      rhs = RawSource(SOURCE_TYPE_TELEMETRY, sensor * 3 + sign);

  } else {

    YAML::Node node(src_str);
    std::string ana_str;
    node >> ana_str;
    int ana_idx = getCurrentFirmware()->getAnalogInputIndex(ana_str.c_str());
    if (ana_idx >= 0) {
      rhs.type = SOURCE_TYPE_STICK;
      rhs.index = ana_idx;
    }

    int trm_idx = getCurrentFirmware()->getTrimSourcesIndex(src_str.c_str());
    if (trm_idx >= 0) {
      rhs.type = SOURCE_TYPE_TRIM;
      rhs.index = trm_idx;
    }

    int cyc_idx = getCurrentFirmware()->getRawSourceCyclicIndex(src_str.c_str());
    if (cyc_idx >= 0) {
      rhs.type = SOURCE_TYPE_CYC;
      rhs.index = cyc_idx;
    }

    int sp_idx = getCurrentFirmware()->getRawSourceSpecialTypesIndex(src_str.c_str());
    if (sp_idx >= 0) {
      rhs.type = SOURCE_TYPE_SPECIAL;
      rhs.index = sp_idx;
    }

    if (node.IsScalar() && node.as<std::string>() == "MAX") {
      rhs.type = SOURCE_TYPE_MAX;
      rhs.index = 0;
    }
  }

  return rhs;
}

namespace YAML {
Node convert<RawSource>::encode(const RawSource& rhs)
{
  return Node(YamlRawSourceEncode(rhs));
}

bool convert<RawSource>::decode(const Node& node, RawSource& rhs)
{
  std::string src_str = node.Scalar();
  rhs = YamlRawSourceDecode(src_str);
  return true;
}
}  // namespace YAML
