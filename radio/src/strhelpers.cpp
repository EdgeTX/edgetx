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

#include "opentx.h"
#include "strhelpers.h"

#if !defined(BOOT)
const char s_charTab[] = "_-.,";

char hex2zchar(uint8_t hex)
{
  return (hex >= 10 ? hex-9 : 27+hex);
}

char hex2char(uint8_t hex)
{
  return (hex >= 10 ? hex-9+'A' : hex+'0');
}

char zchar2char(int8_t idx)
{
  if (idx == 0) return ' ';
  if (idx < 0) {
    if (idx > -27) return 'a' - idx - 1;
    idx = -idx;
  }
  if (idx < 27) return 'A' + idx - 1;
  if (idx < 37) return '0' + idx - 27;
  if (idx <= 40) return *(s_charTab+idx-37);
#if LEN_SPECIAL_CHARS > 0
  if (idx <= (LEN_STD_CHARS + LEN_SPECIAL_CHARS)) return 'z' + 5 + idx - 40;
#endif
  return ' ';
}

char char2lower(char c)
{
  return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}

int8_t char2zchar(char c)
{
  if (c == '_') return 37;
#if LEN_SPECIAL_CHARS > 0
  if ((int8_t)c < 0 && c+128 <= LEN_SPECIAL_CHARS) return 41 + (c+128);
#endif
  if (c >= 'a') return 'a' - c - 1;
  if (c >= 'A') return c - 'A' + 1;
  if (c >= '0') return c - '0' + 27;
  if (c == '-') return 38;
  if (c == '.') return 39;
  if (c == ',') return 40;
  return 0;
}

void str2zchar(char * dest, const char * src, int size)
{
  memset(dest, 0, size);
  for (int c=0; c<size && src[c]; c++) {
    dest[c] = char2zchar(src[c]);
  }
}

int zchar2str(char * dest, const char * src, int size)
{
  for (int c=0; c<size; c++) {
    dest[c] = zchar2char(src[c]);
  }
  do {
    dest[size--] = '\0';
  } while (size >= 0 && dest[size] == ' ');
  return size+1;
}

int strnlen(const char *src, int max_size)
{
  for (int i=0; i<max_size; i++) {
    if (src[i] == '\0')
      return i;
  }

  return max_size;
}

unsigned int effectiveLen(const char * str, unsigned int size)
{
  while (size > 0) {
    if (str[size-1] != ' ' && str[size-1] != '\0')
      return size;
    size--;
  }
  return 0;
}

bool zexist(const char * str, uint8_t size)
{
  for (int i=0; i<size; i++) {
    if (str[i] != 0)
      return true;
  }
  return false;
}

uint8_t zlen(const char * str, uint8_t size)
{
  while (size > 0) {
    if (str[size-1] != 0)
      return size;
    size--;
  }
  return 0;
}

char * strcat_zchar(char * dest, const char * name, uint8_t size, const char spaceSym, const char * defaultName, uint8_t defaultNameSize, uint8_t defaultIdx)
{
  int8_t len = 0;

  if (name) {
    memcpy(dest, name, size);
    dest[size] = '\0';

    int8_t i = size-1;

    while (i>=0) {
      if (!len && dest[i])
        len = i+1;
      if (len) {
        if (dest[i] == spaceSym)
          dest[i] = '_';
      }
      i--;
    }
  }

  if (len == 0 && defaultName) {
    strcpy(dest, defaultName);
    dest[defaultNameSize] = (char)((defaultIdx / 10) + '0');
    dest[defaultNameSize + 1] = (char)((defaultIdx % 10) + '0');
    len = defaultNameSize + 2;
  }

  return &dest[len];
}
#endif

#if !defined(BOOT)
char * getStringAtIndex(char * dest, const char * s, int idx)
{
  uint8_t len = s[0];
  strncpy(dest, s+1+len*idx, len);
  dest[len] = '\0';
  return dest;
}

char * strAppendStringWithIndex(char * dest, const char * s, int idx)
{
  return strAppendUnsigned(strAppend(dest, s), abs(idx));
}

constexpr int secondsPerDay = 24 * 3600;
constexpr int secondsPer99Hours = 99*3600 + 59*60 + 59;
constexpr int secondsPerYear = 365 * secondsPerDay;

