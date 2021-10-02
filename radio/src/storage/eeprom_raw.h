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

#ifndef _EEPROM_RAW_H_
#define _EEPROM_RAW_H_

#include <inttypes.h>
#include <stdint.h>

bool eeLoadGeneral(bool allowFixes);
void eeDeleteModel( uint8_t id );
bool eeCopyModel(uint8_t dst, uint8_t src);
void eeSwapModels(uint8_t id1, uint8_t id2);

#if defined(SDCARD)
const char * eeBackupModel(uint8_t i_fileSrc);
const char * eeRestoreModel(uint8_t i_fileDst, char *model_name);
#endif

enum EepromWriteState {
  EEPROM_IDLE = 0,
  EEPROM_START_WRITE,
  EEPROM_ERASING_FILE_BLOCK1,
  EEPROM_ERASING_FILE_BLOCK1_WAIT,
  EEPROM_ERASE_FILE_BLOCK2,
  EEPROM_ERASING_FILE_BLOCK2,
  EEPROM_ERASING_FILE_BLOCK2_WAIT,
  EEPROM_WRITE_BUFFER,
  EEPROM_WRITING_BUFFER,
  EEPROM_WRITING_BUFFER_WAIT,
  EEPROM_WRITE_NEXT_BUFFER,
  EEPROM_ERASING_FAT_BLOCK,
  EEPROM_ERASING_FAT_BLOCK_WAIT,
  EEPROM_WRITE_NEW_FAT,
  EEPROM_WRITING_NEW_FAT,
  EEPROM_WRITING_NEW_FAT_WAIT,
  EEPROM_END_WRITE
};

extern volatile EepromWriteState eepromWriteState;
inline bool eepromIsWriting()
{
  return (eepromWriteState != EEPROM_IDLE);
}
void eepromWriteProcess();
void eepromWriteWait(EepromWriteState state = EEPROM_IDLE);
bool eepromOpen();

#endif // _EEPROM_RAW_H_
