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

#include "yaml_switchconfig.h"

#include "boards.h"
#include "eeprominterface.h"

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
    {2, "Ele"},
    {3, "Thr"},
    {4, "Ali"},
};

YamlPotConfig::YamlPotConfig(const GeneralSettings::InputConfig* rhs)
{
  Board::Type board = getCurrentBoard();

  for (int i = 0; i < Boards::getCapability(board, Board::Inputs); i++) {
    potConfig[i].tag = Boards::getInputName(board, i).toStdString();
    potConfig[i].type = rhs[i].type;
    memcpy(potConfig[i].name, rhs[i].name, sizeof(HARDWARE_NAME_LEN));
    potConfig[i].flexType = rhs[i].flexType;
    potConfig[i].inverted = rhs[i].inverted;
  }
}

void YamlPotConfig::copy(GeneralSettings::InputConfig* rhs) const
{
  Board::Type board = getCurrentBoard();

  for (int i = 0; i < Boards::getCapability(board, Board::Inputs); i++) {
    if (potConfig[i].type == (unsigned int)Board::AIT_FLEX) {
      memcpy(rhs[i].name, potConfig[i].name, sizeof(HARDWARE_NAME_LEN));
      rhs[i].flexType = (Board::FlexType)potConfig[i].flexType;
      rhs[i].inverted = potConfig[i].inverted;
    }
  }
}

void YamlSliderConfig::copy(GeneralSettings::InputConfig* rhs) const
{
  Board::Type board = getCurrentBoard();

  for (int i = 0; i < Boards::getCapability(board, Board::Inputs); i++) {
    if (sliderConfig[i].type == (unsigned int)Board::AIT_FLEX) {
      memcpy(rhs[i].name, sliderConfig[i].name, sizeof(HARDWARE_NAME_LEN));
      rhs[i].flexType = (Board::FlexType)sliderConfig[i].flexType;
      rhs[i].inverted = sliderConfig[i].inverted;
    }
  }
}

YamlStickConfig::YamlStickConfig(const GeneralSettings::InputConfig* rhs)
{
  Board::Type board = getCurrentBoard();

  for (int i = 0; i < Boards::getCapability(board, Board::Inputs); i++) {
    stickConfig[i].tag = std::to_string(i);
    stickConfig[i].type = rhs[i].type;
    memcpy(stickConfig[i].name, rhs[i].name, sizeof(HARDWARE_NAME_LEN));
  }
}

void YamlStickConfig::copy(GeneralSettings::InputConfig* rhs) const
{
  Board::Type board = getCurrentBoard();

  for (int i = 0; i < Boards::getCapability(board, Board::Inputs); i++) {
    if (stickConfig[i].type == (unsigned int)Board::AIT_STICK) {
      memcpy(rhs[i].name, stickConfig[i].name, sizeof(HARDWARE_NAME_LEN));
    }
  }
}

YamlSwitchConfig::YamlSwitchConfig(const GeneralSettings::SwitchConfig* rhs)
{
  Board::Type board = getCurrentBoard();

  for (int i = 0; i < Boards::getCapability(board, Board::Switches); i++) {
    switchConfig[i].tag = Boards::getSwitchName(board, i).toStdString();
    switchConfig[i].type = rhs[i].type;
    memcpy(switchConfig[i].name, rhs[i].name, sizeof(HARDWARE_NAME_LEN));
    // switchConfig[i].inverted = rhs[i].inverted;
  }
}