char * getTimerString(char * dest, int tme, uint8_t hours)
{
  char * s = dest;
  div_t qr;

  if (tme < 0) {
    tme = -tme;
    *s++ = '-';
  }

  if (tme < secondsPerDay) {
    qr = div((int) tme, 60);

    if (hours) {
      div_t qr2 = div(qr.quot, 60);
      *s++ = '0' + (qr2.quot / 10);
      *s++ = '0' + (qr2.quot % 10);
      *s++ = ':';
      qr.quot = qr2.rem;
    }

    if (!hours && qr.quot > 99) {
      *s++ = '0' + (qr.quot / 100);
      qr.quot = qr.quot % 100;
    }

    *s++ = '0' + (qr.quot / 10);
    *s++ = '0' + (qr.quot % 10);
    *s++ = ':';
    *s++ = '0' + (qr.rem / 10);
    *s++ = '0' + (qr.rem % 10);
    *s = '\0';
  }
  else if (tme < secondsPer99Hours) {
    qr = div(tme, 3600);
    div_t qr2 = div(qr.rem, 60);
    *s++ = '0' + (qr.quot / 10);
    *s++ = '0' + (qr.quot % 10);
    *s++ = 'H';
    *s++ = '0' + (qr2.quot / 10);
    *s++ = '0' + (qr2.quot % 10);
    *s = '\0';
  }
  else if (tme < secondsPerYear) {
    qr = div(tme, secondsPerDay);
    div_t qr2 = div(qr.rem, 60);
    *s++ = '0' + (qr.quot / 100);
    *s++ = '0' + (qr.quot / 10);
    *s++ = '0' + (qr.quot % 10);
    *s++ = 'D';
    *s++ = '0' + (qr2.quot / 10);
    *s++ = '0' + (qr2.quot % 10);
    *s++ = 'H';
    *s = '\0';
  }
  else {
    qr = div(tme, secondsPerYear);
    div_t qr2 = div(qr.rem, secondsPerDay);
    *s++ = '0' + (qr.quot / 10);
    *s++ = '0' + (qr.quot % 10);
    *s++ = 'Y';
    *s++ = 'Y';
    *s++ = '0' + (qr2.quot / 10);
    *s++ = '0' + (qr2.quot % 10);
    *s++ = 'D';
    *s = '\0';
  }
  return dest;
}

char * getCurveString(char * dest, int idx)
{
  if (idx == 0) {
    return getStringAtIndex(dest, STR_MMMINV, 0);
  }

  char * s = dest;
  if (idx < 0) {
    *s++ = '!';
    idx = -idx;
  }

  if (g_model.curves[idx - 1].name[0])
    strAppend(s, g_model.curves[idx - 1].name, LEN_CURVE_NAME);
  else
    strAppendStringWithIndex(s, STR_CV, idx);

  return dest;
}

char * getGVarString(char * dest, int idx)
{
  char * s = dest;
  if (idx < 0) {
    *s++ = '-';
    idx = -idx-1;
  }

  if (g_model.gvars[idx].name[0])
    strAppend(s, g_model.gvars[idx].name, LEN_GVAR_NAME);
  else
    strAppendStringWithIndex(s, STR_GV, idx+1);

  return dest;
}

char * getFlightModeString(char * dest, int8_t idx)
{
  char * s = dest;

  if (idx==0) {
    strcpy(s, "---");
    return dest;
  }

  if (idx < 0) {
    *s++ = '!';
    idx = -idx;
  }

  s = strAppend(s, STR_FM);
  strAppendUnsigned(s, idx - 1);
  return dest;
}

char * getSwitchWarningString(char * dest, swsrc_t idx)
{
  char * s = dest;
  uint8_t state = g_model.switchWarningState >> (3*idx) & 0x07;

  *s++ = 'A' + idx;
  if (state == 0)
    *s = '\0';
  else {
    *s++ = "x\300-\301"[state];
    *s = '\0';
  }
  return dest;
}

#if !defined(PCBSKY9X)
char * getSwitchName(char * dest, swsrc_t idx)
{
  div_t swinfo = switchInfo(idx);
  if (g_eeGeneral.switchNames[swinfo.quot][0] != '\0') {
    dest = strAppend(dest, g_eeGeneral.switchNames[swinfo.quot], LEN_SWITCH_NAME);
  }
  else {
    *dest++ = 'S';
#if defined(PCBX7) && !defined(RADIO_TX12)
    if (swinfo.quot >= 5)
        *dest++ = 'H' + swinfo.quot - 5;
      else if (swinfo.quot == 4)
#if defined(RADIO_T12)
        *dest++ = 'G';
#else
        *dest++ = 'F';
#endif
      else
        *dest++ = 'A'+swinfo.quot;
#else
    *dest++ = 'A' + swinfo.quot;
#endif
  }
  return dest;
}
#endif

