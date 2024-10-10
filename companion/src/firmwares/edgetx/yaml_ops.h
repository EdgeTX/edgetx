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

#include <yaml-cpp/yaml.h>
#include "helpers.h"
#include "boards.h"

#include <algorithm>
#include <QString>

typedef std::pair<int, std::string> YamlLookupTableElmt;
typedef std::vector<YamlLookupTableElmt> YamlLookupTable;

class ioffset_int {

public:
  ioffset_int(int& val, int offset)
    : value(val), offset(offset)
  {}

  void assign(int newValue) const {
    value = newValue - offset;
  }

private:
  int& value;
  int offset;
};

template<typename T>
class ifactor {

public:
  ifactor(T& val, int factor)
    : value(val), factor(factor)
  {}

  void assign(T newValue) const {
    value = newValue / factor;
  }

private:
  T& value;
  T factor;
};

YAML::Node operator >> (const YAML::Node& node, const YamlLookupTable& lut);
void operator >> (const YAML::Node& node, const ioffset_int& value);

template<typename T>
void operator >> (const YAML::Node& node, const ifactor<T>& value)
{
  if (node && node.IsScalar()) {
    value.assign(node.as<T>());
  }
}

template <typename T>
void operator >> (const YAML::Node& node, T& value)
{
  if (node && !node.IsNull()) {
    value = node.as<T>();
  }
}

// type specialisation
template <> void operator >> (const YAML::Node& node, bool& value);

template <size_t N>
void operator >> (const YAML::Node& node, char (&value)[N])
{
  if (node && node.IsScalar()) {
    auto str = node.as<std::string>();
    str.copy(value, N-1);
    value[std::min(str.size(), N-1)] = '\0';
  }
}

template <typename T, size_t N>
void operator>>(const YAML::Node& node, T (&value)[N])
{
  if (!node) return;
  if (node.IsMap()) {
    for (const auto& elmt : node) {
      int idx = -1;
      try {
        idx = std::stoi(elmt.first.Scalar());
      } catch (...) {
      }
      if (idx < 0 || idx >= (int)N) continue;

      if (elmt.second.IsMap() && (elmt.second.size() == 1) &&
          elmt.second["val"]) {
        elmt.second["val"] >> value[idx];
      } else {
        elmt.second >> value[idx];
      }
    }
  } else if (node.IsSequence()) {
    int idx = 0;
    for (const auto& elmt : node) {
      elmt >> value[idx];
      idx++;
    }
  }
}

void YamlValidateName(char *input, Board::Type board);
void YamlValidateLabel(QString &input);

namespace YAML {

std::string LookupValue(const YamlLookupTable& lut, const int& value);
Node operator << (const YamlLookupTable& lut, const int& value);

template <typename T, size_t N>
Node encode_array(const T (&rhs)[N])
{
  Node node;
  for (size_t i = 0; i < N; i++) {
    Node elmt;
    elmt = rhs[i];
    node.push_back(elmt);
  }
  return node;
}

template <typename T>
struct convert_enum
{
  static YAML::Node encode(const YamlLookupTable& lut, const T& rhs)
  {
    return lut << rhs;
  }
  static bool decode(const YAML::Node& node, const YamlLookupTable& lut, T& rhs)
  {
    if (node) {
      YAML::Node conv = node >> lut;
      if (conv.IsScalar()) {
        rhs = (T)conv.as<int>();
      }
    }
    return true;
  }
};

}

#define ENUM_CONVERTER(enum_type, lut)                         \
  template <>                                                  \
  struct convert<enum_type> {                                  \
    static YAML::Node encode(const enum_type& rhs)             \
    {                                                          \
      return convert_enum<enum_type>::encode(lut,rhs);       \
    }                                                          \
    static bool decode(const YAML::Node& node, enum_type& rhs) \
    {                                                          \
      return convert_enum<enum_type>::decode(node, lut, rhs);   \
    }                                                          \
  }

extern SemanticVersion radioSettingsVersion;
extern SemanticVersion modelSettingsVersion;
