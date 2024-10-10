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

#include "yaml_ops.h"
#include "labelvalidator.h"
#include "namevalidator.h"

SemanticVersion radioSettingsVersion;
SemanticVersion modelSettingsVersion;

YAML::Node operator >> (const YAML::Node& node, const YamlLookupTable& lut)
{
  if (node && node.IsScalar()) {
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

void operator >> (const YAML::Node& node, const ioffset_int& value)
{
  if (node && node.IsScalar()) {
    value.assign(node.as<int>());
  }
}

template <>
void operator >> (const YAML::Node& node, bool& value)
{
  if (node && node.IsScalar()) {
    value = node.as<int>();
  }
}

void YamlValidateLabel(QString &input)
{
  LabelValidator *lv = new LabelValidator();

  if (!lv->isValid(input)) {
    lv->fixup(input);
    input = input.trimmed();
  }

  delete lv;
}

void YamlValidateName(char *input, Board::Type board)
{
  NameValidator *nv = new NameValidator(board);
  QString in(input);

  if (!nv->isValid(in)) {
    nv->fixup(in);
    in = in.trimmed();
  }

  strcpy(input, in.toLatin1().data());
  delete nv;
}

namespace YAML {

std::string LookupValue(const YamlLookupTable& lut, const int& value)
{
  const auto& it =
    find_if(lut.begin(), lut.end(), [=](const YamlLookupTableElmt& elmt) {
        if (elmt.first == value) return true;
        return false;
      });

  if (it != lut.end()) {
    return it->second;
  }

  return std::string();
}

Node operator << (const YamlLookupTable& lut, const int& value)
{
  std::string str = LookupValue(lut, value);
  if (!str.empty()) {
    return Node(str);
  }
  return Node();
}

}
