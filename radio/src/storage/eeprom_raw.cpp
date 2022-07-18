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

#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "opentx.h"
#include "timers.h"
#include "conversions/conversions.h"
#include "VirtualFS.h"

#define EEPROM_MARK           0x84697771 /* thanks ;) */
#define EEPROM_ZONE_SIZE      (8*1024)
#define EEPROM_BUFFER_SIZE    256
#define EEPROM_FAT_SIZE       128
#define EEPROM_MAX_ZONES      (EEPROM_SIZE / EEPROM_ZONE_SIZE)
#define EEPROM_MAX_FILES      (EEPROM_MAX_ZONES - 1)
#define FIRST_FILE_AVAILABLE  (1+MAX_MODELS)

PACK(struct EepromHeaderFile
{
  uint8_t zoneIndex:7;
  uint8_t exists:1;
});

PACK(struct EepromHeader
{
  uint32_t         mark;
  uint32_t         index;
  EepromHeaderFile files[EEPROM_MAX_FILES];
});

PACK(struct EepromFileHeader
{
  uint16_t fileIndex;
  uint16_t size;
});

EepromHeader eepromHeader __DMA;
volatile EepromWriteState eepromWriteState = EEPROM_IDLE;
uint8_t eepromWriteZoneIndex = FIRST_FILE_AVAILABLE;
uint8_t eepromWriteFileIndex;
uint16_t eepromWriteSize;
uint8_t * eepromWriteSourceAddr;
uint32_t eepromWriteDestinationAddr;
uint16_t eepromFatAddr = 0;
uint8_t eepromWriteBuffer[EEPROM_BUFFER_SIZE] __DMA;

void eepromWaitReadStatus()
{
  while (!eepromReadStatus()) { }
}

void eepromWaitTransferComplete()
{
  while (!eepromIsTransferComplete()) { }
}

void eepromEraseBlock(uint32_t address, bool blocking=true)
{
  // TRACE("eepromEraseBlock(%d)", address);

  eepromBlockErase(address);

  if (blocking) {
    eepromWaitTransferComplete();
    eepromWaitReadStatus();
  }
}

void eepromRead(uint8_t * buffer, size_t address, size_t size)
{
  // TRACE("eepromRead(%p, %d, %d)", buffer, address, size);

  eepromStartRead(buffer, address, size);
  eepromWaitTransferComplete();
}

void eepromWrite(uint8_t * buffer, size_t address, size_t size, bool blocking=true)
{
  // TRACE("eepromWrite(%p, %d, %d)", buffer, address, size);

  eepromStartWrite(buffer, address, size);

  if (blocking) {
    eepromWaitTransferComplete();
    eepromWaitReadStatus();
  }
}

bool eepromOpen()
{
  TRACE("eepromOpen");

  int32_t bestFatAddr = -1;
  uint32_t bestFatIndex = 0;
  eepromFatAddr = 0;
  while (eepromFatAddr < EEPROM_ZONE_SIZE) {
    eepromRead((uint8_t *)&eepromHeader, eepromFatAddr, sizeof(eepromHeader.mark) + sizeof(eepromHeader.index));
    if (eepromHeader.mark == EEPROM_MARK && eepromHeader.index >= bestFatIndex) {
      bestFatAddr = eepromFatAddr;
      bestFatIndex = eepromHeader.index;
    }
    eepromFatAddr += EEPROM_FAT_SIZE;
  }
  if (bestFatAddr >= 0) {
    eepromFatAddr = bestFatAddr;
    eepromRead((uint8_t *)&eepromHeader, eepromFatAddr, sizeof(eepromHeader));
    return true;
  }
  else {
    return false;
  }
}

