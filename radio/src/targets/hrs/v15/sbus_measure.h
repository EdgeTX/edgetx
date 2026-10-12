/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * V15 external-port SBUS measure (J10 pin3 / PJ8 half-duplex).
 * Auto-detects inverted and non-inverted polarity.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define SBUS_MEASURE_MAX_CHANNELS 16
#define SBUS_MEASURE_CH_MIN_US 800
#define SBUS_MEASURE_CH_MAX_US 2200
#define SBUS_MEASURE_SIGNAL_TIMEOUT_10MS 30  // 300 ms
#define SBUS_MEASURE_POLARITY_TRY_10MS 12    // 120 ms per polarity while searching
#define SBUS_MEASURE_OC_DELTA_MA 1000
/** Full SBUS frame snapshot for UI digital waveform (25 bytes). */
#define SBUS_MEASURE_WAVE_BYTES 25

/** Polarity reported to UI. */
enum {
  SBUS_MEASURE_POL_SEARCHING = 0,
  SBUS_MEASURE_POL_INVERTED = 1,  // Futaba / idle-low
  SBUS_MEASURE_POL_NORMAL = 2,    // non-inverted / idle-high
};

#if defined(__cplusplus)
extern "C" {
#endif

bool v15SbusMeasureIsActive(void);
bool v15SbusMeasureHasSignal(void);
bool v15SbusMeasureIsPowerOn(void);
bool v15SbusMeasureOvercurrentFault(void);
int16_t v15SbusMeasureGetCurrentMa(void);

uint8_t v15SbusMeasureGetPolarity(void);
uint8_t v15SbusMeasureGetFlags(void);
bool v15SbusMeasureFailsafe(void);
bool v15SbusMeasureFrameLost(void);
bool v15SbusMeasureCh17(void);
bool v15SbusMeasureCh18(void);

uint8_t v15SbusMeasureGetChannelCount(void);
uint16_t v15SbusMeasureGetChannelUs(uint8_t ch);
uint16_t v15SbusMeasureGetChannelRaw(uint8_t ch);
uint16_t v15SbusMeasureGetFrameHz(void);

/**
 * True when a locked SBUS stream looks like ExpressLRS "DJI RS Pro"
 * (µs values packed into 11-bit slots, not CRSF raw).
 */
bool v15SbusMeasureLooksLikeDjiRs(void);

/** Monotonic counter bumped on each valid SBUS frame (for UI wave). */
uint32_t v15SbusMeasureGetWaveSeq(void);
/** Copy latest frame bytes for digital wave preview; returns length copied. */
uint8_t v15SbusMeasureCopyWaveBytes(uint8_t* dst, uint8_t maxLen);

void v15SbusMeasureStart(void);
void v15SbusMeasureStop(void);
void v15SbusMeasureTask(void);

#if defined(__cplusplus)
}

void v15SbusMeasureOpen(void);
#endif
