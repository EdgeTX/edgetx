/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * Feed Ext RX measure channels into trainerInput when trainer mode is MASTER.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

/** Mixer-rate housekeeping (trainer connect warning). */
void v15MeasureTrainerFeedTask(void);

/**
 * Push 11-bit SBUS/CRSF slots into trainerInput (stock bus scale).
 * Call from measure frame RX path for full wire frame rate.
 */
void v15MeasureTrainerFeedPushBusRaw(const uint16_t* raw11, uint8_t count);

/**
 * Push channel widths in microseconds into trainerInput (stock CPPM scale).
 * Call from PPM / SUMD frame path (mid 1500 us).
 */
void v15MeasureTrainerFeedPushUs(const uint16_t* us, uint8_t count);

/**
 * Push ELRS DJI RS Pro channel units (mid 1024, sticks ~352..1696).
 * Maps to standard servo us then stock CPPM trainer scale.
 */
void v15MeasureTrainerFeedPushDjiRs(const uint16_t* ch, uint8_t count);

bool v15MeasureTrainerFeedIsActive(void);

#if defined(__cplusplus)
}
#endif