uint32_t readFile(int index, uint8_t * data, uint32_t size)
{
  if (eepromHeader.files[index].exists) {
    EepromFileHeader header;
    uint32_t address = eepromHeader.files[index].zoneIndex * EEPROM_ZONE_SIZE;
    eepromRead((uint8_t *)&header, address, sizeof(header));
    if (size < header.size) {
      header.size = size;
    }
    if (header.size > 0) {
      eepromRead(data, address + sizeof(header), header.size);
      size -= header.size;
    }
    if (size > 0) {
      memset(data + header.size, 0, size);
    }
    return header.size;
  }
  else {
    return 0;
  }
}

void eepromIncFatAddr()
{
  eepromHeader.index += 1;
  eepromFatAddr += EEPROM_FAT_SIZE;
  if (eepromFatAddr >= EEPROM_ZONE_SIZE) {
    eepromFatAddr = 0;
  }
}

void writeFile(int index, uint8_t * data, uint32_t size)
{
  uint32_t zoneIndex = eepromHeader.files[eepromWriteZoneIndex].zoneIndex;
  eepromHeader.files[eepromWriteZoneIndex].exists = 0;
  eepromHeader.files[eepromWriteZoneIndex].zoneIndex = eepromHeader.files[index].zoneIndex;
  eepromHeader.files[index].exists = (size > 0);
  eepromHeader.files[index].zoneIndex = zoneIndex;
  eepromWriteFileIndex = index;
  eepromWriteSourceAddr = data;
  eepromWriteSize = size;
  eepromWriteDestinationAddr = zoneIndex * EEPROM_ZONE_SIZE;
  eepromWriteState = EEPROM_START_WRITE;
  eepromWriteZoneIndex += 1;
  if (eepromWriteZoneIndex >= EEPROM_MAX_FILES) {
    eepromWriteZoneIndex = FIRST_FILE_AVAILABLE;
  }
  eepromIncFatAddr();
}

void eeDeleteModel(uint8_t index)
{
  storageCheck(true);
  memclear(&modelHeaders[index], sizeof(ModelHeader));
  writeFile(index+1, (uint8_t *)&g_model, 0);
  eepromWriteWait();
}

bool eeCopyModel(uint8_t dst, uint8_t src)
{
  storageCheck(true);

  uint32_t eepromWriteSourceAddr = eepromHeader.files[src+1].zoneIndex * EEPROM_ZONE_SIZE;
  uint32_t eepromWriteDestinationAddr = eepromHeader.files[dst+1].zoneIndex * EEPROM_ZONE_SIZE;

  // erase blocks
  eepromEraseBlock(eepromWriteDestinationAddr);
  eepromEraseBlock(eepromWriteDestinationAddr+EEPROM_BLOCK_SIZE);

  // write model
  for (int pos=0; pos<EEPROM_ZONE_SIZE; pos+=EEPROM_BUFFER_SIZE) {
    eepromRead(eepromWriteBuffer, eepromWriteSourceAddr+pos, EEPROM_BUFFER_SIZE);
    eepromWrite(eepromWriteBuffer, eepromWriteDestinationAddr+pos, EEPROM_BUFFER_SIZE);
  }

  // write FAT
  eepromHeader.files[dst+1].exists = 1;
  eepromIncFatAddr();
  eepromWriteState = EEPROM_WRITE_NEW_FAT;
  eepromWriteWait();

  modelHeaders[dst] = modelHeaders[src];

  return true;
}

void eeSwapModels(uint8_t id1, uint8_t id2)
{
  storageCheck(true);
  {
    EepromHeaderFile tmp = eepromHeader.files[id1+1];
    eepromHeader.files[id1+1] = eepromHeader.files[id2+1];
    eepromHeader.files[id2+1] = tmp;
  }
  eepromIncFatAddr();
  eepromWriteState = EEPROM_WRITE_NEW_FAT;
  eepromWriteWait();

  {
    ModelHeader tmp = modelHeaders[id1];
    modelHeaders[id1] = modelHeaders[id2];
    modelHeaders[id2] = tmp;
  }
}

// For conversions ...
uint16_t eeLoadGeneralSettingsData()
{
  return readFile(0, (uint8_t *)&g_eeGeneral, sizeof(g_eeGeneral));
}

