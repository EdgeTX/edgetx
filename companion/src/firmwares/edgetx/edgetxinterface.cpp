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
  if (data.size() == 0)
    return true;

  YAML::Node node = loadYamlFromByteArray(data);
  if (!node.IsSequence()) return false;

  int modelIdx = 0;
  for (const auto& cat : node) {
    if (!cat.IsMap()) continue;

    for (const auto& cat_map : cat) {
      categories.push_back(cat_map.first.Scalar().c_str());

      const auto& models = cat_map.second;
      if (!models.IsSequence()) continue;

      for (const auto& model : models) {
        std::string filename, name;
        model["filename"] >> filename;
        model["name"] >> name;
        modelFiles.push_back(
            {filename, name, (int)categories.size() - 1, modelIdx++});
      }
    }
  }

  return true;
}

bool loadModelFromYaml(ModelData& model, const QByteArray& data)
{
    YAML::Node node = loadYamlFromByteArray(data);
    node >> model;

  return true;
}

bool loadRadioSettingsFromYaml(GeneralSettings& settings, const QByteArray& data)
{
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
  std::vector<CategoryData> cats = categories;
  std::vector<EtxModelMetadata> files = { modelFiles.begin(), modelFiles.end() };

  std::stable_sort(files.begin(), files.end(),
                   [](const EtxModelMetadata &a, const EtxModelMetadata &b) {
                     return a.category < b.category;
                   });

  int catIdx = 0;
  for (const auto& cat : cats) {
    node[catIdx++][cat.name] = YAML::Node();
  }

  for (const auto& modelFile: modelFiles) {

    YAML::Node cat_attrs;
    cat_attrs["filename"] = modelFile.filename;
    cat_attrs["name"] = modelFile.name;

    catIdx = modelFile.category;
    if (catIdx >= (int)cats.size()) {
      catIdx = 0;
      if (cats.size() == 0) {
        cats.push_back("Models");
      }
    }

    const std::string cat_name = cats[catIdx].name;
    node[catIdx][cat_name].push_back(cat_attrs);
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

std::string patchFilenameToYaml(const std::string& str)
{
  constexpr const char MODELS_EXT[] = ".bin";
  constexpr const char YAML_EXT[] = ".yml";
  constexpr unsigned bin_len = sizeof(MODELS_EXT) - 1;

  // patch file extension
  auto const dot_pos = str.find_last_of('.');
  if ((dot_pos != std::string::npos) && (str.substr(dot_pos) == MODELS_EXT)) {
    std::string tmp = str;
    return tmp.replace(dot_pos, bin_len, YAML_EXT);
  }
  return str;
}
