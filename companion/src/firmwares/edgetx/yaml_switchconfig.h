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

template <unsigned int N, const YamlLookupTable& name_lut>
struct YamlNameConfig {
  char name[N][HARDWARE_NAME_LEN + 1];

  YamlNameConfig() { memset(name, 0, sizeof(name)); }

  YamlNameConfig(const char in_names[N][HARDWARE_NAME_LEN + 1])
  {
    memcpy(name, in_names, sizeof(name));
  }

  void copy(char out_names[N][HARDWARE_NAME_LEN + 1])
  {
    memcpy(out_names, name, sizeof(name));
  }
};

extern const YamlLookupTable sticksLut;
typedef YamlNameConfig<CPN_MAX_SWITCHES, sticksLut> YamlStickConfig;

template <unsigned int N, const YamlLookupTable& name_lut,
          const YamlLookupTable& cfg_lut>
struct YamlKnobConfig : public YamlNameConfig<N, name_lut> {
  unsigned int config[N];

  YamlKnobConfig() { memset(config, 0, sizeof(config)); }

  YamlKnobConfig(const char in_names[N][HARDWARE_NAME_LEN + 1],
                 const unsigned int in_configs[N]) :
      YamlNameConfig<N, name_lut>(in_names)
  {
    memset(config, 0, sizeof(config));
  }

  void copy(char out_names[N][HARDWARE_NAME_LEN + 1],
            unsigned int out_configs[N])
  {
    YamlNameConfig<N, name_lut>::copy(out_names);
    memcpy(out_configs, config, sizeof(config));
  }
};

// SA:
//    type: 3pos
//    name:
extern const YamlLookupTable switchesLut;
extern const YamlLookupTable switchConfigLut;
typedef YamlKnobConfig<CPN_MAX_SWITCHES, switchesLut, switchConfigLut>
    YamlSwitchConfig;

// S1:
//    type: with_detent
//    name:
extern const YamlLookupTable potsLut;
extern const YamlLookupTable potConfigLut;
typedef YamlKnobConfig<CPN_MAX_KNOBS, potsLut, potConfigLut> YamlPotConfig;

// S1:
//    type: with_detent
//    name:
extern const YamlLookupTable slidersLut;
extern const YamlLookupTable sliderConfigLut;
typedef YamlKnobConfig<CPN_MAX_SLIDERS, slidersLut, sliderConfigLut>
    YamlSliderConfig;

namespace YAML
{
template <unsigned int N, const YamlLookupTable& name_lut>
struct convert<YamlNameConfig<N, name_lut> > {
  static Node encode(const YamlNameConfig<N, name_lut>& rhs)
  {
    Node node;
    for (const auto& kv : name_lut) {
      node[kv.second]["name"] = rhs.name[kv.first];
    }
    return node;
  }
  static bool decode(const Node& node, YamlNameConfig<N, name_lut>& rhs)
  {
    if (!node.IsMap()) return false;
    int idx = 0;
    for (const auto& kv : node) {
      kv.first >> name_lut >> idx;
      kv.second["name"] >> rhs.name[idx];
    }
    return true;
  }
};

template <unsigned int N, const YamlLookupTable& name_lut,
          const YamlLookupTable& cfg_lut>
struct convert<YamlKnobConfig<N, name_lut, cfg_lut> > {
  static Node encode(const YamlKnobConfig<N, name_lut, cfg_lut>& rhs)
  {
    Node node;
    for (const auto& kv : name_lut) {
      node[kv.second]["type"] = cfg_lut << rhs.config[kv.first];
      node[kv.second]["name"] = rhs.name[kv.first];
    }
    return node;
  }
  static bool decode(const Node& node,
                     YamlKnobConfig<N, name_lut, cfg_lut>& rhs)
  {
    if (!node.IsMap()) return false;
    int idx = 0;
    for (const auto& kv : node) {
      kv.first >> name_lut >> idx;
      kv.second["type"] >> cfg_lut >> rhs.config[idx];
      kv.second["name"] >> rhs.name[idx];
    }
    return true;
  }
};
}  // namespace YAML
