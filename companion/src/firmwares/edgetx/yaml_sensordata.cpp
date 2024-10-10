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

#include "yaml_sensordata.h"

static const YamlLookupTable sensorType = {
    {SensorData::TELEM_TYPE_CUSTOM, "TYPE_CUSTOM"},
    {SensorData::TELEM_TYPE_CALCULATED, "TYPE_CALCULATED"},
};

static const YamlLookupTable sensorFormula = {
    {SensorData::TELEM_FORMULA_ADD, "FORMULA_ADD"},
    {SensorData::TELEM_FORMULA_AVERAGE, "FORMULA_AVERAGE"},
    {SensorData::TELEM_FORMULA_MIN, "FORMULA_MIN"},
    {SensorData::TELEM_FORMULA_MAX, "FORMULA_MAX"},
    {SensorData::TELEM_FORMULA_MULTIPLY, "FORMULA_MULTIPLY"},
    {SensorData::TELEM_FORMULA_TOTALIZE, "FORMULA_TOTALIZE"},
    {SensorData::TELEM_FORMULA_CELL, "FORMULA_CELL"},
    {SensorData::TELEM_FORMULA_CONSUMPTION, "FORMULA_CONSUMPTION"},
    {SensorData::TELEM_FORMULA_DIST, "FORMULA_DIST"},
};

namespace YAML
{
Node convert<SensorData>::encode(const SensorData& rhs)
{
  Node node;
  node["type"] = LookupValue(sensorType, rhs.type);

  Node cfg;
  if (rhs.type == SensorData::TELEM_TYPE_CUSTOM) {
    node["id1"]["id"] = rhs.id;
    node["subId"] = rhs.subid;

    node["id2"]["instance"] = rhs.instance;
    cfg["custom"]["ratio"] = rhs.ratio;
    cfg["custom"]["offset"] = rhs.offset;
  } else {
    if (rhs.persistent) {
      node["persistent"] = 1;
      node["id1"]["persistentValue"] = rhs.persistentValue;
    }

    node["id2"]["formula"] = LookupValue(sensorFormula, rhs.formula);

    switch (rhs.formula) {
      case SensorData::TELEM_FORMULA_CELL:
        cfg["cell"]["source"] = rhs.source;
        cfg["cell"]["index"] = rhs.index;
        break;
      case SensorData::TELEM_FORMULA_ADD:
      case SensorData::TELEM_FORMULA_AVERAGE:
      case SensorData::TELEM_FORMULA_MULTIPLY:
      case SensorData::TELEM_FORMULA_MIN:
      case SensorData::TELEM_FORMULA_MAX: {
        Node sources;
        for (int i = 0; i < 4; i++) {
          if (rhs.sources[i]) {
            sources[std::to_string(i)]["val"] = rhs.sources[i];
          }
        }
        cfg["calc"]["sources"] = sources;
      } break;
      case SensorData::TELEM_FORMULA_DIST:
        if (rhs.gps) {
          cfg["dist"]["gps"] = rhs.gps;
        }
        if (rhs.alt) {
          cfg["dist"]["alt"] = rhs.alt;
        }
        break;
      case SensorData::TELEM_FORMULA_CONSUMPTION:
      case SensorData::TELEM_FORMULA_TOTALIZE:
        cfg["consumption"]["source"] = rhs.amps;
      default:
        break;
    }
  }

  node["label"] = rhs.label;
  node["unit"] = rhs.unit;
  node["prec"] = rhs.prec;
  node["autoOffset"] = (int)rhs.autoOffset;
  node["filter"] = (int)rhs.filter;
  node["logs"] = (int)rhs.logs;
  node["persistent"] = (int)rhs.persistent;
  node["onlyPositive"] = (int)rhs.onlyPositive;

  if (cfg && cfg.IsMap()) {
    node["cfg"] = cfg;
  }
  return node;
}

bool convert<SensorData>::decode(const Node& node, SensorData& rhs)
{
  node["type"] >> sensorType >> rhs.type;
  if (node["id1"]) {
    Node id1 = node["id1"];
    id1["id"] >> rhs.id;
    id1["persistentValue"] >> rhs.persistentValue;
  }
  node["subId"] >> rhs.subid;
  if (node["id2"]) {
    Node id2 = node["id2"];
    id2["instance"] >> rhs.instance;
    id2["formula"] >> sensorFormula >> rhs.formula;
  }

  if (rhs.type == SensorData::TELEM_TYPE_CUSTOM) {
    rhs.rxIdx = (rhs.instance >> 5) & 0x03;     // 2 bits Rx idx
    rhs.moduleIdx = (rhs.instance >> 7) & 0x1;  // 1 bit module idx
  }

  node["label"] >> rhs.label;
  node["unit"] >> rhs.unit;
  node["prec"] >> rhs.prec;
  node["autoOffset"] >> rhs.autoOffset;
  node["filter"] >> rhs.filter;
  node["logs"] >> rhs.logs;
  node["persistent"] >> rhs.persistent;
  node["onlyPositive"] >> rhs.onlyPositive;

  if (node["cfg"]) {
    Node cfg = node["cfg"];
    if (cfg.IsMap()) {
      if (cfg["custom"]) {
        Node custom = cfg["custom"];
        if (custom.IsMap()) {
          custom["ratio"] >> rhs.ratio;
          custom["offset"] >> rhs.offset;
        }
      } else if (cfg["cell"]) {
        Node cell = cfg["cell"];
        if (cell.IsMap()) {
          cell["source"] >> rhs.source;
          cell["index"] >> rhs.index;
        }
      } else if (cfg["calc"]) {
        Node calc = cfg["calc"];
        if (calc.IsMap()) {
          calc["sources"] >> rhs.sources;
        }
      } else if (cfg["consumption"]) {
        Node consumption = cfg["consumption"];
        if (consumption.IsMap()) {
          consumption["source"] >> rhs.amps;
        }
      } else if (cfg["dist"]) {
        Node dist = cfg["dist"];
        if (dist.IsMap()) {
          dist["gps"] >> rhs.gps;
          dist["alt"] >> rhs.alt;
        }
      }
    }
  }

  return true;
}
}  // namespace YAML
