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

#include "labeled.h"
#include "firmwares/opentx/opentxinterface.h"
#include "firmwares/edgetx/edgetxinterface.h"
#include "progressdialog.h"

#include <regex>

StorageType LabelsStorageFormat::probeFormat()
{
  if (QFile(filename + "/RADIO/radio.yml").exists()) // converted
    return getStorageType("radio.yml");
  else
    return getStorageType(filename);
}

bool LabelsStorageFormat::load(RadioData & radioData)
{
  int steps = 2;  // initialisation and load radio.yml
  bool hasLabels = getCurrentFirmware()->getCapability(HasModelLabels);

  if (hasLabels)
    steps++;

  StorageType st = getStorageType(filename);

  if (st == STORAGE_TYPE_UNKNOWN)
    st = probeFormat();

  if (getStorageType(filename) == STORAGE_TYPE_UNKNOWN && probeFormat() == STORAGE_TYPE_ETX) {
    if (!QFile(filename + "/" + "RADIO/radio.yml").exists())
      qDebug() << tr("Cannot find %1/RADIO/radio.yml").arg(filename);
    else
      qDebug() << tr("Found %1/RADIO/radio.yml").arg(filename);

    if (!QFile(filename + "/" + "MODELS/models.yml").exists())
      qDebug() << tr("Cannot find %1/MODELS/models.yml").arg(filename);
    else
      qDebug() << tr("Found %1/MODELS/models.yml").arg(filename);
  } else {
    if (!QFile(filename).exists())
      qDebug() << tr("Cannot find %1").arg(filename);
    else
      qDebug() << tr("Found %1").arg(filename);
  }

  // Scan for all models
  EtxModelfiles modelFiles;

  statusMsg(tr("Scanning for models..."));

  std::list<std::string> filelist;
  if (!getFileList(filelist))
    return false;

  const std::regex yml_regex("MODELS/(model([0-9]+)\\.yml)", std::regex_constants::icase);

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

  progressSetMaximum(steps + modelFiles.size());
  steps = 0;
  progressSetValue(++steps);

  if (!loadRadioSettings(radioData.generalSettings))
    return false;

  board = (Board::Type)radioData.generalSettings.variant;

  // Labels - Read the labels from labels.yml file
  //        - Always scan for /MODELS/modelXX.yml
  //        - Update possible labels with all found in models too
  progressSetValue(++steps);

  QByteArray labelslistBuffer;
  int sortOrder = 0;

  if (hasLabels && loadFile(labelslistBuffer, "MODELS/labels.yml", true)) {
    try {
      if (!loadLabelsListFromYaml(radioData.labels, sortOrder, modelFiles, labelslistBuffer)) {
        fatalMsg(tr("Can't load MODELS/labels.yml"));
      }
    } catch(const std::runtime_error& e) {
      fatalMsg(tr("Can't load MODELS/labels.yml") + ":\n" + QString(e.what()));
    }

    progressSetValue(++steps);
  }

  // Save Sort Order
  radioData.sortOrder = sortOrder;

  int modelIdx = 0;

  if (hasLabels)
    radioData.models.resize(modelFiles.size());

  QList<QString> modelImages;

  for (const auto& mc : modelFiles) {
    qDebug() << "Filename: " << mc.filename.c_str();

    if (!hasLabels) {
      if (mc.modelIdx >= 0 && mc.modelIdx < (int)radioData.models.size()) {
        modelIdx = mc.modelIdx;
        if (!radioData.models[modelIdx].isEmpty()) {
          statusMsg(tr("Warning: file %1 skipped as slot %2 already used")
                    .arg(mc.filename.c_str()).arg(mc.modelIdx + 1), QtWarningMsg);
          continue;
        }
      } else {
        statusMsg(tr("Warning: file %1 skipped as slot %2 not available")
                  .arg(mc.filename.c_str()).arg(mc.modelIdx + 1), QtWarningMsg);
        continue;
      }
    }

    QByteArray modelBuffer;
    QString filename = "MODELS/" + QString::fromStdString(mc.filename);

    if (!loadFile(modelBuffer, filename)) {
      fatalMsg(tr("Cannot load %1").arg(filename));
      return false;
    }

    // Please note:
    //  ModelData() use memset to clear everything to 0
    //
    auto& model = radioData.models[modelIdx];

    try {
      if (!loadModelFromYaml(model,modelBuffer)) {
        fatalMsg(tr("Cannot convert to yaml %1").arg(filename));
        return false;
      }
    } catch(const std::runtime_error& e) {
      fatalMsg(tr("Cannot convert to yaml %1:\n%2").arg(filename).arg(QString(e.what())));
      return false;
    }

    if (!loadChecklist(model))
      return false;

    if (!model.isBitmapEmpty()) {
      const QString fname(model.getImageFilename());

      if (!modelImages.contains(fname)) {
        modelImages.append(fname);
      }
    }

    model.modelIndex = modelIdx;
    model.filename = mc.filename;

    if (hasLabels && model.filename == radioData.generalSettings.currModelFilename)
      radioData.generalSettings.currModelIndex = modelIdx;

    model.used = true;
    modelIdx++;
    progressSetValue(++steps);
    statusMsg(tr("Loaded: %1").arg(filename));
  }

  statusMsg(tr("Loading model images..."));

  for (const auto &fname : modelImages) {
    if (!loadImageFile(fname, true))
      return false;
  }

  // Add the labels in the models
  if (hasLabels) {
    statusMsg(tr("Loading labels..."));
    radioData.addLabelsFromModels();

    // If no labels, add a Favorites
    if(!radioData.labels.size()) {
      RadioData::LabelData ld = { tr("Favorites"), false };
      radioData.labels.append(ld);
    }
  }

  return true;
}

