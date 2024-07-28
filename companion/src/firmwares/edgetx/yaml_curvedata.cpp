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

#include "yaml_curvedata.h"

// CurveHeader curves[MAX_CURVES];
// int8_t    points[MAX_CURVE_POINTS];

void YamlReadCurvePoints(const YAML::Node& node, CurveData* curves)
{
  int pidx = 0;
  for (int i = 0; i < CPN_MAX_CURVES; i++) {
    CurveData& curve = curves[i];
    for (int j = 0; j < curve.count; j++) {
      std::string pidx_str = std::to_string(pidx++);
      if (node[pidx_str]) {
        int y = 0;
        node[pidx_str]["val"] >> y;
        curve.points[j].y = (int8_t)y;
      }
    }
    if (curve.type == CurveData::CURVE_TYPE_CUSTOM) {
      // Custom curve: X points are trailing Y points
      curve.points[0].x = -100;
      for (int k = 1; k < curve.count - 1; k++) {
        std::string pidx_str = std::to_string(pidx++);
        if (node[pidx_str]) {
          int x = 0;
          node[pidx_str]["val"] >> x;
          curve.points[k].x = (int8_t)x;
        }
      }
      curve.points[curve.count - 1].x = +100;
    }
  }
}

void YamlWriteCurvePoints(YAML::Node& node, const CurveData* curves)
{
  int pidx = 0;
  for (int i = 0; i < CPN_MAX_CURVES; i++) {

    //TODO: assert max number of points
    //      and raise exception

    const CurveData& curve = curves[i];
    if (!curve.isEmpty()) {
      for (int k = 0; k < curve.count; k++) {
        node[std::to_string(pidx++)]["val"] = (int)curve.points[k].y;
      }
      if (curve.type == CurveData::CURVE_TYPE_CUSTOM) {
        for (int k = 1; k < curve.count - 1; k++) {
          node[std::to_string(pidx++)]["val"] = (int)curve.points[k].x;
        }
      }
    } else {
      pidx += 5;
    }
  }
}

namespace YAML
{
Node convert<CurveData>::encode(const CurveData& rhs)
{
  Node node;
  node["type"] = (int)rhs.type;
  node["smooth"] = (int)rhs.smooth;
  node["points"] = rhs.count - 5;
  node["name"] = rhs.name;
  return node;
}

bool convert<CurveData>::decode(const Node& node, CurveData& rhs)
{
  int curve_type = 0;
  node["type"] >> curve_type;
  rhs.type = (CurveData::CurveType)curve_type;
  node["smooth"] >> rhs.smooth;
  node["points"] >> ioffset_int(rhs.count, -5);
  node["name"] >> rhs.name;
  return true;
}
}
