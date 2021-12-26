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
#include "sdcard_yaml.h"
#include "modelslist.h"
#include "VirtualFS.h"

#include "yaml/yaml_tree_walker.h"
#include "yaml/yaml_parser.h"
#include "yaml/yaml_datastructs.h"
#include "yaml/yaml_bits.h"


#if defined(EEPROM_RLC)
 #include "storage/eeprom_common.h"
 #include "storage/eeprom_rlc.h"
#endif

const char * readYamlFile(const char* fullpath, const YamlParserCalls* calls, void* parser_ctx, ChecksumResult* checksum_result)
{
    VfsFile  file;
    size_t bytes_read;
    uint32_t total_bytes = 0;

    VirtualFS& vfs = VirtualFS::instance();
    VfsError result = vfs.openFile(file, fullpath, VfsOpenFlags::OPEN_EXISTING | VfsOpenFlags::READ);
    if (result != VfsError::OK) {
        return STORAGE_ERROR(result);
    }

    YamlParser yp; //TODO: move to re-usable buffer
    yp.init(calls, parser_ctx);

    uint16_t calculated_checksum = 0xFFFF;
    uint16_t file_checksum = 0;

    bool first_block = true;
    char buffer[32];
    while (file.read(buffer, sizeof(buffer)-1, bytes_read) == VfsError::OK) {
      if (bytes_read == 0)  // EOF
        break;
      total_bytes += bytes_read;

      uint16_t skip = 0;
      if(first_block) {
        // Get the 'checksum' value and skip from further YAML processing
        // The checksum must be first in the first buffer read from file
        first_block = false;
        const char *skipValue = "checksum: ";
        if(strncmp(buffer, skipValue, strlen(skipValue)) == 0) {
          skip = 10;
          char* startPos = buffer + strlen(skipValue);
          char* endPos = startPos;
          // Advance through the value
          while((*endPos != '\r') && (*endPos != '\n')) {
            if (endPos > buffer + bytes_read) {
              return STORAGE_ERROR(	VfsError::INVAL );
            }
            endPos++;
          }
          // Skip trailing newline
          while((*endPos == '\r') || (*endPos == '\n')) {
            *endPos = 0;
            endPos++;
          }

          file_checksum = atoi(startPos);
          skip = endPos - buffer;
        }
      }

      // Calculate checksum on read block only if we are called with a pointer to write the resulting checksum
      if (checksum_result != NULL) {
        calculated_checksum = crc16(0, (const uint8_t *)buffer + skip, bytes_read - skip, calculated_checksum);
      }

      if (file.eof()) yp.set_eof();
      if (yp.parse(buffer + skip, bytes_read - skip) != YamlParser::CONTINUE_PARSING)
        break;
    }
    file.close();

    if (checksum_result != NULL) {
      // Special case to handle "old" files with no checksum field
      // 25 was arbitrarily chosen as the minimum realistic file size
      // - The issue is to allow old files to pass, while still detecting garbled files
      if ( (file_checksum == 0) && (total_bytes > 25) ) {
        *checksum_result = ChecksumResult::Success;
      } else {
        // Normal case - compare read and calculated checksum
        if (calculated_checksum == file_checksum) {
          *checksum_result = ChecksumResult::Success;
        } else {
          *checksum_result = ChecksumResult::Failed;
        }
      }
    }

    return NULL;
}

//
// SDCARD storage interface
//

static const char * attemptLoad(const char *filename, ChecksumResult* checksum_status)
{
  YamlTreeWalker tree;
  tree.reset(get_radiodata_nodes(), (uint8_t*)&g_eeGeneral);
  return readYamlFile(filename, YamlTreeWalker::get_parser_calls(), &tree, checksum_status);
}

