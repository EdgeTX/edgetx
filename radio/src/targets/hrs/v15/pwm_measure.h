/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * V15 external-port receiver PWM pulse-width measure (J10 pin3 / PJ8).
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PWM_MEASURE_PULSE_MIN_US 500
#define PWM_MEASURE_PULSE_MAX_US 2500
#define PWM_MEASURE_VIEW_US 2500
/** Compensate SSR / path rising-edge delay (measured ~2 us low). */
#define PWM_MEASURE_PULSE_OFFSET_US 2
#define PWM_MEASURE_PERIOD_MIN_US 2000   // ~500 Hz
#define PWM_MEASURE_PERIOD_MAX_US 50000  // ~20 Hz
#define PWM_MEASURE_SIGNAL_TIMEOUT_10MS 20  // 200 ms without edge => lost
#define PWM_MEASURE_UI_HZ 20
/** Same trip threshold as servo tester (+mA vs previous / baseline). */
#define PWM_MEASURE_OC_DELTA_MA 1000

#if defined(__cplusplus)
extern "C" {
#endif

bool v15PwmMeasureIsActive(void);
bool v15PwmMeasureHasSignal(void);
bool v15PwmMeasureIsPowerOn(void);
bool v15PwmMeasureOvercurrentFault(void);
int16_t v15PwmMeasureGetCurrentMa(void);

uint16_t v15PwmMeasureGetPulseUs(void);
uint16_t v15PwmMeasureGetPeriodUs(void);
uint16_t v15PwmMeasureGetFreqHz(void);
uint16_t v15PwmMeasureGetMinUs(void);
uint16_t v15PwmMeasureGetMaxUs(void);

void v15PwmMeasureResetStats(void);

void v15PwmMeasureStart(void);
void v15PwmMeasureStop(void);

/** Call from UI / main loop (~10–50 Hz). */
void v15PwmMeasureTask(void);

#if defined(__cplusplus)
}

void v15PwmMeasureOpen(void);
#endif