bool LabelsStorageFormat::write(RadioData & radioData)
{
  // TODO
  // move all unique radio settings to a separate file eg RADIO/hardware.yml.
  // These settings/values should never be transferred to another radio.
  // Also some are at a point in time eg calibration and
  // therefore not restored to the radio from say an etx file

  progressSetValue(0);
  progressSetMaximum(100);

  // retrieve models list + delete models + calib + write settings
  int steps = 4;

  bool hasLabels = getCurrentFirmware()->getCapability(HasModelLabels);
  steps += (int)hasLabels;

  for (const auto& model : radioData.models) {
    if (!model.isEmpty())
      steps += 2; // assume yaml and image for every model
  }

  progressSetMaximum(steps);
  steps = 0;

  std::list<std::string> filelist;
  getFileList(filelist);

  if (filelist.size()) {
    // Delete all old modelxx.yml from radio MODELS folder before writing new modelxx.yml files
    progressSetInfoAndMsg(tr("Deleting existing models..."));

    progressSetValue(++steps);
    const std::regex yml_regex("MODELS/(model([0-9]+)\\.yml)", std::regex_constants::icase);

    for (const auto& f : filelist) {
      std::smatch match;
      if (std::regex_match(f, match, yml_regex)) {
        if (match.size() == 3) {
          if (!deleteFile(QString(f.c_str()))) {
            fatalMsg(tr("Unable to delete file: %1").arg(QDir::toNativeSeparators(QString(f.c_str()))));
            return false;
          } else {
            statusMsg(tr("Deleted file: %1").arg(QString(f.c_str())));
          }
        }
      }
    }
  }

  progressSetValue(++steps);

  // may not exist on a new sd card or path
  const QString settingsPath("RADIO/radio.yml");
  const QString fullSettingsPath(filename % "/" % settingsPath);
  const QString displaySettingsPath(QDir::toNativeSeparators(fullSettingsPath));

  if (QFile(fullSettingsPath).exists()) {
    progressSetInfoAndMsg(tr("Preserve radio calibration..."));
    GeneralSettings gsCur;

    if (loadRadioSettings(gsCur)) {
      GeneralSettings & gsNew = radioData.generalSettings;
      gsNew.txVoltageCalibration = gsCur.txVoltageCalibration;

      for (int i = 0; i < CPN_MAX_INPUTS; i++) {
        gsNew.inputConfig[i].calib = gsCur.inputConfig[i].calib;
      }
    } else {
      fatalMsg(tr("Error reading radio calibration from %1")
                  .arg(displaySettingsPath));
      return false;
    }
  } else {
    qDebug() << "Radio calibration not found" << displaySettingsPath;
  }

  progressSetValue(++steps);
  progressSetInfoAndMsg(tr("Writing radio settings..."));

  QByteArray radioSettingsBuffer;
  if (!writeRadioSettingsToYaml(radioData.generalSettings, radioSettingsBuffer)) {
    fatalMsg(tr("Error converting radio settings to yaml"));
    return false;
  }

  if (!writeFile(radioSettingsBuffer, settingsPath)) {
    fatalMsg(tr("Error writing: %1").arg(displaySettingsPath));
    return false;
  }

  progressSetValue(++steps);
  progressSetInfoAndMsg(tr("Writing models..."));

  EtxModelfiles modelFiles;
  QList<QString> modelImages;

  for (const auto& model : radioData.models) {
    if (model.isEmpty())
      continue;

    QString modelFilename;

    if (hasLabels) {
      std::string ymlFilename = patchFilenameToYaml(model.filename);
      modelFilename = QString("MODELS/%1").arg(QString::fromStdString(ymlFilename));
      modelFiles.push_back({ymlFilename, std::string(model.name)});
    } else {
      modelFilename = QString("MODELS/model%1.yml").arg(model.modelIndex, 2, 10, QLatin1Char('0'));
    }

    QByteArray modelData;
    if (!writeModelToYaml(model, modelData)) {
      fatalMsg(tr("Error converting model to yaml: %1").arg(model.name.toQString()));
      return false;
    }

    if (!writeFile(modelData, modelFilename)) {
      fatalMsg(tr("Error writing: %1").arg(QDir::toNativeSeparators(filename % "/" % modelFilename)));
      return false;
    }

    if (!model.isBitmapEmpty()) {
      const QString fname(model.getImageFilename());

      if (!modelImages.contains(fname)) {
        modelImages.append(fname);
      }
    }

    if (!writeChecklist(model))
      return false;

    statusMsg(tr("Model written: %1").arg(model.name.toQString()));
    progressSetValue(++steps);
  }

  progressSetInfoAndMsg(tr("Writing model images..."));

  for (const auto &fname : modelImages) {
    if (!writeImageFile(fname))
      return false;
    else
      progressSetValue(++steps);
  }

  if (hasLabels) {
    progressSetInfoAndMsg(tr("Writing labels..."));

    QByteArray labelsListBuffer;
    if (!writeLabelsListToYaml(radioData, labelsListBuffer)) {
      fatalMsg(tr("Error converting labels to yaml"));
      return false;
    }

    const QString labelsPath("MODELS/labels.yml");
    if (!writeFile(labelsListBuffer, labelsPath)) {
      fatalMsg(tr("Error writing: %1").arg(QDir::toNativeSeparators(filename % "/" % labelsPath)));
      return false;
    }
  }

  progressSetValue(++steps);
  return true;
}