const char * loadRadioSettingsYaml(bool checks)
{
    // YAML reader
    TRACE("YAML radio settings reader");

    ChecksumResult checksum_status;
    const char* p = attemptLoad(RADIO_SETTINGS_YAML_PATH, &checksum_status);

    if(!checks)
      return p;

    if((p != NULL) || (checksum_status != ChecksumResult::Success) ) {
      VirtualFS &vfs = VirtualFS::instance();
      // Read failed or checksum check failed
      VfsError result = VfsError::OK;
      TRACE("radio settings: Reading failed");
      if ( (p == NULL) && g_eeGeneral.manuallyEdited) {
        // Read sussessfull, checksum failed, manuallyEdited set
        TRACE("File has been manually edited - ignoring checksum mismatch");
        g_eeGeneral.manuallyEdited = 0;
        storageDirty(EE_GENERAL);   // Trigger a save on sucessfull recovery
      } else {
        TRACE("File is corrupted, attempting alternative file");
        vfs.unlink(RADIO_SETTINGS_ERRORFILE_YAML_PATH);
        result = vfs.rename(RADIO_SETTINGS_YAML_PATH, RADIO_SETTINGS_ERRORFILE_YAML_PATH); // Save corrupted file for later analysis
        p = attemptLoad(RADIO_SETTINGS_TMPFILE_YAML_PATH, &checksum_status);
        if (p == NULL && (checksum_status == ChecksumResult::Success)) {
            vfs.unlink(RADIO_SETTINGS_YAML_PATH);
            result = vfs.rename(RADIO_SETTINGS_TMPFILE_YAML_PATH, RADIO_SETTINGS_YAML_PATH);  // Rename previously saved file to active file
            if (result != VfsError::OK) {
              ALERT(STR_STORAGE_WARNING, TR_RADIO_DATA_UNRECOVERABLE, AU_BAD_RADIODATA);
              return STORAGE_ERROR(result);
            }
        }
        TRACE("Unable to recover radio data");
        ALERT(STR_STORAGE_WARNING, p == NULL ? TR_RADIO_DATA_RECOVERED : TR_RADIO_DATA_UNRECOVERABLE, AU_BAD_RADIODATA);
      }
    }
    return p;
}

const char * loadRadioSettings()
{
    VfsFileInfo fno;
    VirtualFS& vfs = VirtualFS::instance();
    if ( (vfs.fstat(RADIO_SETTINGS_YAML_PATH, fno) != VfsError::OK) && ((vfs.fstat(RADIO_SETTINGS_TMPFILE_YAML_PATH, fno) != VfsError::OK)) ) {
      // If neither the radio configuraion YAML file or the temporary file generated on write exist, this must be a first run with YAML support.
      // - thus requiring a conversion from binary to YAML.
      return "no radio settings";
    }

#if defined(DEFAULT_INTERNAL_MODULE)
    g_eeGeneral.internalModule = DEFAULT_INTERNAL_MODULE;
#endif

    const char* error = loadRadioSettingsYaml(true);
    if (!error) {
      g_eeGeneral.chkSum = evalChkSum();
    }
    postRadioSettingsLoad();

    return error;
}



struct yaml_checksummer_ctx {
    VfsError result;
    uint16_t checksum;
    bool checksum_invalid;
};

static bool yaml_checksummer(void* opaque, const char* str, size_t len)
{
    yaml_checksummer_ctx* ctx = (yaml_checksummer_ctx*)opaque;

    ctx->checksum = crc16(0, (const uint8_t *) str, len, ctx->checksum);
    return true;
}

bool YamlFileChecksum(const YamlNode* root_node, uint8_t* data, uint16_t* checksum)
{
    YamlTreeWalker tree;
    tree.reset(root_node, data);

    yaml_checksummer_ctx ctx;
    ctx.result = VfsError::OK;
    ctx.checksum = 0xFFFF;
    ctx.checksum_invalid = false;

    if (!tree.generate(yaml_checksummer, &ctx)) {
        if (ctx.result != VfsError::OK) {
          ctx.checksum_invalid = true;
          return false;
        }
    }

    if(checksum != NULL) {
      *checksum = ctx.checksum;
    }

    return true;
}


struct yaml_writer_ctx {
    VfsFile* file;
    VfsError result;
};

static bool yaml_writer(void* opaque, const char* str, size_t len)
{
    size_t bytes_written;
    yaml_writer_ctx* ctx = (yaml_writer_ctx*)opaque;

#if defined(DEBUG_YAML)
    TRACE_NOCRLF("%.*s",len,str);
#endif

    ctx->result = ctx->file->write(str, len, bytes_written);
    return (ctx->result == VfsError::OK) && (bytes_written == len);
}

const char* writeFileYaml(const char* path, const YamlNode* root_node, uint8_t* data, uint16_t checksum)
{
    VfsFile file;

    VirtualFS::instance().unlink(path);
    VfsError result = VirtualFS::instance().openFile(file, path, VfsOpenFlags::CREATE_ALWAYS | VfsOpenFlags::WRITE);
    if (result != VfsError::OK) {
        return STORAGE_ERROR(result);
    }
    YamlTreeWalker tree;
    tree.reset(root_node, data);

    yaml_writer_ctx ctx;
    ctx.file = &file;
    ctx.result = VfsError::OK;

    // Try to add CRC
    if (checksum != 0) {
      if (!yaml_writer(&ctx, YAMLFILE_CHECKSUM_TAG_NAME, strlen(YAMLFILE_CHECKSUM_TAG_NAME))) return NULL;
      if (!yaml_writer(&ctx, ": ", 2)) return STORAGE_ERROR(VfsError::INVAL);
      const char* p_out = NULL;
      p_out = yaml_unsigned2str((int)checksum);
      if (p_out && !yaml_writer(&ctx, p_out, strlen(p_out))) return STORAGE_ERROR(VfsError::INVAL);
      yaml_writer(&ctx, "\r\n", 2);
    }


    if (!tree.generate(yaml_writer, &ctx)) {
        if (ctx.result != VfsError::OK) {
            file.close();
            return STORAGE_ERROR(ctx.result);
        }
    }

    file.close();
    return NULL;
}

