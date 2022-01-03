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

struct YamlStickLookup {
  static int name2idx(const std::string& name);
  static std::string idx2name(unsigned int idx);
};

template <unsigned int N, class name_lookup>
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

typedef YamlNameConfig<CPN_MAX_SWITCHES, YamlStickLookup> YamlStickConfig;

template <unsigned int N, class name_lookup, const YamlLookupTable& cfg_lut>
struct YamlKnobConfig : public YamlNameConfig<N, name_lookup> {
  unsigned int config[N];

  YamlKnobConfig() { memset(config, 0, sizeof(config)); }

  YamlKnobConfig(const char in_names[N][HARDWARE_NAME_LEN + 1],
                 const unsigned int in_configs[N]) :
      YamlNameConfig<N, name_lookup>(in_names)
  {
    memcpy(config, in_configs, sizeof(config));
  }

  void copy(char out_names[N][HARDWARE_NAME_LEN + 1],
            unsigned int out_configs[N])
  {
    YamlNameConfig<N, name_lookup>::copy(out_names);
    memcpy(out_configs, config, sizeof(config));
  }
};

// SA:
//    type: 3pos
//    name:
struct YamlSwitchLookup {
  static int name2idx(const std::string& name);
  static std::string idx2name(unsigned int idx);
};
extern const YamlLookupTable switchConfigLut;
typedef YamlKnobConfig<CPN_MAX_SWITCHES, YamlSwitchLookup, switchConfigLut>
    YamlSwitchConfig;

// S1:
//    type: with_detent
//    name:
struct YamlPotLookup {
  static int name2idx(const std::string& name);
  static std::string idx2name(unsigned int idx);
};
extern const YamlLookupTable potConfigLut;
typedef YamlKnobConfig<CPN_MAX_POTS, YamlPotLookup, potConfigLut> YamlPotConfig;

// S1:
//    type: with_detent
//    name:
struct YamlSliderLookup {
  static int name2idx(const std::string& name);
  static std::string idx2name(unsigned int idx);
};
extern const YamlLookupTable sliderConfigLut;
typedef YamlKnobConfig<CPN_MAX_SLIDERS, YamlSliderLookup, sliderConfigLut>
    YamlSliderConfig;

namespace YAML
{
template <unsigned int N, class name_lookup>
struct convert<YamlNameConfig<N, name_lookup> > {
  static Node encode(const YamlNameConfig<N, name_lookup>& rhs)
  {
    Node node;
    for (unsigned int i=0; i<N; i++) {
      std::string name = name_lookup::idx2name(i);
      if (!name.empty() && rhs.name[i][0]) {
        node[name]["name"] = rhs.name[i];
      }
    }
    return node;
  }
  static bool decode(const Node& node, YamlNameConfig<N, name_lookup>& rhs)
  {
    if (!node.IsMap()) return false;
    int idx = 0;
    for (const auto& kv : node) {
      idx = name_lookup::name2idx(kv.first.Scalar());
      if (idx >= 0) {
        kv.second["name"] >> rhs.name[idx];
      }
    }
    return true;
  }
};

template <unsigned int N, class name_lookup, const YamlLookupTable& cfg_lut>
struct convert<YamlKnobConfig<N, name_lookup, cfg_lut> > {
  static Node encode(const YamlKnobConfig<N, name_lookup, cfg_lut>& rhs)
  {
    Node node;
    for (unsigned int i=0; i<N; i++) {
      std::string name = name_lookup::idx2name(i);
      if (!name.empty() && (rhs.config[i] || rhs.name[i][0])) {
        node[name]["type"] = cfg_lut << rhs.config[i];
        node[name]["name"] = rhs.name[i];
      }
    }
    return node;
  }
  static bool decode(const Node& node,
                     YamlKnobConfig<N, name_lookup, cfg_lut>& rhs)
  {
    if (!node.IsMap()) return false;
    int idx = 0;
    for (const auto& kv : node) {
      idx = name_lookup::name2idx(kv.first.Scalar());
      if (idx >= 0) {
        kv.second["type"] >> cfg_lut >> rhs.config[idx];
        kv.second["name"] >> rhs.name[idx];
      }
    }
    return true;
  }
};
}  // namespace YAML
