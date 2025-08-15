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

#include "yaml.h"
#include "edgetxinterface.h"
#include "eeprominterface.h"
#include "yaml_ops.h"

#include <QFile>
#include <QDir>

bool YamlFormat::loadFile(QByteArray & filedata)
{
  QString path = filename;
  QFile file(path);
  if (!file.open(QFile::ReadOnly)) {
    setError(tr("Error opening file %1:\n%2.").arg(path).arg(file.errorString()));
    return false;
  }
  filedata = file.readAll();
  qDebug() << "File" << path << "read, size:" << filedata.size();
  file.close();
  return true;
}

bool YamlFormat::writeFile(const QByteArray & filedata)
{
  QString path = filename;
  QFile file(path);
  if (!file.open(QFile::WriteOnly)) {
    setError(tr("Error opening file %1 in write mode:\n%2.").arg(path).arg(file.errorString()));
    return false;
  }
  file.write(filedata.data(), filedata.size());
  file.close();
  qDebug() << "File" << path << "written, size:" << filedata.size();
  return true;
}

bool YamlFormat::load(RadioData & radioData)
{
  QByteArray data;
  if (!loadFile(data)) {
    setError(tr("Cannot read %1").arg(filename));
    return false;
  }

  bool ret = false;
  std::istringstream data_istream(data.toStdString());
  YAML::Node node = YAML::Load(data_istream);

  if (node.IsMap()) {
    if (node["board"].IsScalar())
      ret = loadSettings(radioData, data);
    else if (node["header"].IsMap())
      ret = loadModel(radioData, data);
    else
      setError(tr("Unable to determine content type for file %1").arg(filename));
  }
  else
    setError(tr("File %1 is not a valid format").arg(filename));

  return ret;
}

bool YamlFormat::loadSettings(RadioData & radioData, const QByteArray & data)
{
  qDebug() << "File" << filename << "appears to contain radio settings data";

  try {
    if (!loadRadioSettingsFromYaml(radioData.generalSettings, data)) {
      setError(tr("Cannot load ") + filename);
      return false;
    }
  } catch(const std::runtime_error& e) {
    setError(tr("Cannot load ") + filename + ":\n" + QString(e.what()));
    return false;
  }

  return true;
}

bool YamlFormat::loadModel(RadioData & radioData, const QByteArray & data)
{
  qDebug() << "File" << filename << "appears to contain model data";

  int modelIdx = 0;
  radioData.models.resize(1);
  auto& model = radioData.models[modelIdx];

  try {
    if (!loadModelFromYaml(model, data)) {
      setError(tr("Cannot load ") + filename);
      return false;
    }
  } catch(const std::runtime_error& e) {
    setError(tr("Cannot load ") + filename + ":\n" + QString(e.what()));
    return false;
  }

  model.modelIndex = modelIdx;
  model.used = true;

  radioData.generalSettings.currModelIndex = modelIdx;
  radioData.fixModelFilenames();

  //  without knowing the radio this model came from the old to new radio conversion can cause more issues than it tries to solve
  //  so leave fixing incompatibilities to the user
  board = getCurrentBoard();
  radioData.generalSettings.variant = getCurrentBoard();

  setWarning(tr("Please check all radio and model settings as no conversion could be performed."));
  return true;
}

bool YamlFormat::write(const ModelData & modelData)
{
  QByteArray data;
  writeModelToYaml(modelData, data);

  if (!writeFile(data))
    return false;

  return true;
}