char * getSwitchPositionName(char * dest, swsrc_t idx)
{
  if (idx == SWSRC_NONE) {
    return getStringAtIndex(dest, STR_VSWITCHES, 0);
  }
  else if (idx == SWSRC_OFF) {
    return getStringAtIndex(dest, STR_OFFON, 0);
  }

  char * s = dest;
  if (idx < 0) {
    *s++ = '!';
    idx = -idx;
  }

#if defined(PCBSKY9X)
  #define IDX_TRIMS_IN_STR_VSWITCHES   (1+SWSRC_LAST_SWITCH)
  #define IDX_ON_IN_STR_VSWITCHES      (IDX_TRIMS_IN_STR_VSWITCHES+SWSRC_LAST_TRIM-SWSRC_FIRST_TRIM+2)
  if (idx <= SWSRC_LAST_SWITCH) {
    getStringAtIndex(s, STR_VSWITCHES, idx);
  }
#else
#define IDX_TRIMS_IN_STR_VSWITCHES   (1)
#define IDX_ON_IN_STR_VSWITCHES      (IDX_TRIMS_IN_STR_VSWITCHES + SWSRC_LAST_TRIM - SWSRC_FIRST_TRIM + 1)
  if (idx <= SWSRC_LAST_SWITCH) {
    div_t swinfo = switchInfo(idx);
    s = getSwitchName(s, idx);
    *s++ = (STR_CHAR_UP "-" STR_CHAR_DOWN)[swinfo.rem];
    *s = '\0';
  }
#endif // PCBSKY9X

#if NUM_XPOTS > 0
  else if (idx <= SWSRC_LAST_MULTIPOS_SWITCH) {
    div_t swinfo = div(int(idx - SWSRC_FIRST_MULTIPOS_SWITCH), XPOTS_MULTIPOS_COUNT);
    char temp[LEN_ANA_NAME+1];
    getSourceString(temp, MIXSRC_FIRST_POT+swinfo.quot);
    temp[LEN_ANA_NAME]= '\0';
    strAppendStringWithIndex(s, temp, swinfo.rem+1);
  }
#endif

  else if (idx <= SWSRC_LAST_TRIM) {
    getStringAtIndex(s, STR_VSWITCHES, IDX_TRIMS_IN_STR_VSWITCHES+idx-SWSRC_FIRST_TRIM);
  }
  else if (idx <= SWSRC_LAST_LOGICAL_SWITCH) {
    *s++ = 'L';
    strAppendUnsigned(s, idx-SWSRC_FIRST_LOGICAL_SWITCH+1, 2);
  }
  else if (idx <= SWSRC_ONE) {
    getStringAtIndex(s, STR_VSWITCHES, IDX_ON_IN_STR_VSWITCHES + idx - SWSRC_ON);
  }
  else if (idx <= SWSRC_LAST_FLIGHT_MODE) {
    strAppendStringWithIndex(s, STR_FM, idx-SWSRC_FIRST_FLIGHT_MODE);
  }
  else if (idx == SWSRC_TELEMETRY_STREAMING) {
    strcpy(s, "Tele");
  }
  else if (idx == SWSRC_RADIO_ACTIVITY) {
    strcpy(s, "Act");
  }
#if defined(DEBUG_LATENCY)
  else if (idx == SWSRC_LATENCY_TOGGLE) {
    strcpy(s, "Ltc");
  }
#endif
  else {
    strncpy(s, g_model.telemetrySensors[idx-SWSRC_FIRST_SENSOR].label, TELEM_LABEL_LEN);
    s[TELEM_LABEL_LEN] = '\0';
  }

  return dest;
}

