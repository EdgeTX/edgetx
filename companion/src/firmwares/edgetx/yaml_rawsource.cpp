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

#include "yaml_rawsource.h"
#include "eeprominterface.h"
#include "boardjson.h"

static const YamlLookupTable spacemouseLut = {
  {  0, "INVALID"       },
  {  1, "SPACEMOUSE_A"  },
  {  2, "SPACEMOUSE_B"  },
  {  3, "SPACEMOUSE_C"  },
  {  4, "SPACEMOUSE_D"  },
  {  5, "SPACEMOUSE_E"  },
  {  6, "SPACEMOUSE_F"  },
};

std::string YamlRawSourceEncode(const RawSource& rhs)
{
  Board::Type board = getCurrentBoard();
  Boards b = Boards(board);
  std::string src_str;
  div_t qr;
  char c = 'A';

  int32_t sval = rhs.index;

  if (rhs.index < 0) {
    sval = -sval;
    src_str += "!";
  }

  switch (rhs.type) {
    case SOURCE_TYPE_VIRTUAL_INPUT:
      src_str += "I" + std::to_string(sval - 1);
      break;
    case SOURCE_TYPE_FUNCTIONSWITCH_GROUP:
      src_str += "GR" + std::to_string(sval);
      break;
    case SOURCE_TYPE_LUA_OUTPUT:
      qr = div(sval - 1, 16);
      src_str += "lua(";
      src_str += std::to_string(qr.quot);
      src_str += ",";
      src_str += std::to_string(qr.rem);
      src_str += ")";
      break;
    case SOURCE_TYPE_INPUT:
      src_str += Boards::getInputYamlName(sval - 1, BoardJson::YLT_REF).toStdString();
      break;
    case SOURCE_TYPE_TRIM:
      src_str += Boards::getTrimYamlName(sval - 1, BoardJson::YLT_REF).toStdString();
      break;
    case SOURCE_TYPE_MIN:
      src_str += "MIN";
      break;
    case SOURCE_TYPE_MAX:
      src_str += "MAX";
      break;
    case SOURCE_TYPE_SWITCH:
      src_str += Boards::getSwitchYamlName(sval - 1, BoardJson::YLT_REF).toStdString();
      break;
    case SOURCE_TYPE_CUSTOM_SWITCH:
      src_str += "ls(";
      src_str += std::to_string(sval);
      src_str += ")";
      break;
    case SOURCE_TYPE_CYC:
      src_str += "CYC" + std::to_string(sval);
      break;
    case SOURCE_TYPE_PPM:
      src_str += "tr(";
      src_str += std::to_string(sval - 1);
      src_str += ")";
      break;
    case SOURCE_TYPE_CH:
      src_str += "ch(";
      src_str += std::to_string(sval - 1);
      src_str += ")";
      break;
    case SOURCE_TYPE_GVAR:
      src_str += "gv(";
      src_str += std::to_string(sval - 1);
      src_str += ")";
      break;
    case SOURCE_TYPE_SPECIAL:
      src_str += b.getRawSourceSpecialTypeTag(sval);
      break;
    case SOURCE_TYPE_TIMER:
      src_str += "Tmr" + std::to_string(sval);
      break;
    case SOURCE_TYPE_TELEMETRY:
      qr = div(sval - 1, 3);
      src_str += "tele(";
      switch (qr.rem) {
        case 0:
          break;
        case 1:
          src_str += '-';
          break;
        case 2:
          src_str += '+';
          break;
      }
      src_str += std::to_string(qr.quot);
      src_str += ")";
      break;
    case SOURCE_TYPE_SPACEMOUSE:
      src_str += "SPACEMOUSE_";
      c += sval - 1;
      src_str += c;
      break;
    default:
      src_str = "NONE";
      break;
  }
  return src_str;
}

