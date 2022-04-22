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

#include "strhelpers.h"
#include <string.h>

#if !defined(BOOT)
#include "opentx.h"

const char s_charTab[] = "_-.,";

char hex2zchar(uint8_t hex) { return (hex >= 10 ? hex - 9 : 27 + hex); }

char hex2char(uint8_t hex) { return (hex >= 10 ? hex - 9 + 'A' : hex + '0'); }

char zchar2char(int8_t idx)
{
  if (idx == 0) return ' ';
  if (idx < 0) {
    if (idx > -27) return 'a' - idx - 1;
    idx = -idx;
  }
  if (idx < 27) return 'A' + idx - 1;
  if (idx < 37) return '0' + idx - 27;
  if (idx <= 40) return *(s_charTab + idx - 37);
#if LEN_SPECIAL_CHARS > 0
  if (idx <= (LEN_STD_CHARS + LEN_SPECIAL_CHARS)) return 'z' + 5 + idx - 40;
#endif
  return ' ';
}

char char2lower(char c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }

int8_t char2zchar(char c)
{
  if (c == '_') return 37;
#if LEN_SPECIAL_CHARS > 0
  if ((int8_t)c < 0 && c + 128 <= LEN_SPECIAL_CHARS) return 41 + (c + 128);
#endif
  if (c >= 'a') return 'a' - c - 1;
  if (c >= 'A') return c - 'A' + 1;
  if (c >= '0') return c - '0' + 27;
  if (c == '-') return 38;
  if (c == '.') return 39;
  if (c == ',') return 40;
  return 0;
}

void str2zchar(char *dest, const char *src, int size)
{
  memset(dest, 0, size);
  for (int c = 0; c < size && src[c]; c++) {
    dest[c] = char2zchar(src[c]);
  }
}

int zchar2str(char *dest, const char *src, int size)
{
  for (int c = 0; c < size; c++) {
    dest[c] = zchar2char(src[c]);
  }
  do {
    dest[size--] = '\0';
  } while (size >= 0 && dest[size] == ' ');
  return size + 1;
}

int strnlen(const char *src, int max_size)
{
  for (int i = 0; i < max_size; i++) {
    if (src[i] == '\0') return i;
  }

  return max_size;
}

unsigned int effectiveLen(const char *str, unsigned int size)
{
  while (size > 0) {
    if (str[size - 1] != ' ' && str[size - 1] != '\0') return size;
    size--;
  }
  return 0;
}

bool zexist(const char *str, uint8_t size)
{
  for (int i = 0; i < size; i++) {
    if (str[i] != 0) return true;
  }
  return false;
}

uint8_t zlen(const char *str, uint8_t size)
{
  while (size > 0) {
    if (str[size - 1] != 0) return size;
    size--;
  }
  return 0;
}

