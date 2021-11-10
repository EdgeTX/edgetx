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

#include "gtests.h"

extern const char * eepromFile;

#if defined(RTC_BACKUP_RAM)
#include "storage/rtc_backup.h"
namespace Backup {
#define BACKUP
#include "datastructs_private.h"
PACK(struct RamBackupUncompressed {
  ModelData model;
  RadioData radio;
});
#undef BACKUP
};
extern Backup::RamBackupUncompressed ramBackupUncompressed;
TEST(Storage, BackupAndRestore)
{
  rambackupWrite();
  Backup::RamBackupUncompressed ramBackupRestored;
  if (uncompress((uint8_t *)&ramBackupRestored, sizeof(ramBackupRestored), ramBackup->data, ramBackup->size) != sizeof(ramBackupUncompressed))
    TRACE("ERROR uncompress");
  if (memcmp(&ramBackupUncompressed, &ramBackupRestored, sizeof(ramBackupUncompressed)) != 0)
    TRACE("ERROR restore");
}
#endif

#if defined(EEPROM) && defined(EEPROM_RLC)
#include "storage/eeprom_rlc.h"

TEST(Eeprom, 100_random_writes)
{
  eepromFile = NULL; // in memory
  RlcFile f;
  uint8_t buf[1000];
  uint8_t buf2[1000];

  storageFormat();

  for (int i = 0; i < 100; i++) {
    int size = rand() % 800;
    for (int j = 0; j < size; j++) {
      buf[j] = rand() < (RAND_MAX / 10000 * i) ? 0 : (j & 0xff);
    }
    f.writeRlc(5, 5, buf, size, 100);
    // printf("size=%4d red=%4d\n\n\n", size, f.size());
    f.openRd(5);
    uint16_t n = f.readRlc(buf2,size+1);
    EXPECT_EQ(n, size);
    EXPECT_EQ(memcmp(buf, buf2, size), 0);
  }
}

TEST(Eeprom, test2)
{
  eepromFile = NULL; // in memory
  RlcFile f;
  uint8_t buf[1000];

  storageFormat();

  for (int i = 0; i < 1000; i++)
    buf[i] = '6' + i % 4;

  f.writeRlc(6, 6, buf, 300, 100);

  f.openRd(6);
  uint16_t sz=0;
  for(int i=0; i<500; i++){
    uint8_t b;
    uint16_t n=f.readRlc(&b,1);
    if (n) {
      EXPECT_EQ(b, ('6' + sz % 4));
    }
    sz+=n;
  }
  EXPECT_EQ(sz, 300);
}

TEST(Eeprom, storageCheckImmediately)
{
  eepromFile = NULL; // in memory
  // RlcFile f;
  uint8_t buf[1000];

  storageFormat();

  for (int i = 0; i < 1000; i++)
    buf[i]='6'+i%4;

  theFile.writeRlc(6, 6, buf, 300, false);

  storageCheck(true);

  theFile.openRd(6);
  uint16_t sz=0;
  for (int i = 0; i < 500; i++) {
    uint8_t b;
    uint16_t n = theFile.readRlc(&b, 1);
    if (n) {
      EXPECT_EQ(b, ('6' + sz % 4));
    }
    sz += n;
  }
  EXPECT_EQ(sz, 300);
}

TEST(Eeprom, copy)
{
  eepromFile = NULL; // in memory

  uint8_t buf[1000];

  storageFormat();

  for (int i = 0; i < 1000; i++)
    buf[i] = '6' + i % 4;

  theFile.writeRlc(5, 6, buf, 300, true);

  theFile.copy(6, 5);

  theFile.openRd(6);
  uint16_t sz = 0;
  for (int i = 0; i < 500; i++) {
    uint8_t b;
    uint16_t n = theFile.readRlc(&b, 1);
    if (n) {
      EXPECT_EQ(b, ('6' + sz % 4));
    }
    sz += n;
  }
  EXPECT_EQ(sz, 300);
}

TEST(Eeprom, rm)
{
  eepromFile = NULL; // in memory

  uint8_t buf[1000];

  storageFormat();

  for (int i = 0; i < 1000; i++)
    buf[i] = '6' + i % 4;

  theFile.writeRlc(5, 6, buf, 300, true);

  EXPECT_EQ(EFile::exists(5), true);

  EFile::rm(5);

  EXPECT_EQ(EFile::exists(5), false);

  theFile.openRd(5);
  uint16_t sz=0;
  for(int i=0; i<500; i++){
    uint8_t b;
    uint16_t n = theFile.readRlc(&b, 1);
    if (n) {
      EXPECT_EQ(b, ('6' + sz % 4));
    }
    sz += n;
  }
  EXPECT_EQ(sz, 0);
}
#endif