uint16_t eeLoadModelData(uint8_t index)
{
  return readFile(index+1, (uint8_t *)&g_model, sizeof(g_model));
}

void writeGeneralSettings()
{
  writeFile(0, (uint8_t *)&g_eeGeneral, sizeof(g_eeGeneral));
}

void writeModel(int index)
{
  writeFile(index+1, (uint8_t *)&g_model, sizeof(g_model));
}

bool eeLoadGeneral(bool allowFixes)
{
  eeLoadGeneralSettingsData();

  if (g_eeGeneral.version != EEPROM_VER) {
    TRACE("EEPROM version %d instead of %d", g_eeGeneral.version, EEPROM_VER);
    return false;
  }
  return true;
}

bool eeModelExists(uint8_t id)
{
  return (eepromHeader.files[id+1].exists);
}

void eeLoadModelHeader(uint8_t id, ModelHeader * header)
{
  readFile(id+1, (uint8_t *)header, sizeof(ModelHeader));
}

void storageFormat()
{
  eepromFatAddr = 0;
  eepromHeader.mark = EEPROM_MARK;
  eepromHeader.index = 0;
  for (int i=0; i<EEPROM_MAX_FILES; i++) {
    eepromHeader.files[i].exists = 0;
    eepromHeader.files[i].zoneIndex = i+1;
  }
  eepromEraseBlock(0);
  eepromEraseBlock(EEPROM_BLOCK_SIZE);
  eepromWrite((uint8_t *)&eepromHeader, 0, sizeof(eepromHeader));
}

void eepromWriteWait(EepromWriteState state/* = EEPROM_IDLE*/)
{
  while (eepromWriteState != state) {
    // Waits a little bit for CS transitions
    RTOS_WAIT_MS(2);
    eepromWriteProcess();
#ifdef SIMU
    sleep(5/*ms*/);
#endif
  }
}

void storageCheck(bool immediately)
{
  if (immediately) {
    eepromWriteWait();
  }

  assert(eepromWriteState == EEPROM_IDLE);

  if (storageDirtyMsk & EE_GENERAL) {
    TRACE("eeprom write general");
    storageDirtyMsk -= EE_GENERAL;
    writeGeneralSettings();
    if (immediately)
      eepromWriteWait();
    else
      return;
  }

  if (storageDirtyMsk & EE_MODEL) {
    TRACE("eeprom write model");
    storageDirtyMsk -= EE_MODEL;
    writeModel(g_eeGeneral.currModel);
    if (immediately)
      eepromWriteWait();
  }
}