char *strcat_zchar(char *dest, const char *name, uint8_t size,
                   const char spaceSym, const char *defaultName,
                   uint8_t defaultNameSize, uint8_t defaultIdx)
{
  int8_t len = 0;

  if (name) {
    memcpy(dest, name, size);
    dest[size] = '\0';

    int8_t i = size - 1;

    while (i >= 0) {
      if (!len && dest[i]) len = i + 1;
      if (len) {
        if (dest[i] == spaceSym) dest[i] = '_';
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
char *getStringAtIndex(char *dest, const char **s, int idx)
{
  strcpy(dest, s[idx]);
  return dest;
}

char *strAppendStringWithIndex(char *dest, const char *s, int idx)
{
  return strAppendUnsigned(strAppend(dest, s), abs(idx));
}

#define SECONDSPERMIN 60
#define SECONDSPERHOUR (60 * SECONDSPERMIN)
#define SECONDSPERDAY (24 * SECONDSPERHOUR)
#define SECONDSPERYEAR (365 * SECONDSPERDAY)

char *getFormattedTimerString(char *dest, int32_t tme, TimerOptions timerOptions)
{
  char *s = dest;
  div_t qr;
  int val = abs(tme);
  uint8_t digit_group = 0;
  const bool bLowerCase = !(timerOptions.options & SHOW_TIMER_UPPER_CASE);
  const bool showTime = timerOptions.options & SHOW_TIME;
  uint8_t numDigitGroupRequired = (timerOptions.options >> 2) & 0x7;
  const bool hmFormat = timerOptions.options & SHOW_TIMER_HM_FORMAT;

  if(!numDigitGroupRequired) numDigitGroupRequired = 3;

  if (tme < 0) {
    tme = -tme;
    *s++ = '-';
  }

  // years
  qr = div((int)val, SECONDSPERYEAR);
  if (qr.quot != 0) {
    qr = div((int)val, SECONDSPERYEAR);
    *s++ = '0' + (qr.quot / 10);
    *s++ = '0' + (qr.quot % 10);
    *s++ = bLowerCase ? 'y' : 'Y';
    val = qr.rem;
    digit_group++;
  }
   if (digit_group == numDigitGroupRequired) {
    *s = 0;
    return dest;
  } 
  // days
  qr = div((int)val, SECONDSPERDAY);
  if (qr.quot != 0 || digit_group != 0) {
    qr = div((int)val, SECONDSPERDAY);
    *s++ = '0' + (qr.quot / 10);
    *s++ = '0' + (qr.quot % 10);
    *s++ = bLowerCase ? 'd' : 'D';
    val = qr.rem;
    digit_group++;
  }
  if (digit_group == numDigitGroupRequired) {
    *s = 0;
    return dest;
  }
  // hours
  qr = div((int)val, SECONDSPERHOUR);
  if (qr.quot != 0 || digit_group != 0) {
    qr = div((int)val, SECONDSPERHOUR);
    *s++ = '0' + (qr.quot / 10);
    *s++ = '0' + (qr.quot % 10);
    digit_group++;
    // if format hm is selected h should be always printed
    if (digit_group == numDigitGroupRequired && !hmFormat) {
      *s = 0;
      return dest;
    }
    if(numDigitGroupRequired < 3 || hmFormat)
      *s++ = bLowerCase ? 'h' : 'H';
    else
      *s++ = ':';
    val = qr.rem;
  }
  if (digit_group == numDigitGroupRequired) {
    *s = 0;
    return dest;
  }
  // minutes
  qr = div((int)val, SECONDSPERMIN);
  *s++ = '0' + (qr.quot / 10);
  *s++ = '0' + (qr.quot % 10);
  digit_group++;
  if (digit_group == numDigitGroupRequired) {
    *s = 0;
    return dest;
  }
  if(!showTime && hmFormat)
    *s++ = bLowerCase ? 'm' : 'M';
  else
    *s++ = ':';
  
  // seconds
  *s++ = '0' + (qr.rem / 10);
  *s++ = '0' + (qr.rem % 10);
  *s = 0;

  return dest;
}

void splitTimer(char *sDb0, char *sDb1, char *sUnit0, char *sUnit1, int tme,
                bool bLowerCase)
{
  char *s0 = sDb0;
  char *s1 = sDb1;
  char *s2 = sUnit0;
  char *s3 = sUnit1;
  s0[0] = s1[0] = s0[1] = s1[1] = '0';
  div_t qr;
  int val = tme;
  uint8_t digit_group = 0;

  /*
  if (tme < 0) {
    tme = -tme;
    *s++ = '-';
  }
  */
  // years
  qr = div((int)val, SECONDSPERYEAR);
  if (qr.quot != 0) {
    qr = div((int)val, SECONDSPERYEAR);
    *s0++ += (qr.quot / 10);
    *s0++ += (qr.quot % 10);
    *s0 = 0;
    ;
    *s2++ = bLowerCase ? 'y' : 'Y';
    *s2 = 0;
    digit_group++;
    val = qr.rem;
  }
  // days
  qr = div((int)val, SECONDSPERDAY);
  if (digit_group == 1) {
    *s1++ += (qr.quot / 10);
    *s1++ += (qr.quot % 10);
    *s1 = 0;
    *s3++ = bLowerCase ? 'd' : 'D';
    *s3 = 0;
    return;
  }
  if (qr.quot != 0) {
    *s0++ += (qr.quot / 10);
    *s0++ += (qr.quot % 10);
    *s0 = 0;
    ;
    *s2++ = bLowerCase ? 'd' : 'D';
    *s2 = 0;
    digit_group++;
    val = qr.rem;
  }
  // hours
  qr = div((int)val, SECONDSPERHOUR);
  if (digit_group == 1) {
    *s1++ += (qr.quot / 10);
    *s1++ += (qr.quot % 10);
    *s1 = 0;
    *s3++ = bLowerCase ? 'h' : 'H';
    *s3 = 0;
    return;
  }
  if (qr.quot != 0) {
    *s0++ += (qr.quot / 10);
    *s0++ += (qr.quot % 10);
    *s0 = 0;
    *s2++ = bLowerCase ? 'h' : 'H';
    *s2 = 0;
    digit_group++;
    val = qr.rem;
  }
  // minutes
  qr = div((int)val, SECONDSPERMIN);
  if (digit_group == 1) {
    *s1++ += (qr.quot / 10);
    *s1++ += (qr.quot % 10);
    *s1 = 0;
    *s3++ = bLowerCase ? 'm' : 'M';
    *s3 = 0;
    return;
  }
  *s0++ += (qr.quot / 10);
  *s0++ += (qr.quot % 10);
  *s0 = 0;
  *s2++ = bLowerCase ? 'm' : 'M';
  *s2 = 0;
  // seconds
  *s1++ += (qr.rem / 10);
  *s1++ += (qr.rem % 10);
  *s1 = 0;
  *s3++ = bLowerCase ? 's' : 'S';
  *s3 = 0;
}

char *getCurveString(char *dest, int idx)
{
  if (idx == 0) {
    return getStringAtIndex(dest, STR_MMMINV, 0);
  }

  char *s = dest;
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

char *getGVarString(char *dest, int idx)
{
  char *s = dest;
  if (idx < 0) {
    *s++ = '-';
    idx = -idx - 1;
  }

  if (idx >= MAX_GVARS) {
    s[0] = '\0';
    return s;
  }
  
  if (g_model.gvars[idx].name[0])
    strAppend(s, g_model.gvars[idx].name, LEN_GVAR_NAME);
  else
    strAppendStringWithIndex(s, STR_GV, idx + 1);

  return dest;
}

char *getFlightModeString(char *dest, int8_t idx)
{
  char *s = dest;

  if (idx == 0) {
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

int getRawSwitchIdx(char sw)
{
  if (sw < 'A' || sw > 'Z')
    return -1;

#if defined(PCBX7) && !defined(RADIO_TX12) && !defined(RADIO_ZORRO) && !defined(RADIO_TX12MK2)
  if (sw >= 'H')
    return sw - 'H' + 5;
#if defined(RADIO_T12)
  else if (sw == 'G')
#else
  else if (sw == 'F')
#endif
    return 4;
  else
    return sw - 'A';
#else
  return sw - 'A';
#endif
}

char getRawSwitchFromIdx(int idx)
{
#if defined(PCBX7) && !defined(RADIO_TX12) && !defined(RADIO_TX12MK2) && !defined(RADIO_ZORRO) && !defined(RADIO_TPRO)
    if (idx >= 5)
      return 'H' + idx - 5;
    else if (idx == 4)
  #if defined(RADIO_T12)
      return 'G';
  #else
      return 'F';
  #endif
    else
      return 'A' + idx;
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_T8)
    if (idx < 6)
        return 'A' + idx;
    else
        return 'A' + idx + 2;
#else
    return 'A' + idx;
#endif
}

char *getSwitchName(char *dest, swsrc_t idx)
{
  div_t swinfo = switchInfo(idx);
  if (g_eeGeneral.switchNames[swinfo.quot][0] != '\0') {
    dest =
        strAppend(dest, g_eeGeneral.switchNames[swinfo.quot], LEN_SWITCH_NAME);
  } 
  else {
#if defined(FUNCTION_SWITCHES) 
    if (swinfo.quot >= NUM_REGULAR_SWITCHES)  {
      int fsIdx = swinfo.quot - NUM_REGULAR_SWITCHES;
      if(ZEXIST(g_model.switchNames[fsIdx])){
        dest = strAppend(dest, g_model.switchNames[fsIdx], LEN_SWITCH_NAME);
      }
      else {
        *dest++ = 'S';
        *dest++ = 'W';
        *dest++ = '1' + swinfo.quot - 4;
      }
      return dest;
    }  
#endif
    *dest++ = 'S';
    *dest++ = getRawSwitchFromIdx(swinfo.quot);
  }
  return dest;
}

static const char* _switch_state_str[] {
  " ",
  STR_CHAR_UP,
  "-",
  STR_CHAR_DOWN,
};

const char* getSwitchWarnSymbol(uint8_t pos)
{
  // 0: NONE
  // 1: UP
  // 2: MIDDLE
  // 3: DOWN
  if (pos >= 4) return "";
  return _switch_state_str[pos];
}

const char* getSwitchPositionSymbol(uint8_t pos)
{
  // 0: UP
  // 1: MIDDLE
  // 2: DOWN
  if (pos >= 3) return "";
  return _switch_state_str[pos + 1];
}

char *getSwitchPositionName(char *dest, swsrc_t idx)
{
  if (idx == SWSRC_NONE) {
    return getStringAtIndex(dest, STR_VSWITCHES, 0);
  } else if (idx == SWSRC_OFF) {
    return getStringAtIndex(dest, STR_OFFON, 0);
  }

  char *s = dest;
  if (idx < 0) {
    *s++ = '!';
    idx = -idx;
  }

#define IDX_TRIMS_IN_STR_VSWITCHES (1)
#define IDX_ON_IN_STR_VSWITCHES \
  (IDX_TRIMS_IN_STR_VSWITCHES + SWSRC_LAST_TRIM - SWSRC_FIRST_TRIM + 1)

  if (idx <= SWSRC_LAST_SWITCH) {
    div_t swinfo = switchInfo(idx);
    s = getSwitchName(s, idx);
    s = strAppend(s, getSwitchPositionSymbol(swinfo.rem), 2);
    *s = '\0';
  }

#if NUM_XPOTS > 0
  else if (idx <= SWSRC_LAST_MULTIPOS_SWITCH) {
    div_t swinfo =
        div(int(idx - SWSRC_FIRST_MULTIPOS_SWITCH), XPOTS_MULTIPOS_COUNT);
    char temp[LEN_ANA_NAME + 2];
    getSourceString(temp, MIXSRC_FIRST_POT + swinfo.quot);
    strAppendStringWithIndex(s, temp, swinfo.rem + 1);
  }
#endif

  else if (idx <= SWSRC_LAST_TRIM) {
    getStringAtIndex(s, STR_VSWITCHES,
                     IDX_TRIMS_IN_STR_VSWITCHES + idx - SWSRC_FIRST_TRIM);
  } else if (idx <= SWSRC_LAST_LOGICAL_SWITCH) {
    *s++ = 'L';
    strAppendUnsigned(s, idx - SWSRC_FIRST_LOGICAL_SWITCH + 1, 2);
  } else if (idx <= SWSRC_ONE) {
    getStringAtIndex(s, STR_VSWITCHES,
                     IDX_ON_IN_STR_VSWITCHES + idx - SWSRC_ON);
  } else if (idx <= SWSRC_LAST_FLIGHT_MODE) {
    strAppendStringWithIndex(s, STR_FM, idx - SWSRC_FIRST_FLIGHT_MODE);
  } else if (idx == SWSRC_TELEMETRY_STREAMING) {
    strcpy(s, "Tele");
  } else if (idx == SWSRC_RADIO_ACTIVITY) {
    strcpy(s, "Act");
  }
#if defined(DEBUG_LATENCY)
  else if (idx == SWSRC_LATENCY_TOGGLE) {
    strcpy(s, "Ltc");
  }
#endif
  else {
    strncpy(s, g_model.telemetrySensors[idx - SWSRC_FIRST_SENSOR].label,
            TELEM_LABEL_LEN);
    s[TELEM_LABEL_LEN] = '\0';
  }

  return dest;
}

// this should be declared in header, but it used so much foreign symbols that
// we declare it in cpp-file and pre-instantiate it for the uses
template <size_t L>
char *getSourceString(char (&dest)[L], mixsrc_t idx)
{
  size_t dest_len = L;

  if (idx == MIXSRC_NONE) {
    return getStringAtIndex(dest, STR_VSRCRAW, 0);
  } else if (idx <= MIXSRC_LAST_INPUT) {
    idx -= MIXSRC_FIRST_INPUT;
    static_assert(L > sizeof(STR_CHAR_INPUT) - 1, "dest string too small");
    dest_len -= sizeof(STR_CHAR_INPUT) - 1;
    char* pos = strAppend(dest, STR_CHAR_INPUT, sizeof(STR_CHAR_INPUT) - 1);
    if (g_model.inputNames[idx][0] != '\0' && (dest_len > sizeof(g_model.inputNames[idx]))) {
      memset(pos, 0, sizeof(g_model.inputNames[idx]) + 1);
      size_t input_len = std::min(dest_len - 1, sizeof(g_model.inputNames[idx]));
      strncpy(pos, g_model.inputNames[idx], input_len);
      pos[input_len] = '\0';
    } else {
      strAppendUnsigned(pos, idx + 1, 2);
    }
  }
  
#if defined(LUA_INPUTS)
  else if (idx <= MIXSRC_LAST_LUA) {
#if defined(LUA_MODEL_SCRIPTS)
    div_t qr = div(idx - MIXSRC_FIRST_LUA, MAX_SCRIPT_OUTPUTS);
    if (qr.quot < MAX_SCRIPTS &&
        qr.rem < scriptInputsOutputs[qr.quot].outputsCount) {

      static_assert(L > sizeof(STR_CHAR_LUA) - 1, "dest string too small");
      dest_len -= sizeof(STR_CHAR_LUA) - 1;
      char* pos = strAppend(dest, STR_CHAR_LUA, sizeof(STR_CHAR_LUA) - 1);

      if (g_model.scriptsData[qr.quot].name[0] != '\0') {
        // instance Name is not empty : dest = InstanceName/OutputName
        snprintf(pos, dest_len, "%.*s/%.*s", sizeof(g_model.scriptsData[qr.quot].name), g_model.scriptsData[qr.quot].name,
                 (int)sizeof(scriptInputsOutputs[qr.quot].outputs[qr.rem].name), scriptInputsOutputs[qr.quot].outputs[qr.rem].name);
      } else {
        // instance Name is empty : dest = n-ScriptFileName/OutputName
        snprintf(pos, dest_len, "%d-%.*s/%.*s", qr.quot + 1,
                 (int)sizeof(g_model.scriptsData[qr.quot].file), g_model.scriptsData[qr.quot].file,
                 (int)sizeof(scriptInputsOutputs[qr.quot].outputs[qr.rem].name), scriptInputsOutputs[qr.quot].outputs[qr.rem].name);
      }
    }
#else
    strncpy(dest, "N/A", L-1);
#endif
  }
#endif
  else if (idx <= MIXSRC_LAST_POT) {
    if (g_eeGeneral.anaNames[idx - MIXSRC_Rud][0]) {
      char* pos = dest;
      if (idx <= MIXSRC_LAST_STICK) {
        pos = strAppend(pos, STR_CHAR_STICK, sizeof(STR_CHAR_STICK) - 1);
        dest_len -= sizeof(STR_CHAR_STICK) - 1;
#if NUM_SLIDERS > 0
      } else if (idx < MIXSRC_FIRST_SLIDER) {
        pos = strAppend(pos, STR_CHAR_POT, sizeof(STR_CHAR_POT) - 1);
        dest_len -= sizeof(STR_CHAR_POT) - 1;
      } else {
        pos = strAppend(pos, STR_CHAR_SLIDER, sizeof(STR_CHAR_SLIDER) - 1);
        dest_len -= sizeof(STR_CHAR_SLIDER) - 1;
#else
      } else {
        pos = strAppend(pos, STR_CHAR_POT, sizeof(STR_CHAR_POT) - 1);
        dest_len -= sizeof(STR_CHAR_POT) - 1;
#endif
      }
      idx -= MIXSRC_Rud;
      size_t ana_len = std::min(sizeof(g_eeGeneral.anaNames[idx]), dest_len - 1);
      strncpy(pos, g_eeGeneral.anaNames[idx], ana_len);
      pos[ana_len] = '\0';
    } else {
      idx -= MIXSRC_Rud;
      getStringAtIndex(dest, STR_VSRCRAW, idx + 1);
    }
  } else if (idx <= MIXSRC_LAST_TRIM) {
    idx -= MIXSRC_Rud;
    getStringAtIndex(dest, STR_VSRCRAW, idx + 1);
  } else if (idx <= MIXSRC_LAST_SWITCH) {
    idx -= MIXSRC_FIRST_SWITCH;
    if (g_eeGeneral.switchNames[idx][0] != '\0') {
      copyToTerminated(dest, g_eeGeneral.switchNames[idx]);
    } else {
      getStringAtIndex(dest, STR_VSRCRAW,
                       idx + MIXSRC_FIRST_SWITCH - MIXSRC_Rud + 1);
    }
  } else if (idx <= MIXSRC_LAST_LOGICAL_SWITCH) {
    getSwitchPositionName(dest, SWSRC_SW1 + idx - MIXSRC_SW1);
  } else if (idx <= MIXSRC_LAST_TRAINER) {
    strAppendStringWithIndex(dest, STR_PPM_TRAINER,
                             idx - MIXSRC_FIRST_TRAINER + 1);
  } else if (idx <= MIXSRC_LAST_CH) {
    auto ch = idx - MIXSRC_CH1;
    if (g_model.limitData[ch].name[0] != '\0') {
      copyToTerminated(dest, g_model.limitData[ch].name);
    } else {
      strAppendStringWithIndex(dest, STR_CH, ch + 1);
    }
  } else if (idx <= MIXSRC_LAST_GVAR) {
    strAppendStringWithIndex(dest, STR_GV, idx - MIXSRC_GVAR1 + 1);
  } else if (idx < MIXSRC_FIRST_TIMER) {
    getStringAtIndex(dest, STR_VSRCRAW,
                     idx - MIXSRC_Rud + 1 - MAX_LOGICAL_SWITCHES -
                         MAX_TRAINER_CHANNELS - MAX_OUTPUT_CHANNELS -
                         MAX_GVARS);
  } else if (idx <= MIXSRC_LAST_TIMER) {
    if (g_model.timers[idx - MIXSRC_FIRST_TIMER].name[0] != '\0') {
      copyToTerminated(dest, g_model.timers[idx - MIXSRC_FIRST_TIMER].name);
    } else {
      getStringAtIndex(dest, STR_VSRCRAW,
                       idx - MIXSRC_Rud + 1 - MAX_LOGICAL_SWITCHES -
                           MAX_TRAINER_CHANNELS - MAX_OUTPUT_CHANNELS -
                           MAX_GVARS);
    }
  } else {
    idx -= MIXSRC_FIRST_TELEM;
    div_t qr = div(idx, 3);
    char* pos = strAppend(dest, STR_CHAR_TELEMETRY, 2);
    pos = strAppend(pos, g_model.telemetrySensors[qr.quot].label,
                    sizeof(g_model.telemetrySensors[qr.quot].label));
    if (qr.rem) *pos = (qr.rem == 2 ? '+' : '-');
    *++pos = '\0';
  }
  dest[L - 1] = '\0'; // assert the termination
  return dest; 
}

// pre-instantiate for use from external
// all other instantiations are done from this file
template char *getSourceString<16>(char (&dest)[16], mixsrc_t idx);

static char tmpHelpersString[32];

char *getSourceString(mixsrc_t idx)
{
  return getSourceString(tmpHelpersString, idx);
}

char *getCurveString(int idx) { return getCurveString(tmpHelpersString, idx); }

char *getTimerString(int32_t tme, TimerOptions timerOptions)
{
  return getFormattedTimerString(tmpHelpersString, tme, timerOptions);
}

char *getTimerString(char *dest, int32_t tme, TimerOptions timerOptions)
{
  return getFormattedTimerString(dest, tme, timerOptions);
}

char *getSwitchPositionName(swsrc_t idx)
{
  return getSwitchPositionName(tmpHelpersString, idx);
}

char *getGVarString(int idx) { return getGVarString(tmpHelpersString, idx); }

#if defined(LIBOPENUI)
char *getValueWithUnit(char *dest, size_t len, int32_t val, uint8_t unit,
                       LcdFlags flags)
{
  if (unit == UNIT_CELLS) unit = UNIT_VOLTS;
  if ((flags & NO_UNIT) || (unit == UNIT_RAW)) {
    flags = flags & (~NO_UNIT);
    BitmapBuffer::formatNumberAsString(dest, len, val, flags);
  } else {
    BitmapBuffer::formatNumberAsString(dest, len, val, flags, 0, nullptr,
                                       STR_VTELEMUNIT[unit]);
  }

  return dest;
}

template <size_t L>
char *getSensorCustomValueString(char (&dest)[L], uint8_t sensor, int32_t val,
                                 LcdFlags flags)
{
  if (sensor >= MAX_TELEMETRY_SENSORS) { return dest; }

  // TelemetryItem & telemetryItem = telemetryItems[sensor];
  TelemetrySensor & telemetrySensor = g_model.telemetrySensors[sensor];

  size_t len = L - 1;
  if (telemetrySensor.unit == UNIT_DATETIME ||
      telemetrySensor.unit == UNIT_GPS || telemetrySensor.unit == UNIT_TEXT) {
    strAppend(dest, "N/A", len);
    return dest;
  }

  if (telemetrySensor.unit == UNIT_BITFIELD) {
    if (IS_FRSKY_SPORT_PROTOCOL()) {
      if (telemetrySensor.id >= RBOX_STATE_FIRST_ID &&
          telemetrySensor.id <= RBOX_STATE_LAST_ID) {
        if (telemetrySensor.subId == 0) {
          if (val == 0) {
            strAppend(dest, "OK", len);
            return dest;
          }
          for (uint8_t i = 0; i < 16; i++) {
            if (val & (1 << i)) {
              if (len < 8) return dest;
              auto pos = strAppend(dest, "CH", len);
              len -= 2;
              pos = strAppendUnsigned(pos, i + 1, 2);
              len -= 2;
              strAppend(pos, " KO", len);
              return dest;
            }
          }
        } else {
          if (val == 0) {
            strAppend(dest, "Rx OK", len);
            return dest;
          }
          
          static const char *const RXS_STATUS[] = {
            "Rx1 Ovl", "Rx2 Ovl",  "SBUS Ovl", "Rx1 FS", "Rx1 LF", "Rx2 FS",
            "Rx2 LF",  "Rx1 Lost", "Rx2 Lost", "Rx1 NS", "Rx2 NS",
          };
          for (uint8_t i = 0; i < DIM(RXS_STATUS); i++) {
            if (val & (1 << i)) {
              strAppend(dest, RXS_STATUS[i], len);
              return dest;
            }
          }
        }
      }
    }
  } else {
    if (telemetrySensor.prec > 0) {
      flags |= (telemetrySensor.prec == 1 ? PREC1 : PREC2);
    }
    getValueWithUnit(dest, len, val, telemetrySensor.unit, flags);
  }

  return dest;
}

template <size_t L>
char *getSourceCustomValueString(char (&dest)[L], source_t source, int32_t val,
                                 LcdFlags flags)
{
  size_t len = L - 1;
  if (source >= MIXSRC_FIRST_TELEM) {
    source = (source - MIXSRC_FIRST_TELEM) / 3;
    return getSensorCustomValueString(dest, source, val, flags);
  }
  else if (source >= MIXSRC_FIRST_TIMER || source == MIXSRC_TX_TIME) {
    if (L < LEN_TIMER_STRING) return dest;
    if (source == MIXSRC_TX_TIME) flags |= TIMEHOUR;

    TimerOptions timerOptions;
    timerOptions.options = SHOW_TIMER;
    if ((flags & TIMEHOUR) != 0) timerOptions.options = SHOW_TIME;

    return getTimerString(dest, val, timerOptions);
  }
  else if (source == MIXSRC_TX_VOLTAGE) {
    BitmapBuffer::formatNumberAsString(dest, len, val, flags | PREC1);
    return dest;
  }
#if defined(INTERNAL_GPS)
  else if (source == MIXSRC_TX_GPS) {
    strAppend(dest, "N/A", len);
    return dest;
  }
#endif
#if defined(GVARS)
  else if (source >= MIXSRC_FIRST_GVAR && source <= MIXSRC_LAST_GVAR) {
    uint8_t gvar_idx = source - MIXSRC_FIRST_GVAR;
    auto gvar = &g_model.gvars[gvar_idx];
    uint8_t prec = gvar->prec;
    if (prec > 0) {
      flags |= (prec == 1 ? PREC1 : PREC2);
    }
    uint8_t unit = gvar->unit ? UNIT_PERCENT : UNIT_RAW;
    getValueWithUnit(dest, len, val, unit, flags);
  }
#endif
#if defined(LUA_INPUTS)
  else if (source >= MIXSRC_FIRST_LUA && source <= MIXSRC_LAST_LUA) {
    BitmapBuffer::formatNumberAsString(dest, len, val, flags);
  }
#endif
  else if (source < MIXSRC_FIRST_CH) {
    val = calcRESXto100(val);
    BitmapBuffer::formatNumberAsString(dest, len, val, flags);
  }
  else if (source <= MIXSRC_LAST_CH) {
    val = calcRESXto100(val);
#if defined(PPM_UNIT_PERCENT_PREC1)
    BitmapBuffer::formatNumberAsString(dest, len, val, flags | PREC1);
#else
    BitmapBuffer::formatNumberAsString(dest, len, val, flags);
#endif
  }
  else {
    BitmapBuffer::formatNumberAsString(dest, len, val, flags);
  }

  return dest;
}

std::string formatNumberAsString(int32_t val, LcdFlags flags, uint8_t len,
                                 const char *prefix, const char *suffix)
{
  char s[49];
  BitmapBuffer::formatNumberAsString(s, 49, val, flags, len, prefix, suffix);
  return std::string(s);
}

char *getSourceCustomValueString(source_t source, int32_t val, LcdFlags flags)
{
  return getSourceCustomValueString(tmpHelpersString, source, val, flags);
}

#endif // defined(LIBOPENUI)
#endif // !defined(BOOT)

char *strAppendUnsigned(char *dest, uint32_t value, uint8_t digits,
                        uint8_t radix)
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

char *strAppendSigned(char *dest, int32_t value, uint8_t digits, uint8_t radix)
{
  if (value < 0) {
    *dest++ = '-';
    value = -value;
  }
  return strAppendUnsigned(dest, (uint32_t)value, digits, radix);
}

char *strAppend(char *dest, const char *source, int len)
{
  while ((*dest++ = *source++)) {
    if (--len == 0) {
      *dest = '\0';
      return dest;
    }
  }
  return dest - 1;
}

char *strSetCursor(char *dest, int position)
{
  *dest++ = 0x1F;
  *dest++ = position;
  *dest = '\0';
  return dest;
}

char *strAppendFilename(char *dest, const char *filename, const int size)
{
  memset(dest, 0, size);
  for (int i = 0; i < size; i++) {
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

char *strAppendDate(char *str, bool time)
{
  str[0] = '-';
  struct gtm utm;
  gettime(&utm);
  div_t qr = div(utm.tm_year + TM_YEAR_BASE, 10);
  str[4] = '0' + qr.rem;
  qr = div(qr.quot, 10);
  str[3] = '0' + qr.rem;
  qr = div(qr.quot, 10);
  str[2] = '0' + qr.rem;
  str[1] = '0' + qr.quot;
  str[5] = '-';
  qr = div(utm.tm_mon + 1, 10);
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
  } else {
    str[11] = '\0';
    return &str[11];
  }
}
#endif

#if !defined(BOOT)
#endif