const char * writeGeneralSettings()
{
    TRACE("YAML radio settings writer");
	VirtualFS &vfs = VirtualFS::instance();
    uint16_t file_checksum = 0;

    YamlFileChecksum(get_radiodata_nodes(), (uint8_t*)&g_eeGeneral, &file_checksum);
    g_eeGeneral.manuallyEdited = false;

    const char *p = writeFileYaml(RADIO_SETTINGS_TMPFILE_YAML_PATH, get_radiodata_nodes(),
                         (uint8_t*)&g_eeGeneral, file_checksum);
    TRACE("generalSettings written with checksum %u", file_checksum);

    if (p != NULL) {
        return p;
    }
    vfs.unlink(RADIO_SETTINGS_YAML_PATH);

    VfsError result = vfs.rename(RADIO_SETTINGS_TMPFILE_YAML_PATH, RADIO_SETTINGS_YAML_PATH);
    if(result != VfsError::OK)
        return STORAGE_ERROR(result);

    return nullptr;
}


const char * readModelYaml(const char * filename, uint8_t * buffer, uint32_t size, const char* pathName)
{
    // YAML reader
    TRACE("YAML model reader");

    bool init_model = true;
    const YamlNode* data_nodes = nullptr;
    if (size == sizeof(g_model)) {
        data_nodes = get_modeldata_nodes();
    }
    else if (size == sizeof(PartialModel)) {
        data_nodes = get_partialmodel_nodes();
        init_model = false;
    }
    else {
        TRACE("cannot find YAML data nodes for object size (size=%d)", size);
        return "YAML size error";
    }

    char path[256];
    getModelPath(path, filename, pathName);

    YamlTreeWalker tree;
    tree.reset(data_nodes, buffer);

    // wipe memory before reading YAML
    memset(buffer,0,size);

    if (init_model) {
      auto md = reinterpret_cast<ModelData*>(buffer);
#if defined(FLIGHT_MODES) && defined(GVARS)
      // reset GVars to default values
      // Note: taken from opentx.cpp::modelDefault()
      //TODO: new func in gvars
      for (int p=1; p<MAX_FLIGHT_MODES; p++) {
        for (int i=0; i<MAX_GVARS; i++) {
          md->flightModeData[p].gvars[i] = GVAR_MAX+1;
        }
      }
#endif
      // is that necessary ???
      // md->swashR.collectiveWeight = 100;
      // md->swashR.aileronWeight    = 100;
      // md->swashR.elevatorWeight   = 100;

      md->rfAlarms.warning = 45;
      md->rfAlarms.critical = 42;
    }

    return readYamlFile(path, YamlTreeWalker::get_parser_calls(), &tree, NULL);
}

static const char _wrongExtentionError[] = "wrong file extension";

const char* readModel(const char* filename, uint8_t* buffer, uint32_t size, const char* pathName)
{
  const char* ext = strrchr(filename, '.');
  if (!ext || strncmp(ext, YAML_EXT, 4) != 0) {
    return _wrongExtentionError;
  }

  return readModelYaml(filename, buffer, size, pathName);
}

const char * writeModelYaml(const char* filename)
{
    TRACE("YAML model writer");
    char path[256];
    getModelPath(path, filename);
    return writeFileYaml(path, get_modeldata_nodes(), (uint8_t*)&g_model,0 );
}

#if !defined(STORAGE_MODELSLIST)
// EEPROM slot simulation based on file names:
// - /MODELS/model[00-99].yml

void getModelNumberStr(uint8_t idx, char* model_idx)
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
    char fname[MODELIDX_STRLEN + sizeof(YAML_EXT)];
    getModelNumberStr(id, fname);
    strcat(fname, YAML_EXT);
    readModelYaml(fname, reinterpret_cast<uint8_t*>(&partial), sizeof(partial));
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

  VfsFileInfo fno;
  return VirtualFS::instance().fstat(fname, fno) == VfsError::OK;
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

  return VirtualFS::instance().copyFile(fname_src, fname_dst) == VfsError::OK;
}

static void swapModelHeaders(uint8_t id1, uint8_t id2)
{
  char tmp[sizeof(g_model.header)];
  memcpy(tmp, &modelHeaders[id1], sizeof(ModelHeader));
  memcpy(&modelHeaders[id1], &modelHeaders[id2], sizeof(ModelHeader));
  memcpy(&modelHeaders[id2], tmp, sizeof(ModelHeader));
}

