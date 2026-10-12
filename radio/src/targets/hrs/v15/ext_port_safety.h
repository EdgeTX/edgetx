/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * V15 EXT-port safety gate: probe jack voltage with MCU signal path closed.
 * Path (CHIP_FUN) may open only when measured voltage is <= 5.0 V.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/** Reject opening the MCU signal path when jack voltage is above this (mV). */
#define EXT_PORT_SAFE_VOLTAGE_MAX_MV 5000

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Probe EXT signal-entry voltage (BAT-S2 / HC138 Y1) with CHIP_FUN off
 * and port +5V off. Does not use cell or system mux channels.
 * Returns true if safe to open the MCU signal path (voltage <= 5.0 V).
 * On failure to sense, treats as unsafe (fail closed).
 */
bool v15ExtPortProbeSignalSafe(uint16_t* voltageMv);

/** Probe then open CHIP_FUN only if safe (probe briefly cuts +5V). */
bool v15ExtPortTryOpenSignalPath(void);

/** Open CHIP_FUN without probe; leaves port +5V unchanged. */
void v15ExtPortEnsureSignalPathOpen(void);

/** Force-close MCU signal path (CHIP_FUN low). */
void v15ExtPortCloseSignalPath(void);

/** True after the last TryOpen/Probe blocked due to high voltage or sense fail. */
bool v15ExtPortIsBlocked(void);
uint16_t v15ExtPortLastVoltageMv(void);
/** Static UTF-8 message for POPUP_WARNING / status line. */
const char* v15ExtPortBlockedMessage(void);

#if defined(__cplusplus)
}
#endif