bool LabelsStorageFormat::loadChecklist(ModelData & model)
{
  const QString fname("MODELS/" + model.getChecklistFilename());
  //qDebug() << "Searching for checklist file:" << fname;

  if (!loadFile(model.checklistData, fname, true))
    return false;

  return true;
}

bool LabelsStorageFormat::writeChecklist(const ModelData & model)
{
  if (!model.checklistData.isEmpty()) {
    const QString fname("MODELS/" + model.getChecklistFilename());
    //qDebug() << "Writing checklist file:" << fname;

    if (!writeFile(model.checklistData, fname))
      return false;
  }

  return true;
}

bool LabelsStorageFormat::loadRadioSettings(GeneralSettings & generalSettings)
{
  QByteArray radioSettingsBuffer;

  const QString file("RADIO/radio.yml");
  const QString filePath(QDir::toNativeSeparators(filename + "/" + file));

  if (!loadFile(radioSettingsBuffer, file)) {
    fatalMsg(tr("Cannot read %1").arg(filePath));
    return false;
  }

  try {
    if (!loadRadioSettingsFromYaml(generalSettings, radioSettingsBuffer)) {
      fatalMsg(tr("Cannot convert to yaml %1").arg(filePath));
      return false;
    }
  } catch(const std::runtime_error& e) {
    fatalMsg(tr("Cannot convert to yaml %1:\n%2").arg(filePath).arg(QString(e.what())));
    return false;
  }

  return true;
}
