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

#include "yaml_screendata.h"
#include "yaml_rawsource.h"
#include "yaml_ops.h"

#include <sstream>
#include <iomanip>

static const YamlLookupTable zoneOptionValueEnum = {
    {ZOV_Unsigned, "Unsigned"}, {ZOV_Signed, "Signed"}, {ZOV_Bool, "Bool"},
    {ZOV_String, "String"},     {ZOV_Source, "Source"}, {ZOV_Color, "Color"},
};

std::string color_to_hex( unsigned int i )
{
  std::stringstream stream;
  stream << "0x" 
         << std::setfill ('0') << std::setw(6)
         << std::hex << i;
  return stream.str();
}

namespace YAML
{
ENUM_CONVERTER(ZoneOptionValueEnum, zoneOptionValueEnum);

Node convert<ZoneOptionValueTyped>::encode(const ZoneOptionValueTyped& rhs)
{
  Node node, value;
  node["type"] = rhs.type;

  switch (rhs.type) {
    case ZOV_Unsigned:
      value["unsignedValue"] = rhs.value.unsignedValue;
      break;
    case ZOV_Signed:
      value["signedValue"] = rhs.value.signedValue;
      break;
    case ZOV_Bool:
      value["boolValue"] = rhs.value.boolValue;
      break;
    case ZOV_String:
      value["stringValue"] = std::string(rhs.value.stringValue);
      break;
    case ZOV_Source:
      value["source"] = rhs.value.sourceValue;
      break;
    case ZOV_Color:
      if (rhs.value.colorValue & 0x80000000) {
        value["color"] = color_to_hex(rhs.value.colorValue & 0xFFFFFF);
      } else {
        std::string s("COLIDX");
        s += std::to_string(rhs.value.colorValue);
        value["color"] = s;
      }
      break;
    default:
      break;
  }
  node["value"] = value;
  return node;
}

bool convert<ZoneOptionValueTyped>::decode(const Node& node,
                                           ZoneOptionValueTyped& rhs)
{
  node["type"] >> rhs.type;
  if (node["value"]) {
    Node value = node["value"];
    if (value.IsMap()) {
      value["unsignedValue"] >> rhs.value.unsignedValue;
      value["signedValue"] >> rhs.value.signedValue;
      value["boolValue"] >> rhs.value.boolValue;
      value["stringValue"] >> rhs.value.stringValue;
      value["source"] >> rhs.value.sourceValue;
      std::string s;
      value["color"] >> s;
      if (s.substr(0, 6) == "COLIDX") {
        // Index color
        rhs.value.colorValue = stoi(s.substr(6));
      } else {
        unsigned int c;
        sscanf(s.c_str(), "0x%x", &c);
        // Mark as RGB color
        rhs.value.colorValue = c | 0x80000000;
      }
    }
  }
  return true;
}

template <>
struct convert<WidgetPersistentData> {
  static Node encode(const WidgetPersistentData& rhs)
  {
    Node node;
    for (int i=0; i<MAX_WIDGET_OPTIONS; i++) {
      if (!rhs.options[i].isEmpty()) {
        Node options;
        options = rhs.options[i];
        if (options && options.IsMap()) {
          node["options"][std::to_string(i)] = options;
        }
      }
    }
    return node;
  }

  static bool decode(const Node& node, WidgetPersistentData& rhs)
  {
    node["options"] >> rhs.options;
    return true;
  }
};

Node convert<ZonePersistentData>::encode(const ZonePersistentData& rhs)
{
  Node node;
  node["widgetName"] = rhs.widgetName;
  node["widgetData"] = rhs.widgetData;
  return node;
}

bool convert<ZonePersistentData>::decode(const Node& node, ZonePersistentData& rhs)
{
  node["widgetName"] >> rhs.widgetName;
  node["widgetData"] >> rhs.widgetData;
  return true;
}

Node convert<ScreenData>::encode(const ScreenData& rhs)
{
  Node node;
  node["LayoutId"] = rhs.layoutId;
  node["layoutData"] = rhs.layoutPersistentData;
  return node;
}

bool convert<ScreenData>::decode(const Node& node, ScreenData& rhs)
{
  node["LayoutId"] >> rhs.layoutId;
  node["layoutData"] >> rhs.layoutPersistentData;
  return true;
}
}  // namespace YAML
