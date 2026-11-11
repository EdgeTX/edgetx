/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * V15 external-port CRSF measure (J10 pin3 / PJ8 half-duplex).
 * Auto-detects baud rate and inverted / non-inverted polarity.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define CRSF_MEASURE_MAX_CHANNELS 16
#define CRSF_MEASURE_CH_MIN_US 800
#define CRSF_MEASURE_CH_MAX_US 2200
#define CRSF_MEASURE_SIGNAL_TIMEOUT_10MS 30  // 300 ms
#define CRSF_MEASURE_CONFIG_TRY_10MS 12      // 120 ms per baud×polarity
#define CRSF_MEASURE_OC_DELTA_MA 1000
/** Frame snapshot for UI digital waveform (full CHANNELS ~26 B). */
#define CRSF_MEASURE_WAVE_BYTES 40

/** Polarity reported to UI. */
enum {
  CRSF_MEASURE_POL_SEARCHING = 0,
  CRSF_MEASURE_POL_INVERTED = 1,
  CRSF_MEASURE_POL_NORMAL = 2,
};

#if defined(__cplusplus)
extern "C" {
#endif

bool v15CrsfMeasureIsActive(void);
bool v15CrsfMeasureHasSignal(void);
bool v15CrsfMeasureIsPowerOn(void);
bool v15CrsfMeasureOvercurrentFault(void);
int16_t v15CrsfMeasureGetCurrentMa(void);

uint8_t v15CrsfMeasureGetPolarity(void);
uint32_t v15CrsfMeasureGetBaudrate(void);

uint8_t v15CrsfMeasureGetChannelCount(void);
uint16_t v15CrsfMeasureGetChannelUs(uint8_t ch);
uint16_t v15CrsfMeasureGetChannelRaw(uint8_t ch);
uint16_t v15CrsfMeasureGetFrameHz(void);

/** Monotonic counter bumped on each valid CHANNELS frame (for UI wave). */
uint32_t v15CrsfMeasureGetWaveSeq(void);
/** Copy latest frame bytes for digital wave preview; returns length copied. */
uint8_t v15CrsfMeasureCopyWaveBytes(uint8_t* dst, uint8_t maxLen);

void v15CrsfMeasureStart(void);
void v15CrsfMeasureStop(void);
void v15CrsfMeasureTask(void);

#if defined(__cplusplus)
}

void v15CrsfMeasureOpen(void);
#endif
