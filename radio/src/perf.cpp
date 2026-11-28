/*
 * Copyright (C) EdgeTX
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

#include "perf.h"

#include <string.h>

#include "os/time.h"
#include "stm32_hal.h"

#define PERF_PROBE_NAME(id, name) name,
static const char* const _perf_names[PERF_PROBE_COUNT] = {
    PERF_PROBE_LIST(PERF_PROBE_NAME)};
#undef PERF_PROBE_NAME

static PerfStats _perf_stats[PERF_PROBE_COUNT];

// Stats whose epoch differs are stale: they are cleared by their own
// writer on the next record, so a reset never races with a writer.
static volatile uint32_t _perf_epoch = 1;
static uint32_t _perf_reset_ms = 0;
static uint32_t _perf_cyc_per_us = 0;

static inline uint32_t cyc_per_us()
{
  if (!_perf_cyc_per_us) _perf_cyc_per_us = SystemCoreClock / 1000000;
  return _perf_cyc_per_us;
}

static inline PerfStats& current(PerfProbe probe)
{
  PerfStats& s = _perf_stats[probe];
  uint32_t epoch = _perf_epoch;
  if (s.epoch != epoch) {
    memset(&s, 0, sizeof(s));
    s.min = UINT32_MAX;
    s.epoch = epoch;
  }
  return s;
}

void perfRecord(PerfProbe probe, uint32_t cycles)
{
  PerfStats& s = current(probe);

  s.count++;
  s.sum += cycles;
  if (cycles < s.min) s.min = cycles;
  if (cycles > s.max) s.max = cycles;

  uint32_t us = cycles / cyc_per_us();
  uint32_t b = us ? 32 - __CLZ(us) : 0;
  if (b >= PERF_HIST_BUCKETS) b = PERF_HIST_BUCKETS - 1;
  s.hist[b]++;
}

void perfMark(PerfProbe probe)
{
  uint32_t now = ticksNow();
  PerfStats& s = current(probe);
  uint32_t last = s.last_mark;
  bool has_mark = s.has_mark;
  s.last_mark = now;
  s.has_mark = true;
  if (has_mark) perfRecord(probe, now - last);
}

static void enable_cycle_counter()
{
  // A debugger session may have stopped the counter (DEMCR.TRCENA
  // cleared on disconnect); CYCCNT itself is left running untouched.
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

bool perfCounterRunning()
{
  return (CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) &&
         (DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk);
}

void perfReset()
{
  enable_cycle_counter();
  _perf_cyc_per_us = SystemCoreClock / 1000000;
  _perf_reset_ms = time_get_ms();
  _perf_epoch = _perf_epoch + 1;
}

bool perfGetStats(PerfProbe probe, PerfStats& stats)
{
  if (probe >= PERF_PROBE_COUNT) return false;
  // The writer may update while we copy: at worst one sample is torn.
  memcpy(&stats, &_perf_stats[probe], sizeof(stats));
  return stats.epoch == _perf_epoch && stats.count > 0;
}

const char* perfProbeName(PerfProbe probe)
{
  return probe < PERF_PROBE_COUNT ? _perf_names[probe] : "?";
}

uint32_t perfMsSinceReset() { return time_get_ms() - _perf_reset_ms; }

uint32_t perfCyclesPerUs() { return cyc_per_us(); }
