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

#include "boardjson.h"
#include "datahelpers.h"
#include "constants.h"

static const StringTagMappingTable boardTypeJsonFileLookupTable = {
    {std::to_string(Board::BOARD_UNKNOWN),              ""},       // this must be the first entry
    {std::to_string(Board::BOARD_TARANIS_X7),           "x7"},
    {std::to_string(Board::BOARD_TARANIS_X7_ACCESS),    "x7access"},
    {std::to_string(Board::BOARD_TARANIS_X9D),          "x9d"},
    {std::to_string(Board::BOARD_TARANIS_X9DP),         "x9d+"},
    {std::to_string(Board::BOARD_TARANIS_X9DP_2019),    "x9d+2019"},
    {std::to_string(Board::BOARD_TARANIS_X9E),          "x9e"},
    {std::to_string(Board::BOARD_TARANIS_X9LITE),       "x9lite"},
    {std::to_string(Board::BOARD_TARANIS_X9LITES),      "x9lites"},
    {std::to_string(Board::BOARD_X10),                  "x10"},
    {std::to_string(Board::BOARD_X10_EXPRESS),          "x10express"},
    {std::to_string(Board::BOARD_HORUS_X12S),           "x12s"},
    {std::to_string(Board::BOARD_TARANIS_XLITE),        "xlite"},
    {std::to_string(Board::BOARD_TARANIS_XLITES),       "xlites"},
    {std::to_string(Board::BOARD_JUMPER_T12),           "t12"},
    {std::to_string(Board::BOARD_JUMPER_T16),           "t16"},
    {std::to_string(Board::BOARD_JUMPER_T18),           "t18"},
    {std::to_string(Board::BOARD_JUMPER_T20),           "t20"},
    {std::to_string(Board::BOARD_JUMPER_TLITE),         "tlite"},
    {std::to_string(Board::BOARD_JUMPER_TLITE_F4),      "tlitef4"},
    {std::to_string(Board::BOARD_JUMPER_TPRO),          "tpro"},
    {std::to_string(Board::BOARD_JUMPER_TPROV2),        "tprov2"},
    {std::to_string(Board::BOARD_RADIOMASTER_BOXER),    "boxer"},
    {std::to_string(Board::BOARD_RADIOMASTER_T8),       "t8"},
    {std::to_string(Board::BOARD_RADIOMASTER_TX12),     "tx12"},
    {std::to_string(Board::BOARD_RADIOMASTER_TX12_MK2), "tx12mk2"},
    {std::to_string(Board::BOARD_RADIOMASTER_TX16S),    "tx16s"},
    {std::to_string(Board::BOARD_RADIOMASTER_ZORRO),    "zorro"},
    {std::to_string(Board::BOARD_RADIOMASTER_POCKET),   "pocket"},
    {std::to_string(Board::BOARD_BETAFPV_LR3PRO),       "lr3pro"},
    {std::to_string(Board::BOARD_IFLIGHT_COMMANDO8),    "commando8"},
    {std::to_string(Board::BOARD_FLYSKY_NV14),          "nv14"},
    {std::to_string(Board::BOARD_FLYSKY_PL18),          "pl18"},
};

static const StringTagMappingTable inputTypesLookupTable = {
    {std::to_string(Board::AIT_STICK),   "STICK"},
    {std::to_string(Board::AIT_FLEX),    "FLEX"},
    {std::to_string(Board::AIT_VBAT),    "VBAT"},
    {std::to_string(Board::AIT_RTC_BAT), "RTC_BAT"},
};

static const StringTagMappingTable flexTypesLookupTable = {
    {std::to_string(Board::FLEX_NONE),        "NONE"},
    {std::to_string(Board::FLEX_POT),         "POT"},
    {std::to_string(Board::FLEX_POT_CENTER),  "POT_CENTER"},
    {std::to_string(Board::FLEX_SLIDER),      "SLIDER"},
    {std::to_string(Board::FLEX_MULTIPOS),    "MULTIPOS"},
    {std::to_string(Board::FLEX_AXIS_X),      "AXIS_X"},
    {std::to_string(Board::FLEX_AXIS_Y),      "AXIS_Y"},
    {std::to_string(Board::FLEX_SWITCH),      "SWITCH"},
};