void eepromWriteProcess()
{
  // TRACE("eepromWriteProcess(state=%d)", eepromWriteState);

  switch (eepromWriteState) {
    case EEPROM_ERASING_FILE_BLOCK1:
    case EEPROM_ERASING_FILE_BLOCK2:
    case EEPROM_WRITING_BUFFER:
    case EEPROM_ERASING_FAT_BLOCK:
    case EEPROM_WRITING_NEW_FAT:
      if (eepromIsTransferComplete()) {
        eepromWriteState = EepromWriteState(eepromWriteState + 1);
      }
      break;

    case EEPROM_ERASING_FILE_BLOCK1_WAIT:
    case EEPROM_ERASING_FILE_BLOCK2_WAIT:
    case EEPROM_WRITING_BUFFER_WAIT:
    case EEPROM_ERASING_FAT_BLOCK_WAIT:
    case EEPROM_WRITING_NEW_FAT_WAIT:
      if (eepromReadStatus()) {
        eepromWriteState = EepromWriteState(eepromWriteState + 1);
      }
      break;

    case EEPROM_START_WRITE:
      eepromWriteState = EEPROM_ERASING_FILE_BLOCK1;
      eepromEraseBlock(eepromWriteDestinationAddr, false);
      break;

    case EEPROM_ERASE_FILE_BLOCK2:
      eepromWriteState = EEPROM_ERASING_FILE_BLOCK2;
      eepromEraseBlock(eepromWriteDestinationAddr + EEPROM_BLOCK_SIZE, false);
      break;

    case EEPROM_WRITE_BUFFER:
    {
      EepromFileHeader * header = (EepromFileHeader *)eepromWriteBuffer;
      header->fileIndex = eepromWriteFileIndex;
      header->size = eepromWriteSize;
      uint32_t size = min<uint32_t>(EEPROM_BUFFER_SIZE-sizeof(EepromFileHeader), eepromWriteSize);
      memcpy(eepromWriteBuffer+sizeof(EepromFileHeader), eepromWriteSourceAddr, size);
      eepromWriteState = EEPROM_WRITING_BUFFER;
      eepromWrite(eepromWriteBuffer, eepromWriteDestinationAddr, sizeof(EepromFileHeader)+size, false);
      eepromWriteSourceAddr += size;
      eepromWriteDestinationAddr += sizeof(EepromFileHeader)+size;
      eepromWriteSize -= size;
      break;
    }

    case EEPROM_WRITE_NEXT_BUFFER:
    {
      uint32_t size = min<uint32_t>(EEPROM_BUFFER_SIZE, eepromWriteSize);
      if (size > 0) {
        memcpy(eepromWriteBuffer, eepromWriteSourceAddr, size);
        eepromWriteState = EEPROM_WRITING_BUFFER;
        eepromWrite(eepromWriteBuffer, eepromWriteDestinationAddr, size, false);
        eepromWriteSourceAddr += size;
        eepromWriteDestinationAddr += size;
        eepromWriteSize -= size;
        break;
      }
      else if (eepromFatAddr == 0 || eepromFatAddr == EEPROM_BLOCK_SIZE) {
        eepromWriteState = EEPROM_ERASING_FAT_BLOCK;
        eepromEraseBlock(eepromFatAddr, false);
        break;
      }
    }
    /* no break */

    case EEPROM_WRITE_NEW_FAT:
      eepromWriteState = EEPROM_WRITING_NEW_FAT;
      eepromWrite((uint8_t *)&eepromHeader, eepromFatAddr, sizeof(eepromHeader), false);
      break;

    case EEPROM_END_WRITE:
      eepromWriteState = EEPROM_IDLE;
      break;

    default:
      break;
  }
}

uint16_t eeModelSize(uint8_t index)
{
  uint16_t result = 0;

  if (eepromHeader.files[index+1].exists) {
    uint32_t address = eepromHeader.files[index+1].zoneIndex * EEPROM_ZONE_SIZE;
    EepromFileHeader header;
    eepromRead((uint8_t *)&header, address, sizeof(header));
    result = header.size;
  }

  return result;
}

const char * eeBackupModel(uint8_t i_fileSrc)
{
  char * buf = reusableBuffer.modelsel.mainname;
  VirtualFS& vfs = VirtualFS::instance();
  VfsFile archiveFile;
  size_t written;

  storageCheck(true);

  if (!VirtualFS::instance().defaultStorageAvailable()) {
    return STR_NO_SDCARD;
  }

  // check and create folder here
  strcpy(buf, STR_MODELS_PATH);
  const char * error = vfs.checkAndCreateDirectory(buf);
  if (error) {
    return error;
  }

  buf[sizeof(MODELS_PATH)-1] = '/';
  strcpy(strcat_modelname(&buf[sizeof(MODELS_PATH)], i_fileSrc), STR_MODELS_EXT);

  VfsError result = vfs.openFile(archiveFile, buf, VfsOpenFlags::CREATE_ALWAYS | VfsOpenFlags::WRITE);
  if (result != VfsError::OK) {
    return STORAGE_ERROR(result);
  }


  uint16_t size = eeModelSize(i_fileSrc);

  *(uint32_t*)&buf[0] = ETX_FOURCC;
  buf[4] = g_eeGeneral.version;
  buf[5] = 'M';
  *(uint16_t*)&buf[6] = size;

  result = archiveFile.write(buf, 8, written);
  if (result != VfsError::OK || written != 8) {
    archiveFile.close();
    return STORAGE_ERROR(result);
  }

  uint32_t address = eepromHeader.files[i_fileSrc+1].zoneIndex * EEPROM_ZONE_SIZE + sizeof(EepromFileHeader);
  while (size > 0) {
    uint16_t blockSize = min<uint16_t>(size, EEPROM_BUFFER_SIZE);
    eepromRead(eepromWriteBuffer, address, blockSize);
    result = archiveFile.write(eepromWriteBuffer, blockSize, written);
    if (result != VfsError::OK || written != blockSize) {
      archiveFile.close();
      return STORAGE_ERROR(result);
    }
    size -= blockSize;
    address += blockSize;
  }

  archiveFile.close();


  return NULL;
}

