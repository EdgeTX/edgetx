/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * V15 external-port MAVLink measure (J10 pin3 / PJ8 half-duplex).
 * Auto-tries baud × polarity until valid MAVLink frames lock.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAVLINK_MEASURE_SIGNAL_TIMEOUT_10MS 50  // 500 ms
#define MAVLINK_MEASURE_POLARITY_TRY_10MS 20    // 200 ms per baud×polarity
#define MAVLINK_MEASURE_OC_DELTA_MA 1000
#define MAVLINK_MEASURE_MAX_LINES 12
#define MAVLINK_MEASURE_LINE_TEXT_LEN 52

/** Polarity reported to UI. */
enum {
  MAVLINK_MEASURE_POL_SEARCHING = 0,
  MAVLINK_MEASURE_POL_INVERTED = 1,
  MAVLINK_MEASURE_POL_NORMAL = 2,
};

#if defined(__cplusplus)
extern "C" {
#endif

bool v15MavlinkMeasureIsActive(void);
bool v15MavlinkMeasureHasSignal(void);
bool v15MavlinkMeasureIsPowerOn(void);
bool v15MavlinkMeasureOvercurrentFault(void);
int16_t v15MavlinkMeasureGetCurrentMa(void);

uint8_t v15MavlinkMeasureGetPolarity(void);
uint32_t v15MavlinkMeasureGetBaudrate(void);
/** 1 = MAVLink v1, 2 = MAVLink v2. */
uint8_t v15MavlinkMeasureGetVersion(void);
uint8_t v15MavlinkMeasureGetSysId(void);
uint8_t v15MavlinkMeasureGetCompId(void);
uint32_t v15MavlinkMeasureGetMsgId(void);
uint16_t v15MavlinkMeasureGetMsgHz(void);

uint8_t v15MavlinkMeasureGetLineCount(void);
/** Copy one display line (0 .. GetLineCount()-1); returns bytes written. */
uint8_t v15MavlinkMeasureGetLineText(uint8_t index, char* buf, uint8_t buflen);

void v15MavlinkMeasureStart(void);
void v15MavlinkMeasureStop(void);
void v15MavlinkMeasureTask(void);

#if defined(__cplusplus)
}

void v15MavlinkMeasureOpen(void);
#endif
