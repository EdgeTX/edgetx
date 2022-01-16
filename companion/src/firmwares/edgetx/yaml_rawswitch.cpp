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
#include "eeprominterface.h"

std::string YamlRawSwitchEncode(const RawSwitch& rhs)
{
  std::string sw_str;
  int32_t sval = rhs.index;
  if (rhs.index < 0) {
    sval = -sval;
    sw_str += "!";
  }

  int multiposcnt = Boards::getCapability(getCurrentBoard(), Board::MultiposPotsPositions);

  switch (rhs.type) {
  case SWITCH_TYPE_SWITCH:
    sw_str += getCurrentFirmware()->getSwitchesTag((sval - 1) / 3);
    sw_str += std::to_string((sval - 1) % 3);
    break;

  case SWITCH_TYPE_VIRTUAL:
    sw_str += "L";
    sw_str += std::to_string(sval);
    break;

  case SWITCH_TYPE_MULTIPOS_POT:
    sw_str += "6P";
    sw_str += std::to_string((sval - 1) / multiposcnt);
    sw_str += std::to_string((sval - 1) % multiposcnt);
    break;

  case SWITCH_TYPE_TRIM:
    sw_str += getCurrentFirmware()->getTrimSwitchesTag(sval - 1);
    break;

  case SWITCH_TYPE_FLIGHT_MODE:
    sw_str += "FM";
    sw_str += std::to_string(sval - 1);
    break;

  case SWITCH_TYPE_SENSOR:
    sw_str += "T";
    sw_str += std::to_string(sval + 1);
    break;

  case SWITCH_TYPE_TIMER_MODE:
    //  TODO: check as it appears to be depreciated as not in RawSwitch data model and not in radio yaml export
    break;

  default:
    sw_str += getCurrentFirmware()->getRawSwitchTypesTag(rhs.type);
    break;
  }
  return sw_str;
}

RawSwitch YamlRawSwitchDecode(const std::string& sw_str)
{
  RawSwitch rhs;  // constructor sets to SWITCH_TYPE_NONE
  const char* val = sw_str.data();
  size_t val_len = sw_str.size();
  std::string sw_str_tmp = sw_str;

  // yaml-cpp does not escape this
  if (val_len > 0 && val[0] == '\\') {
    val++; val_len--;
    sw_str_tmp = sw_str_tmp.substr(1);
  }

  bool neg = false;
  if (val_len > 0 && val[0] == '!') {
    neg = true;
    val++;
    val_len--;
    sw_str_tmp = sw_str_tmp.substr(1);
  }

  int multiposcnt = Boards::getCapability(getCurrentBoard(), Board::MultiposPotsPositions);

  //  TODO: validate all expected numeric chars are numeric not just first

  if (val_len >= 2 && val[0] == 'L' && (val[1] >= '0' && val[1] <= '9')) {

    int sw_idx = std::stoi(sw_str_tmp.substr(1, val_len - 1));
    if (sw_idx <= CPN_MAX_LOGICAL_SWITCHES) {
      rhs = RawSwitch(SWITCH_TYPE_VIRTUAL, sw_idx);
    }

  } else if (val_len > 3 && val[0] == '6' && val[1] == 'P' &&
             (val[2] >= '0' && val[2] <= '9') &&
             (val[3] >= '0' && val[3] < (multiposcnt + '0'))) {

    rhs = RawSwitch(SWITCH_TYPE_MULTIPOS_POT,
                    (val[2] - '0') * multiposcnt + (val[3] - '0') + 1);

  } else if (val_len == 3 && val[0] == 'F' && val[1] == 'M' &&
             (val[2] >= '0' && val[2] <= '9')) {

    rhs = RawSwitch(SWITCH_TYPE_FLIGHT_MODE, val[2] - '0' + 1);

  } else if (val_len >= 2 && val[0] == 'T' &&
             (val[1] >= '0' && val[1] <= '9')) {

    // starts at T1
    int sensor_idx = std::stoi(sw_str_tmp.substr(1, val_len - 1));
    if ((sensor_idx > 0) && (sensor_idx <= CPN_MAX_SENSORS)) {
      rhs = RawSwitch(SWITCH_TYPE_SENSOR, sensor_idx - 1);
    }

  } else if (sw_str_tmp.substr(0, 4) == std::string("Trim")) {

    int tsw_idx = getCurrentFirmware()->getTrimSwitchesIndex(sw_str_tmp.c_str());
    if (tsw_idx >= 0) {
      rhs.type = SWITCH_TYPE_TRIM;
      rhs.index = tsw_idx + 1;
    }

  } else if (val_len >= 3 && val[0] == 'S' &&
             (val[1] >= 'A' && val[1] <= 'Z') &&
             (val[2] >= '0' && val[2] <= '2')) {

    int sw_idx = getCurrentFirmware()->getSwitchesIndex(sw_str_tmp.substr(0, 2).c_str());
    if (sw_idx >= 0) {
      rhs.type = SWITCH_TYPE_SWITCH;
      rhs.index = sw_idx * 3 + (val[2] - '0' + 1);
    }

  //  TODO: SWITCH_TYPE_TIMER_MODE
  //        check as it appears to be depreciated as not in RawSwitch data model and not in radio yaml export

  } else {
    //  types which do not use index
    int sw_type = getCurrentFirmware()->getRawSwitchTypesIndex(sw_str_tmp.c_str());
    if (sw_type >= 0) {
      rhs.type = (RawSwitchType)sw_type;
      rhs.index = 0;
    }
  }

  if (neg) {
    rhs.index = -rhs.index;
  }
  return rhs;
}

namespace YAML {

Node convert<RawSwitch>::encode(const RawSwitch& rhs)
{
  return Node(YamlRawSwitchEncode(rhs));
}

bool convert<RawSwitch>::decode(const Node& node, RawSwitch& rhs)
{
  rhs = YamlRawSwitchDecode(node.Scalar());
  return true;
}
}  // namespace YAML
