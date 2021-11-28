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

namespace YAML {

  template<>
  struct convert<TrainerMix> {

    // static YAML::Node encode(const TrainerMix& rhs) {
    //       Node node;
    //       // TODO: convert struct into Node
    //       return node;
    // }

    static bool decode(const YAML::Node& node, TrainerMix& rhs)
    {
      node["srcChn"] >> rhs.src;
      //node["mode"] >> rhs.mode; // TODO: string mapping
      node["studWeight"] >> rhs.weight;
      return true;
    }
  };

  template<>
  struct convert<TrainerData> {

    // static YAML::Node encode(const TrainerData& rhs) {
    //       Node node;
    //       // TODO: convert struct into Node
    //       return node;
    // }

    static bool decode(const YAML::Node& node, TrainerData& rhs)
    {
      if (node["calib"]) {
        const YAML::Node& calib = node["calib"];
        for (int i = 0; i < 4; i++) {
          calib[i] >> rhs.calib[i];
        }
      }

      if (node["mix"]) {
        const YAML::Node& mix = node["mix"];
        for (int i = 0; i < 4; i++) {
          mix[i] >> rhs.mix[i];
        }
      }
      return true;
    }
  };
}
