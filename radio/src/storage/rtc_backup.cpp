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

#include "edgetx.h"
#include "rtc_backup.h"

namespace Backup {
#define BACKUP
#include "datastructs_private.h"
PACK(struct RamBackupUncompressed {
  ModelData model;
  RadioData radio;
});
#undef BACKUP
};

#include "datacopy.inc"

Backup::RamBackupUncompressed ramBackupUncompressed __DMA;

#if defined(SIMU)
RamBackup _ramBackup;
RamBackup * ramBackup = &_ramBackup;
#else
RamBackup * ramBackup = (RamBackup *)BKPSRAM_BASE;
#endif

void rambackupWrite()
{
  copyRadioData(&ramBackupUncompressed.radio, &g_eeGeneral);
  copyModelData(&ramBackupUncompressed.model, &g_model);
  ramBackup->size = compress(ramBackup->data, sizeof(ramBackup->data),
                             (const uint8_t *)&ramBackupUncompressed,
                             sizeof(ramBackupUncompressed));

  TRACE("RamBackupWrite sdsize=%d backupsize=%d rlcsize=%d",
        sizeof(ModelData) + sizeof(RadioData),
        sizeof(Backup::RamBackupUncompressed), ramBackup->size);
}

bool rambackupRestore()
{
  if (ramBackup->size == 0)
    return false;

  if (uncompress((uint8_t *)&ramBackupUncompressed, sizeof(ramBackupUncompressed), ramBackup->data, ramBackup->size) != sizeof(ramBackupUncompressed))
    return false;

  memset(&g_eeGeneral, 0, sizeof(g_eeGeneral));
  memset(&g_model, 0, sizeof(g_model));
  copyRadioData(&g_eeGeneral, &ramBackupUncompressed.radio);
  copyModelData(&g_model, &ramBackupUncompressed.model);
  return true;
}
