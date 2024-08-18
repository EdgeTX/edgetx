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

#include "yaml_rawswitch.h"
#include "eeprominterface.h"
#include "boardjson.h"

//   v2.10, v2.9
static const StringTagMappingTable funcSwitchConversionTable = {
    {"SA",  "SA"},
    {"SB",  "SB"},
    {"SC",  "SC"},
    {"SD",  "SD"},
    {"SW1", "SE"},
    {"SW2", "SF"},
    {"SW3", "SG"},
    {"SW4", "SH"},
    {"SW5", "SI"},
    {"SW6", "SJ"},
};

std::string YamlRawSwitchEncode(const RawSwitch& rhs)
{
  Board::Type board = getCurrentBoard();
  Boards b = Boards(board);
  std::string sw_str;
  int32_t sval = rhs.index;
  if (rhs.index < 0) {
    sval = -sval;
    sw_str += "!";
  }

  int multiposcnt = Boards::getCapability(board, Board::MultiposPotsPositions);

  switch (rhs.type) {
  case SWITCH_TYPE_SWITCH:
    sw_str += Boards::getSwitchYamlName((sval - 1) / 3, BoardJson::YLT_REF).toStdString();
    sw_str += std::to_string((sval - 1) % 3);
    break;

  case SWITCH_TYPE_VIRTUAL:
    sw_str += "L";
    sw_str += std::to_string(sval);
    break;

  case SWITCH_TYPE_MULTIPOS_POT:
    sw_str += "6P";
    sw_str += std::to_string((sval - 1) / multiposcnt - Boards::getCapability(board, Board::Sticks));
    sw_str += std::to_string((sval - 1) % multiposcnt);
    break;

  case SWITCH_TYPE_TRIM:
    sw_str += b.getTrimSwitchTag(sval - 1);
    break;

  case SWITCH_TYPE_FLIGHT_MODE:
    sw_str += "FM";
    sw_str += std::to_string(sval - 1);
    break;

  case SWITCH_TYPE_SENSOR:
    sw_str += "T";
    sw_str += std::to_string(sval);
    break;

  default:
    sw_str += b.getRawSwitchTypeTag(rhs.type);
    break;
  }
  return sw_str;
}

RawSwitch YamlRawSwitchDecode(const std::string& sw_str)
{
  Board::Type board = getCurrentBoard();
  Boards b = Boards(board);
  RawSwitch rhs;  // constructor sets to SWITCH_TYPE_NONE
  const char* val = sw_str.data();
  size_t val_len = sw_str.size();
  std::string sw_str_tmp = sw_str;

  // yaml-cpp does not escape this
  if (val_len > 0 && val[0] == '\\') {
    val++;
    val_len--;
    sw_str_tmp = sw_str_tmp.substr(1);
  }

  bool neg = false;
  if (val_len > 0 && val[0] == '!') {
    neg = true;
    val++;
    val_len--;
    sw_str_tmp = sw_str_tmp.substr(1);
  }

  const int multiposcnt = Boards::getCapability(board, Board::MultiposPotsPositions);

  //  TODO: validate all expected numeric chars are numeric not just first

  if (val_len >= 2 && val[0] == 'L' && (val[1] >= '0' && val[1] <= '9')) {

    int sw_idx = std::stoi(sw_str_tmp.substr(1, val_len - 1));
    if (sw_idx <= CPN_MAX_LOGICAL_SWITCHES) {
      rhs = RawSwitch(SWITCH_TYPE_VIRTUAL, sw_idx);
    }

  //  format 6Piip where ii = input index - number sticks and p = pos index 0-5
  } else if (val_len > 3 && val[0] == '6' && val[1] == 'P' &&
             (val[2] >= '0' && val[2] <= '9') &&
             (val[val_len - 1] >= '0' && val[val_len - 1] < (multiposcnt + '0'))) {

    RawSwitchType mp_type = SWITCH_TYPE_MULTIPOS_POT;
    int mp_index = 0;
    int mp_input_index = 0;

    try {
      mp_input_index = std::stoi(sw_str_tmp.substr(2, val_len - 3));

      if (modelSettingsVersion < SemanticVersion(QString(CPN_ADC_REFACTOR_VERSION))) {
        if (IS_HORUS_X10(board) || IS_FAMILY_T16(board)) {
          if (mp_input_index > 2)
            mp_input_index += 2;
        }
      }

      mp_index = (mp_input_index + Boards::getCapability(board, Board::Sticks)) * multiposcnt + (val[val_len - 1] - '0') + 1;

    } catch(...) {
      mp_type = SWITCH_TYPE_NONE;
      mp_index = 0;
    }

    rhs = RawSwitch(mp_type, mp_index);

  } else if (val_len == 3 && val[0] == 'F' && val[1] == 'M' &&
             (val[2] >= '0' && val[2] <= '9')) {

    rhs = RawSwitch(SWITCH_TYPE_FLIGHT_MODE, val[2] - '0' + 1);

  } else if (val_len >= 2 && val[0] == 'T' &&
             (val[1] >= '0' && val[1] <= '9')) {

    // starts at T1
    int sensor_idx = std::stoi(sw_str_tmp.substr(1, val_len - 1));
    if ((sensor_idx > 0) && (sensor_idx <= CPN_MAX_SENSORS)) {
      rhs = RawSwitch(SWITCH_TYPE_SENSOR, sensor_idx);
    }

  } else if (sw_str_tmp.substr(0, 4) == std::string("Trim")) {

    int tsw_idx = b.getTrimSwitchIndex(sw_str_tmp.c_str());
    if (tsw_idx >= 0) {
      rhs.type = SWITCH_TYPE_TRIM;
      rhs.index = tsw_idx + 1;
    }

  } else if ((val_len >= 4 && (
              (val[0] == 'F' && val[1] == 'L') ||
              (val[0] == 'S' && val[1] == 'W')) &&
              val[2] >= '1' && val[2] <= '9' &&
              val[val_len - 1] >= '0' && val[val_len - 1] <= '2') ||
             (val_len >= 3 && val[0] == 'S' &&
              val[1] >= 'A' && val[1] <= 'Z' &&
              val[2] >= '0' && val[2] <= '2')) {

    std::string tmp = sw_str_tmp.substr(0, val_len - 1);

    if (modelSettingsVersion < SemanticVersion(QString(CPN_ADC_REFACTOR_VERSION)) && IS_JUMPER_TPRO(board))
      tmp = DataHelpers::getStringTagMappingName(funcSwitchConversionTable, tmp.c_str());

    int sw_idx = Boards::getSwitchYamlIndex(tmp.c_str(), BoardJson::YLT_REF);
    if (sw_idx >= 0) {
      rhs.type = SWITCH_TYPE_SWITCH;
      rhs.index = sw_idx * 3 + (val[val_len - 1] - '0' + 1);
    }

  } else {
    int sw_type = b.getRawSwitchTypeIndex(sw_str_tmp.c_str());
    if (sw_type >= 0) {
      rhs.type = (RawSwitchType)sw_type;
      if (rhs.type == SWITCH_TYPE_TELEMETRY || rhs.type == SWITCH_TYPE_TRAINER  || rhs.type == SWITCH_TYPE_ACT || rhs.type == SWITCH_TYPE_ONE)
        //  these types use index = 1
        rhs.index = 1;
      else
        //  types which do not use index
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
