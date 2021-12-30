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

#include "categorized.h"
#include "firmwares/opentx/opentxinterface.h"
#include "firmwares/edgetx/edgetxinterface.h"
#include "miniz.c"    //  Can only be included once!

#include <regex>

bool CategorizedStorageFormat::load(RadioData & radioData)
{
  StorageType st = getStorageType(filename);
  if (st == STORAGE_TYPE_UNKNOWN) {
    st = probeFormat();
  }
  if (st == STORAGE_TYPE_ETX || st == STORAGE_TYPE_YML) {
    return loadYaml(radioData);
  } else {
    return loadBin(radioData);
  }
}

bool CategorizedStorageFormat::write(const RadioData & radioData)
{
  StorageType st = getStorageType(filename);
  if (st == STORAGE_TYPE_UNKNOWN) {
    st = probeFormat();
  }
  if (st == STORAGE_TYPE_ETX || st == STORAGE_TYPE_YML ||
      st == STORAGE_TYPE_UNKNOWN) {
    return writeYaml(radioData);
  } else {
    return writeBin(radioData);
  }
}

StorageType CategorizedStorageFormat::probeFormat()
{
  if (QFile(filename + "/RADIO/radio.yml").exists()) // converted
    return getStorageType("radio.yml");
  else if (QFile(filename + "/RADIO/radio.bin").exists()) // unconverted
    return getStorageType("radio.bin");
  else
    return getStorageType(filename);
}

bool CategorizedStorageFormat::loadBin(RadioData & radioData)
{
  QByteArray radioSettingsBuffer;
  if (!loadFile(radioSettingsBuffer, "RADIO/radio.bin")) {
    setError(tr("Can't extract RADIO/radio.bin"));
    return false;
  }

  OpenTxEepromInterface * loadInterface = loadRadioSettingsFromByteArray(radioData.generalSettings, radioSettingsBuffer);
  if (!loadInterface) {
    return false;
  }

  board = loadInterface->getBoard();

  QByteArray modelsListBuffer;
  if (!loadFile(modelsListBuffer, "RADIO/models.txt")) {
    setError(tr("Can't extract RADIO/models.txt"));
    return false;
  }

  QList<QByteArray> lines = modelsListBuffer.split('\n');
  int modelIndex = 0;
  int categoryIndex = -1;
  foreach (const QByteArray & lineArray, lines) {
    QString line = QString(lineArray).trimmed();
    if (line.isEmpty()) continue;
    // qDebug() << "parsing line" << line;

    if (line.startsWith('[') && line.endsWith(']')) {
      // ignore categories for boards that do not support them
      if (getCurrentFirmware()->getCapability(HasModelCategories)) {
        QString name = line.mid(1, line.size() - 2);
        CategoryData category(qPrintable(name));
        radioData.categories.push_back(category);
        categoryIndex++;
        qDebug() << "added category" << name;
      }
      continue;
    }

    // determine if we have a model number
    QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    if (parts.size() == 2) {
      // parse model number
      int modelNumber = parts[0].toInt();
      if (modelNumber > 0 && modelNumber > modelIndex && modelNumber < getCurrentFirmware()->getCapability(Models)) {
        modelIndex = modelNumber;
        qDebug() << "advanced model number to" << modelIndex;
      }
      else {
        if (modelNumber != modelIndex) qDebug() << "Invalid model number" << parts[0];
      }
      parts.removeFirst();
    }
    if (parts.size() == 1) {
      // parse model file name and load
      QString fileName = parts[0];
      qDebug() << "Loading model from file" << fileName << "into slot" << modelIndex;
      QByteArray modelBuffer;
      if (!loadFile(modelBuffer, QString("MODELS/%1").arg(fileName))) {
        setError(tr("Can't extract %1").arg(fileName));
        return false;
      }
      if ((int)radioData.models.size() <= modelIndex) {
        radioData.models.resize(modelIndex + 1);
      }
      if (!loadModelFromByteArray(radioData.models[modelIndex], modelBuffer)) {
        setError(tr("Error loading models"));
        return false;
      }
      strncpy(radioData.models[modelIndex].filename, qPrintable(fileName), sizeof(radioData.models[modelIndex].filename));
      if (IS_FAMILY_HORUS_OR_T16(board) && !strcmp(radioData.generalSettings.currModelFilename, qPrintable(fileName))) {
        radioData.generalSettings.currModelIndex = modelIndex;
        qDebug() << "currModelIndex =" << modelIndex;
      }
      if (getCurrentFirmware()->getCapability(HasModelCategories)) {
        radioData.models[modelIndex].category = categoryIndex;
      }
      radioData.models[modelIndex].used = true;
      modelIndex++;
      continue;
    }

    // invalid line
    // TODO add to parsing report
    qDebug() << "Invalid line" <<line;
    continue;
  }
  return true;
}

