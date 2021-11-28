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

#include <yaml-cpp/yaml.h>

typedef std::pair<int, std::string> YamlLookupTableElmt;
typedef std::vector<YamlLookupTableElmt> YamlLookupTable;

YAML::Node operator>>(const YAML::Node& node, const YamlLookupTable& lut)
{
  if (node) {
    std::string str = node.as<std::string>();
    const auto& it =
        find_if(lut.begin(), lut.end(), [=](const YamlLookupTableElmt& elmt) {
          if (elmt.second == str) return true;
          return false;
        });

    if (it != lut.end()) {
      return YAML::Node(it->first);
    }  
    return YAML::Node();
  }

  return node;
}

template <typename T>
void operator>>(const YAML::Node& node, T& value)
{
  if (node && !node.IsNull()) {
    value = node.as<T>();
  }
}

template <>
void operator>>(const YAML::Node& node, bool& value)
{
  if (node && node.IsScalar()) {
    value = node.as<int>();
  }
}

template <size_t N>
void operator>>(const YAML::Node& node, char (&value)[N])
{
  if (node && node.IsScalar()) {
    auto str = node.as<std::string>();
    str.copy(value, N-1);
    value[std::min(str.size(), N-1)] = '\0';
  }
}

#define ENUM_CONVERTER(enum_type, lut)                         \
  template <>                                                  \
  struct convert<enum_type> {                                  \
    static bool decode(const YAML::Node& node, enum_type& rhs) \
    {                                                          \
      if (node) {                                              \
        YAML::Node conv = node >> lut;                         \
        if (conv.IsScalar()) {                                 \
          rhs = (enum_type)conv.as<int>();                     \
        }                                                      \
      }                                                        \
      return true;                                             \
    }                                                          \
  }
