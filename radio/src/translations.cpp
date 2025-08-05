/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "edgetx.h"

const char CHR_HOUR = TR_CHR_HOUR;
const char CHR_INPUT = TR_CHR_INPUT;

#define TR_MONITOR_CHANNELS TR_MONITOR_CHANNELS1, TR_MONITOR_CHANNELS2, TR_MONITOR_CHANNELS3, TR_MONITOR_CHANNELS4
#define TR_TIMER_MODES      TR_OFFON,TR_START,TR_THROTTLE_LABEL,TR_THROTTLE_PERCENT_LABEL,TR_THROTTLE_START
#define TR_PHASES_HEADERS   TR_PHASES_HEADERS_NAME, TR_PHASES_HEADERS_SW, TR_PHASES_HEADERS_RUD_TRIM, TR_PHASES_HEADERS_ELE_TRIM, TR_PHASES_HEADERS_THT_TRIM, TR_PHASES_HEADERS_AIL_TRIM, TR_PHASES_HEADERS_FAD_IN, TR_PHASES_HEADERS_FAD_OUT
#define TR_LIMITS_HEADERS   TR_LIMITS_HEADERS_NAME, TR_LIMITS_HEADERS_SUBTRIM, TR_LIMITS_HEADERS_MIN, TR_LIMITS_HEADERS_MAX, TR_LIMITS_HEADERS_DIRECTION, TR_LIMITS_HEADERS_CURVE, TR_LIMITS_HEADERS_PPMCENTER, TR_LIMITS_HEADERS_SUBTRIMMODE
#define TR_LSW_HEADERS      TR_FUNC, TR_V1, TR_V2, TR_V2, TR_AND_SWITCH, TR_DURATION, TR_DELAY, TR_PERSISTENT

// Static string
#define STR(x) const char STR_##x[] = TR_##x
// Static string array
#define STRARRAY(x) const char* const STR_##x[] = { TR_##x }

#include "string_list.h"

#if defined(COLORLCD)
STR(SF_SET_SCREEN);
#else
const char STR_SF_SET_SCREEN[] = TR_SF_SET_TELEMSCREEN;
#endif

#if defined(AUDIO) && defined(BUZZER)
STR(SPEAKER);
STR(BUZZER);
#endif

const char STR_RX[] = "Rx";
