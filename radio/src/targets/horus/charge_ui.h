/*
 * Copyright (C) EdgeTX
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#if defined(V16_CHARGE_UI)

/** Full-screen TX USB charge telemetry (V16: CSD203 + charger stat GPIO). */
void chargeUiTask(void);

/** If charger LED is active at power-down, drive charge UI until it clears, then exit. */
void shutdownWaitIfCharging(void);

#endif
