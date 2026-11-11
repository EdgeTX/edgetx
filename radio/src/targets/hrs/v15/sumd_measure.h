/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * V15 external-port SUMD measure (J10 pin3 / PJ8 half-duplex).
 * Auto-detects inverted and non-inverted polarity.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define SUMD_MEASURE_MAX_CHANNELS 16
#define SUMD_MEASURE_CH_MIN_US 800
#define SUMD_MEASURE_CH_MAX_US 2200
#define SUMD_MEASURE_SIGNAL_TIMEOUT_10MS 30  // 300 ms
#define SUMD_MEASURE_POLARITY_TRY_10MS 12    // 120 ms per polarity while searching
#define SUMD_MEASURE_OC_DELTA_MA 1000
/** Full SUMD frame snapshot for UI digital waveform (max 37 B @ 16 ch). */
#define SUMD_MEASURE_WAVE_BYTES 40

/** Polarity reported to UI. */
enum {
  SUMD_MEASURE_POL_SEARCHING = 0,
  SUMD_MEASURE_POL_INVERTED = 1,
  SUMD_MEASURE_POL_NORMAL = 2,
};

#if defined(__cplusplus)
extern "C" {
#endif

bool v15SumdMeasureIsActive(void);
bool v15SumdMeasureHasSignal(void);
bool v15SumdMeasureIsPowerOn(void);
bool v15SumdMeasureOvercurrentFault(void);
int16_t v15SumdMeasureGetCurrentMa(void);

uint8_t v15SumdMeasureGetPolarity(void);
/** 0 = failsafe/hold, 1 = OK live. */
uint8_t v15SumdMeasureGetStatus(void);

uint8_t v15SumdMeasureGetChannelCount(void);
uint16_t v15SumdMeasureGetChannelUs(uint8_t ch);
uint16_t v15SumdMeasureGetChannelRaw(uint8_t ch);
uint16_t v15SumdMeasureGetFrameHz(void);

/** Monotonic counter bumped on each valid SUMD frame (for UI wave). */
uint32_t v15SumdMeasureGetWaveSeq(void);
/** Copy latest frame bytes for digital wave preview; returns length copied. */
uint8_t v15SumdMeasureCopyWaveBytes(uint8_t* dst, uint8_t maxLen);

void v15SumdMeasureStart(void);
void v15SumdMeasureStop(void);
void v15SumdMeasureTask(void);

#if defined(__cplusplus)
}

void v15SumdMeasureOpen(void);
#endif
