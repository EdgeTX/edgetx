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

#include "edgetxinterface.h"

#include "yaml_ops.h"
#include "yaml_generalsettings.h"

static YAML::Node loadYamlFromByteArray(const QByteArray& data)
{
    // TODO: use real streaming to avoid memory copies
    std::istringstream data_istream(data.toStdString());
    return YAML::Load(data_istream);
}

bool loadModelFromYaml(ModelData& model, const QByteArray& data)
{
  try {
    YAML::Node node = loadYamlFromByteArray(data);
    //model << node;
  } catch (const std::runtime_error& e) {
    qDebug() << "YAML::ParserException: " << e.what();
    return false;
  }

  return true;
}

bool loadRadioSettingsFromYaml(GeneralSettings& settings, const QByteArray& data)
{
  try {
    YAML::Node node = loadYamlFromByteArray(data);
    node >> settings;
  } catch (const std::runtime_error& e) {
    qDebug() << "YAML::ParserException: " << e.what();
    return false;
  }

  return true;
}

bool writeModelToYaml(const ModelData& model, QByteArray& data)
{
    return false;
}

bool writeRadioSettingsToYaml(const GeneralSettings& settings, QByteArray& data)
{
    return false;
}
