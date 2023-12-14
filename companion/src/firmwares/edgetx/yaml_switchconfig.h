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

#pragma once

#include "yaml_ops.h"
#include "generalsettings.h"
#include <string.h>

struct InputConfig {
  std::string tag = std::string();
  unsigned int type = 0;
  char name[HARDWARE_NAME_LEN + 1] = {'\0'};
  unsigned int flexType = 0;
  bool inverted = false;
};

struct YamlPotConfig {
  InputConfig potConfig[CPN_MAX_INPUTS];

  YamlPotConfig() = default;
  YamlPotConfig(const GeneralSettings::InputConfig* rhs);
  void copy(GeneralSettings::InputConfig* rhs) const;
};

struct YamlSliderConfig {
  InputConfig sliderConfig[CPN_MAX_INPUTS];

  YamlSliderConfig() = default;
  void copy(GeneralSettings::InputConfig* rhs) const;
};

struct YamlStickConfig {
  InputConfig stickConfig[CPN_MAX_INPUTS];

  YamlStickConfig() = default;
  YamlStickConfig(const GeneralSettings::InputConfig* rhs);
  void copy(GeneralSettings::InputConfig* rhs) const;
};

struct SwitchConfig {
  std::string tag = std::string();
  char name[HARDWARE_NAME_LEN + 1] = {'\0'};
  unsigned int type = 0;
  bool inverted = false;
};

struct YamlSwitchConfig {
  SwitchConfig switchConfig[CPN_MAX_SWITCHES];

  YamlSwitchConfig() = default;
  YamlSwitchConfig(const GeneralSettings::SwitchConfig* rhs);
  void copy(GeneralSettings::SwitchConfig* rhs) const;
};

namespace YAML
{
template <>
struct convert<InputConfig> {
  static Node encode(const InputConfig& rhs);
  static bool decode(const Node& node, InputConfig& rhs);
};

template <>
struct convert<SwitchConfig> {
  static Node encode(const SwitchConfig& rhs);
  static bool decode(const Node& node, SwitchConfig& rhs);
};

template <>
struct convert<YamlPotConfig> {
  static Node encode(const YamlPotConfig& rhs);
  static bool decode(const Node& node, YamlPotConfig& rhs);
};

template <>
struct convert<YamlSliderConfig> {
  static bool decode(const Node& node, YamlSliderConfig& rhs);
};

template <>
struct convert<YamlStickConfig> {
  static Node encode(const YamlStickConfig& rhs);
  static bool decode(const Node& node, YamlStickConfig& rhs);
};

template <>
struct convert<YamlSwitchConfig> {
  static Node encode(const YamlSwitchConfig& rhs);
  static bool decode(const Node& node, YamlSwitchConfig& rhs);
};

}  // namespace YAML
