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

#include "edgetx.h"
#include "modelslist.h"
#include "conversions/conversions.h"

const char * writeFile(const char * filename, const uint8_t * data, uint16_t size)
{
  TRACE("writeFile(%s)", filename);

  FIL file;
  unsigned char buf[8];
  UINT written;

  FRESULT result = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE);
  if (result != FR_OK) {
    return SDCARD_ERROR(result);
  }

  *(uint32_t*)&buf[0] = OTX_FOURCC;
  buf[4] = EEPROM_VER;
  buf[5] = 'M';
  *(uint16_t*)&buf[6] = size;

  result = f_write(&file, buf, 8, &written);
  if (result != FR_OK || written != 8) {
    f_close(&file);
    return SDCARD_ERROR(result);
  }

  result = f_write(&file, data, size, &written);
  if (result != FR_OK || written != size) {
    f_close(&file);
    return SDCARD_ERROR(result);
  }

  f_close(&file);
  return NULL;
}

const char * writeModel()
{
  char path[256];
  getModelPath(path, g_eeGeneral.currModelFilename);

  sdCheckAndCreateDirectory(MODELS_PATH);
  return writeFile(path, (uint8_t *)&g_model, sizeof(g_model));
}

const char * openFile(const char * fullpath, FIL * file, uint16_t * size, uint8_t * version)
{
  FRESULT result = f_open(file, fullpath, FA_OPEN_EXISTING | FA_READ);
  if (result != FR_OK) {
    return SDCARD_ERROR(result);
  }

  if (f_size(file) < 8) {
    f_close(file);
    return STR_INCOMPATIBLE;
  }

  UINT read;
  char buf[8];

  result = f_read(file, (uint8_t *)buf, sizeof(buf), &read);
  if ((result != FR_OK) || (read != sizeof(buf))) {
    f_close(file);
    return SDCARD_ERROR(result);
  }

  //TODO: move this code into some checkCompatibleFormat()

  *version = (uint8_t)buf[4];
  if (*(uint32_t*)&buf[0] != OTX_FOURCC || *version < FIRST_CONV_EEPROM_VER || *version > EEPROM_VER || buf[5] != 'M') {
    f_close(file);
    return STR_INCOMPATIBLE;
  }

  *size = *(uint16_t*)&buf[6];
  return nullptr;
}

const char * loadFile(const char * fullpath, uint8_t * data, uint16_t maxsize, uint8_t * version)
{
  FIL      file;
  UINT     read;
  uint16_t size;

  TRACE("loadFile(%s)", fullpath);

  const char * err = openFile(fullpath, &file, &size, version);
  if (err)
    return err;

  size = min<uint16_t>(maxsize, size);
  FRESULT result = f_read(&file, data, size, &read);
  if (result != FR_OK || read != size) {
    f_close(&file);
    return SDCARD_ERROR(result);
  }

  f_close(&file);
  return nullptr;
}

const char * readModel(const char * filename, uint8_t * buffer, uint32_t size, uint8_t * version)
{
  char path[256];
  getModelPath(path, filename);
  return loadFile(path, buffer, size, version);
}


const char * loadRadioSettings(const char * path)
{
  uint8_t version;
  const char * error = loadFile(path, (uint8_t *)&g_eeGeneral, sizeof(g_eeGeneral), &version);
  if (error) {
    TRACE("loadRadioSettings error=%s", error);
    return error;
  }

#if defined(STORAGE_CONVERSIONS)
  if (version < EEPROM_VER) {
    convertRadioData(version);
  }
#endif

  postRadioSettingsLoad();

  return nullptr;
}

const char * loadRadioSettings()
{
  return loadRadioSettings(RADIO_SETTINGS_PATH);
}

const char * writeGeneralSettings()
{
  return writeFile(RADIO_SETTINGS_PATH, (uint8_t *)&g_eeGeneral, sizeof(g_eeGeneral));
}

void storageCreateModelsList()
{
  FIL file;

  FRESULT result = f_open(&file, RADIO_MODELSLIST_PATH, FA_CREATE_ALWAYS | FA_WRITE);
  if (result == FR_OK) {
    f_puts("[" DEFAULT_CATEGORY "]\n" DEFAULT_MODEL_FILENAME "\n", &file);
    f_close(&file);
  }
}