RawSource YamlRawSourceDecode(const std::string& src_str)
{
  Board::Type board = getCurrentBoard();
  Boards b = Boards(board);
  RawSource rhs;
  const char* val = src_str.data();
  size_t val_len = src_str.size();
  std::string src_str_tmp = src_str;
  bool neg = false;

  if (val_len > 0 && val[0] == '!') {
    neg = true;
    val++;
    val_len--;
    src_str_tmp = src_str_tmp.substr(1);
  }

  if (val_len > 1 && val[0] == 'I'
      && (val[1] >= '0') && (val[1] <= '9')) {

    int idx = std::stoi(src_str_tmp.substr(1));
    if (idx < CPN_MAX_INPUTS) {
      rhs = RawSource(SOURCE_TYPE_VIRTUAL_INPUT, idx + 1);
    }

  } else if ((val_len == 2 &&
              val[0] == 'S' &&
              val[1] >= 'A' && val[1] <= 'S') ||  // ST used for surface steering
             (val_len == 3 && (
              (val[0] == 'F' && val[1] == 'L') ||
              (val[0] == 'S' && val[1] == 'W')) &&
              val[2] >= '1' && val[2] <= '9')) {

    int idx = Boards::getSwitchYamlIndex(src_str_tmp.c_str(), BoardJson::YLT_REF);
    if (idx >= 0) {
      rhs = RawSource(SOURCE_TYPE_SWITCH, idx + 1);
    }
  } else if ((val_len == 3 &&
              val[0] == 'G' &&
              val[1] >= 'R' &&
              val[2] >= '1' && val[2] <= '3')) {

    int idx = std::stoi(src_str_tmp.substr(2));
    if (idx >= 0) {
      rhs = RawSource(SOURCE_TYPE_FUNCTIONSWITCH_GROUP, idx);
    }
  } else if (val_len > 4 &&
             val[0] == 'l' &&
             val[1] == 'u' &&
             val[2] == 'a' &&
             val[3] == '(') {

    std::stringstream src(src_str_tmp.substr(4));
    int script = 0, output = 0;                   //  TODO: check rename outputs to inputs???
    src >> script;
    src.ignore();
    src >> output;
    if (script < CPN_MAX_SCRIPTS && output < CPN_MAX_SCRIPT_INPUTS)
      rhs = RawSource(SOURCE_TYPE_LUA_OUTPUT, script * 16 + output + 1);

  } else if (val_len > 3 &&
             val[0] == 'l' &&
             val[1] == 's' &&
             val[2] == '(') {

    std::stringstream src(src_str_tmp.substr(3));
    int ls = 0;
    src >> ls;
    if (ls > 0 && ls <= CPN_MAX_LOGICAL_SWITCHES)
      rhs = RawSource(SOURCE_TYPE_CUSTOM_SWITCH, ls);

    // appears depreciated - maybe early support for T20 as SWn the current std and no match in encode
  } else if (val_len > 3 &&
             val[0] == 'f' &&
             val[1] == 's' &&
             val[2] == '(') {

    std::stringstream src(src_str_tmp.substr(3));
    int fs = 0;
    src >> fs;
    if (fs > 0) {
      int fsidx = Boards::getSwitchYamlIndex(QString("SW%1").arg(fs), BoardJson::YLT_REF);
      if (fsidx >= 0)
        rhs = RawSource(SOURCE_TYPE_SWITCH, fsidx + 1);
    }

  } else if (val_len > 3 &&
             val[0] == 't' &&
             val[1] == 'r' &&
             val[2] == '(') {

    std::stringstream src(src_str_tmp.substr(3));
    int tr = 0;
    src >> tr;
    if (tr < getCurrentFirmware()->getCapability(TrainerInputs))
      rhs = RawSource(SOURCE_TYPE_PPM, tr + 1);

  } else if (val_len > 3 &&
             val[0] == 'c' &&
             val[1] == 'h' &&
             val[2] == '(') {

    std::stringstream src(src_str_tmp.substr(3));
    int ch = 0;
    src >> ch;
    if (ch < CPN_MAX_CHNOUT)
      rhs = RawSource(SOURCE_TYPE_CH, ch + 1);

  } else if (val_len > 3 &&
             val[0] == 'g' &&
             val[1] == 'v' &&
             val[2] == '(') {

    std::stringstream src(src_str_tmp.substr(3));
    int gv = 0;
    src >> gv;
    if (gv < CPN_MAX_GVARS)
      rhs = RawSource(SOURCE_TYPE_GVAR, gv + 1);

  } else if (val_len > 5 &&
             val[0] == 't' &&
             val[1] == 'e' &&
             val[2] == 'l' &&
             val[3] == 'e' &&
             val[4] == '(') {

    std::stringstream src(src_str_tmp.substr(5));

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
      rhs = RawSource(SOURCE_TYPE_TELEMETRY, sensor * 3 + sign + 1);

  } else if (val_len > 3 &&
             val[0] == 'C' &&
             val[1] == 'Y' &&
             val[2] == 'C' &&
             val[3] >= '1' && val[3] <= '3') {

    std::stringstream src(src_str_tmp.substr(3));
    int cyc = 0;
    src >> cyc;
    if (cyc <= CPN_MAX_CYC)
      rhs = RawSource(SOURCE_TYPE_CYC, cyc);

  } else {

    YAML::Node node(src_str_tmp);

    if (node.IsScalar() && node.as<std::string>() == "MAX") {
      rhs.type = SOURCE_TYPE_MAX;
      rhs.index = 1;
    }

    if (node.IsScalar() && node.as<std::string>() == "MIN") {
      rhs.type = SOURCE_TYPE_MIN;
      rhs.index = 1;
    }

    std::string ana_str;
    node >> ana_str;

    if (modelSettingsVersion < SemanticVersion(QString(CPN_ADC_REFACTOR_VERSION))) {
      ana_str = Boards::getLegacyAnalogMappedInputTag(ana_str.c_str());
      int idx = Boards::getInputIndex(ana_str.c_str(), Board::LVT_TAG);
      if (idx >= 0 && Boards::isInputStick(idx))
        ana_str = Boards::getInputName(idx).toStdString();
    }

    int ana_idx = Boards::getInputYamlIndex(ana_str.c_str(), BoardJson::YLT_REF);
    if (ana_idx >= 0) {
      rhs.type = SOURCE_TYPE_INPUT;
      rhs.index = ana_idx + 1;
    }

    std::string trim_str;
    node >> trim_str;

    if (modelSettingsVersion < SemanticVersion(QString(CPN_ADC_REFACTOR_VERSION))) {
      int idx = b.getLegacyTrimSourceIndex(src_str_tmp.c_str());
      if (idx >= 0)
        trim_str = Boards::getTrimYamlName(idx, BoardJson::YLT_REF).toStdString();
    }

    int trm_idx = Boards::getTrimYamlIndex(trim_str.c_str(), BoardJson::YLT_REF);
    if (trm_idx >= 0) {
      rhs.type = SOURCE_TYPE_TRIM;
      rhs.index = trm_idx + 1;
    }

    std::string timer_str;
    node >> timer_str;

    if (modelSettingsVersion < SemanticVersion(QString(CPN_ADC_REFACTOR_VERSION))) {
      if (timer_str.size() == 6 && timer_str.substr(0, 5) == "TIMER") {
        timer_str = "Tmr" + timer_str.substr(5, 1);
      }
    }

    if (timer_str.substr(0, 3) == "Tmr" && timer_str.substr(3, 1) >= "1" && timer_str.substr(3, 1) <= "9") {
      if (std::stoi(timer_str.substr(3, 1)) <= CPN_MAX_TIMERS) {
        rhs.type = SOURCE_TYPE_TIMER;
        rhs.index = std::stoi(timer_str.substr(3, 1));
      }
    }

    std::string special_str;
    node >> special_str;

    int sp_idx = b.getRawSourceSpecialTypeIndex(special_str.c_str());
    if (sp_idx > 0) {
      rhs.type = SOURCE_TYPE_SPECIAL;
      rhs.index = sp_idx;
    }

    if (node.IsScalar() && node.as<std::string>().size() == 12 && node.as<std::string>().substr(0, 11) == "SPACEMOUSE_") {
      int sm_idx = 0;
      node >> spacemouseLut >> sm_idx;
      if (sm_idx >= 0) {
        rhs.type = SOURCE_TYPE_SPACEMOUSE;
        rhs.index = sm_idx + 1;
      }
    }
  }

  if (neg)
    rhs.index = -rhs.index;

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