static const StringTagMappingTable switchTypesLookupTable = {
    {std::to_string(Board::SWITCH_NOT_AVAILABLE), "NONE"},
    {std::to_string(Board::SWITCH_TOGGLE),        "TOGGLE"},
    {std::to_string(Board::SWITCH_2POS),          "2POS"},
    {std::to_string(Board::SWITCH_3POS),          "3POS"},
    {std::to_string(Board::SWITCH_FSWITCH),       "FSWITCH"},
};

BoardJson::BoardJson(Board::Type board) :
  m_board(board),
  m_inputs(new InputsTable),
  m_switches(new SwitchesTable),
  m_trims(new TrimsTable),
  m_flexCnt(0),
  m_funcSwitchesCnt(0),
  m_gyrosCnt(0),
  m_joysticksCnt(0),
  m_potsCnt(0),
  m_slidersCnt(0),
  m_sticksCnt(0),
  m_switchesCnt(0),
  m_rtcbat(false),
  m_vbat(false)
{
  m_jsonFile = DataHelpers::getStringNameMappingTag(boardTypeJsonFileLookupTable, std::to_string(board).c_str());
//  qDebug() << Boards::getBoardName(m_board);
}

BoardJson::~BoardJson()
{
  delete m_inputs;
  delete m_switches;
  delete m_trims;
}

// static
void BoardJson::addJoysticksGyros(Board::Type board, InputsTable * inputs)
{
  // TODO which is to be trusted as they are not the same as Horis X12S json is only one with JSx & JSy!!!!!
  // if compiler directive then use better to use firmware options
  // from old Boards::getCapability - use this until decision
  if (IS_FAMILY_HORUS_OR_T16(board)) {
    if (getInputIndex(inputs, "JSx") < 0) {
      InputDefn defn;
      defn.name = "JSx";
      defn.stype = "FLEX";
      defn.type = AIT_FLEX;
      defn.label = "JSx";
      defn.shortLabel = "X";
      defn.dflt = "AXIS_X";
      defn.flexType = FLEX_AXIS_X;
      defn.inverted = false;
      inputs->insert(inputs->end(), defn);
    }

    if (getInputIndex(inputs, "JSy") < 0) {
      InputDefn defn;
      defn.name = "JSy";
      defn.stype = "FLEX";
      defn.type = AIT_FLEX;
      defn.label = "JSy";
      defn.shortLabel = "Y";
      defn.dflt = "AXIS_Y";
      defn.flexType = FLEX_AXIS_Y;
      defn.inverted = false;
      inputs->insert(inputs->end(), defn);
    }
  }
  // TODO json files do not contain gyro defs
  // if compiler directive then use better to use firmware options
  // from old Boards::getCapability - use this until decision
  if (IS_TARANIS_XLITES(board) || IS_FAMILY_HORUS_OR_T16(board)) {
    if (getInputIndex(inputs, "TILT_X") < 0) {
      InputDefn defn;
      defn.name = "TILT_X";
      defn.stype = "FLEX";
      defn.type = AIT_FLEX;
      defn.label = "Tltx";
      defn.shortLabel = "X";
      defn.dflt = "AXIS_X";
      defn.flexType = FLEX_AXIS_X;
      defn.inverted = false;
      inputs->insert(inputs->end(), defn);
    }

    if (getInputIndex(inputs, "TILT_Y") < 0) {
      InputDefn defn;
      defn.name = "TILT_Y";
      defn.stype = "FLEX";
      defn.type = AIT_FLEX;
      defn.label = "Tlty";
      defn.shortLabel = "Y";
      defn.dflt = "AXIS_Y";
      defn.flexType = FLEX_AXIS_Y;
      defn.inverted = false;
      inputs->insert(inputs->end(), defn);
    }
  }
}