char * getSourceString(char * dest, mixsrc_t idx)
{
  if (idx == MIXSRC_NONE) {
    return getStringAtIndex(dest, STR_VSRCRAW, 0);
  }
  else if (idx <= MIXSRC_LAST_INPUT) {
    idx -= MIXSRC_FIRST_INPUT;
    *dest = CHAR_INPUT;
    if (strlen(g_model.inputNames[idx])) {
      memset(dest + 1, 0, LEN_INPUT_NAME+1);
      strncpy(dest + 1, g_model.inputNames[idx], LEN_INPUT_NAME);
    }
    else {
      strAppendUnsigned(dest + 1, idx + 1, 2);
    }
  }
#if defined(LUA_INPUTS)
  else if (idx <= MIXSRC_LAST_LUA) {
#if defined(LUA_MODEL_SCRIPTS)
    div_t qr = div(idx-MIXSRC_FIRST_LUA, MAX_SCRIPT_OUTPUTS);
    if (qr.quot < MAX_SCRIPTS && qr.rem < scriptInputsOutputs[qr.quot].outputsCount) {
      *dest++ = CHAR_LUA;
      // *dest++ = '1'+qr.quot;
      strcpy(dest, scriptInputsOutputs[qr.quot].outputs[qr.rem].name);
    }
#else
    strcpy(dest, "N/A");
#endif
  }
#endif
  else if (idx <= MIXSRC_LAST_POT) {

    if (g_eeGeneral.anaNames[idx - MIXSRC_Rud][0]) {

      //TODO: add correct symbol
      if(idx <= MIXSRC_LAST_STICK) {
        dest[0] = CHAR_STICK;
#if NUM_SLIDERS > 0
      } else if (idx < MIXSRC_FIRST_SLIDER) {
        dest[0] = CHAR_POT;
      } else {
        dest[0] = CHAR_SLIDER;
#else
      } else {
        dest[0] = CHAR_POT;
#endif
      }

      idx -= MIXSRC_Rud;
      memcpy(dest+1, g_eeGeneral.anaNames[idx], LEN_ANA_NAME);
      dest[LEN_ANA_NAME+1] = '\0';
    }
    else {
      idx -= MIXSRC_Rud;
      getStringAtIndex(dest, STR_VSRCRAW, idx + 1);
    }
  }
  else if (idx <= MIXSRC_LAST_TRIM) {
    idx -= MIXSRC_Rud;
    getStringAtIndex(dest, STR_VSRCRAW, idx + 1);
  }
  else if (idx <= MIXSRC_LAST_SWITCH) {
    idx -= MIXSRC_FIRST_SWITCH;
    if (g_eeGeneral.switchNames[idx][0] != '\0') {
      strncpy(dest, g_eeGeneral.switchNames[idx], LEN_SWITCH_NAME);
      dest[LEN_SWITCH_NAME] = '\0';
    }
    else {
      getStringAtIndex(dest, STR_VSRCRAW, idx + MIXSRC_FIRST_SWITCH - MIXSRC_Rud + 1);
    }
  }
  else if (idx <= MIXSRC_LAST_LOGICAL_SWITCH) {
    getSwitchPositionName(dest, SWSRC_SW1 + idx - MIXSRC_SW1);
  }
  else if (idx <= MIXSRC_LAST_TRAINER) {
    strAppendStringWithIndex(dest, STR_PPM_TRAINER, idx - MIXSRC_FIRST_TRAINER + 1);
  }
  else if (idx <= MIXSRC_LAST_CH) {
    strAppendStringWithIndex(dest, STR_CH, idx - MIXSRC_CH1 + 1);
  }
  else if (idx <= MIXSRC_LAST_GVAR) {
    strAppendStringWithIndex(dest, STR_GV, idx - MIXSRC_GVAR1 + 1);
  }
  else if (idx < MIXSRC_FIRST_TIMER) {
    getStringAtIndex(dest, STR_VSRCRAW, idx-MIXSRC_Rud+1-MAX_LOGICAL_SWITCHES-MAX_TRAINER_CHANNELS-MAX_OUTPUT_CHANNELS-MAX_GVARS);
  }
  else if (idx <= MIXSRC_LAST_TIMER) {
    if(g_model.timers[idx-MIXSRC_FIRST_TIMER].name[0] != '\0') {
      strncpy(dest,g_model.timers[idx-MIXSRC_FIRST_TIMER].name, LEN_TIMER_NAME);
      dest[LEN_TIMER_NAME] = '\0';
    }
    else {
      getStringAtIndex(dest, STR_VSRCRAW, idx-MIXSRC_Rud+1-MAX_LOGICAL_SWITCHES-MAX_TRAINER_CHANNELS-MAX_OUTPUT_CHANNELS-MAX_GVARS);
    }
  }
  else {
    idx -= MIXSRC_FIRST_TELEM;
    div_t qr = div(idx, 3);
    dest[0] = CHAR_TELEMETRY;
    char  * pos = strAppend(&dest[1], g_model.telemetrySensors[qr.quot].label, sizeof(g_model.telemetrySensors[qr.quot].label));
    if (qr.rem) * pos = (qr.rem==2 ? '+' : '-');
    * ++pos = '\0';
  }

  return dest;
}
#endif

