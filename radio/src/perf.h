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

#pragma once

/*
  Performance probes (PERF_PROBES build option).

  Each probe collects count, min, max, average and a log2 histogram of
  durations, timed with the DWT cycle counter. Results are read with the
  CLI command "perf" and cleared with "perf reset".

  Usage:
    PERF_START(MIXER);            // same scope as the matching stop
    ...
    PERF_STOP(MIXER);             // or PERF_STOP_AS(MIXER, <probe>)

    PERF_MARK(MIXER_PERIOD);      // records the time since the last mark

  Every probe must be recorded from a single context (one task or one
  interrupt). Without PERF_PROBES, all macros compile to nothing.
*/

#include <stdint.h>

// X(id, name): the name is what "perf" prints.
#define PERF_PROBE_LIST(X)                                               \
  X(MIXER, "mixer")                   /* one mixer task cycle */         \
  X(MIXER_PERIOD, "mixer.period")     /* start-to-start interval */      \
  X(MIXER_ADC, "mixer.adc")           /* getADC() */                     \
  X(MIXER_SWITCHES, "mixer.switches") /* getSwitchesPosition() */        \
  X(MIXER_EVAL, "mixer.eval")         /* evalMixes(), no FM fade */      \
  X(MIXER_EVAL_FADE, "mixer.eval_fade") /* evalMixes() during a fade */  \
  X(MIXER_PERIODIC, "mixer.periodic") /* doMixerPeriodicUpdates() */     \
  X(MIXER_LS_TICK, "mixer.ls_tick")   /* logicalSwitchesTimerTick() */   \
  X(MIXER_LS, "mixer.ls")             /* evalLogicalSwitches() */        \
  X(MIXER_SF, "mixer.sf")             /* evalFunctions(), GF + SF */     \
  X(ADC_READ, "adc.read")             /* adcRead() */                    \
  X(TIMER_10MS, "timer.10ms")         /* per10ms() */                    \
  X(TIMER_10MS_PERIOD, "timer.10ms.period")                              \
  X(TELEMETRY, "timer.telemetry")     /* telemetryWakeup() */            \
  X(LOGS, "timer.logs")               /* logsWrite() */                  \
  X(SPACEMOUSE, "timer.spacemouse")                                      \
  X(HAPTIC, "haptic")                 /* per5ms() */                     \
  X(AUDIO, "audio")                   /* one audio task cycle */         \
  X(AUDIO_PERIOD, "audio.period")                                        \
  X(AUDIO_CONSUME, "audio.consume")                                      \
  X(UI, "ui")                         /* perMain() */                    \
  X(UI_PERIODIC, "ui.periodic")       /* perMain() housekeeping */       \
  X(UI_GUI, "ui.gui")                 /* guiMain() */                    \
  X(UI_LUA, "ui.lua")                 /* luaTask() */                    \
  X(LCD_REFRESH, "lcd.refresh")       /* LVGL refresh that drew */       \
  X(LCD_FLUSH, "lcd.flush")           /* display driver flush */

#define PERF_PROBE_ENUM(id, name) PERF_##id,
typedef enum { PERF_PROBE_LIST(PERF_PROBE_ENUM) PERF_PROBE_COUNT } PerfProbe;
#undef PERF_PROBE_ENUM

#if defined(__cplusplus) && defined(PERF_PROBES) && !defined(SIMU) && \
    !defined(BOOT)

#include "delays_driver.h"

#define PERF_ENABLED 1
#define PERF_HIST_BUCKETS 20

struct PerfStats {
  uint32_t epoch;
  uint32_t count;
  uint32_t min;        // cycles
  uint32_t max;        // cycles
  uint64_t sum;        // cycles
  uint32_t last_mark;  // cycles, PERF_MARK only
  bool has_mark;
  // bucket b: [2^(b-1), 2^b) us, bucket 0: < 1 us, last bucket open-ended
  uint32_t hist[PERF_HIST_BUCKETS];
};

void perfRecord(PerfProbe probe, uint32_t cycles);
void perfMark(PerfProbe probe);
void perfReset();
// Copies a consistent-enough snapshot; returns false if nothing was
// recorded since the last reset.
bool perfGetStats(PerfProbe probe, PerfStats& stats);
const char* perfProbeName(PerfProbe probe);
uint32_t perfMsSinceReset();
uint32_t perfCyclesPerUs();
bool perfCounterRunning();

#define PERF_START(p) const uint32_t _perf_t0_##p = ticksNow()
#define PERF_STOP(p) perfRecord(PERF_##p, ticksNow() - _perf_t0_##p)
#define PERF_STOP_AS(p, probe) perfRecord(probe, ticksNow() - _perf_t0_##p)
#define PERF_MARK(p) perfMark(PERF_##p)

#else

#define PERF_ENABLED 0
#define PERF_START(p) (void)0
#define PERF_STOP(p) (void)0
#define PERF_STOP_AS(p, probe) (void)(probe)
#define PERF_MARK(p) (void)0

#endif