// called from Boards::getCapability if no capability match
const int BoardJson::getCapability(const Board::Capability capability) const
{
  switch (capability) {
    case Board::FlexInputs:
      return m_flexCnt;

    case Board::FunctionSwitches:
      return m_funcSwitchesCnt;

    case GyroAnalogs:
      return m_gyrosCnt;

    case HasRTC:
      return m_rtcbat;

    case HasVBat:
      return m_vbat;

    case Board::Inputs:
      return m_inputs->size();

    case Joysticks:
      return m_joysticksCnt;

    case Board::NumTrims:
      return m_trims->size();

    case Board::Pots:
      return m_potsCnt;

    case Board::Sliders:
      return m_slidersCnt;

    case Board::Sticks:
      return m_sticksCnt;

    case Board::Switches:
      return m_switchesCnt;

    default:
      return 0;
  }
}

const int BoardJson::getInputIndex(const QString name) const
{
  return getInputIndex(m_inputs, name);
}

// static
int BoardJson::getInputIndex(const InputsTable * inputs, QString name)
{
  for (int i = 0; i < (int)inputs->size(); i++) {
    if (inputs->at(i).name.c_str() == name)
      return i;
  }

  return -1;
}

const QString BoardJson::getInputName(int index) const
{
  return getInputName(m_inputs, index);
}

// static
QString BoardJson::getInputName(const InputsTable * inputs, int index)
{
  if (index > -1 && index < (int)inputs->size())
    return inputs->at(index).name.c_str();

  return CPN_STR_UNKNOWN_ITEM;
}

const QString BoardJson::getInputLabel(int index) const
{
  return getInputLabel(m_inputs, index);
}

// static
QString BoardJson::getInputLabel(const InputsTable * inputs, int index)
{
  if (index > -1 && index < (int)inputs->size())
    return inputs->at(index).label.c_str();

  return CPN_STR_UNKNOWN_ITEM;
}

const int BoardJson::getSwitchIndex(const QString name) const
{
  return getSwitchIndex(m_switches, name);
}

// static
int BoardJson::getSwitchIndex(const SwitchesTable * switches, QString name)
{
  for (int i = 0; i < (int)switches->size(); i++) {
    if (switches->at(i).name.c_str() == name)
      return i;
  }

  return -1;
}

const QString BoardJson::getSwitchName(int index) const
{
  return getSwitchName(m_switches, index);
}

// static
QString BoardJson::getSwitchName(const SwitchesTable * switches, int index)
{
  if (index > -1 && index < (int)switches->size())
    return switches->at(index).name.c_str();

  return CPN_STR_UNKNOWN_ITEM;
}

const int BoardJson::getInputsCalibrated() const
{
  return getInputsCalibrated(m_inputs);
}

// static
int BoardJson::getInputsCalibrated(const InputsTable * inputs)
{
  unsigned int cnt = 0;

  for (const auto &defn : *inputs) {
    if (isInputCalibrated(defn)) cnt++;
  }

  return cnt;
}

const int BoardJson::getInputNameOffset(QString name)
{
  return getInputNameOffset(m_inputs, name);
}

// static
int BoardJson::getInputNameOffset(const InputsTable * inputs, QString name)
{
  for (int i = 0; i < (int)inputs->size(); i++) {
    if (name == inputs->at(i).name.c_str())
      return i;
  }

  return 0;
}

const int BoardJson::getInputPotIndex(int index)
{
  if (getCapability(Board::Pots) > 0)
    return getInputNameOffset(m_inputs, QString("P%1").arg(index));

  return -1;
}

const int BoardJson::getInputSliderIndex(int index)
{
  if (getCapability(Board::Sliders) > 0)
    return getInputNameOffset(m_inputs, QString("SL%1").arg(index));

  return -1;
}

