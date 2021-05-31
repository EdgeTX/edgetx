/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _STRHELPERS_H_
#define _STRHELPERS_H_

#include "opentx_types.h"

char hex2zchar(uint8_t hex);
char hex2char(uint8_t hex);
char zchar2char(int8_t idx);
char char2lower(char c);
int8_t char2zchar(char c);
void str2zchar(char *dest, const char *src, int size);
int zchar2str(char *dest, const char *src, int size);

char * strAppend(char * dest, const char * source, int len = 0);
char * strAppendUnsigned(char * dest, uint32_t value, uint8_t digits = 0, uint8_t radix = 10);
char * strAppendSigned(char * dest, int32_t value, uint8_t digits = 0, uint8_t radix = 10);
char * strSetCursor(char * dest, int position);
char * strAppendDate(char * str, bool time=false);
char * strAppendFilename(char * dest, const char * filename, const int size);

#if !defined(BOOT)
char * getStringAtIndex(char * dest, const char * s, int idx);
char * strAppendStringWithIndex(char * dest, const char * s, int idx);
#define LEN_TIMER_STRING               10 // "-00:00:00"
char * getTimerString(char * dest, int tme, uint8_t hours=0);
char * getCurveString(char * dest, int idx);
char * getGVarString(char * dest, int idx);
char * getGVarString(int idx);
char * getSwitchPositionName(char * dest, swsrc_t idx);
char * getSwitchName(char * dest, swsrc_t idx);
char * getSourceString(char * dest, mixsrc_t idx);
#endif

char * getFlightModeString(char * dest, int8_t idx);
#define SWITCH_WARNING_STR_SIZE 3
char * getSwitchWarningString(char * dest, swsrc_t idx);

char * getSourceString(mixsrc_t idx);
char * getSwitchPositionName(swsrc_t idx);
char * getSwitchWarningString(swsrc_t idx);
char * getCurveString(int idx);
char * getTimerString(int32_t tme, uint8_t hours);

#endif // _STRHELPERS_H_
