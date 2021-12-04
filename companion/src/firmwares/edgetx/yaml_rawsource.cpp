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

// SOURCE_TYPE_TRIM
const YamlLookupTable trimSourceLut = {
  {  0, "TrimRud"  },
  {  1, "TrimEle"  },
  {  2, "TrimThr"  },
  {  3, "TrimAil"  },
  {  4, "TrimT5"  },
  {  5, "TrimT6"  },
};

// SOURCE_TYPE_CYC
const YamlLookupTable cycSourceLut = {
  {  0, "CYC1"  },
  {  1, "CYC2"  },
  {  2, "CYC3"  },
};

// SOURCE_TYPE_MAX
  // {  MIXSRC_MAX, "MAX"  },

const YamlLookupTable specialSourceLut = {
  {  0, "TX_VOLTAGE" },
  {  1, "TX_TIME"  },
  {  2, "ONE"  },
  {  3, "TIMER1"  },
  {  4, "TIMER2"  },
  {  5, "TIMER3"  },
  // TODO: {  ???, "TX_GPS" },
};

namespace YAML {
Node convert<RawSource>::encode(const RawSource& rhs)
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
      src_str = LookupValue(trimSourceLut, rhs.index);
      break;
    case SOURCE_TYPE_MAX:
      src_str += "MAX";
      break;
    case SOURCE_TYPE_SWITCH:
      src_str += "S";
      src_str += (rhs.index + 'A');
      break;
    case SOURCE_TYPE_CUSTOM_SWITCH:
      src_str += "ls(";
      src_str += std::to_string(rhs.index);
      src_str += ")";
      break;
    case SOURCE_TYPE_CYC:
      src_str = LookupValue(cycSourceLut, rhs.index);
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
      src_str = LookupValue(specialSourceLut, rhs.index);
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
      break;
    default:
      src_str = "NONE";
      break;
  }

  return Node(src_str);
}

bool convert<RawSource>::decode(const Node& node, RawSource& rhs)
{
  std::string src_str = node.Scalar();
  const char* val = src_str.data();
  size_t val_len = src_str.size();

  if (val_len > 0 && val[0] == 'I') {

    rhs = RawSource(SOURCE_TYPE_VIRTUAL_INPUT, std::stoi(src_str.substr(1)));

  } else if (val_len >= 2
             && val[0] == 'S'
             && val[1] >= 'A'
             && val[1] <= 'Z') {

    rhs = RawSource(SOURCE_TYPE_SWITCH, val[1] - 'A');
      
  } else if (val_len > 4 &&
             val[0] == 'l' &&
             val[1] == 'u' &&
             val[2] == 'a' &&
             val[3] == '(') {

    src_str = src_str.substr(4);

    size_t next_pos = 0;
    int script = std::stoi(src_str, &next_pos);

    if (next_pos >= src_str.size()) return false;
    src_str = src_str.substr(next_pos + 1);

    int output = std::stoi(src_str);
    rhs = RawSource(SOURCE_TYPE_LUA_OUTPUT, script * 16 + output);

  } else if (val_len > 3 &&
             val[0] == 'l' &&
             val[1] == 's' &&
             val[2] == '(') {

    src_str = src_str.substr(3);
    int ls = std::stoi(src_str);
    rhs = RawSource(SOURCE_TYPE_CUSTOM_SWITCH, ls);

  } else if (val_len > 3 &&
             val[0] == 't' &&
             val[1] == 'r' &&
             val[2] == '(') {

    src_str = src_str.substr(3);
    int tr = std::stoi(src_str);
    rhs = RawSource(SOURCE_TYPE_PPM, tr);
    
  } else if (val_len > 3 &&
             val[0] == 'c' &&
             val[1] == 'h' &&
             val[2] == '(') {

    src_str = src_str.substr(3);
    int ch = std::stoi(src_str);
    rhs = RawSource(SOURCE_TYPE_CH, ch);
    
  } else if (val_len > 3 &&
             val[0] == 'g' &&
             val[1] == 'v' &&
             val[2] == '(') {

    src_str = src_str.substr(3);
    int gv = std::stoi(src_str);
    rhs = RawSource(SOURCE_TYPE_GVAR, gv);

  } else if (val_len > 5 &&
             val[0] == 't' &&
             val[1] == 'e' &&
             val[2] == 'l' &&
             val[3] == 'e' &&
             val[4] == '(') {

    src_str = src_str.substr(5);

    // parse sign
    uint8_t sign = 0;
    if (src_str[0] == '-') {
      sign = 1;
      src_str = src_str.substr(1);
    } else if (src_str[0] == '+') {
      sign = 2;
      src_str = src_str.substr(1);
    }

    int sensor = std::stoi(src_str);
    rhs = RawSource(SOURCE_TYPE_TELEMETRY, sensor * 3 + sign);

  } else {

    std::string ana_str;
    node >> ana_str;
    int ana_idx = getCurrentFirmware()->getAnalogInputIndex(ana_str.c_str());
    if (ana_idx >= 0) {
      rhs.type = SOURCE_TYPE_STICK;
      rhs.index = ana_idx;
    }

    YAML::Node conv = node >> trimSourceLut;
    if (conv.IsScalar()) {
      rhs.type = SOURCE_TYPE_TRIM;
      rhs.index = conv.as<int>();
    }

    conv = node >> cycSourceLut;
    if (conv.IsScalar()) {
      rhs.type = SOURCE_TYPE_CYC;
      rhs.index = conv.as<int>();
    }

    conv = node >> specialSourceLut;
    if (conv.IsScalar()) {
      rhs.type = SOURCE_TYPE_SPECIAL;
      rhs.index = conv.as<int>();
    }

    if (node.IsScalar() && node.as<std::string>() == "MAX") {
      rhs.type = SOURCE_TYPE_MAX;
      rhs.index = 0;
    }
  }
  // TODO: raw analogs

  return true;
}
}  // namespace YAML
