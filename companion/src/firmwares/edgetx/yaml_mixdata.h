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
#include "mixdata.h"

int32_t YamlSourceNumRefDecode(const YAML::Node& node);
std::string YamlSourceNumRefEncode(int32_t sval);

uint32_t YamlReadFlightModes(const YAML::Node& node);
std::string YamlWriteFlightModes(uint32_t val);


namespace YAML
{
template <>
struct convert<CurveReference> {
  static Node encode(const CurveReference& rhs);
  static bool decode(const Node& node, CurveReference& rhs);
};

template <>
struct convert<MixData> {
  static Node encode(const MixData& rhs);
  static bool decode(const Node& node, MixData& rhs);
};
}  // namespace YAML
