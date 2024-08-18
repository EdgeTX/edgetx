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

#pragma once

#include <cstring>
#include <string>

#include "definitions.h"
#include "edgetx_types.h"

class TelemetryItem;

#define IS_UFT8_2BYTES(c) ((((uint8_t)c) & 0xE0) == 0xC0)

#define IS_UTF8_3BYTES(c) ((((uint8_t)c) & 0xF0) == 0xE0)

#define IS_UTF8(c) (IS_UFT8_2BYTES(c) || IS_UTF8_3BYTES(c))

#define UTF8_WIDTH(c) (IS_UFT8_2BYTES(c) ? 2 : IS_UTF8_3BYTES(c) ? 3 : 1)

#define SHOW_TIME 0x1
#define SHOW_TIMER 0x0
#define SHOW_TIMER_UPPER_CASE 0x2
#define SHOW_TIMER_HM_FORMAT 0x5

PACK(typedef struct {
  uint8_t showTime : 1;
  uint8_t upperCase : 1;
  uint8_t numDigitGroups : 3;
  uint8_t hmFormat : 1;
  uint8_t reserved : 2;
})
TimerDisplayOptions;

typedef union {
  uint8_t options;
  TimerDisplayOptions displayOptions;
} TimerOptions;

const char* sanitizeForFilename(const char* name, int len);

char hex2zchar(uint8_t hex);
char hex2char(uint8_t hex);
char zchar2char(int8_t idx);
char char2lower(char c);
int8_t char2zchar(char c);
void str2zchar(char *dest, const char *src, int size);
int zchar2str(char *dest, const char *src, int size);
int strnlen(const char *src, int max_size);
unsigned int effectiveLen(const char *str, unsigned int size);

char *strAppend(char *dest, const char *source, int len = 0);
char *strAppendUnsigned(char *dest, uint32_t value, uint8_t digits = 0,
                        uint8_t radix = 10);
char *strAppendSigned(char *dest, int32_t value, uint8_t digits = 0,
                      uint8_t radix = 10);
char *strSetCursor(char *dest, int position);
char *strAppendDate(char *str, bool time = false);
char *strAppendFilename(char *dest, const char *filename, const int size);
std::string formatNumberAsString(int32_t val, LcdFlags flags = 0,
                                 uint8_t len = 0, const char *prefix = nullptr,
                                 const char *suffix = nullptr);
void formatNumberAsString(char *buffer, const uint8_t buffer_size, int32_t val,
                          LcdFlags flags = 0, uint8_t len = 0,
                          const char *prefix = nullptr,
                          const char *suffix = nullptr);

#if !defined(BOOT)
char *getStringAtIndex(char *dest, const char *const *s, int idx);
char *strAppendStringWithIndex(char *dest, const char *s, int idx);
#define LEN_TIMER_STRING 10  // "-00:00:00"
char *getTimerString(char *dest, int32_t tme,
                     TimerOptions timerOptions = {.options = 0});
char *getFormattedTimerString(char *dest, int32_t tme,
                              TimerOptions timerOptions);
char *getCurveString(char *dest, int idx);
char *getGVarString(char *dest, int idx);
char *getGVarString(int idx);
char *getValueOrGVarString(char *dest, size_t len, gvar_t value, gvar_t vmin,
                           gvar_t vmax, LcdFlags flags = 0,
                           const char *suffix = nullptr, gvar_t offset = 0, bool usePPMUnit = false);
char *getValueOrSrcVarString(char *dest, size_t len, gvar_t value, gvar_t vmin,
                             gvar_t vmax, LcdFlags flags = 0,
                             const char *suffix = nullptr, gvar_t offset = 0, bool usePPMUnit = false);
const char *getSwitchWarnSymbol(uint8_t pos);
const char *getSwitchPositionSymbol(uint8_t pos);
char *getSwitchPositionName(char *dest, swsrc_t idx);
char *getSwitchName(char *dest, uint8_t idx);

const char *getAnalogLabel(uint8_t type, uint8_t idx);
const char *getAnalogShortLabel(uint8_t idx);
const char *getMainControlLabel(uint8_t idx);
const char *getTrimLabel(uint8_t idx);
const char *getTrimSourceLabel(uint16_t src_raw, int8_t trim_src);
const char *getPotLabel(uint8_t idx);
char *getCustomSwitchesGroupName(char *dest, uint8_t idx);

template <size_t L>
char *getSourceString(char (&dest)[L], mixsrc_t idx);

template <size_t L>
char *getSourceCustomValueString(char (&dest)[L], mixsrc_t source, int32_t val,
                                 LcdFlags flags);

#endif

char *getFlightModeString(char *dest, int8_t idx);

char *getSourceString(mixsrc_t idx);
char *getSourceCustomValueString(mixsrc_t source, int32_t val, LcdFlags flags);
char *getSwitchPositionName(swsrc_t idx);
char *getCurveString(int idx);
char *getTimerString(int32_t tme, TimerOptions timerOptions = {.options = 0});
void splitTimer(char *s0, char *s1, char *s2, char *s3, int tme,
                bool bLowercase = true);

template <size_t N>
std::string stringFromNtString(const char (&a)[N])
{
  return std::string(a, strnlen(a, N));
}
template <size_t L>
void copyToUnTerminated(char (&dest)[L], const char *const src)
{
  strncpy(dest, src, L);
}
template <size_t L>
void copyToUnTerminated(char (&dest)[L], const std::string &src)
{
  strncpy(dest, src.c_str(), L);
}
template <typename S>
void clearStruct(S &s)
{
  memset((void *)&s, 0, sizeof(S));
}

template <size_t N>
using offset_t = std::integral_constant<size_t, N>;

template <size_t L1, size_t L2>
int strncasecmp(char (&s1)[L1], const char (&s2)[L2])
{
  static constexpr size_t len = (L1 < L2) ? L1 : L2;
  return strncasecmp(s1, s2, len);
}

template <size_t L1>
int strncasecmp(char (&s1)[L1], const char *const s2)
{
  return strncasecmp(s1, s2, L1);
}

std::string getValueWithUnit(int val, uint8_t unit, LcdFlags flags);
std::string getGVarValue(uint8_t gvar, gvar_t value, LcdFlags flags);
std::string getGPSCoord(int32_t value, const char *direction,
                        bool seconds = true);
std::string getGPSSensorValue(int32_t longitude, int32_t latitude,
                              LcdFlags flags);
std::string getGPSSensorValue(TelemetryItem &telemetryItem, LcdFlags flags);
std::string getTelemDate(TelemetryItem &telemetryItem);
std::string getTelemTime(TelemetryItem &telemetryItem);

int countDigits(int number);

// Timezone handling
extern int8_t minTimezone();
extern int8_t maxTimezone();
extern std::string timezoneDisplay(int tz);
extern int timezoneIndex(int8_t tzHour, int8_t tzMinute);
extern int8_t timezoneHour(int tz);
extern int8_t timezoneMinute(int tz);
extern int timezoneOffsetSeconds(int8_t tzHour, int8_t tzMinute);
