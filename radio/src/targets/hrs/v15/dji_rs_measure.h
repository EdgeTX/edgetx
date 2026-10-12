/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * V15 external-port DJI RS Pro measure (J10 pin3 / PJ8 half-duplex).
 * SBUS-compatible framing with DJI channel scaling for display.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define DJI_RS_MEASURE_MAX_CHANNELS 16
/** ELRS packs DJI microseconds into SBUS slots (recenter 176–848, sticks 352–1696). */
#define DJI_RS_MEASURE_CH_MIN_US 176
#define DJI_RS_MEASURE_CH_MAX_US 1696
#define DJI_RS_MEASURE_SIGNAL_TIMEOUT_10MS 30  // 300 ms
#define DJI_RS_MEASURE_POLARITY_TRY_10MS 12    // 120 ms per polarity while searching
#define DJI_RS_MEASURE_OC_DELTA_MA 1000
/** Full SBUS-style frame snapshot for UI digital waveform (25 bytes). */
#define DJI_RS_MEASURE_WAVE_BYTES 25

/** Polarity reported to UI. */
enum {
  DJI_RS_MEASURE_POL_SEARCHING = 0,
  DJI_RS_MEASURE_POL_INVERTED = 1,
  DJI_RS_MEASURE_POL_NORMAL = 2,
};

#if defined(__cplusplus)
extern "C" {
#endif

bool v15DjiRsMeasureIsActive(void);
bool v15DjiRsMeasureHasSignal(void);
bool v15DjiRsMeasureIsPowerOn(void);
bool v15DjiRsMeasureOvercurrentFault(void);
int16_t v15DjiRsMeasureGetCurrentMa(void);

uint8_t v15DjiRsMeasureGetPolarity(void);
uint8_t v15DjiRsMeasureGetFlags(void);
bool v15DjiRsMeasureFailsafe(void);
bool v15DjiRsMeasureFrameLost(void);
bool v15DjiRsMeasureCh17(void);
bool v15DjiRsMeasureCh18(void);

uint8_t v15DjiRsMeasureGetChannelCount(void);
uint16_t v15DjiRsMeasureGetChannelUs(uint8_t ch);
uint16_t v15DjiRsMeasureGetChannelRaw(uint8_t ch);
uint16_t v15DjiRsMeasureGetFrameHz(void);

/** Monotonic counter bumped on each valid frame (for UI wave). */
uint32_t v15DjiRsMeasureGetWaveSeq(void);
/** Copy latest frame bytes for digital wave preview; returns length copied. */
uint8_t v15DjiRsMeasureCopyWaveBytes(uint8_t* dst, uint8_t maxLen);

void v15DjiRsMeasureStart(void);
void v15DjiRsMeasureStop(void);
void v15DjiRsMeasureTask(void);

#if defined(__cplusplus)
}

void v15DjiRsMeasureOpen(void);
#endif
