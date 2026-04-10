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
#include "model_arena.h"
#include "mixes.h"
#include "expos.h"
#include "customfn.h"

namespace Backup {
#define BACKUP
#include "datastructs_private.h"
static constexpr uint32_t ARENA_SIZE_FORMULA =
    MAX_MIXERS * sizeof(MixData) +
    MAX_EXPOS * sizeof(ExpoData) +
    MAX_CURVES * sizeof(CurveHeader) +
    MAX_CURVE_POINTS * sizeof(int8_t) +
    MAX_LOGICAL_SWITCHES * sizeof(LogicalSwitchData) +
    MAX_SPECIAL_FUNCTIONS * sizeof(CustomFunctionData);
// Must match the cap in rtc_backup.cpp
static constexpr uint32_t ARENA_SIZE =
    ARENA_SIZE_FORMULA < MODEL_ARENA_SIZE ? ARENA_SIZE_FORMULA : MODEL_ARENA_SIZE;
static constexpr uint32_t RADIO_ARENA_BKP_SIZE =
    MAX_SPECIAL_FUNCTIONS * sizeof(CustomFunctionData);
PACK(struct RamBackupUncompressed {
  ModelData model;
  uint16_t arenaCounts[MODEL_ARENA_NUM_SECTIONS];
  uint8_t arena[ARENA_SIZE];
  RadioData radio;
  uint16_t radioArenaCounts[RADIO_ARENA_NUM_SECTIONS];
  uint8_t radioArena[RADIO_ARENA_BKP_SIZE];
});
#undef BACKUP
};

extern Backup::RamBackupUncompressed ramBackupUncompressed;

TEST(Storage, BackupAndRestore)
{
  rambackupWrite();
  Backup::RamBackupUncompressed ramBackupRestored;
  EXPECT_EQ(uncompress((uint8_t *)&ramBackupRestored, sizeof(ramBackupRestored),
                       ramBackup->data, ramBackup->size),
            sizeof(ramBackupUncompressed));
  EXPECT_EQ(memcmp(&ramBackupUncompressed, &ramBackupRestored,
                    sizeof(ramBackupUncompressed)), 0);
}

TEST(Storage, BackupArenaRoundTrip)
{
  MODEL_RESET();

  // Allocate arena sections and populate data using AllocAt helpers
  MixData* mix0 = mixAllocAt(0);
  mix0->srcRaw = SourceRef_(SOURCE_TYPE_STICK, 2);
  mix0->swtch = SwitchRef_(SWITCH_TYPE_SWITCH, 3);
  mix0->weight.setNumeric(75);
  mix0->destCh = 0;
  strncpy(mix0->name, "Mix1", sizeof(mix0->name));

  ExpoData* expo0 = expoAllocAt(0);
  expo0->srcRaw = SourceRef_(SOURCE_TYPE_STICK, 0);
  expo0->weight.setNumeric(100);
  expo0->chn = 0;
  strncpy(expo0->name, "Exp1", sizeof(expo0->name));

  LogicalSwitchData* lsw0 = lswAllocAt(0);
  lsw0->func = LS_FUNC_VPOS;
  lsw0->andsw = SwitchRef_(SWITCH_TYPE_ON, 0);

  CustomFunctionData* cfn0 = customFnAllocAt(0);
  cfn0->swtch = SwitchRef_(SWITCH_TYPE_SWITCH, 1);
  cfn0->func = FUNC_PLAY_SOUND;

  // Backup
  rambackupWrite();

  // Save expected values
  SourceRef savedMixSrc = mix0->srcRaw;
  SwitchRef savedMixSw = mix0->swtch;
  int16_t savedMixWeight = mix0->weight.numericValue();
  SourceRef savedExpoSrc = expo0->srcRaw;
  uint8_t savedLswFunc = lsw0->func;
  SwitchRef savedCfnSw = cfn0->swtch;

  // Clear everything
  memset(&g_model, 0, sizeof(g_model));
  inputNameIndexReset();
  g_modelArena.clear();

  // Restore
  EXPECT_TRUE(rambackupRestore());

  // Verify arena section counts
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_MIXES), 1);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_EXPOS), 1);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 1);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_CUSTOM_FN), 1);

  // Verify arena element data (backup-relevant fields)
  mix0 = mixAddress(0);
  EXPECT_EQ(mix0->srcRaw, savedMixSrc);
  EXPECT_EQ(mix0->swtch, savedMixSw);
  EXPECT_EQ(mix0->weight.numericValue(), savedMixWeight);

  expo0 = expoAddress(0);
  EXPECT_EQ(expo0->srcRaw, savedExpoSrc);

  lsw0 = lswAddress(0);
  EXPECT_EQ(lsw0->func, savedLswFunc);

  cfn0 = customFnAddress(0);
  EXPECT_EQ(cfn0->swtch, savedCfnSw);

  // Verify names are NOT restored (NOBACKUP fields)
  EXPECT_EQ(mix0->name[0], '\0');
  EXPECT_EQ(expo0->name[0], '\0');
}
#endif
