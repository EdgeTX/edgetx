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

#include "yaml_switchconfig.h"

#include "boards.h"
#include "eeprominterface.h"
#include "generalsettings.h"
#include "boardjson.h"

const YamlLookupTable switchConfigLut = {
    {Board::SWITCH_NOT_AVAILABLE, "none"},
    {Board::SWITCH_TOGGLE, "toggle"},
    {Board::SWITCH_2POS, "2pos"},
    {Board::SWITCH_3POS, "3pos"},
};

const YamlLookupTable potConfigLut = {
    {Board::FLEX_NONE, "none"},
    {Board::FLEX_POT, "without_detent"},
    {Board::FLEX_POT_CENTER, "with_detent"},
    {Board::FLEX_SLIDER, "slider"},
    {Board::FLEX_MULTIPOS, "multipos_switch"},
    {Board::FLEX_AXIS_X, "axis_x"},
    {Board::FLEX_AXIS_Y, "axis_y"},
    {Board::FLEX_SWITCH, "switch"},
};

// pre v2.10
const YamlLookupTable sliderConfigLut = {
    {Board::SLIDER_NONE, "none"},
    {Board::SLIDER_WITH_DETENT, "with_detent"},
};

// pre v2.10
const YamlLookupTable sticksLut = {
    {0, "Rud"},
    {1, "Ele"},
    {2, "Thr"},
    {3, "Ali"},
};

YamlPotConfig::YamlPotConfig(const GeneralSettings::InputConfig* rhs)
{
  for (int i = 0; i < Boards::getCapability(getCurrentBoard(), Board::Inputs); i++) {
    if (Boards::isInputConfigurable(i)) {
      config[i].tag = Boards::getInputYamlName(i, BoardJson::YLT_CONFIG).toStdString();
      config[i].type = rhs[i].type;
      memcpy(config[i].name, rhs[i].name, sizeof(HARDWARE_NAME_LEN));
      config[i].flexType = rhs[i].flexType;
      config[i].inverted = rhs[i].inverted;
    }
  }
}

void YamlPotConfig::copy(GeneralSettings::InputConfig* rhs) const
{
  for (int i = 0; i < Boards::getCapability(getCurrentBoard(), Board::Inputs); i++) {
    if (config[i].type == (unsigned int)Board::AIT_FLEX) {
      memcpy(rhs[i].name, config[i].name, sizeof(HARDWARE_NAME_LEN));
      rhs[i].flexType = (Board::FlexType)config[i].flexType;
      rhs[i].inverted = config[i].inverted;
    }
  }
}

void YamlSliderConfig::copy(GeneralSettings::InputConfig* rhs) const
{
  for (int i = 0; i < Boards::getCapability(getCurrentBoard(), Board::Inputs); i++) {
    if (config[i].type == (unsigned int)Board::AIT_FLEX) {
      memcpy(rhs[i].name, config[i].name, sizeof(HARDWARE_NAME_LEN));
      rhs[i].flexType = (Board::FlexType)config[i].flexType;
      rhs[i].inverted = config[i].inverted;
    }
  }
}

YamlStickConfig::YamlStickConfig(const GeneralSettings::InputConfig* rhs)
{
  for (int i = 0; i < Boards::getCapability(getCurrentBoard(), Board::Inputs); i++) {
    config[i].tag = std::to_string(i);
    config[i].type = rhs[i].type;
    memcpy(config[i].name, rhs[i].name, sizeof(HARDWARE_NAME_LEN));
  }
}

void YamlStickConfig::copy(GeneralSettings::InputConfig* rhs) const
{
  for (int i = 0; i < Boards::getCapability(getCurrentBoard(), Board::Inputs); i++) {
    if (config[i].type == (unsigned int)Board::AIT_STICK) {
      memcpy(rhs[i].name, config[i].name, sizeof(HARDWARE_NAME_LEN));
    }
  }
}

YamlSwitchConfig::YamlSwitchConfig(const GeneralSettings::SwitchConfig* rhs)
{
  for (int i = 0; i < Boards::getCapability(getCurrentBoard(), Board::Switches); i++) {
    config[i].tag = Boards::getSwitchYamlName(i, BoardJson::YLT_CONFIG).toStdString();
    config[i].type = rhs[i].type;
    memcpy(config[i].name, rhs[i].name, sizeof(HARDWARE_NAME_LEN));
    // config[i].inverted = rhs[i].inverted;
  }
}

void YamlSwitchConfig::copy(GeneralSettings::SwitchConfig* rhs) const
{
  for (int i = 0; i < Boards::getCapability(getCurrentBoard(), Board::Switches); i++) {
    if (config[i].type != (unsigned int)Board::SWITCH_NOT_AVAILABLE) {
      memcpy(rhs[i].name, config[i].name, sizeof(HARDWARE_NAME_LEN));
      rhs[i].type = (Board::SwitchType)config[i].type;
      // rhs[i].inverted = config[i].inverted;
    }
  }
}