const int BoardJson::getInputTypeOffset(Board::AnalogInputType type)
{
  return getInputTypeOffset(m_inputs, type);
}

// static
int BoardJson::getInputTypeOffset(const InputsTable * inputs, Board::AnalogInputType type)
{
  for (int i = 0; i < (int)inputs->size(); i++) {
    if (type == inputs->at(i).type)
      return i;
  }

  return 0;
}

const Board::InputInfo BoardJson::getInputInfo(int index) const
{
  return getInputInfo(m_inputs, index);
}

// static
Board::InputInfo BoardJson::getInputInfo(const InputsTable * inputs, int index)
{
  Board::InputInfo info;

  if (index >= 0 && index < (int)inputs->size()) {
    InputDefn defn = inputs->at(index);
    info.name = defn.name;
    info.type = defn.type;
    info.label = defn.label;
    info.shortLabel = defn.shortLabel;
    info.flexType = defn.flexType;
    info.inverted = defn.inverted;
  }

  return info;
}

const Board::SwitchInfo BoardJson::getSwitchInfo(int index) const
{
  return getSwitchInfo(m_switches, index);
}

// static
Board::SwitchInfo BoardJson::getSwitchInfo(const SwitchesTable * switches, int index)
{
  Board::SwitchInfo info;

  if (index >= 0 && index < (int)switches->size()) {
    SwitchDefn defn = switches->at(index);
    info.type = defn.type;
    info.name = defn.name;
    info.inverted = defn.inverted;
  }

  return info;
}

const bool BoardJson::isInputCalibrated(int index) const
{
  return isInputCalibrated(m_inputs->at(index));
}

// static
bool BoardJson::isInputCalibrated(const InputDefn & defn)
{
  return ((defn.type == Board::AIT_STICK ||
          (defn.type == Board::AIT_FLEX && defn.flexType > Board::FLEX_NONE && defn.flexType < Board::FLEX_AXIS_X)));
}

const bool BoardJson::isInputConfigurable(int index) const
{
  return isInputConfigurable(m_inputs->at(index));
}

// static
bool BoardJson::isInputConfigurable(const InputDefn & defn)
{
  return isStick(defn) || isPot(defn) || isSlider(defn);
}

const bool BoardJson::isInputPot(int index) const
{
  return !isStick(m_inputs->at(index));
}

const bool BoardJson::isInputStick(int index) const
{
  return isStick(m_inputs->at(index));
}

// static
bool BoardJson::isFlex(const InputDefn & defn)
{
  return defn.type == Board::AIT_FLEX;
}

// static
bool BoardJson::isMultipos(const InputDefn & defn)
{
  return defn.type == Board::AIT_FLEX && defn.flexType == Board::FLEX_MULTIPOS;
}

// static
bool BoardJson::isPot(const InputDefn & defn)
{
  const char* val = defn.name.data();
  size_t len = defn.name.size();

  return (defn.type == Board::AIT_FLEX &&
          ((len > 1 && val[0] == 'P' && val[1] >= '0' && val[1] <= '9') ||
           (len > 3 && val[0] == 'E' && val[1] == 'X' && val[2] == 'T' && val[3] >= '0' && val[3] <= '9')));
}

// static
bool BoardJson::isSlider(const InputDefn & defn)
{
  const char* val = defn.name.data();
  size_t len = defn.name.size();

  return (defn.type == Board::AIT_FLEX && len > 2 &&
          val[0] == 'S' && val[1] == 'L' && val[2] >= '0' && val[2] <= '9');
}

// static
bool BoardJson::isFlexSwitch(const InputDefn & defn)
{
  return defn.type == Board::AIT_FLEX && defn.flexType == Board::FLEX_SWITCH;
}

// static
bool BoardJson::isFuncSwitch(const SwitchDefn & defn)
{
  return defn.type == Board::SWITCH_FSWITCH;
}

