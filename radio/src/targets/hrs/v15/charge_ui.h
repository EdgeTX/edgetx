/*
 * Copyright (C) EdgeTX
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

/** Full-screen TX USB charge telemetry (V15 + CSD203 + charger stat GPIO). */
void v15ChargeUiTask(void);

/** V15: if charger LED is active at power-down, drive charge UI until it clears, then exit. */
#if defined(RADIO_V15) && defined(MODULE_BATTERY_SENSOR) && defined(USB_CHARGER)
void v15ShutdownWaitIfCharging(void);
#endif