bool CategorizedStorageFormat::writeBin(const RadioData & radioData)
{
  QByteArray modelsList;   // models.txt
  QByteArray radioSettingsData; // radio.bin
  size_t numModels = radioData.models.size();
  size_t numCategories = radioData.categories.size();
  std::vector<std::vector<QString>> sortedModels(numCategories);

  writeRadioSettingsToByteArray(radioData.generalSettings, radioSettingsData);
  if (!writeFile(radioSettingsData, "RADIO/radio.bin")) {
    return false;
  }

  for (size_t m=0; m<numModels; m++) {
    const ModelData & model = radioData.models[m];
    if (model.isEmpty()) continue;

    QString modelFilename = QString("MODELS/%1").arg(model.filename);
    QByteArray modelData;
    writeModelToByteArray(model, modelData);
    if (!writeFile(modelData, modelFilename)) {
      return false;
    }

    // For firmware that doesn't support categories, we can just construct
    // models.txt as we iterate thru the models vector. For firmware that does
    // support categories, we have a bit of work to do in order to sort the
    // models first by category index, so we use the sortedModels data
    // structure to do that.
    if (!getCurrentFirmware()->getCapability(HasModelCategories)) {
      // Use format with model number and file name. This is needed because
      // radios without category support can have unused model slots
      modelsList.append(QString("%1 %2\n").arg(m).arg(model.filename).toLocal8Bit());
    } else {
      sortedModels[model.category].push_back(QString("%1\n").arg(model.filename));
    }
  }

  if (getCurrentFirmware()->getCapability(HasModelCategories)) {
    for (size_t c=0; c<numCategories; c++) {
      modelsList.append(QString()
                            .sprintf("[%s]\n", radioData.categories[c].name)
                            .toLocal8Bit());
      numModels = sortedModels[c].size();
      for (size_t m=0; m<numModels; m++) {
        modelsList.append(sortedModels[c][m].toLocal8Bit());
      }
    }
  }

  if (!writeFile(modelsList, "RADIO/models.txt")) {
    return false;
  }

  return true;
}

