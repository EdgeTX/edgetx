/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * V15 external-port CPPM / PPM frame measure (J10 pin3 / PJ8).
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PPM_MEASURE_MAX_CHANNELS 8
#define PPM_MEASURE_CH_MIN_US 800
#define PPM_MEASURE_CH_MAX_US 2200
#define PPM_MEASURE_SYNC_MIN_US 4000
#define PPM_MEASURE_SYNC_MAX_US 30000
/** Same SSR rising-edge compensation as PWM measure. */
#define PPM_MEASURE_PULSE_OFFSET_US 2
/** Extra CH1 offset (hardware: first pulse after sync reads ~12 µs low). */
#define PPM_MEASURE_CH1_EXTRA_OFFSET_US 12
#define PPM_MEASURE_SIGNAL_TIMEOUT_10MS 30  // 300 ms
#define PPM_MEASURE_OC_DELTA_MA 1000
/** Short separator pulse used when reconstructing the CPPM wave preview. */
#define PPM_MEASURE_WAVE_SEP_US 300

#if defined(__cplusplus)
extern "C" {
#endif

bool v15PpmMeasureIsActive(void);
bool v15PpmMeasureHasSignal(void);
bool v15PpmMeasureIsPowerOn(void);
bool v15PpmMeasureOvercurrentFault(void);
int16_t v15PpmMeasureGetCurrentMa(void);

uint8_t v15PpmMeasureGetChannelCount(void);
uint16_t v15PpmMeasureGetChannelUs(uint8_t ch);
uint16_t v15PpmMeasureGetFrameUs(void);
uint16_t v15PpmMeasureGetFrameHz(void);

/** Monotonic counter bumped on each published frame (for UI wave). */
uint32_t v15PpmMeasureGetWaveSeq(void);
/** Copy last-frame channel widths for wave preview; returns channel count. */
uint8_t v15PpmMeasureCopyWaveChannels(uint16_t* dst, uint8_t maxCh);
/** Frame period (µs) paired with the wave channel snapshot. */
uint16_t v15PpmMeasureGetWaveFrameUs(void);

void v15PpmMeasureStart(void);
void v15PpmMeasureStop(void);
void v15PpmMeasureTask(void);

#if defined(__cplusplus)
}

void v15PpmMeasureOpen(void);
#endif
