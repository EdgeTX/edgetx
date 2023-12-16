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

#include "yaml_calibdata.h"
#include "eeprominterface.h"

YamlCalibData::YamlCalibData()
{
  memset(calib, 0, sizeof(calib));
}

YamlCalibData::YamlCalibData(const GeneralSettings::InputConfig* rhs)
{
  Board::Type board = getCurrentBoard();

  for (int i = 0; i < Boards::getCapability(board, Board::Inputs); i++) {
    if (Boards::isInputCalibrated(board, i)) {
      calib[i].mid = rhs[i].calib.mid;
      calib[i].spanNeg = rhs[i].calib.spanNeg;
      calib[i].spanPos = rhs[i].calib.spanPos;
    }
  }
}

void YamlCalibData::copy(GeneralSettings::InputConfig* rhs) const
{
  Board::Type board = getCurrentBoard();

  for (int i = 0; i < Boards::getCapability(board, Board::Inputs); i++) {
    if (Boards::isInputCalibrated(board, i)) {
      rhs[i].calib.mid = calib[i].mid;
      rhs[i].calib.spanNeg = calib[i].spanNeg;
      rhs[i].calib.spanPos = calib[i].spanPos;
    }
  }
}

namespace YAML
{

Node convert<GeneralSettings::InputCalib>::encode(const GeneralSettings::InputCalib& rhs)
{
  Node node;
  node["mid"] = rhs.mid;
  node["spanNeg"] = rhs.spanNeg;
  node["spanPos"] = rhs.spanPos;
  return node;
}

bool convert<GeneralSettings::InputCalib>::decode(const Node& node, GeneralSettings::InputCalib& rhs)
{
  if (!node.IsMap()) return false;
  node["mid"] >> rhs.mid;
  node["spanNeg"] >> rhs.spanNeg;
  node["spanPos"] >> rhs.spanPos;
  return true;
}

Node convert<YamlCalibData>::encode(const YamlCalibData& rhs)
{
  Node node;
  Board::Type board = getCurrentBoard();
  const int analogs = Boards::getCapability(board, Board::Inputs);

  for (int i = 0; i < analogs; i++) {
    if (Boards::isInputCalibrated(board, i)) {
      std::string tag = Boards::getInputTag(board, i).toStdString();
      node[tag] = rhs.calib[i];
    }
  }

  return node;
}

bool convert<YamlCalibData>::decode(const Node& node, YamlCalibData& rhs)
{
  if (!node.IsMap()) return false;

  Board::Type board = getCurrentBoard();

  for (const auto& kv : node) {
    std::string tag;
    kv.first >> tag;

    if (radioSettingsVersion < SemanticVersion(QString(CPN_ADC_REFACTOR_VERSION)))
      tag = Boards::getLegacyAnalogMappedInputTag(board, tag.c_str());

    int idx = Boards::getInputIndex(board, tag.c_str());

    if (idx >= 0)
      kv.second >> rhs.calib[idx];
  }

  return true;
}

}  // namespace YAML
