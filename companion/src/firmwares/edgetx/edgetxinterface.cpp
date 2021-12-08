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
#include "yaml_modeldata.h"

#include <QMessageBox>

static YAML::Node loadYamlFromByteArray(const QByteArray& data)
{
    // TODO: use real streaming to avoid memory copies
    std::istringstream data_istream(data.toStdString());
    return YAML::Load(data_istream);
}

static void writeYamlToByteArray(const YAML::Node& node, QByteArray& data)
{
    // TODO: use real streaming to avoid memory copies
    std::stringstream data_ostream;
    data_ostream << node;
    data = QByteArray::fromStdString(data_ostream.str());

    qDebug() << "Saving YAML:";
    qDebug() << data_ostream.str().c_str();
}

bool loadModelsListFromYaml(std::vector<CategoryData>& categories,
                            EtxModelfiles& modelFiles,
                            const QByteArray& data)
{
  try {
    YAML::Node node = loadYamlFromByteArray(data);
    if (!node.IsSequence()) return false;

    for (const auto& cat : node) {

      if (!cat.IsMap()) continue;

      for (const auto& cat_map : cat) {
        categories.push_back(cat_map.first.Scalar().c_str());

        const auto& models = cat_map.second;
        if (!models.IsSequence()) continue;

        for (const auto& model : models) {
          std::string filename;
          model["filename"] >> filename;
          modelFiles.push_back({ filename, categories.size()-1 });
        }
      }
    }

  } catch (const std::runtime_error& e) {
    qDebug() << "YAML::ParserException: " << e.what();
    return false;
  }

  return true;
}

bool loadModelFromYaml(ModelData& model, const QByteArray& data)
{
  try {
    YAML::Node node = loadYamlFromByteArray(data);
    node >> model;
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
    if (settings.version < CPN_CURRENT_SETTINGS_VERSION) {
      qDebug() << "Older version of settings detected:" << settings.version;
    }
    else if (settings.version > CPN_CURRENT_SETTINGS_VERSION) {
      qDebug() << "Newer version of settings detected:" << settings.version;
      QMessageBox::critical(NULL, CPN_STR_APP_NAME, QCoreApplication::translate("EdgeTXInterface", "Companion does not support settings version %1!").arg(settings.version));
      return false;
    }
  } catch (const std::runtime_error& e) {
    qDebug() << "YAML::ParserException: " << e.what();
    return false;
  }

  return true;
}

// TODO:
//   'modelFiles' should be ordered by Category index to avoid
//   turning the sequence into a map.
//
bool writeModelsListToYaml(const std::vector<CategoryData>& categories,
                           const EtxModelfiles& modelFiles,
                           QByteArray& data)
{
  YAML::Node node;
  for (const auto& modelFile: modelFiles) {

    YAML::Node cat_attrs;
    cat_attrs["filename"] = modelFile.first;

    if (modelFile.second >= (int)categories.size()) {
      return false;
    }

    const std::string cat_name = categories[modelFile.second].name;
    node[modelFile.second][cat_name].push_back(cat_attrs);
  }

  writeYamlToByteArray(node, data);
  return true;
}

bool writeModelToYaml(const ModelData& model, QByteArray& data)
{
  YAML::Node node;
  node = model;

  writeYamlToByteArray(node, data);
  return true;
}

bool writeRadioSettingsToYaml(const GeneralSettings& settings, QByteArray& data)
{
  YAML::Node node;
  node = settings;

  writeYamlToByteArray(node, data);
  return true;
}
