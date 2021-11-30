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

#include "yaml_rawswitch.h"

#define XPOTS_MULTIPOS_COUNT 6

const YamlLookupTable trimSwitchLut = {
  {  0, "TrimRudLeft"  },
  {  1, "TrimRudRight"  },
  {  2, "TrimEleDown"  },
  {  3, "TrimEleUp"  },
  {  4, "TrimThrDown"  },
  {  5, "TrimThrUp"  },
  {  6, "TrimAilLeft"  },
  {  7, "TrimAilRight"  },
  {  8, "TrimT5Down"  },
  {  9, "TrimT5Up"  },
  {  10, "TrimT6Down"  },
  {  11, "TrimT6Up"  },
};

const YamlLookupTable switchTypeLut = {
  {  SWITCH_TYPE_NONE, "NONE" },
  {  SWITCH_TYPE_ON, "ON"  },
  {  SWITCH_TYPE_ONE, "ONE"  },
  {  SWITCH_TYPE_TELEMETRY, "TELEMETRY_STREAMING"  },
  {  SWITCH_TYPE_ACT, "RADIO_ACTIVITY"  },
  {  SWITCH_TYPE_OFF, "OFF"  },
};

namespace YAML {

ENUM_CONVERTER(RawSwitchType, switchTypeLut);

bool convert<RawSwitch>::decode(const Node& node, RawSwitch& rhs)
{
  const std::string& sw_str = node.Scalar();
  const char* val = sw_str.data();
  size_t val_len = sw_str.size();

  bool neg = false;
  if (val_len > 0 && val[0] == '!') {
    neg = true;
    val++;
    val_len--;
  }

  if (val_len >= 2 && val[0] == 'L' && (val[1] >= '0' && val[1] <= '9')) {
    rhs = RawSwitch(SWITCH_TYPE_VIRTUAL,
                    std::stoi(sw_str.substr(1, val_len - 1)) - 1);
  } else if (val_len > 3 && val[0] == '6' && val[1] == 'P' &&
             (val[2] >= '0' && val[2] <= '9') &&
             (val[3] >= '0' && val[3] < (XPOTS_MULTIPOS_COUNT + '0'))) {
    rhs = RawSwitch(SWITCH_TYPE_MULTIPOS_POT,
                    (val[2] - '0') * XPOTS_MULTIPOS_COUNT + (val[3] - '0'));
  } else if (val_len == 3 && val[0] == 'F' && val[1] == 'M' &&
             (val[2] >= '0' && val[2] <= '9')) {
    rhs = RawSwitch(SWITCH_TYPE_FLIGHT_MODE, val[2] - '0');
  } else if (val_len >= 2 && val[0] == 'T' &&
             (val[1] >= '0' && val[1] <= '9')) {
    rhs = RawSwitch(SWITCH_TYPE_SENSOR,
                    std::stoi(sw_str.substr(1, val_len - 1)) - 1);
  } else if (sw_str.substr(0, 3) == std::string("Trim")) {  // "Trim"

    rhs.type = SWITCH_TYPE_TRIM;
    node >> trimSwitchLut >> rhs.index;
  } else if (val_len >= 3 && val[0] == 'S' &&
             (val[1] >= 'A' && val[1] <= 'Z') &&
             (val[2] >= '0' && val[2] <= '2')) {
    rhs = RawSwitch(SWITCH_TYPE_SWITCH, (val[1] - 'A') * 3 + val[2] - '0');
  } else {
    // TODO: check if we have a match?
    //       or does RawSwitch default to NONE anyway?
    //
    node >> switchTypeLut >> rhs.type;
    rhs.index = 0;
  }

  if (neg) {
    rhs.index = -rhs.index;
  }

  return true;
}
}  // namespace YAML
