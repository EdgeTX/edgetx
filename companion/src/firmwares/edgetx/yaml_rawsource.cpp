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

// SOURCE_TYPE_STICK
const YamlLookupTable analogSourceLut = {
  {  0, "Rud"  },
  {  1, "Ele"  },
  {  2, "Thr"  },
  {  3, "Ail"  },
  {  4, "S1"  },
  {  5, "6POS"  },
  {  6, "S2"  },
  {  7, "EXT1"  }, {  7, "S3"  }, // rename to use only S3
  {  8, "EXT2"  }, {  8, "S4"  }, // rename to use only S4
  // {  9, "EXT3"  },
  // {  10, "EXT4"  },
  {  9, "LS"  },
  {  10, "RS"  },
  {  11, "MOUSE1"  },
  {  12, "MOUSE2"  },
};

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

//ENUM_CONVERTER(RawSourceType, sourceTypeLut);

Node convert<RawSource>::encode(const RawSource& rhs)
{
  std::string src_str;
  return Node(src_str);
}

bool convert<RawSource>::decode(const Node& node, RawSource& rhs)
{
  std::string src_str = node.Scalar();
  const char* val = src_str.data();
  size_t val_len = src_str.size();

  if (val_len > 0 && val[0] == 'I') {

    rhs = RawSource(SOURCE_TYPE_VIRTUAL_INPUT, std::stoi(src_str.substr(1)));

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

      YAML::Node conv = node >> analogSourceLut;
      if (conv.IsScalar()) {
        rhs.type = SOURCE_TYPE_STICK;
        rhs.index = conv.as<int>();
      }

      conv = node >> trimSourceLut;
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
  }
  // TODO: raw analogs

  return true;
}
}  // namespace YAML