// static
bool BoardJson::isGyro(const InputDefn & defn)
{
  const char* val = defn.name.data();
  size_t len = defn.name.size();

  return (defn.type == Board::AIT_FLEX && len > 5 &&
          val[0] == 'T' && val[1] == 'I'  && val[2] == 'L' && val[3] == 'T' && val[4] == '_' && (val[5] == 'X' || val[5] == 'Y'));
}

// static
bool BoardJson::isJoystick(const InputDefn & defn)
{
  const char* val = defn.name.data();
  size_t len = defn.name.size();

  return (defn.type == Board::AIT_FLEX && len > 2 &&
          val[0] == 'J' && val[1] == 'S' && (val[2] == 'x' || val[2] == 'y'));
}

// static
bool BoardJson::isRTCBat(const InputDefn & defn)
{
  return defn.type == Board::AIT_RTC_BAT;
}

// static
bool BoardJson::isStick(const InputDefn & defn)
{
  return defn.type == Board::AIT_STICK;
}

// static
bool BoardJson::isSwitch(const SwitchDefn & defn)
{
  return defn.type != Board::SWITCH_FSWITCH;
}

// static
bool BoardJson::isVBat(const InputDefn & defn)
{
  return defn.type == Board::AIT_VBAT;
}

bool BoardJson::loadDefinition()
{
  // safety net for BoardFactory::instance
  if (m_board == Board::BOARD_UNKNOWN)
    return true;

  if (!loadFile(m_board, m_jsonFile, m_inputs, m_switches, m_trims))
    return false;

  // json files do not normally define joysticks or gyros
  addJoysticksGyros(m_board, m_inputs);

  m_flexCnt = setFlexCount(m_inputs);
  m_gyrosCnt = setGyrosCount(m_inputs);
  m_joysticksCnt = setJoysticksCount(m_inputs);
  m_potsCnt = setPotsCount(m_inputs);
  m_slidersCnt = setSlidersCount(m_inputs);
  m_sticksCnt = setSticksCount(m_inputs);
  m_funcSwitchesCnt = setFuncSwitchesCount(m_switches);
  m_switchesCnt = setSwitchesCount(m_switches);
  m_rtcbat = setRTCBat(m_inputs);
  m_vbat = setVBat(m_inputs);

  // json files do not normally specify stick labels so load legacy labels
  for (int i = 0; i < getCapability(Board::Sticks); i++) {
    if (m_inputs->at(i).label.empty())
      m_inputs->at(i).label = setStickLabel(i);
  }

//  qDebug() << "inputs:" << getCapability(Board::Inputs) <<
//              "sticks:" << getCapability(Board::Sticks) <<
//              "flex:" << getCapability(Board::FlexInputs) <<
//              "pots:" << getCapability(Board::Pots) <<
//              "sliders:" << getCapability(Board::Sliders) <<
//              "gyros:" << getCapability(Board::GyroAnalogs) <<
//              "joysticks:" << getCapability(Board::Joysticks) <<
//              "trims:" << getCapability(Board::NumTrims) <<
//              "switches:" << getCapability(Board::Switches) <<
//              "funcswitches:" << getCapability(Board::FunctionSwitches) <<
//              "rtcbat:" << getCapability(Board::HasRTC) <<
//              "vbat:" << getCapability(Board::HasVBat);

  return true;
}

