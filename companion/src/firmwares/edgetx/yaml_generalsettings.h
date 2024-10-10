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
#include "generalsettings.h"

struct YamlTelemetryBaudrate {
  unsigned int value;

  YamlTelemetryBaudrate() = default;
  YamlTelemetryBaudrate(const unsigned int* moduleBaudrate);
  void toCpn(unsigned int* moduleBaudrate, unsigned int variant);
};

namespace YAML {

  template<>
  struct convert<GeneralSettings> {

    static Node encode(const GeneralSettings& rhs);
    static bool decode(const Node& node, GeneralSettings& rhs);
  };
}
