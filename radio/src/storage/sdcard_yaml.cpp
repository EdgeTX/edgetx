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

#include "opentx.h"
#include "storage.h"
#include "sdcard_common.h"

#include "modelslist.h"

#include "yaml/yaml_tree_walker.h"
#include "yaml/yaml_parser.h"
#include "yaml/yaml_datastructs.h"

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

void storageCreateModelsList()
{
    modelslist.clear();
    ModelsCategory* cat = modelslist.createCategory(DEFAULT_CATEGORY, false);
    cat->addModel(DEFAULT_MODEL_FILENAME);
    modelslist.save();
}

//
// SDCARD storage interface
//


const char * loadRadioSettings()
{
    // YAML reader
    TRACE("YAML radio settings reader");

    YamlTreeWalker tree;
    tree.reset(get_radiodata_nodes(), (uint8_t*)&g_eeGeneral);

    return readYamlFile(RADIO_SETTINGS_YAML_PATH, YamlTreeWalker::get_parser_calls(), &tree);
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


const char * readModel(const char * filename, uint8_t * buffer, uint32_t size, uint8_t * version)
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

    //#if defined(FLIGHT_MODES) && defined(GVARS)
    // reset GVars to default values
    // Note: taken from opentx.cpp::modelDefault()
    //TODO: new func in gvars
    for (int p=1; p<MAX_FLIGHT_MODES; p++) {
        for (int i=0; i<MAX_GVARS; i++) {
            g_model.flightModeData[p].gvars[i] = GVAR_MAX+1;
        }
    }
    //#endif
    
    return readYamlFile(path, YamlTreeWalker::get_parser_calls(), &tree);
}

const char * writeModel()
{
    // YAML reader
    TRACE("YAML model writer");

    char path[256];
    getModelPath(path, g_eeGeneral.currModelFilename);

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