void swapModels(uint8_t id1, uint8_t id2)
{
  char model_idx_1[MODELIDX_STRLEN];
  char model_idx_2[MODELIDX_STRLEN];
  getModelNumberStr(id1, model_idx_1);
  getModelNumberStr(id2, model_idx_2);

  GET_FILENAME(fname1, MODELS_PATH, model_idx_1, YAML_EXT);
  GET_FILENAME(fname1_tmp, MODELS_PATH, model_idx_1, ".tmp");
  GET_FILENAME(fname2, MODELS_PATH, model_idx_2, YAML_EXT);

  VirtualFS &vfs = VirtualFS::instance();
  VfsFileInfo fno;
  if (vfs.fstat(fname2,fno) != VfsError::OK) {
    if (vfs.fstat(fname1,fno) == VfsError::OK) {
      if (vfs.rename(fname1, fname2) == VfsError::OK)
        swapModelHeaders(id1,id2);
    }
    return;
  }

  if (vfs.fstat(fname1,fno) != VfsError::OK) {
    vfs.rename(fname2, fname1);
    return;
  }

  // just in case...
  vfs.unlink(fname1_tmp);

  if (vfs.rename(fname1, fname1_tmp) != VfsError::OK) {
    TRACE("Error renaming 1");
    return;
  }

  if (vfs.rename(fname2, fname1) != VfsError::OK) {
    TRACE("Error renaming 2");
    return;
  }

  if (vfs.rename(fname1_tmp, fname2) != VfsError::OK) {
    TRACE("Error renaming 1 tmp");
    return;
  }

  swapModelHeaders(id1,id2);
}

int8_t deleteModel(uint8_t idx)
{
  char model_idx[MODELIDX_STRLEN];
  getModelNumberStr(idx, model_idx);
  GET_FILENAME(fname, MODELS_PATH, model_idx, YAML_EXT);

  if (VirtualFS::instance().unlink(fname) != VfsError::OK) {
    return -1;
  }

  modelHeaders[idx].name[0] = '\0';
  return 0;
}

const char * backupModel(uint8_t idx)
{
  char * buf = reusableBuffer.modelsel.mainname;

  // check and create folder here
  const char * error = VirtualFS::instance().checkAndCreateDirectory(STR_BACKUP_PATH);
  if (error) {
    return error;
  }

  strncpy(buf, modelHeaders[idx].name, sizeof(g_model.header.name));
  buf[sizeof(g_model.header.name)] = '\0';

  int8_t i = sizeof(g_model.header.name)-1;
  uint8_t len = 0;
  while (i > 0) {
    if (!len && buf[i])
      len = i+1;
    if (len) {
      if (!buf[i])
        buf[i] = '_';
    }
    i--;
  }

  if (len == 0) {
    uint8_t num = idx + 1;
    strcpy(buf, STR_MODEL);
    buf[PSIZE(TR_MODEL)] = (char)((num / 10) + '0');
    buf[PSIZE(TR_MODEL) + 1] = (char)((num % 10) + '0');
    len = PSIZE(TR_MODEL) + 2;
  }

#if defined(RTCLOCK)
  char * tmp = strAppendDate(&buf[len]);
  len = tmp - buf;
#endif

  strcpy(&buf[len], STR_YAML_EXT);

#ifdef SIMU
  TRACE("SD-card backup filename=%s", buf);
#endif

  char model_idx[MODELIDX_STRLEN + sizeof(YAML_EXT)];
  getModelNumberStr(idx, model_idx);
  strcat(model_idx, STR_YAML_EXT);

  return STORAGE_ERROR(VirtualFS::instance().copyFile(model_idx, STR_MODELS_PATH, buf, STR_BACKUP_PATH));
}

const char * restoreModel(uint8_t idx, char *model_name)
{
  char * buf = reusableBuffer.modelsel.mainname;
  strcpy(buf, model_name);
  strcpy(&buf[strlen(buf)], STR_YAML_EXT);

  char model_idx[MODELIDX_STRLEN + sizeof(YAML_EXT)];
  getModelNumberStr(idx, model_idx);
  strcat(model_idx, STR_YAML_EXT);

  const char* error = STORAGE_ERROR(VirtualFS::instance().copyFile(buf, STR_BACKUP_PATH, model_idx, STR_MODELS_PATH));
  if (!error) {
    loadModelHeader(idx, &modelHeaders[idx]);
  }

  return error;
}

#endif

bool storageReadRadioSettings(bool checks)
{
  return loadRadioSettingsYaml(checks) == nullptr;
}
