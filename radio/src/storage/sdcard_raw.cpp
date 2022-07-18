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
#include "VirtualFS.h"
#include "modelslist.h"
#include "conversions/conversions.h"

const char *writeFileBin(const char *filename, const uint8_t *data,
                         uint16_t size, uint8_t version)
{
  TRACE("writeFileBin(%s)", filename);

  VfsFile file;
  unsigned char buf[8];
  size_t written;

  VfsError result = VirtualFS::instance().openFile(file, filename, VfsOpenFlags::CREATE_ALWAYS | VfsOpenFlags::WRITE);
  if (result != VfsError::OK) {
    return STORAGE_ERROR(result);
  }

  *(uint32_t*)&buf[0] = ETX_FOURCC;
  buf[4] = version;
  buf[5] = 'M';
  *(uint16_t*)&buf[6] = size;

  result = file.write(buf, 8, written);
  if (result != VfsError::OK || written != 8) {
    file.close();
    return STORAGE_ERROR(result);
  }

  result = file.write(data, size, written);
  if (result != VfsError::OK || written != size) {
    file.close();
    return STORAGE_ERROR(result);
  }

  file.close();
  return NULL;
}

// TODO: move to "common"
const char * writeModelBin()
{
  char path[256];
  getModelPath(path, g_eeGeneral.currModelFilename);

  VirtualFS::instance().checkAndCreateDirectory(MODELS_PATH);
  return writeFileBin(path, (uint8_t *)&g_model, sizeof(g_model));
}

// TODO: move partly to common
const char * openFile(const char * fullpath, VfsFile& file)
{
  VfsError result = VirtualFS::instance().openFile(file, fullpath, VfsOpenFlags::OPEN_EXISTING | VfsOpenFlags::READ);
  if (result != VfsError::OK) {
    return STORAGE_ERROR(result);
  }

  return nullptr;
}

const char* openFileBin(const char * fullpath, VfsFile& file, uint16_t&  size, uint8_t * version)
{
  const char* err = openFile(fullpath, file);
  if (err) return err;

  if (file.size() < 8) {
    file.close();
    return STR_INCOMPATIBLE;
  }

  size_t read;
  char buf[8];

  VfsError result = file.read((uint8_t *)buf, sizeof(buf), read);
  if ((result != VfsError::OK) || (read != sizeof(buf))) {
    file.close();
    return STORAGE_ERROR(result);
  }

  *version = (uint8_t)buf[4];
  if (*(uint32_t *)&buf[0] != ETX_FOURCC || *version < FIRST_CONV_EEPROM_VER ||
      *version > EEPROM_VER || buf[5] != 'M') {
    file.close();
    return STR_INCOMPATIBLE;
  }

  size = *(uint16_t*)&buf[6];
  return nullptr;
}

const char *loadFileBin(const char *fullpath, uint8_t *data,
                        uint16_t maxsize, uint8_t *version)
{
  VfsFile  file;
  size_t   read;
  uint16_t size;

  TRACE("loadFileBin(%s)", fullpath);

  const char * err = openFileBin(fullpath, file, size, version);
  if (err)
    return err;

  size = min<uint16_t>(maxsize, size);
  VfsError result = file.read(data, size, read);
  if (result != VfsError::OK || read != size) {
    file.close();
    return STORAGE_ERROR(result);
  }

  file.close();
  return nullptr;
}

#if defined(SDCARD_RAW)
const char* readModel(const char* filename, uint8_t* buffer, uint32_t size)
{
  uint8_t version = 0;
  // Conversions from EEPROM are done in batch when converting the radio file.
  // It is not supported on a model by model base when loaded.
  const char *error = readModelBin(filename, buffer, size, &version);
  if (error) return error;

#if defined(STORAGE_CONVERSIONS)
  if (version < EEPROM_VER) {
    convertBinModelData(filename, version);
    error = readModelBin(filename, buffer, size, &version);
  }
#endif

  return error;
}
#endif

const char *readModelBin(const char *filename, uint8_t *buffer, uint32_t size,
                         uint8_t *version)
{
  char path[256];
  getModelPath(path, filename);
  return loadFileBin(path, buffer, size, version);
}

const char * loadRadioSettingsBin(const char * path)
{
  uint8_t version;
  const char * error = loadFileBin(path, (uint8_t *)&g_eeGeneral, sizeof(g_eeGeneral), &version);
  if (error) {
    TRACE("loadRadioSettings error=%s", error);
    return error;
  }

#if defined(STORAGE_CONVERSIONS)
  if (version < EEPROM_VER) {
    convertBinRadioData(path, version);
  }
#endif

  postRadioSettingsLoad();

  return nullptr;
}

const char * loadRadioSettingsBin()
{
  return loadRadioSettingsBin(RADIO_SETTINGS_PATH);
}

const char * writeGeneralSettingsBin()
{
  return writeFileBin(RADIO_SETTINGS_PATH, (uint8_t *)&g_eeGeneral, sizeof(g_eeGeneral));
}

#if !defined(SDCARD_YAML)
void storageCreateModelsList()
{
  VfsFile file;

  VfsError result = VirtualFS::instance().openFile(file, RADIO_MODELSLIST_PATH, VfsOpenFlags::CREATE_ALWAYS | VfsOpenFlags::WRITE);
  if (result == VfsError::OK) {
    file.puts("[" DEFAULT_CATEGORY "]\n" DEFAULT_MODEL_FILENAME "\n");
    file.close();
  }
}
#endif
