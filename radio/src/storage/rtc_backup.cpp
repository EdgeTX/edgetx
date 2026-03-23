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
#include "model_arena.h"

namespace Backup {
#define BACKUP
#include "datastructs_private.h"

// Backup arena size: uses Backup:: struct sizes (NOBACKUP fields stripped).
// Capped to MODEL_ARENA_SIZE since backup data can never exceed live arena.
static constexpr uint32_t ARENA_SIZE_FORMULA =
    MAX_MIXERS * sizeof(MixData) +
    MAX_EXPOS * sizeof(ExpoData) +
    MAX_CURVES * sizeof(CurveHeader) +
    MAX_CURVE_POINTS * sizeof(int8_t) +
    MAX_LOGICAL_SWITCHES * sizeof(LogicalSwitchData) +
    MAX_SPECIAL_FUNCTIONS * sizeof(CustomFunctionData);
static constexpr uint32_t ARENA_SIZE =
    ARENA_SIZE_FORMULA < MODEL_ARENA_SIZE ? ARENA_SIZE_FORMULA : MODEL_ARENA_SIZE;

PACK(struct RamBackupUncompressed {
  ModelData model;
  uint8_t arena[ARENA_SIZE];
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

#if !defined(BKPSRAM_BASE) && defined(D3_BKPSRAM_BASE)
  #define BKPSRAM_BASE D3_BKPSRAM_BASE
#endif

RamBackup * ramBackup = (RamBackup *)BKPSRAM_BASE;
#endif

// Pack live arena elements into backup format (strips NOBACKUP fields like names)
static uint32_t packArenaForBackup(uint8_t* dst, uint32_t dstSize)
{
  uint8_t* start = dst;
  const ModelDynData& dyn = g_model.dyn;

  // Mixes
  auto* liveMix = (MixData*)g_modelArena.sectionBase(ARENA_MIXES);
  auto* bkpMix = (Backup::MixData*)dst;
  for (int i = 0; i < dyn.mixCount; i++)
    copyMixData(&bkpMix[i], &liveMix[i]);
  dst += dyn.mixCount * sizeof(Backup::MixData);

  // Expos
  auto* liveExpo = (ExpoData*)g_modelArena.sectionBase(ARENA_EXPOS);
  auto* bkpExpo = (Backup::ExpoData*)dst;
  for (int i = 0; i < dyn.expoCount; i++)
    copyExpoData(&bkpExpo[i], &liveExpo[i]);
  dst += dyn.expoCount * sizeof(Backup::ExpoData);

  // Curves
  auto* liveCurve = (CurveHeader*)g_modelArena.sectionBase(ARENA_CURVES);
  auto* bkpCurve = (Backup::CurveHeader*)dst;
  for (int i = 0; i < dyn.curveCount; i++)
    copyCurveHeader(&bkpCurve[i], &liveCurve[i]);
  dst += dyn.curveCount * sizeof(Backup::CurveHeader);

  // Points (raw bytes, no NOBACKUP fields)
  memcpy(dst, g_modelArena.sectionBase(ARENA_POINTS), dyn.pointsCount);
  dst += dyn.pointsCount;

  // Logical switches
  auto* liveLsw = (LogicalSwitchData*)g_modelArena.sectionBase(ARENA_LOGICAL_SW);
  auto* bkpLsw = (Backup::LogicalSwitchData*)dst;
  for (int i = 0; i < dyn.logicalSwCount; i++)
    copyLogicalSwitchData(&bkpLsw[i], &liveLsw[i]);
  dst += dyn.logicalSwCount * sizeof(Backup::LogicalSwitchData);

  // Custom functions
  auto* liveCfn = (CustomFunctionData*)g_modelArena.sectionBase(ARENA_CUSTOM_FN);
  auto* bkpCfn = (Backup::CustomFunctionData*)dst;
  for (int i = 0; i < dyn.customFnCount; i++)
    copyCustomFunctionData(&bkpCfn[i], &liveCfn[i]);
  dst += dyn.customFnCount * sizeof(Backup::CustomFunctionData);

  return dst - start;
}

// Unpack backup arena elements into live arena (restores NOBACKUP fields as zero)
static void unpackArenaFromBackup(const uint8_t* src)
{
  const ModelDynData& dyn = g_model.dyn;

  // Mixes
  auto* bkpMix = (Backup::MixData*)src;
  auto* liveMix = (MixData*)g_modelArena.sectionBase(ARENA_MIXES);
  for (int i = 0; i < dyn.mixCount; i++)
    copyMixData(&liveMix[i], &bkpMix[i]);
  src += dyn.mixCount * sizeof(Backup::MixData);

  // Expos
  auto* bkpExpo = (Backup::ExpoData*)src;
  auto* liveExpo = (ExpoData*)g_modelArena.sectionBase(ARENA_EXPOS);
  for (int i = 0; i < dyn.expoCount; i++)
    copyExpoData(&liveExpo[i], &bkpExpo[i]);
  src += dyn.expoCount * sizeof(Backup::ExpoData);

  // Curves
  auto* bkpCurve = (Backup::CurveHeader*)src;
  auto* liveCurve = (CurveHeader*)g_modelArena.sectionBase(ARENA_CURVES);
  for (int i = 0; i < dyn.curveCount; i++)
    copyCurveHeader(&liveCurve[i], &bkpCurve[i]);
  src += dyn.curveCount * sizeof(Backup::CurveHeader);

  // Points (raw bytes)
  memcpy(g_modelArena.sectionBase(ARENA_POINTS), src, dyn.pointsCount);
  src += dyn.pointsCount;

  // Logical switches
  auto* bkpLsw = (Backup::LogicalSwitchData*)src;
  auto* liveLsw = (LogicalSwitchData*)g_modelArena.sectionBase(ARENA_LOGICAL_SW);
  for (int i = 0; i < dyn.logicalSwCount; i++)
    copyLogicalSwitchData(&liveLsw[i], &bkpLsw[i]);
  src += dyn.logicalSwCount * sizeof(Backup::LogicalSwitchData);

  // Custom functions
  auto* bkpCfn = (Backup::CustomFunctionData*)src;
  auto* liveCfn = (CustomFunctionData*)g_modelArena.sectionBase(ARENA_CUSTOM_FN);
  for (int i = 0; i < dyn.customFnCount; i++)
    copyCustomFunctionData(&liveCfn[i], &bkpCfn[i]);
}

void rambackupWrite()
{
  copyRadioData(&ramBackupUncompressed.radio, &g_eeGeneral);
  copyModelData(&ramBackupUncompressed.model, &g_model);

  // Pack arena elements into backup format (strips NOBACKUP fields)
  memset(ramBackupUncompressed.arena, 0, sizeof(ramBackupUncompressed.arena));
  uint32_t arenaUsed = packArenaForBackup(
      ramBackupUncompressed.arena, sizeof(ramBackupUncompressed.arena));

  ramBackup->size = compress(ramBackup->data, sizeof(ramBackup->data),
                             (const uint8_t *)&ramBackupUncompressed,
                             sizeof(ramBackupUncompressed));

  TRACE("RamBackupWrite arenaUsed=%d backupsize=%d rlcsize=%d",
        (int)arenaUsed,
        (int)sizeof(Backup::RamBackupUncompressed), ramBackup->size);
}

bool rambackupRestore()
{
  if (ramBackup->size == 0)
    return false;

  if (uncompress((uint8_t *)&ramBackupUncompressed, sizeof(ramBackupUncompressed),
                 ramBackup->data, ramBackup->size) != sizeof(ramBackupUncompressed))
    return false;

  memset(&g_eeGeneral, 0, sizeof(g_eeGeneral));
  memset(&g_model, 0, sizeof(g_model));
  copyRadioData(&g_eeGeneral, &ramBackupUncompressed.radio);
  copyModelData(&g_model, &ramBackupUncompressed.model);

  // Restore arena: clear first (zeros NOBACKUP fields), then unpack
  g_modelArena.clear();
  g_modelArena.layout(g_model.dyn);
  unpackArenaFromBackup(ramBackupUncompressed.arena);

  return true;
}
