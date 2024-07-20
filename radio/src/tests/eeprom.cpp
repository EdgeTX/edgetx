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
