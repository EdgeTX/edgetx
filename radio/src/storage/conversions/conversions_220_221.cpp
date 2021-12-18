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

#include <cstdlib>
#include <cstring>
#include <string>

#include "definitions.h"
#include "datastructs_220.h"

#include "sdcard.h"

#include <storage/yaml/yaml_node.h>

using namespace bin_storage_220;
#include <storage/sdcard_common.h>

#if defined(SDCARD_YAML)
#include <storage/sdcard_yaml.h>
#endif

// see "yaml/yaml_datastructs.h"
namespace yaml_conv_220 {
    const YamlNode* get_radiodata_nodes();
    const YamlNode* get_modeldata_nodes();
    const YamlNode* get_partialmodel_nodes();
};

#if STORAGE_CONVERSIONS < 221

#if defined(STORAGE_MODELSLIST)

void patchFilenameToYaml(char* str);

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

#if defined(COLORLCD)
void patchWidgetOptions(ZonePersistentData* zones, unsigned n_zones)
{
  for (unsigned i = 0; i < n_zones; i++) {
    auto& zone = zones[i];
    auto name_len = strnlen(zone.widgetName, sizeof(zone.widgetName));
    std::string widgetName(zone.widgetName, name_len);

    if (widgetName.empty()) continue;

    if (widgetName == "Value") {
      zone.widgetData.options[0].type = ZOV_Source;
      zone.widgetData.options[1].type = ZOV_Color;
    } else if (widgetName == "Gauge") {
      zone.widgetData.options[0].type = ZOV_Source;
      zone.widgetData.options[3].type = ZOV_Color;
    } else if (widgetName == "Outputs") {
      zone.widgetData.options[2].type = ZOV_Color;
      zone.widgetData.options[3].type = ZOV_Color;
      zone.widgetData.options[4].type = ZOV_Color;
    } else if (widgetName == "Text") {
      zone.widgetData.options[1].type = ZOV_Color;
    }
  }
}

static void patchModelData(uint8_t* data)
{
  auto md = reinterpret_cast<bin_storage_220::ModelData*>(data);
  for (auto& screen : md->screenData) {

    if (!strnlen(screen.LayoutId, sizeof(screen.LayoutId)))
      break;

    patchWidgetOptions(screen.layoutData.zones, MAX_LAYOUT_ZONES);
  }

  patchWidgetOptions(md->topbarData.zones, MAX_TOPBAR_ZONES);
}
#else
  #define patchModelData nullptr
#endif

const char* convertModelData_220_to_221(const char* path)
{
  constexpr unsigned md_size = sizeof(bin_storage_220::ModelData);
  return convertData_220_to_221(path, md_size,
                                yaml_conv_220::get_modeldata_nodes(),
                                patchModelData);
}

static void patchRadioData(uint8_t* data)
{
  auto rd = reinterpret_cast<bin_storage_220::RadioData*>(data);
  rd->version = 221;
  patchFilenameToYaml(rd->currModelFilename);
}

const char* convertRadioData_220_to_221(const char* path)
{
  constexpr unsigned rd_size = sizeof(bin_storage_220::RadioData);
  return convertData_220_to_221(path, rd_size,
                                yaml_conv_220::get_radiodata_nodes(),
                                patchRadioData);
}

#else

#include <storage/eeprom_common.h>


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

#endif
