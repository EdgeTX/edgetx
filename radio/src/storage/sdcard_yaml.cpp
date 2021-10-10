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
#include "opentx_helpers.h"
#include "storage.h"
#include "sdcard_common.h"
#include "sdcard_raw.h"

#include "modelslist.h"

#include "yaml/yaml_tree_walker.h"
#include "yaml/yaml_parser.h"
#include "yaml/yaml_datastructs.h"

#if defined(EEPROM_RLC)
 #include "storage/eeprom_rlc.h"
#endif

const char * readYamlFile(const char* fullpath, const YamlParserCalls* calls, void* parser_ctx)
{
    FIL  file;
    UINT bytes_read;

    FRESULT result = f_open(&file, fullpath, FA_OPEN_EXISTING | FA_READ);
    if (result != FR_OK) {
        return SDCARD_ERROR(result);
    }
      
    YamlParser yp; //TODO: move to re-usable buffer
    yp.init(calls, parser_ctx);

    char buffer[32];
    while (f_read(&file, buffer, sizeof(buffer), &bytes_read) == FR_OK) {

      // reached EOF?
      if (bytes_read == 0)
        break;
      
      if (yp.parse(buffer, bytes_read) != YamlParser::CONTINUE_PARSING)
        break;
    }

    f_close(&file);
    return NULL;
}


//
// Generic storage interface
//

#if !defined(EEPROM_RLC)
void storageCreateModelsList()
{
    modelslist.clear();
    ModelsCategory* cat = modelslist.createCategory(DEFAULT_CATEGORY, false);
    cat->addModel(DEFAULT_MODEL_FILENAME);
    modelslist.save();
}
#endif

//
// SDCARD storage interface
//

const char * loadRadioSettingsYaml()
{
    // YAML reader
    TRACE("YAML radio settings reader");

    YamlTreeWalker tree;
    tree.reset(get_radiodata_nodes(), (uint8_t*)&g_eeGeneral);

    return readYamlFile(RADIO_SETTINGS_YAML_PATH, YamlTreeWalker::get_parser_calls(), &tree);
}

const char * loadRadioSettings()
{
    FILINFO fno;
    if (f_stat(RADIO_SETTINGS_YAML_PATH, &fno) != FR_OK) {
#if !defined(EEPROM_RLC)
      return loadRadioSettingsBin();
#else
      // Load from EEPROM
      if (!eepromOpen() || !eeLoadGeneral(true)) {
        return "ERROR";
      }
      return nullptr;
#endif
    }

    return loadRadioSettingsYaml();
}

struct yaml_writer_ctx {
    FIL*    file;
    FRESULT result;
};

static bool yaml_writer(void* opaque, const char* str, size_t len)
{
    UINT bytes_written;
    yaml_writer_ctx* ctx = (yaml_writer_ctx*)opaque;

    TRACE_NOCRLF("%.*s",len,str);

    ctx->result = f_write(ctx->file, str, len, &bytes_written);
    return (ctx->result == FR_OK) && (bytes_written == len);
}

const char * writeGeneralSettings()
{
    // YAML reader
    TRACE("YAML radio settings writer");

    FIL file;

    FRESULT result = f_open(&file, RADIO_SETTINGS_YAML_PATH, FA_CREATE_ALWAYS | FA_WRITE);
    if (result != FR_OK) {
        return SDCARD_ERROR(result);
    }
      
    YamlTreeWalker tree;
    tree.reset(get_radiodata_nodes(), (uint8_t*)&g_eeGeneral);

    yaml_writer_ctx ctx;
    ctx.file = &file;
    ctx.result = FR_OK;
    
    if (!tree.generate(yaml_writer, &ctx)) {
        if (ctx.result != FR_OK) {
            f_close(&file);
            return SDCARD_ERROR(ctx.result);
        }
    }

    f_close(&file);
    return NULL;
}


const char * readModelYaml(const char * filename, uint8_t * buffer, uint32_t size, uint8_t * version)
{
    // YAML reader
    TRACE("YAML model reader");

    const YamlNode* data_nodes = nullptr;
    if (size == sizeof(g_model)) {
        data_nodes = get_modeldata_nodes();
    }
    else if (size == sizeof(PartialModel)) {
        data_nodes = get_partialmodel_nodes();
    }
    else {
        TRACE("cannot find YAML data nodes for object size (size=%d)", size);
        return "YAML size error";
    }
    
    char path[256];
    getModelPath(path, filename);

    YamlTreeWalker tree;
    tree.reset(data_nodes, buffer);

    // wipe memory before reading YAML
    memset(buffer,0,size);

#if defined(FLIGHT_MODES) && defined(GVARS)
    // reset GVars to default values
    // Note: taken from opentx.cpp::modelDefault()
    //TODO: new func in gvars
    for (int p=1; p<MAX_FLIGHT_MODES; p++) {
        for (int i=0; i<MAX_GVARS; i++) {
            g_model.flightModeData[p].gvars[i] = GVAR_MAX+1;
        }
    }
#endif

    *version = 255; // max version number
    return readYamlFile(path, YamlTreeWalker::get_parser_calls(), &tree);
}