// static
bool BoardJson::loadFile(Board::Type board, std::string jsonFile, InputsTable * inputs, SwitchesTable * switches, TrimsTable * trims)
{
  if (board == Board::BOARD_UNKNOWN) {
//    qDebug() << "No board definition for board id:" << board;
    return false;
  }

  if (jsonFile.empty()) {
//    qDebug() << "No json filename for board:" << Boards::getBoardName(board);
    return false;
  }

  //  retrieve from application resources
  QString path = QString(":/hwdefs/%1.json").arg(jsonFile.c_str());
  QFile file(path);

  if (!file.exists()) {
//    qDebug() << "File not found:" << path;
    return false;
  }

  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Unable to open:" << path;
    return false;
  }

  QByteArray buffer = file.readAll();
  file.close();

  if (buffer.isEmpty()) {
    qDebug() << "Unable to read:" << path;
    return false;
  }

  QJsonParseError res;
  QJsonDocument *json = new QJsonDocument();
  *json = QJsonDocument::fromJson(buffer, &res);

  if (res.error || json->isNull() || !json->isObject()) {
    qDebug() << path << "is not a valid json formatted file. Error:" << res.error << res.errorString();
    delete json;
    return false;
  }

  const QJsonObject &obj = json->object();

  if (obj.value("adc_inputs").isObject()) {
    const QJsonObject &adcinputs = obj.value("adc_inputs").toObject();

    if (adcinputs.value("inputs").isArray()) {
      const QJsonArray &in = adcinputs.value("inputs").toArray();

      for (const QJsonValue &input : in)
      {
        if (input.isObject()) {
          const QJsonObject &o = input.toObject();
          InputDefn defn;

          if (!o.value("name").isUndefined())
            defn.name = o.value("name").toString().toStdString();

          if (!o.value("type").isUndefined()) {
            defn.stype = o.value("type").toString().toStdString();
            defn.type = (AnalogInputType)DataHelpers::getStringTagMappingIndex(inputTypesLookupTable, defn.stype.c_str());
          }

          if (!o.value("inverted").isUndefined())
            defn.inverted = o.value("inverted").toBool();

          if (!o.value("label").isUndefined())
            defn.label = o.value("label").toString().toStdString();

          if (!o.value("short_label").isUndefined())
            defn.shortLabel = o.value("short_label").toString().toStdString();

          if (!o.value("default").isUndefined()) {
            defn.dflt = o.value("default").toString().toStdString();
            if (defn.type == AIT_FLEX) {
              int idx = DataHelpers::getStringTagMappingIndex(flexTypesLookupTable, defn.dflt.c_str());
              defn.flexType = (FlexType)(idx < 0 ? FLEX_NONE : idx);
            }
          }

          inputs->insert(inputs->end(), defn);

//          qDebug() << "name:" << defn.name.c_str() <<
//                      "type:" << defn.stype.c_str() << ">" <<
//                      DataHelpers::getStringNameMappingTag(inputTypesLookupTable, std::to_string(defn.type).c_str()).c_str() <<
//                      "label:" << defn.label.c_str() << "short:" << defn.shortLabel.c_str() << "inverted:" << defn.inverted <<
//                      "dflt:" << defn.dflt.c_str() << ">" <<
//                      DataHelpers::getStringNameMappingTag(flexTypesLookupTable, std::to_string(defn.flexType).c_str()).c_str();
        }
      }
    }
  }

  if (obj.value("switches").isArray()) {
    const QJsonArray &swtchs = obj.value("switches").toArray();

    for (const QJsonValue &swtch : swtchs)
    {
      if (swtch.isObject()) {
        const QJsonObject &o = swtch.toObject();
        SwitchDefn sw;

        if (!o.value("name").isUndefined())
          sw.name = o.value("name").toString().toStdString();

        if (!o.value("type").isUndefined()) {
          sw.stype = o.value("type").toString().toStdString();
          int idx = DataHelpers::getStringTagMappingIndex(switchTypesLookupTable, sw.stype.c_str());
          sw.type = idx < 0 ? Board::SWITCH_NOT_AVAILABLE : (Board::SwitchType)idx;
        }

        if (!o.value("flags").isUndefined())
          sw.flags = o.value("flags").toInt();

        if (!o.value("inverted").isUndefined())
          sw.inverted = o.value("inverted").toBool();

        if (!o.value("default").isUndefined()) {
          sw.sdflt = o.value("default").toString().toStdString();
          int idx = DataHelpers::getStringTagMappingIndex(switchTypesLookupTable, sw.sdflt.c_str());
          sw.dflt = idx < 0 ? Board::SWITCH_NOT_AVAILABLE : (Board::SwitchType)idx;
        }

        if (o.value("display").isArray()) {
          const QJsonArray &d = obj.value("display").toArray();
          sw.display.x = (unsigned int)d.at(0).toInt(0);
          sw.display.y = (unsigned int)d.at(1).toInt(0);
        }

        switches->insert(switches->end(), sw);

//        qDebug() << "name:" << sw.name.c_str() << "type:" << sw.stype.c_str() << ">" << Boards::switchTypeToString(sw.type) <<
//                    "flags:" << sw.flags << "default:" << sw.sdflt.c_str() << ">" << Boards::switchTypeToString(sw.dflt) <<
//                    "inverted:" << sw.inverted << "display:" << QString("%1").arg(sw.display.x) << "," << QString("%1").arg(sw.display.y);
      }
    }
  }

  if (obj.value("trims").isArray()) {
    const QJsonArray &trms = obj.value("trims").toArray();

    for (const QJsonValue &trm : trms)
    {
      if (trm.isObject()) {
        const QJsonObject &o = trm.toObject();
        TrimDefn t;

        if (!o.value("name").isUndefined())
          t.name = o.value("name").toString().toStdString();

        trims->insert(trims->end(), t);

//        qDebug() << "name:" << t.name.c_str();
      }
    }
  }

  delete json;
  return true;
}

