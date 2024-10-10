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

#include "yaml_trainerdata.h"
#include "generalsettings.h"

const YamlLookupTable trainerMixMode = {
    {TrainerMix::TRN_MIX_MODE_OFF, "OFF"},
    {TrainerMix::TRN_MIX_MODE_ADD, "ADD"},
    {TrainerMix::TRN_MIX_MODE_SUBST, "REPL"},
};

namespace YAML
{

Node convert<TrainerMix>::encode(const TrainerMix& rhs)
{
  Node node;
  node["srcChn"] = rhs.src;
  node["mode"] = trainerMixMode << rhs.mode;
  node["studWeight"] = rhs.weight;
  return node;
}

bool convert<TrainerMix>::decode(const Node& node, TrainerMix& rhs)
{
  node["srcChn"] >> rhs.src;
  node["mode"] >> trainerMixMode >> rhs.mode;
  node["studWeight"] >> rhs.weight;
  return true;
}

Node convert<TrainerData>::encode(const TrainerData& rhs)
{
  Node node;
  for (int i = 0; i < 4; i++) {
    node["calib"][std::to_string(i)]["val"] = rhs.calib[i];
  }
  for (int i = 0; i < 4; i++) {
    node["mix"][std::to_string(i)] = rhs.mix[i];
  }
  return node;
}

bool convert<TrainerData>::decode(const Node& node, TrainerData& rhs)
{
  node["calib"] >> rhs.calib;
  node["mix"] >> rhs.mix;
  return true;
}

}  // namespace YAML