const char* readModel(const char* filename, uint8_t* buffer, uint32_t size,
                      uint8_t* version)
{
  const char* ext = strrchr(filename, '.');
  if (ext != nullptr) {
    if (!strncmp(ext, YAML_EXT, 4)) {
      return readModelYaml(filename, buffer, size, version);
    }
#if !defined(EEPROM_RLC)
    return readModelBin(filename, buffer, size, version);
#endif
  }
  return nullptr;
}

const char * writeModelYaml(const char* filename)
{
    // YAML reader
    TRACE("YAML model writer");

    char path[256];
    getModelPath(path, filename);

    FIL file;

    FRESULT result = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE);
    if (result != FR_OK) {
        return SDCARD_ERROR(result);
    }
      
    YamlTreeWalker tree;
    tree.reset(get_modeldata_nodes(), (uint8_t*)&g_model);

    yaml_writer_ctx ctx;
    ctx.file = &file;
    ctx.result = FR_OK;
    
    if (!tree.generate(yaml_writer, &ctx)) {
        if (ctx.result != FR_OK) {
            f_close(&file);
            return SDCARD_ERROR(ctx.result);
        }
    }

    f_close(&file);
    return NULL;
}

#if !defined(STORAGE_MODELSLIST)
// EEPROM slot simulation based on file names:
// - /MODELS/model[00-99].yml

constexpr uint8_t MODELIDX_STRLEN = sizeof(MODEL_FILENAME_PREFIX "00");

static void getModelNumberStr(uint8_t idx, char* model_idx)
{
  memcpy(model_idx, MODEL_FILENAME_PREFIX, sizeof(MODEL_FILENAME_PREFIX));
  model_idx[sizeof(MODEL_FILENAME_PREFIX)-1] = '0' + idx / 10;
  model_idx[sizeof(MODEL_FILENAME_PREFIX)]   = '0' + idx % 10;
  model_idx[sizeof(MODEL_FILENAME_PREFIX)+1] = '\0';
}
#endif

const char * writeModel()
{
#if defined(STORAGE_MODELSLIST)
  return writeModelYaml(g_eeGeneral.currModelFilename);
#else
  char fname[MODELIDX_STRLEN + sizeof(YAML_EXT)];
  getModelNumberStr(g_eeGeneral.currModel, fname);
  strcat(fname, YAML_EXT);
  return writeModelYaml(fname);
#endif
}

#if !defined(STORAGE_MODELSLIST)
void loadModelHeader(uint8_t id, ModelHeader* header)
{
  PartialModel partial;
  memclear(&partial, sizeof(PartialModel));

  if (modelExists(id)) {
    uint8_t dummy;
    char fname[MODELIDX_STRLEN + sizeof(YAML_EXT)];
    getModelNumberStr(id, fname);
    strcat(fname, YAML_EXT);
    readModelYaml(fname, reinterpret_cast<uint8_t*>(&partial), sizeof(partial), &dummy);
    memcpy(header, &partial, sizeof(ModelHeader));
  }
}

const char * loadModel(uint8_t idx, bool alarms)
{
  char fname[MODELIDX_STRLEN + sizeof(YAML_EXT)];
  getModelNumberStr(idx, fname);
  strcat(fname, YAML_EXT);
  return loadModel(fname, alarms);
}

bool modelExists(uint8_t idx)
{
  char model_idx[MODELIDX_STRLEN];
  getModelNumberStr(idx, model_idx);
  GET_FILENAME(fname, MODELS_PATH, model_idx, YAML_EXT);

  FILINFO fno;
  return f_stat(fname, &fno) == FR_OK;
}

bool copyModel(uint8_t dst, uint8_t src)
{
  // TODO: overwrite possible?
  char model_idx_src[MODELIDX_STRLEN];
  char model_idx_dst[MODELIDX_STRLEN];
  getModelNumberStr(src, model_idx_src);
  getModelNumberStr(dst, model_idx_dst);
  
  GET_FILENAME(fname_src, MODELS_PATH, model_idx_src, YAML_EXT);
  GET_FILENAME(fname_dst, MODELS_PATH, model_idx_dst, YAML_EXT);

  return sdCopyFile(fname_src, fname_dst);
}

void swapModels(uint8_t id1, uint8_t id2)
{
  //TODO
}

int8_t deleteModel(uint8_t idx)
{
  char model_idx[MODELIDX_STRLEN];
  getModelNumberStr(idx, model_idx);
  
  GET_FILENAME(fname, MODELS_PATH PATH_SEPARATOR MODEL_FILENAME_PREFIX,
               model_idx, YAML_EXT);

  return f_unlink(fname) == FR_OK ? 0 : -1;
}
#endif
