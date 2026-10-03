/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * V15 EXT-port Pulse Scope / logic waveform (J10 pin3 / PJ8).
 * Digital edge capture for visualizing pulse activity — not a protocol decoder.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

enum {
  LOGIC_MEASURE_TB_100US = 0,
  LOGIC_MEASURE_TB_500US,
  LOGIC_MEASURE_TB_1MS,
  LOGIC_MEASURE_TB_2MS,
  LOGIC_MEASURE_TB_5MS,
  LOGIC_MEASURE_TB_10MS,
  LOGIC_MEASURE_TB_20MS,
  LOGIC_MEASURE_TB_50MS,
  LOGIC_MEASURE_TB_COUNT
};

enum {
  LOGIC_MEASURE_MODE_STOP = 0,
  LOGIC_MEASURE_MODE_RUN,
  LOGIC_MEASURE_MODE_SINGLE,
};

#define LOGIC_MEASURE_RING 256
#define LOGIC_MEASURE_COPY_MAX 64
#define LOGIC_MEASURE_OC_DELTA_MA 1000
#define LOGIC_MEASURE_SIGNAL_TIMEOUT_10MS 50  // 500 ms

typedef struct {
  uint16_t tUs;   // time from window start
  uint8_t level;  // level after this edge
} LogicMeasureEdge;

#if defined(__cplusplus)
extern "C" {
#endif

bool v15LogicMeasureIsActive(void);
bool v15LogicMeasureHasSignal(void);
bool v15LogicMeasureIsPowerOn(void);
bool v15LogicMeasureOvercurrentFault(void);
int16_t v15LogicMeasureGetCurrentMa(void);

bool v15LogicMeasureGetLevel(void);
bool v15LogicMeasureIsRateLimited(void);
uint32_t v15LogicMeasureGetEdgesHz(void);
uint16_t v15LogicMeasureGetLastHighUs(void);
uint16_t v15LogicMeasureGetLastLowUs(void);
uint8_t v15LogicMeasureGetDutyPercent(void);
uint16_t v15LogicMeasureGetPeriodUs(void);

void v15LogicMeasureSetTimebase(uint8_t tb);
uint8_t v15LogicMeasureGetTimebase(void);
uint32_t v15LogicMeasureGetTimebaseUs(void);

void v15LogicMeasureRun(void);
void v15LogicMeasureStopCapture(void);
void v15LogicMeasureSingle(void);
uint8_t v15LogicMeasureGetMode(void);

/**
 * Copy edges for the current timebase window (from frozen snap or live ring).
 * *startLevel = level before the first edge in the window.
 * Returns number of edges written (0..maxN).
 */
uint16_t v15LogicMeasureCopyEdges(LogicMeasureEdge* out, uint16_t maxN,
                                  uint8_t* startLevel);

void v15LogicMeasureStart(void);
void v15LogicMeasureStop(void);
void v15LogicMeasureTask(void);

#if defined(__cplusplus)
}

void v15LogicMeasureOpen(void);
#endif
