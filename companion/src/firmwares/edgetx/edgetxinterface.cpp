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

#include "edgetxinterface.h"

#include "crc.h"
#include <iostream>
#include <string>
#include "yaml_ops.h"
#include "yaml_generalsettings.h"
#include "yaml_modeldata.h"
#include "labelvalidator.h"

#include <QMessageBox>


static uint16_t calculateChecksum(const QByteArray& data, uint16_t checksum)
{
  std::string fileContent = data.toStdString();

  size_t startPos = 0;
  size_t pos = 0;
  uint8_t isCRLF = 0;
  std::string newLine[2] =  {"\n","\r\n"};

  pos = fileContent.find("\n", 0);
  if(pos == 0) {
    return checksum;
  }

  if(fileContent[pos-1] == '\r') {
    isCRLF = 1;
    qDebug() << "** File has CRLF line endings";
  } else {
    isCRLF = 0;
    qDebug() << "** File has LF line endings";
  }

  // The first line contains the "checksum" value - if present. Skip it
  if ( fileContent.find("checksum:") == 0) {
    startPos = fileContent.find(newLine[isCRLF]) + newLine[isCRLF].length();
  }

  checksum = crc16(0, (const uint8_t *) &fileContent[startPos], fileContent.length() - startPos, checksum);
  return checksum;
}

static YAML::Node loadYamlFromByteArray(const QByteArray& data)
{
    // TODO: use real streaming to avoid memory copies
    std::istringstream data_istream(data.toStdString());
    return YAML::Load(data_istream);
}

static void writeYamlToByteArray(const YAML::Node& node, QByteArray& data, bool addChecksum = false)
{
    // TODO: use real streaming to avoid memory copies
    std::stringstream data_ostream;

    data_ostream << node;
    data = QByteArray::fromStdString(data_ostream.str());

    qDebug() << "Saving YAML:";

    if(addChecksum) {
      uint16_t checksum = 0xFFFF;
      checksum = calculateChecksum(data, checksum);
      std::stringstream checksum_ostream;
      checksum_ostream << "checksum:  " << checksum << std::endl;
      data.prepend(checksum_ostream.str().c_str());
    }

    qDebug() << data.toStdString().c_str();
}

bool loadLabelsListFromYaml(RadioData::ModelLabels& labels,
                            int& sortOrder,
                            EtxModelfiles& modelFiles,
                            const QByteArray& data)
{
  sortOrder = 0;
  labels.clear();
  if (data.size() == 0)
    return true;
  YAML::Node node = loadYamlFromByteArray(data);
  if (!node.IsMap()) return false; // Root Map (Labels, Sort, Models)
  if (node["Sort"].IsScalar()) {
    bool ok = false;
    sortOrder = QString::fromStdString(node["Sort"].Scalar()).toUInt(&ok);
    if (!ok)
      sortOrder = 0;
  }

  YAML::Node lbls = node["Labels"];
  if (lbls.IsMap()) {
    for (YAML::const_iterator it=lbls.begin(); it!=lbls.end(); ++it) {
      std::string lbl = it->first.as<std::string>();
      RadioData::LabelData ld;
      ld.name = QString::fromStdString(lbl);
      YamlValidateLabel(ld.name);
      if (!ld.name.isEmpty()) {
        if (lbls[lbl]["selected"])
          ld.selected = lbls[lbl]["selected"].as<bool>();
        else
          ld.selected = false;
        labels.append(ld);
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
    if(data.indexOf("checksum:") == 0) {
      int startPos = strlen("checksum:");
      int endPos = data.indexOf("\n");
      if (endPos > 0) {
        while( ((const char)data[startPos] == ' ') && (startPos < data.size())) {
          startPos++;
        }
        if (startPos < data.size()) {
          QByteArray checksumStr = data.mid(startPos, endPos - startPos);
          uint16_t fileChecksum = std::stoi(checksumStr.toStdString());
          uint16_t calculatedChecksum = calculateChecksum(data, 0xFFFF);
          qDebug() << "File checksum:" << fileChecksum;
          qDebug() << "Calculated checksum:" << calculatedChecksum;
          if (fileChecksum != calculatedChecksum) {
            qDebug() << "File checksum mismatch! Got: " << fileChecksum << ", expected: " << calculatedChecksum;
            QMessageBox::critical(NULL, CPN_STR_APP_NAME, QCoreApplication::translate("EdgeTXInterface", "Radio settings file checksum error. You are advised to review the settings"));
            //return false;
          }
        }
      }
    }

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

bool writeLabelsListToYaml(const RadioData &radioData, QByteArray& data)
{
  YAML::Node node;
  foreach(RadioData::LabelData label, radioData.labels) {
    node["Labels"][label.name.toStdString()] = YAML::Null;
    if (label.selected)
      node["Labels"][label.name.toStdString()]["selected"] = true;
  }
  node["Sort"] = radioData.sortOrder;
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

  writeYamlToByteArray(node, data, true);
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