void YamlSwitchConfig::copy(GeneralSettings::SwitchConfig* rhs) const
{
 Board::Type board = getCurrentBoard();

  for (int i = 0; i < Boards::getCapability(board, Board::Switches); i++) {
    if (switchConfig[i].type != (unsigned int)Board::SWITCH_NOT_AVAILABLE) {
      memcpy(rhs[i].name, switchConfig[i].name, sizeof(HARDWARE_NAME_LEN));
      rhs[i].type = (Board::SwitchType)switchConfig[i].type;
      // rhs[i].inverted = switchConfig[i].inverted;
    }
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

  Board::Type board = getCurrentBoard();

  node["name"] >> rhs.name;
  node["type"] >> potConfigLut >> rhs.flexType;
  node["inv"] >> rhs.inverted;

  if (radioSettingsVersion < SemanticVersion(QString(CPN_ADC_REFACTOR_VERSION))) {
    int idx = Boards::getInputIndex(board, rhs.tag.c_str());
    if (idx >= 0) {
      Board::InputInfo info = Boards::getInputInfo(board, idx);
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

  Board::Type board = getCurrentBoard();

  node["type"] >> switchConfigLut >> rhs.type;
  node["name"] >> rhs.name;
  // node["inv"] >> rhs.inverted;  in hwdef json but not implemented in radio yaml

  if (radioSettingsVersion < SemanticVersion(QString(CPN_ADC_REFACTOR_VERSION))) {
    int idx = Boards::getSwitchIndex(board, rhs.tag.c_str());
    if (idx >= 0) {
      Board::SwitchInfo info = Boards::getSwitchInfo(board, idx);
      rhs.inverted = info.inverted;
    }
  }

  return true;
}

Node convert<YamlPotConfig>::encode(const YamlPotConfig& rhs)
{
  Node node;
  Board::Type board = getCurrentBoard();
  const int maxcnt = Boards::getCapability(board, Board::Inputs);

  for (int i = 0; i < maxcnt; i++) {
    if (rhs.potConfig[i].type == (unsigned int)Board::AIT_FLEX && rhs.potConfig[i].flexType != (unsigned int)Board::FLEX_NONE) {
      std::string tag = rhs.potConfig[i].tag;
      node[tag] = rhs.potConfig[i];
    }
  }

  return node;
}

bool convert<YamlPotConfig>::decode(const Node& node, YamlPotConfig& rhs)
{
  if (!node.IsMap()) return false;

  Board::Type board = getCurrentBoard();
  const int maxcnt = Boards::getCapability(board, Board::Inputs);

  for (const auto& kv : node) {
    std::string tag;
    kv.first >> tag;

    if (radioSettingsVersion < SemanticVersion(QString(CPN_ADC_REFACTOR_VERSION)))
      tag = getCurrentFirmware()->getLegacyAnalogsName(tag.c_str());

    int idx = Boards::getInputIndex(board, tag.c_str());

    if (idx >= 0 && idx < maxcnt) {
      kv.second >> rhs.potConfig[idx];
      rhs.potConfig[idx].tag = tag;
      rhs.potConfig[idx].type = Board::AIT_FLEX;
    }
  }

  return true;
}

Node convert<YamlSliderConfig>::encode(const YamlSliderConfig& rhs)
{
  //  legacy function should not be called
  Node node;
  return node;
}

bool convert<YamlSliderConfig>::decode(const Node& node, YamlSliderConfig& rhs)
{
  if (!node.IsMap()) return false;

  Board::Type board = getCurrentBoard();
  const int maxcnt = Boards::getCapability(board, Board::Inputs);

  if (Boards::getCapability(board, Board::Sliders) < 1)
    return true;

  int i = 1;

  for (const auto& kv : node) {
    std::string tag;
    kv.first >> tag;
    int idx = Boards::getInputSliderIndex(board, i);

    if (idx >= 0 && idx < maxcnt) {
      kv.second >> rhs.sliderConfig[idx];
      rhs.sliderConfig[idx].tag = Boards::getInputName(board, idx).toStdString();
      rhs.sliderConfig[idx].type = Board::AIT_FLEX;
    }

    i++;
  }

  return true;
}

Node convert<YamlStickConfig>::encode(const YamlStickConfig& rhs)
{
  Node node;
  Board::Type board = getCurrentBoard();
  const int maxcnt = Boards::getCapability(board, Board::Inputs);

  for (int i = 0; i < maxcnt; i++) {
    if (rhs.stickConfig[i].type == (unsigned int)Board::AIT_STICK) {
      std::string tag = rhs.stickConfig[i].tag;
      node[tag] = rhs.stickConfig[i];
    }
  }

  return node;
}

bool convert<YamlStickConfig>::decode(const Node& node, YamlStickConfig& rhs)
{
  if (!node.IsMap()) return false;

  Board::Type board = getCurrentBoard();
  const int maxcnt = Boards::getCapability(board, Board::Inputs);

  for (const auto& kv : node) {
    std::string tag;
    kv.first >> tag;

    if (!tag.empty()) {
      int idx = std::stoi(tag);

      if (idx >= 0 && idx < maxcnt) {
        kv.second >> rhs.stickConfig[idx];
        rhs.stickConfig[idx].tag = tag;
        rhs.stickConfig[idx].type = Board::AIT_STICK;
      }
    }
  }

  return true;
}

Node convert<YamlSwitchConfig>::encode(const YamlSwitchConfig& rhs)
{
  Node node;
  Board::Type board = getCurrentBoard();
  const int maxcnt = Boards::getCapability(board, Board::Switches);

  for (int i = 0; i < maxcnt; i++) {
    if (rhs.switchConfig[i].type != Board::SWITCH_NOT_AVAILABLE) {
      std::string tag = Boards::getSwitchName(board, i).toStdString();
      node[tag] = rhs.switchConfig[i];
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
    int idx = Boards::getSwitchIndex(board, tag.c_str());

    if (idx >= 0 && idx < maxcnt) {
      kv.second >> rhs.switchConfig[idx];
      rhs.switchConfig[idx].tag = tag;
    }
  }

  return true;
}

}  // namespace YAML
