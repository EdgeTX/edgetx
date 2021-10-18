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

#include "opentx.h"
#include "datastructs_220.h"

// see "yaml/yaml_datastructs.h"
namespace yaml_conv_220 {
    const YamlNode* get_radiodata_nodes();
    const YamlNode* get_modeldata_nodes();
    const YamlNode* get_partialmodel_nodes();
};

#if defined(STORAGE_MODELSLIST)

void patchFilenameToYaml(char* str)
{
  constexpr unsigned bin_len = sizeof(MODELS_EXT) - 1;
  constexpr unsigned yml_len = sizeof(YAML_EXT) - 1;

  // patch file extension
  const char* ext = strrchr(str, '.');
  if (ext && (strlen(ext) == bin_len) &&
      !strncmp(ext, STR_MODELS_EXT, bin_len)) {
    memcpy((void*)ext, (void*)STR_YAML_EXT, yml_len + 1);
  }
}

static const char* convertData_220_to_221(
    const char* path, unsigned size, const YamlNode* root_node,
    void (*patchBinary)(uint8_t*) = nullptr)
{
  auto data = reinterpret_cast<uint8_t*>(malloc(size));
  memset(data, 0, size);

  uint8_t version;
  const char* error = loadFileBin(path, data, size, &version);
  if (!error) {
    if (patchBinary) patchBinary(data);

    char output_fname[FF_MAX_LFN+1];
    strncpy(output_fname, path, FF_MAX_LFN);
    output_fname[FF_MAX_LFN] = '\0';
    
    patchFilenameToYaml(output_fname);
    error = writeFileYaml(output_fname, root_node, data);
  }

  free(data);
  return error;
}

const char* convertModelData_220_to_221(const char* path)
{
  constexpr unsigned md_size = sizeof(bin_storage_220::ModelData);
  return convertData_220_to_221(path, md_size,
                                yaml_conv_220::get_modeldata_nodes());
}

static void patchCurrentModelFilename(uint8_t* data)
{
  auto rd = reinterpret_cast<bin_storage_220::RadioData*>(data);
  patchFilenameToYaml(rd->currModelFilename);
}

const char* convertRadioData_220_to_221(const char* path)
{
  constexpr unsigned rd_size = sizeof(bin_storage_220::RadioData);
  return convertData_220_to_221(path, rd_size,
                                yaml_conv_220::get_radiodata_nodes(),
                                patchCurrentModelFilename);
}

#else

#include <storage/eeprom_common.h>

constexpr uint8_t MODELIDX_STRLEN = sizeof(MODEL_FILENAME_PREFIX "00");

static void getModelNumberStr(uint8_t idx, char* model_idx)
{
  memcpy(model_idx, MODEL_FILENAME_PREFIX, sizeof(MODEL_FILENAME_PREFIX));
  model_idx[sizeof(MODEL_FILENAME_PREFIX)-1] = '0' + idx / 10;
  model_idx[sizeof(MODEL_FILENAME_PREFIX)]   = '0' + idx % 10;
  model_idx[sizeof(MODEL_FILENAME_PREFIX)+1] = '\0';
}

const char* convertModelData_220_to_221(uint8_t id)
{
  constexpr unsigned size = sizeof(bin_storage_220::ModelData);
  auto data = reinterpret_cast<uint8_t*>(malloc(size));
  memset(data, 0, size);

  const char* error = nullptr;
  uint16_t read = eeLoadModelData(id, data, size);
  if (read == size) {
    char model_idx[MODELIDX_STRLEN + sizeof(YAML_EXT)];
    getModelNumberStr(id, model_idx);
    GET_FILENAME(path, MODELS_PATH, model_idx, YAML_EXT);
    error = writeFileYaml(path, yaml_conv_220::get_modeldata_nodes(), data);
  } else {
    // ERROR: size mismatch !!!
  }

  free(data);
  return error;
}

const char* convertRadioData_220_to_221()
{
  constexpr unsigned size = sizeof(bin_storage_220::RadioData);
  auto data = reinterpret_cast<uint8_t*>(malloc(size));
  memset(data, 0, size);

  const char* error = nullptr;
  uint16_t read = eeLoadGeneralSettingsData(data, size);
  if (read == size) {
    error = writeFileYaml(RADIO_SETTINGS_YAML_PATH,
                          yaml_conv_220::get_radiodata_nodes(), data);
  } else {
    // ERROR: size mismatch !!!
  }

  free(data);
  return error;
}

#endif
