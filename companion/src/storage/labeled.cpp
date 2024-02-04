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

#include "labeled.h"
#include "firmwares/opentx/opentxinterface.h"
#include "firmwares/edgetx/edgetxinterface.h"

#include <regex>

bool LabelsStorageFormat::load(RadioData & radioData)
{
  StorageType st = getStorageType(filename);
  if (st == STORAGE_TYPE_UNKNOWN) {
    st = probeFormat();
  }
  if (st == STORAGE_TYPE_ETX || st == STORAGE_TYPE_YML
      || st == STORAGE_TYPE_UNKNOWN) {
    return loadYaml(radioData);
  } else {
    return loadBin(radioData);
  }
}

bool LabelsStorageFormat::write(const RadioData & radioData)
{
  StorageType st = getStorageType(filename);
  if (st == STORAGE_TYPE_ETX || st == STORAGE_TYPE_YML ||
      st == STORAGE_TYPE_UNKNOWN) {
    return writeYaml(radioData);
  }

  return false;
}

StorageType LabelsStorageFormat::probeFormat()
{
  if (QFile(filename + "/RADIO/radio.yml").exists()) // converted
    return getStorageType("radio.yml");
  else if (QFile(filename + "/RADIO/radio.bin").exists()) // unconverted
    return getStorageType("radio.bin");
  else
    return getStorageType(filename);
}

bool LabelsStorageFormat::loadBin(RadioData & radioData)
{
  QByteArray radioSettingsBuffer;
  if (!loadFile(radioSettingsBuffer, "RADIO/radio.bin")) {
    setError(tr("Cannot extract RADIO/radio.bin"));
    return false;
  }

  OpenTxEepromInterface * loadInterface = loadRadioSettingsFromByteArray(radioData.generalSettings, radioSettingsBuffer);
  if (!loadInterface) {
    return false;
  }


  board = loadInterface->getBoard();

  radioData.generalSettings.convertLegacyConfiguration(board);

  QByteArray modelsListBuffer;
  if (!loadFile(modelsListBuffer, "RADIO/models.txt")) {
    setError(tr("Cannot extract RADIO/models.txt"));
    return false;
  }

  QList<QByteArray> lines = modelsListBuffer.split('\n');
   int modelIndex = 0;
   foreach (const QByteArray & lineArray, lines) {
     QString line = QString(lineArray).trimmed();
     if (line.isEmpty()) continue;
     // qDebug() << "parsing line" << line;

     if (line.startsWith('[') && line.endsWith(']')) {
       continue;
     }

    // determine if we have a model number
    QStringList parts = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
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
        setError(tr("Cannot extract %1").arg(fileName));
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
      radioData.models[modelIndex].used = true;
      modelIndex++;
      continue;
    }

    // invalid line
    // TODO add to parsing report
    qDebug() << "Invalid line" <<line;
    continue;
  }

  // Add a Favorites label
  if(getCurrentFirmware()->getCapability(HasModelLabels)) {
   RadioData::LabelData ld = { tr("Favorites"), false };
   radioData.labels.append(ld);
  }
  return true;
}

bool LabelsStorageFormat::writeBin(const RadioData & radioData)
{
  QByteArray radioSettingsData; // radio.bin
  size_t numModels = radioData.models.size();
  size_t numCategories = radioData.labels.size();
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
  }

  return true;
}

