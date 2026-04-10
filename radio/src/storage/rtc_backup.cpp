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
// Capped to MODEL_ARENA_INITIAL_SIZE — models that grew beyond this on a
// growable arena will lose arena data on RTC restore (SD card model file is
// authoritative and will be reloaded on next boot).
static constexpr uint32_t ARENA_SIZE_FORMULA =
    MAX_MIXERS * sizeof(MixData) +
    MAX_EXPOS * sizeof(ExpoData) +
    MAX_CURVES * sizeof(CurveHeader) +
    MAX_CURVE_POINTS * sizeof(int8_t) +
    MAX_LOGICAL_SWITCHES * sizeof(LogicalSwitchData) +
    MAX_SPECIAL_FUNCTIONS * sizeof(CustomFunctionData) +
    MAX_FLIGHT_MODES * sizeof(FlightModeData) +
    MAX_GVARS * sizeof(GVarData) +
    MAX_FLIGHT_MODES * MAX_GVARS * sizeof(gvar_t);
static constexpr uint32_t ARENA_SIZE =
    ARENA_SIZE_FORMULA < MODEL_ARENA_SIZE ? ARENA_SIZE_FORMULA : MODEL_ARENA_SIZE;

// Radio arena backup: only custom functions
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

// Pack live arena elements into backup format (strips NOBACKUP fields like names).
// If the packed data would exceed dstSize, packing stops and the remaining
// sections are zeroed (graceful degradation for models that grew beyond the
// initial arena size).
static uint32_t packArenaForBackup(uint8_t* dst, uint32_t dstSize)
{
  uint8_t* start = dst;
  uint8_t* end = dst + dstSize;

#define PACK_SECTION(section, LiveType, BkpType, copyFn) do { \
  uint16_t n = g_modelArena.sectionCount(section); \
  uint32_t needed = n * sizeof(BkpType); \
  if (dst + needed > end) { \
    TRACE("RamBackup: arena overflow, skipping section %d", section); \
    goto pack_done; \
  } \
  auto* live = (LiveType*)g_modelArena.sectionBase(section); \
  auto* bkp = (BkpType*)dst; \
  for (int i = 0; i < n; i++) copyFn(&bkp[i], &live[i]); \
  dst += needed; \
} while(0)

  PACK_SECTION(ARENA_MIXES, MixData, Backup::MixData, copyMixData);
  PACK_SECTION(ARENA_EXPOS, ExpoData, Backup::ExpoData, copyExpoData);
  PACK_SECTION(ARENA_CURVES, CurveHeader, Backup::CurveHeader, copyCurveHeader);

  // Points (raw bytes, no NOBACKUP fields)
  {
    uint16_t nPoints = g_modelArena.sectionCount(ARENA_POINTS);
    if (dst + nPoints > end) {
      TRACE("RamBackup: arena overflow at points section");
      goto pack_done;
    }
    memcpy(dst, g_modelArena.sectionBase(ARENA_POINTS), nPoints);
    dst += nPoints;
  }

  PACK_SECTION(ARENA_LOGICAL_SW, LogicalSwitchData, Backup::LogicalSwitchData, copyLogicalSwitchData);
  PACK_SECTION(ARENA_CUSTOM_FN, CustomFunctionData, Backup::CustomFunctionData, copyCustomFunctionData);
  PACK_SECTION(ARENA_FLIGHT_MODES, FlightModeData, Backup::FlightModeData, copyFlightModeData);
  PACK_SECTION(ARENA_GVAR_DATA, GVarData, Backup::GVarData, copyGVarData);

  // GVar values (raw gvar_t, no NOBACKUP fields)
  {
    uint16_t nGvarValues = g_modelArena.sectionCount(ARENA_GVAR_VALUES);
    uint32_t nBytes = nGvarValues * sizeof(gvar_t);
    if (dst + nBytes > end) {
      TRACE("RamBackup: arena overflow at gvar values section");
      goto pack_done;
    }
    memcpy(dst, g_modelArena.sectionBase(ARENA_GVAR_VALUES), nBytes);
    dst += nBytes;
  }

pack_done:
#undef PACK_SECTION
  return dst - start;
}