char * strAppendUnsigned(char * dest, uint32_t value, uint8_t digits, uint8_t radix)
{
  if (digits == 0) {
    unsigned int tmp = value;
    digits = 1;
    while (tmp >= radix) {
      ++digits;
      tmp /= radix;
    }
  }
  uint8_t idx = digits;
  while (idx > 0) {
    div_t qr = div(value, radix);
    dest[--idx] = (qr.rem >= 10 ? 'A' - 10 : '0') + qr.rem;
    value = qr.quot;
  }
  dest[digits] = '\0';
  return &dest[digits];
}

char * strAppendSigned(char * dest, int32_t value, uint8_t digits, uint8_t radix)
{
  if (value < 0) {
    *dest++ = '-';
    value = -value;
  }
  return strAppendUnsigned(dest, (uint32_t)value, digits, radix);
}

char * strAppend(char * dest, const char * source, int len)
{
  while ((*dest++ = *source++)) {
    if (--len == 0) {
      *dest = '\0';
      return dest;
    }
  }
  return dest - 1;
}

char * strSetCursor(char * dest, int position)
{
  *dest++ = 0x1F;
  *dest++ = position;
  *dest = '\0';
  return dest;
}

char * strAppendFilename(char * dest, const char * filename, const int size)
{
  memset(dest, 0, size);
  for (int i=0; i<size; i++) {
    char c = *filename++;
    if (c == '\0' || c == '.') {
      *dest = 0;
      break;
    }
    *dest++ = c;
  }
  return dest;
}

#if defined(RTCLOCK)
#include "rtc.h"

char * strAppendDate(char * str, bool time)
{
  str[0] = '-';
  struct gtm utm;
  gettime(&utm);
  div_t qr = div(utm.tm_year+TM_YEAR_BASE, 10);
  str[4] = '0' + qr.rem;
  qr = div(qr.quot, 10);
  str[3] = '0' + qr.rem;
  qr = div(qr.quot, 10);
  str[2] = '0' + qr.rem;
  str[1] = '0' + qr.quot;
  str[5] = '-';
  qr = div(utm.tm_mon+1, 10);
  str[7] = '0' + qr.rem;
  str[6] = '0' + qr.quot;
  str[8] = '-';
  qr = div(utm.tm_mday, 10);
  str[10] = '0' + qr.rem;
  str[9] = '0' + qr.quot;

  if (time) {
    str[11] = '-';
    div_t qr = div(utm.tm_hour, 10);
    str[13] = '0' + qr.rem;
    str[12] = '0' + qr.quot;
    qr = div(utm.tm_min, 10);
    str[15] = '0' + qr.rem;
    str[14] = '0' + qr.quot;
    qr = div(utm.tm_sec, 10);
    str[17] = '0' + qr.rem;
    str[16] = '0' + qr.quot;
    str[18] = '\0';
    return &str[18];
  }
  else {
    str[11] = '\0';
    return &str[11];
  }
}
#endif

#if !defined(BOOT)
static char tmpHelpersString[32];

char * getSwitchWarningString(swsrc_t idx)
{
  return getSwitchWarningString(tmpHelpersString, idx);
}

char * getSourceString(mixsrc_t idx)
{
  return getSourceString(tmpHelpersString, idx);
}

char * getCurveString(int idx)
{
  return getCurveString(tmpHelpersString, idx);
}

char * getTimerString(int32_t tme, uint8_t hours)
{
  return getTimerString(tmpHelpersString, tme, hours);
}

char * getSwitchPositionName(swsrc_t idx)
{
  return getSwitchPositionName(tmpHelpersString, idx);
}

char * getGVarString(int idx)
{
  return getGVarString(tmpHelpersString, idx);
}
#endif