bool LabelsStorageFormat::loadYaml(RadioData & radioData)
{
  if (getStorageType(filename) == STORAGE_TYPE_UNKNOWN && probeFormat() == STORAGE_TYPE_ETX) {
    if (!QFile(filename + "/" + "RADIO/radio.yml").exists())
      qDebug() << tr("Cannot find %1/RADIO/radio.yml").arg(filename);
    else
      qDebug() << tr("Found %1/RADIO/radio.yml").arg(filename);

    if (!QFile(filename + "/" + "MODELS/models.yml").exists())
      qDebug() << tr("Cannot find %1/MODELS/models.yml").arg(filename);
    else
      qDebug() << tr("Found %1/MODELS/models.yml").arg(filename);
  }
  else {
    if (!QFile(filename).exists())
      qDebug() << tr("Cannot find %1").arg(filename);
    else
      qDebug() << tr("Found %1").arg(filename);
  }

  QByteArray radioSettingsBuffer;
  if (!loadFile(radioSettingsBuffer, "RADIO/radio.yml")) {
    setError(tr("Cannot extract RADIO/radio.yml"));
    return false;
  }

  try {
    if (!loadRadioSettingsFromYaml(radioData.generalSettings, radioSettingsBuffer)) {
      setError(tr("Cannot load RADIO/radio.yml"));
      return false;
    }
  } catch(const std::runtime_error& e) {
    setError(tr("Cannot load RADIO/radio.yml") + ":\n" + QString(e.what()));
    return false;
  }

  board = (Board::Type)radioData.generalSettings.variant;

  // get models
  EtxModelfiles modelFiles;

  // Labels - Read the labels from labels.yml file
  //        - Always scan for /MODELS/modelXX.yml
  //        - Update possible labels with all found in models too

  QByteArray labelslistBuffer;
  int sortOrder = 0;
  if (loadFile(labelslistBuffer, "MODELS/labels.yml")) {
    try {
      if (!loadLabelsListFromYaml(radioData.labels, sortOrder, modelFiles, labelslistBuffer)) {
        setError(tr("Can't load MODELS/labels.yml"));
      }
    } catch(const std::runtime_error& e) {
      setError(tr("Can't load MODELS/labels.yml") + ":\n" + QString(e.what()));
    }
  }

  // Save Sort Order
  radioData.sortOrder = sortOrder;

  // Scan for all models
  std::list<std::string> filelist;
  if (!getFileList(filelist)) {
    return false;
  }

  const std::regex yml_regex("MODELS/(model([0-9]+)\\.yml)");
  for(const auto& f : filelist) {
    std::smatch match;
    if (std::regex_match(f, match, yml_regex)) {
      if (match.size() == 3) {
        std::ssub_match modelFile = match[1];
        std::ssub_match modelIdx = match[2];
        modelFiles.push_back({ modelFile.str(), "", std::stoi(modelIdx.str()) });
      }
    }
  }

  int modelIdx = 0;
  bool hasLabels = getCurrentFirmware()->getCapability(HasModelLabels);

  if (hasLabels)
    radioData.models.resize(modelFiles.size());

  for (const auto& mc : modelFiles) {
    qDebug() << "Filename: " << mc.filename.c_str();

    if (!hasLabels) {
      if (mc.modelIdx >= 0 && mc.modelIdx < (int)radioData.models.size()) {
        modelIdx = mc.modelIdx;
        if (!radioData.models[modelIdx].isEmpty()) {
          qDebug() << QString("Warning: file %1 skipped as slot %2 already used").arg(mc.filename.c_str()).arg(mc.modelIdx + 1);
          continue;
        }
      }
      else {
        qDebug() << QString("Warning: file %1 skipped as slot %2 not available").arg(mc.filename.c_str()).arg(mc.modelIdx + 1);
        continue;
      }
    }

    QByteArray modelBuffer;
    QString filename = "MODELS/" + QString::fromStdString(mc.filename);
    if (!loadFile(modelBuffer, filename)) {
      setError(tr("Cannot extract ") + filename);
      return false;
    }

    // Please note:
    //  ModelData() use memset to clear everything to 0
    //
    auto& model = radioData.models[modelIdx];

    try {
      if (!loadModelFromYaml(model,modelBuffer)) {
        setError(tr("Cannot load ") + filename);
        return false;
      }
    } catch(const std::runtime_error& e) {
      setError(tr("Cannot load ") + filename + ":\n" + QString(e.what()));
      return false;
    }

    model.modelIndex = modelIdx;
    strncpy(model.filename, mc.filename.c_str(), sizeof(model.filename)-1);

    if (hasLabels && !strncmp(radioData.generalSettings.currModelFilename,
                                  model.filename, sizeof(model.filename))) {
      radioData.generalSettings.currModelIndex = modelIdx;
    }

    model.used = true;
    modelIdx++;
  }

  // Add the labels in the models
  if (hasLabels) {
    radioData.addLabelsFromModels();

    // If no labels, add a Favorites
    if(!radioData.labels.size()) {
      RadioData::LabelData ld = { tr("Favorites"), false };
      radioData.labels.append(ld);
    }
  }

  return true;
}

bool LabelsStorageFormat::writeYaml(const RadioData & radioData)
{
  QByteArray radioSettingsBuffer;
  if (!writeRadioSettingsToYaml(radioData.generalSettings, radioSettingsBuffer)) {
    return false;
  }

  if (!writeFile(radioSettingsBuffer, "RADIO/radio.yml")) {
    return false;
  }

  bool hasLabels = getCurrentFirmware()->getCapability(HasModelLabels);

  // fetch "MODELS/modelXX.yml"
  std::list<std::string> filelist;
  if (!getFileList(filelist)) {
    setError(tr("Cannot list files"));
    return false;
  }

  // Delete all old modelxx.yml from radio MODELS folder before writing new modelxx.yml files
  const std::regex yml_regex("MODELS/(model([0-9s]+)\\.yml)");
  for(const auto& f : filelist) {
    std::smatch match;
    if (std::regex_match(f, match, yml_regex)) {
      if (match.size() == 3) {
        if (!deleteFile(QString(f.c_str()))) {
          setError(tr("Error deleting files"));
          return false;
        }
      }
    }
  }

  EtxModelfiles modelFiles;
  for (const auto& model : radioData.models) {

    if (model.isEmpty())
      continue;

    QString modelFilename;
    if (hasLabels) {
      std::string ymlFilename = patchFilenameToYaml(model.filename);
      modelFilename =
          QString("MODELS/%1").arg(QString::fromStdString(ymlFilename));
      modelFiles.push_back(
          {ymlFilename, std::string(model.name)});
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

  if (hasLabels) {
    QByteArray labelsListBuffer;
    if (!writeLabelsListToYaml(radioData, labelsListBuffer)
        || !writeFile(labelsListBuffer, "MODELS/labels.yml")) {
      return false;
    }
  }

  return true;
}