// Unpack backup arena elements into live arena (restores NOBACKUP fields as zero)
static void unpackArenaFromBackup(const uint8_t* src)
{
#define UNPACK_SECTION(section, LiveType, BkpType, copyFn) do { \
  uint16_t n = g_modelArena.sectionCount(section); \
  auto* bkp = (BkpType*)src; \
  auto* live = (LiveType*)g_modelArena.sectionBase(section); \
  for (int i = 0; i < n; i++) copyFn(&live[i], &bkp[i]); \
  src += n * sizeof(BkpType); \
} while(0)

  UNPACK_SECTION(ARENA_MIXES, MixData, Backup::MixData, copyMixData);
  UNPACK_SECTION(ARENA_EXPOS, ExpoData, Backup::ExpoData, copyExpoData);
  UNPACK_SECTION(ARENA_CURVES, CurveHeader, Backup::CurveHeader, copyCurveHeader);

  uint16_t nPoints = g_modelArena.sectionCount(ARENA_POINTS);
  memcpy(g_modelArena.sectionBase(ARENA_POINTS), src, nPoints);
  src += nPoints;

  UNPACK_SECTION(ARENA_LOGICAL_SW, LogicalSwitchData, Backup::LogicalSwitchData, copyLogicalSwitchData);
  UNPACK_SECTION(ARENA_CUSTOM_FN, CustomFunctionData, Backup::CustomFunctionData, copyCustomFunctionData);
  UNPACK_SECTION(ARENA_FLIGHT_MODES, FlightModeData, Backup::FlightModeData, copyFlightModeData);
  UNPACK_SECTION(ARENA_GVAR_DATA, GVarData, Backup::GVarData, copyGVarData);

  // GVar values (raw gvar_t, no NOBACKUP fields)
  {
    uint16_t nGvarValues = g_modelArena.sectionCount(ARENA_GVAR_VALUES);
    memcpy(g_modelArena.sectionBase(ARENA_GVAR_VALUES), src,
           nGvarValues * sizeof(gvar_t));
    src += nGvarValues * sizeof(gvar_t);
  }

#undef UNPACK_SECTION
}

void rambackupWrite()
{
  copyRadioData(&ramBackupUncompressed.radio, &g_eeGeneral);
  copyModelData(&ramBackupUncompressed.model, &g_model);

  // Save arena section counts
  memcpy(ramBackupUncompressed.arenaCounts, g_modelArena.counts(),
         sizeof(ramBackupUncompressed.arenaCounts));

  // Pack model arena elements into backup format (strips NOBACKUP fields)
  memset(ramBackupUncompressed.arena, 0, sizeof(ramBackupUncompressed.arena));
  packArenaForBackup(ramBackupUncompressed.arena,
                     sizeof(ramBackupUncompressed.arena));

  // Pack radio arena (custom functions only)
  memcpy(ramBackupUncompressed.radioArenaCounts, g_radioArena.counts(),
         sizeof(ramBackupUncompressed.radioArenaCounts));
  memset(ramBackupUncompressed.radioArena, 0, sizeof(ramBackupUncompressed.radioArena));
  {
    uint16_t n = g_radioArena.sectionCount(RADIO_ARENA_CUSTOM_FN);
    auto* live = (CustomFunctionData*)g_radioArena.sectionBase(RADIO_ARENA_CUSTOM_FN);
    auto* bkp = (Backup::CustomFunctionData*)ramBackupUncompressed.radioArena;
    for (uint16_t i = 0; i < n; i++) copyCustomFunctionData(&bkp[i], &live[i]);
  }

  ramBackup->size = compress(ramBackup->data, sizeof(ramBackup->data),
                             (const uint8_t *)&ramBackupUncompressed,
                             sizeof(ramBackupUncompressed));

  TRACE("RamBackupWrite backupsize=%d rlcsize=%d",
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
  inputNameIndexReset();
  copyRadioData(&g_eeGeneral, &ramBackupUncompressed.radio);
  copyModelData(&g_model, &ramBackupUncompressed.model);

  // Restore model arena layout from saved counts, clear (zeros NOBACKUP fields), then unpack
  g_modelArena.layout(ramBackupUncompressed.arenaCounts);
  g_modelArena.clear();
  unpackArenaFromBackup(ramBackupUncompressed.arena);

  // Restore radio arena (custom functions)
  g_radioArena.layout(ramBackupUncompressed.radioArenaCounts);
  g_radioArena.clear();
  {
    uint16_t n = g_radioArena.sectionCount(RADIO_ARENA_CUSTOM_FN);
    auto* bkp = (Backup::CustomFunctionData*)ramBackupUncompressed.radioArena;
    auto* live = (CustomFunctionData*)g_radioArena.sectionBase(RADIO_ARENA_CUSTOM_FN);
    for (uint16_t i = 0; i < n; i++) copyCustomFunctionData(&live[i], &bkp[i]);
  }

  return true;
}
