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

#pragma once

#include "yaml_ops.h"
#include "generalsettings.h"
#include <string.h>

class GeneralSettings;

#define ENCODE_DECODE_CONFIG(cfgstruct)                     \
  namespace YAML                                            \
  {                                                         \
  template <>                                               \
  struct convert<cfgstruct> {                               \
    static Node encode(const cfgstruct& rhs);               \
    static bool decode(const Node& node, cfgstruct& rhs);   \
  };                                                        \
  }

#define INPUT_SWITCH_CONFIG(name, cfgsize, cfgstruct, gsstruct) \
  struct name {                                                 \
    cfgstruct config[cfgsize];                                  \
                                                                \
    name() = default;                                           \
    name(const GeneralSettings::gsstruct* rhs);                 \
    void copy(GeneralSettings::gsstruct* rhs) const;            \
  };                                                            \
                                                                \
  namespace YAML                                                \
  {                                                             \
  template <>                                                   \
  struct convert<name> {                                        \
    static Node encode(const name& rhs);                        \
    static bool decode(const Node& node, name& rhs);            \
  };                                                            \
  }

struct InputConfig {
  std::string tag = std::string();
  unsigned int type = 0;
  char name[HARDWARE_NAME_LEN + 1] = {'\0'};
  unsigned int flexType = 0;
  bool inverted = false;
};

ENCODE_DECODE_CONFIG(InputConfig)

struct SwitchConfig {
  std::string tag = std::string();
  char name[HARDWARE_NAME_LEN + 1] = {'\0'};
  unsigned int type = 0;
  bool inverted = false;
};

ENCODE_DECODE_CONFIG(SwitchConfig)

struct SwitchFlex {
  std::string tag = std::string();
  std::string channel = std::string();
  int inputIndx = SWITCH_INPUTINDEX_NONE;
};

ENCODE_DECODE_CONFIG(SwitchFlex)

INPUT_SWITCH_CONFIG(YamlPotConfig, CPN_MAX_INPUTS, InputConfig, InputConfig)
INPUT_SWITCH_CONFIG(YamlSliderConfig, CPN_MAX_INPUTS, InputConfig, InputConfig)
INPUT_SWITCH_CONFIG(YamlStickConfig, CPN_MAX_INPUTS, InputConfig, InputConfig)
INPUT_SWITCH_CONFIG(YamlSwitchConfig, CPN_MAX_SWITCHES, SwitchConfig, SwitchConfig)
INPUT_SWITCH_CONFIG(YamlSwitchesFlex, CPN_MAX_SWITCHES_FLEX, SwitchFlex, SwitchConfig)