YamlSwitchesFlex::YamlSwitchesFlex(const GeneralSettings::SwitchConfig* rhs)
{
  for (int i = 0; i < Boards::getCapability(getCurrentBoard(), Board::Switches); i++) {
    if (Boards::isSwitchFlex(i) && rhs[i].inputIdx != SWITCH_INPUTINDEX_NONE) {
      int idx = Boards::getSwitchTagNum(i) - 1;
      if (idx >= 0 && idx < CPN_MAX_SWITCHES_FLEX) {
        config[idx].tag = Boards::getSwitchYamlName(i, BoardJson::YLT_CONFIG).toStdString();
        config[idx].channel = Boards::getInputYamlName(rhs[i].inputIdx, BoardJson::YLT_CONFIG).toStdString();
      }
    }
  }
}

void YamlSwitchesFlex::copy(GeneralSettings::SwitchConfig* rhs) const
{
  for (int i = 0; i < Boards::getCapability(getCurrentBoard(), Board::FlexSwitches); i++) {
    int idx = Boards::getSwitchYamlIndex(config[i].tag.c_str(), BoardJson::YLT_CONFIG);
    rhs[idx].inputIdx = config[i].inputIndx;
  }
}

namespace YAML
{

Node convert<InputConfig>::encode(const InputConfig& rhs)
{
  Node node;
  node["name"] = rhs.name;

  if (rhs.type != Board::AIT_STICK) {
    node["type"] = potConfigLut << rhs.flexType;
    node["inv"] = (int)rhs.inverted;
  }

  return node;
}

bool convert<InputConfig>::decode(const Node& node, InputConfig& rhs)
{
  if (!node.IsMap()) return false;

  node["name"] >> rhs.name;
  node["type"] >> potConfigLut >> rhs.flexType;
  node["inv"] >> rhs.inverted;

  if (radioSettingsVersion < SemanticVersion(QString(CPN_ADC_REFACTOR_VERSION))) {
    int idx = Boards::getInputYamlIndex(rhs.tag.c_str(), BoardJson::YLT_CONFIG);
    if (idx >= 0) {
      Board::InputInfo info = Boards::getInputInfo(idx);
      rhs.inverted = info.inverted;
    }
    else
      rhs.inverted = false;
  }

  return true;
}

Node convert<SwitchConfig>::encode(const SwitchConfig& rhs)
{
  Node node;
  node["type"] = switchConfigLut << rhs.type;
  node["name"] = rhs.name;
  // node["inv"] = (int)rhs.inverted;  in hwdef json but not implemented in radio yaml
  return node;
}

bool convert<SwitchConfig>::decode(const Node& node, SwitchConfig& rhs)
{
  if (!node.IsMap()) return false;

  node["type"] >> switchConfigLut >> rhs.type;
  node["name"] >> rhs.name;
  // node["inv"] >> rhs.inverted;  in hwdef json but not implemented in radio yaml

  if (radioSettingsVersion < SemanticVersion(QString(CPN_ADC_REFACTOR_VERSION))) {
    int idx = Boards::getSwitchYamlIndex(rhs.tag.c_str(), BoardJson::YLT_CONFIG);
    if (idx >= 0) {
      Board::SwitchInfo info = Boards::getSwitchInfo(idx);
      rhs.inverted = info.inverted;
    }
  }

  return true;
}

Node convert<SwitchFlex>::encode(const SwitchFlex& rhs)
{
  Node node;
  node["channel"] = rhs.channel;
  return node;
}

bool convert<SwitchFlex>::decode(const Node& node, SwitchFlex& rhs)
{
  if (!node.IsMap()) return false;

  node["channel"] >> rhs.channel;
  rhs.inputIndx = Boards::getInputYamlIndex(rhs.channel.c_str(), BoardJson::YLT_CONFIG);

  return true;
}

Node convert<YamlPotConfig>::encode(const YamlPotConfig& rhs)
{
  Node node;
  const int maxcnt = Boards::getCapability(getCurrentBoard(), Board::Inputs);

  for (int i = 0; i < maxcnt; i++) {
    if (rhs.config[i].type == (unsigned int)Board::AIT_FLEX && rhs.config[i].flexType != (unsigned int)Board::FLEX_NONE) {
      std::string tag = rhs.config[i].tag;
      node[tag] = rhs.config[i];
    }
  }

  return node;
}

bool convert<YamlPotConfig>::decode(const Node& node, YamlPotConfig& rhs)
{
  if (!node.IsMap()) return false;

  const int maxcnt = Boards::getCapability(getCurrentBoard(), Board::Inputs);

  for (const auto& kv : node) {
    std::string tag;
    kv.first >> tag;

    if (radioSettingsVersion < SemanticVersion(QString(CPN_ADC_REFACTOR_VERSION)))
      tag = Boards::getLegacyAnalogMappedInputTag(tag.c_str());

    int idx = Boards::getInputYamlIndex(tag.c_str(), BoardJson::YLT_CONFIG);

    if (idx >= 0 && idx < maxcnt) {
      kv.second >> rhs.config[idx];
      rhs.config[idx].tag = tag;
      rhs.config[idx].type = Board::AIT_FLEX;
    }
  }

  return true;
}

//
bool convert<YamlSliderConfig>::decode(const Node& node, YamlSliderConfig& rhs)
{
  if (!node.IsMap()) return false;

  const int maxcnt = Boards::getCapability(getCurrentBoard(), Board::Inputs);

  if (Boards::getCapability(getCurrentBoard(), Board::Sliders) < 1)
    return true;

  int i = 1;

  for (const auto& kv : node) {
    std::string tag;
    kv.first >> tag;
    int idx = Boards::getInputSliderIndex(i);

    if (idx >= 0 && idx < maxcnt) {
      kv.second >> rhs.config[idx];
      rhs.config[idx].tag = Boards::getInputTag(idx).toStdString();
      rhs.config[idx].type = Board::AIT_FLEX;
    }

    i++;
  }

  return true;
}

Node convert<YamlStickConfig>::encode(const YamlStickConfig& rhs)
{
  Node node;
  const int maxcnt = Boards::getCapability(getCurrentBoard(), Board::Inputs);

  for (int i = 0; i < maxcnt; i++) {
    if (rhs.config[i].type == (unsigned int)Board::AIT_STICK) {
      std::string tag = rhs.config[i].tag;
      node[tag] = rhs.config[i];
    }
  }

  return node;
}

bool convert<YamlStickConfig>::decode(const Node& node, YamlStickConfig& rhs)
{
  if (!node.IsMap()) return false;

  const int maxcnt = Boards::getCapability(getCurrentBoard(), Board::Inputs);

  for (const auto& kv : node) {
    std::string tag;
    kv.first >> tag;

    if (!tag.empty()) {
      int idx = std::stoi(tag);

      if (idx >= 0 && idx < maxcnt) {
        kv.second >> rhs.config[idx];
        rhs.config[idx].tag = tag;
        rhs.config[idx].type = Board::AIT_STICK;
      }
    }
  }

  return true;
}

Node convert<YamlSwitchConfig>::encode(const YamlSwitchConfig& rhs)
{
  Node node;
  const int maxcnt = Boards::getCapability(getCurrentBoard(), Board::Switches);

  for (int i = 0; i < maxcnt; i++) {
    if (rhs.config[i].type != Board::SWITCH_NOT_AVAILABLE) {
      std::string tag = Boards::getSwitchYamlName(i, BoardJson::YLT_CONFIG).toStdString();
      node[tag] = rhs.config[i];
    }
  }

  return node;
}

bool convert<YamlSwitchConfig>::decode(const Node& node, YamlSwitchConfig& rhs)
{
  if (!node.IsMap()) return false;

  Board::Type board = getCurrentBoard();

  const int maxcnt = Boards::getCapability(board, Board::Switches);

  for (const auto& kv : node) {
    std::string tag;
    kv.first >> tag;
    int idx = Boards::getSwitchYamlIndex(tag.c_str(), BoardJson::YLT_CONFIG);

    if (idx >= 0 && idx < maxcnt) {
      kv.second >> rhs.config[idx];
      rhs.config[idx].tag = tag;
    }
  }

  return true;
}

Node convert<YamlSwitchesFlex>::encode(const YamlSwitchesFlex& rhs)
{
  Node node;
  const int maxcnt = Boards::getCapability(getCurrentBoard(), Board::FlexSwitches);

  for (int i = 0; i < maxcnt; i++) {
    if (!rhs.config[i].tag.empty())
      node[rhs.config[i].tag] = rhs.config[i];
  }

  return node;
}

bool convert<YamlSwitchesFlex>::decode(const Node& node, YamlSwitchesFlex& rhs)
{
  if (!node.IsMap()) return false;

  const int maxcnt = Boards::getCapability(getCurrentBoard(), Board::FlexSwitches);

  for (const auto& kv : node) {
    std::string tag;
    kv.first >> tag;

    const char* val = tag.data();
    size_t len = tag.size();

    if (len > 2 && val[0] == 'F' && val[1] == 'L'  && val[2] > '0' && val[2] <= '9') {
      int idx = std::stoi(tag.substr(2, len - 2)) - 1;

      if (idx >= 0 && idx < maxcnt) {
        kv.second >> rhs.config[idx];
        rhs.config[idx].tag = tag;
      }
    }
  }

  return true;
}

}  // namespace YAML
