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

#include "yaml_ops.h"
#include "generalsettings.h"

struct CalibData {
  int16_t mid;
  int16_t spanNeg;
  int16_t spanPos;
};

struct YamlCalibData {
  CalibData calib[CPN_MAX_ANALOGS];

  YamlCalibData();
  YamlCalibData(const int* calibMid, const int* calibSpanNeg, const int* calibSpanPos);
  void copy(int* calibMid, int* calibSpanNeg, int* calibSpanPos) const;
};

namespace YAML
{
template <>
struct convert<CalibData> {
  static Node encode(const CalibData& rhs);
  static bool decode(const Node& node, CalibData& rhs);
};

template <>
struct convert<YamlCalibData> {
  static Node encode(const YamlCalibData& rhs);
  static bool decode(const Node& node, YamlCalibData& rhs);
};
}  // namespace YAML