bool CategorizedStorageFormat::loadYaml(RadioData & radioData)
{
  if (getStorageType(filename) == STORAGE_TYPE_UNKNOWN && probeFormat() == STORAGE_TYPE_ETX) {
    if (!QFile(filename + "/" + "RADIO/radio.yml").exists())
      qDebug() << tr("Can't find %1/RADIO/radio.yml").arg(filename);
    else
      qDebug() << tr("Found %1/RADIO/radio.yml").arg(filename);

    if (!QFile(filename + "/" + "MODELS/models.yml").exists())
      qDebug() << tr("Can't find %1/MODELS/models.yml").arg(filename);
    else
      qDebug() << tr("Found %1/MODELS/models.yml").arg(filename);
  }
  else {
    if (!QFile(filename).exists())
      qDebug() << tr("Can't find %1").arg(filename);
    else
      qDebug() << tr("Found %1").arg(filename);
  }

  QByteArray radioSettingsBuffer;
  if (!loadFile(radioSettingsBuffer, "RADIO/radio.yml")) {
    setError(tr("Can't extract RADIO/radio.yml"));
    return false;
  }

  try {
    if (!loadRadioSettingsFromYaml(radioData.generalSettings, radioSettingsBuffer)) {
      setError(tr("Can't load RADIO/radio.yml"));
      return false;
    }
  } catch(const std::runtime_error& e) {
    setError(tr("Can't load RADIO/radio.yml") + ":\n" + QString(e.what()));
    return false;
  }

  board = (Board::Type)radioData.generalSettings.variant;

  // get models
  EtxModelfiles modelFiles;

  QByteArray modelslistBuffer;
  if (loadFile(modelslistBuffer, "MODELS/models.yml")) {
    if (!loadModelsListFromYaml(radioData.categories, modelFiles, modelslistBuffer)) {
      setError(tr("Can't load MODELS/models.yml"));
      return false;
    }
  } else {
    // fetch "MODELS/modelXX.yml"
    std::list<std::string> filelist;
    if (!getFileList(filelist)) {
      setError(tr("Can't list files"));
      return false;
    }

    const std::regex yml_regex("MODELS/(model([0-9]+)\\.yml)");
    for(const auto& f : filelist) {
      std::smatch match;
      if (std::regex_match(f, match, yml_regex)) {
        if (match.size() == 3) {
          std::ssub_match modelFile = match[1];
          std::ssub_match modelIdx = match[2];
          modelFiles.push_back({ modelFile.str(), "", 0, std::stoi(modelIdx.str()) });
        }
      }
    }
  }

  radioData.models.resize(modelFiles.size());
  for (const auto& mc : modelFiles) {
    qDebug() << "Filename: " << mc.filename.c_str() << " / Category: " << mc.category;

    QByteArray modelBuffer;
    QString filename = "MODELS/" + QString::fromStdString(mc.filename);
    if (!loadFile(modelBuffer, filename)) {
      setError(tr("Can't extract ") + filename);
      return false;
    }

    // Please note:
    //  ModelData() use memset to clear everything to 0
    //
    auto& model = radioData.models[mc.modelIdx];

    try {
      if (!loadModelFromYaml(model,modelBuffer)) {
        setError(tr("Can't load ") + filename);
        return false;
      }
    } catch(const std::runtime_error& e) {
      setError(tr("Can't load ") + filename + ":\n" + QString(e.what()));
      return false;
    }

    model.category = mc.category;
    model.modelIndex = mc.modelIdx;
    strncpy(model.filename, mc.filename.c_str(), sizeof(model.filename)-1);

    model.used = true;
  }

  return true;
}

bool CategorizedStorageFormat::writeYaml(const RadioData & radioData)
{
  QByteArray radioSettingsBuffer;
  if (!writeRadioSettingsToYaml(radioData.generalSettings, radioSettingsBuffer)) {
    return false;
  }

  if (!writeFile(radioSettingsBuffer, "RADIO/radio.yml")) {
    return false;
  }

  bool hasCategories = getCurrentFirmware()->getCapability(HasModelCategories);

  EtxModelfiles modelFiles;
  for (const auto& model : radioData.models) {

    if (model.isEmpty())
      continue;

    QString modelFilename;
    if (hasCategories) {
      std::string ymlFilename = patchFilenameToYaml(model.filename);
      modelFilename =
          QString("MODELS/%1").arg(QString::fromStdString(ymlFilename));
      modelFiles.push_back(
          {ymlFilename, std::string(model.name), model.category});
    } else {
      modelFilename = QString("MODELS/model%1.yml")
                          .arg(model.modelIndex, 2, 10, QLatin1Char('0'));
    }

    QByteArray modelData;
    writeModelToYaml(model, modelData);
    if (!writeFile(modelData, modelFilename)) {
      return false;
    }
  }

  if (hasCategories) {
    // TODO: sort 'modelFiles' by category index
    QByteArray modelslistBuffer;
    if (!writeModelsListToYaml(radioData.categories, modelFiles, modelslistBuffer)
        || !writeFile(modelslistBuffer, "MODELS/models.yml")) {
      return false;
    }
  }

  return true;
}
