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
#include "customisation_data.h"

namespace YAML
{
template <>
struct convert<ZonePersistentData> {
  static Node encode(const ZonePersistentData& rhs);
  static bool decode(const Node& node, ZonePersistentData& rhs);
};

template <>
struct convert<ZoneOptionValueTyped> {
  static Node encode(const ZoneOptionValueTyped& rhs);
  static bool decode(const Node& node, ZoneOptionValueTyped& rhs);
};

template <int N, int O>
struct convert<WidgetsContainerPersistentData<N,O> > {
  typedef WidgetsContainerPersistentData<N,O> PersistentData;
  static Node encode(const PersistentData& rhs)
  {
    Node node;
    for (int i=0; i<N; i++) {
      if (strlen(rhs.zones[i].widgetName) > 0) {
        node["zones"][std::to_string(i)] = rhs.zones[i];
      }
    }
    for(int i=0; i<O; i++) {
      if (!rhs.options[i].isEmpty()) {
        node["options"][std::to_string(i)] = rhs.options[i];
      }
    }
    return node;
  }

  static bool decode(const Node& node, PersistentData& rhs)
  {
    node["zones"] >> rhs.zones;
    node["options"] >> rhs.options;
    return true;
  }
};

typedef RadioLayout::CustomScreenData ScreenData;

template <>
struct convert<ScreenData> {
  static Node encode(const ScreenData& rhs);
  static bool decode(const Node& node, ScreenData& rhs);
};
}  // namespace YAML