const char * eeRestoreModel(uint8_t i_fileDst, char *model_name)
{
  char * buf = reusableBuffer.modelsel.mainname;
  VfsFile restoreFile;
  size_t read;

  storageCheck(true);

  if (!VirtualFS::instance().defaultStorageAvailable()) {
    return STR_NO_SDCARD;
  }

  strcpy(buf, STR_MODELS_PATH);
  buf[sizeof(MODELS_PATH)-1] = '/';
  strcpy(&buf[sizeof(MODELS_PATH)], model_name);
  strcpy(&buf[strlen(buf)], STR_MODELS_EXT);

  FRESULT result = VirtualFS::instance().openFile(restoreFile, buf, VfsOpenFlags::OPEN_EXISTING | VfsOpenFlags::READ);
  if (result != VfsError::OK) {
    return STORAGE_ERROR(result);
  }

  if (restoreFile.size() < 8) {
    restoreFile.close();
    return STR_INCOMPATIBLE;
  }

  result = restoreFile.read((uint8_t *)buf, 8, read);
  if (result != VfsError::OK || read != 8) {
    restoreFile.close();
    return STORAGE_ERROR(result);
  }

  uint8_t version = (uint8_t)buf[4];
  if (*(uint32_t*)&buf[0] != ETX_FOURCC || version < FIRST_CONV_EEPROM_VER || version > EEPROM_VER || buf[5] != 'M') {
    restoreFile.close();
    return STR_INCOMPATIBLE;
  }

  if (eeModelExists(i_fileDst)) {
    eeDeleteModel(i_fileDst);
  }

  uint16_t size = min<uint16_t>(sizeof(g_model), *(uint16_t*)&buf[6]);
  uint32_t address = eepromHeader.files[i_fileDst+1].zoneIndex * EEPROM_ZONE_SIZE;

  // erase blocks
  eepromEraseBlock(address);
  eepromEraseBlock(address+EEPROM_BLOCK_SIZE);

  // write header
  EepromFileHeader * header = (EepromFileHeader *)eepromWriteBuffer;
  header->fileIndex = i_fileDst+1;
  header->size = size;

  int offset = 4;

  // write model
  do {
    uint16_t blockSize = min<uint16_t>(size, EEPROM_BUFFER_SIZE-offset);
    result = restoreFile.read(eepromWriteBuffer+offset, blockSize, read);
    if (result != VfsError::OK || read != blockSize) {
      g_oLogFile.close();
      return STORAGE_ERROR(result);
    }
    eepromWrite(eepromWriteBuffer, address, blockSize+offset);
    size -= blockSize;
    address += EEPROM_BUFFER_SIZE;
    offset = 0;
  } while (size > 0);

  // write FAT
  eepromHeader.files[i_fileDst+1].exists = 1;
  eepromIncFatAddr();
  eepromWriteState = EEPROM_WRITE_NEW_FAT;
  eepromWriteWait();

  eeLoadModelHeader(i_fileDst, &modelHeaders[i_fileDst]);

#if defined(STORAGE_CONVERSIONS)
  if (version < EEPROM_VER) {
    eeConvertModel(i_fileDst, version);
    eeLoadModel(g_eeGeneral.currModel);
  }
#endif

  return NULL;
}