// static
unsigned int BoardJson::setFlexCount(const InputsTable * inputs)
{
  unsigned int cnt = 0;

  for (const auto &defn : *inputs) {
    if (isFlex(defn)) cnt++;
  }

  return cnt;
}

// static
unsigned int BoardJson::setGyrosCount(const InputsTable * inputs)
{
  unsigned int cnt = 0;

  for (const auto &defn : *inputs) {
    if (isGyro(defn)) cnt++;
  }

  return cnt;
}

// static
unsigned int BoardJson::setJoysticksCount(const InputsTable * inputs)
{
  unsigned int cnt = 0;

  for (const auto &defn : *inputs) {
    if (isJoystick(defn)) cnt++;
  }

  return cnt;
}

// static
unsigned int BoardJson::setPotsCount(const InputsTable * inputs)
{
  unsigned int cnt = 0;

  for (const auto &defn : *inputs) {
    if (isPot(defn)) cnt++;
  }

  return cnt;
}

// static
bool BoardJson::setRTCBat(const InputsTable * inputs)
{
  for (const auto &defn : *inputs) {
    if (isRTCBat(defn)) return true;
  }

  return false;
}

// static
unsigned int BoardJson::setSlidersCount(const InputsTable * inputs)
{
  unsigned int cnt = 0;

  for (const auto &defn : *inputs) {
    if (isSlider(defn)) cnt++;
  }

  return cnt;
}

// static
unsigned int BoardJson::setSticksCount(const InputsTable * inputs)
{
  unsigned int cnt = 0;

  for (const auto &defn : *inputs) {
    if (isStick(defn)) cnt++;
  }

  return cnt;
}

// static
unsigned int BoardJson::setFuncSwitchesCount(const SwitchesTable * switches)
{
  unsigned int cnt = 0;

  for (const auto &swtch : *switches) {
    if (isFuncSwitch(swtch)) cnt++;
  }

  return cnt;
}

// static
std::string BoardJson::setStickLabel(int index)
{
  QStringList strl = { tr("Rud"), tr("Ele"), tr("Thr"), tr("Ail") };
  return strl.value(index, CPN_STR_UNKNOWN_ITEM).toStdString();
}

// static
unsigned int BoardJson::setSwitchesCount(const SwitchesTable * switches)
{
  unsigned int cnt = 0;

  for (const auto &swtch : *switches) {
    if (isSwitch(swtch)) cnt++;
  }

  return cnt;
}

// static
bool BoardJson::setVBat(const InputsTable * inputs)
{
  for (const auto &defn : *inputs) {
    if (isVBat(defn)) return true;
  }

  return false;
}
