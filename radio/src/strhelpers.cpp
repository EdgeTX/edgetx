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
#include "analogs.h"
#include "hal/adc_driver.h"
#include "hal/switch_driver.h"
#include "edgetx.h"
#include "switches.h"

static char _static_str_buffer[32];
static const char s_charTab[] = "_-.,";

const char* sanitizeForFilename(const char* name, int len)
{
  strAppend(_static_str_buffer, name, len);

  char *s = _static_str_buffer;
  // Remove invalid characters in filename
  for (int i = 0; s[i]; i += 1)
    if (s[i] == '"' || s[i] == ':' || s[i] == '\\' ||
        s[i] == '/' || s[i] == '<' || s[i] == '>' ||
        s[i] == '?' || s[i] == '*')
      s[i] = '_';
  
  return _static_str_buffer;
}

char hex2char(uint8_t hex) { return (hex >= 10 ? hex - 10 + 'A' : hex + '0'); }

char char2lower(char c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }

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
std::string getStringAtIndex(const char *const *s, int idx)
{
  return std::string(s[idx]);
}

char *getStringAtIndex(char *dest, const char *const *s, int idx)
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

char *getFormattedTimerString(char *dest, int32_t tme,
                              TimerOptions timerOptions)
{
  char *s = dest;
  div_t qr;
  int val = abs(tme);
  uint8_t digit_group = 0;
  const bool bLowerCase = !(timerOptions.options & SHOW_TIMER_UPPER_CASE);
  const bool showTime = timerOptions.options & SHOW_TIME;
  uint8_t numDigitGroupRequired = (timerOptions.options >> 2) & 0x7;
  const bool hmFormat = timerOptions.options & SHOW_TIMER_HM_FORMAT;

  if (!numDigitGroupRequired) numDigitGroupRequired = 3;

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
    if (numDigitGroupRequired < 3 || hmFormat)
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
  if (!showTime && hmFormat)
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
    *s++ = '-';
    idx = -idx;
  }

  if (curveHeaderAddress(idx - 1)->name[0])
    strAppend(s, curveHeaderAddress(idx - 1)->name, LEN_CURVE_NAME);
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

  if (idx >= getGVarCount()) {
    s[0] = '\0';
    return s;
  }

  if (gvarDataAddress(idx)->name[0])
    strAppend(s, gvarDataAddress(idx)->name, LEN_GVAR_NAME);
  else
    strAppendStringWithIndex(s, STR_GV, idx + 1);

  return dest;
}

#if defined(COLORLCD)
char *getValueOrGVarString(char *dest, size_t len, gvar_t value,
                           LcdFlags flags, const char *suffix,
                           gvar_t offset, bool usePPMUnit)
{
  if (GV_IS_GV_VALUE(value)) {
    int index = GV_INDEX_FROM_VALUE(value);
    return getGVarString(dest, index);
  }

  value = GV_DECODE(value) + offset;
  if (usePPMUnit && g_eeGeneral.ppmunit == PPM_US)
    value = value * 128 / 25;
  formatNumberAsString(dest, len, value, flags, 0, nullptr, suffix);
  return dest;
}

char *getValueOrSrcVarString(char *dest, size_t len, const ValueOrSource& vos,
                           LcdFlags flags, const char *suffix,
                           int16_t offset, bool usePPMUnit)
{
  if (vos.isSource) {
    SourceRef ref = vos.toSourceRef();
    if (ref.type == SOURCE_TYPE_GVAR) {
      getGVarString(dest, ref.isInverted() ? -(int)ref.index - 1 : ref.index);
    } else {
      const char* s = getSourceString(ref);
      strncpy(dest, s, len);
    }
  } else {
    int16_t val = vos.numericValue() + offset;
    if (usePPMUnit && g_eeGeneral.ppmunit == PPM_US)
      val = val * 128 / 25;
    formatNumberAsString(dest, len, val, flags, 0, nullptr, suffix);
  }
  return dest;
}
#endif

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

char* getCustomSwitchesGroupName(char *dest, uint8_t idx)
{
  dest = strAppendStringWithIndex(dest, "GR", idx + 1);

  return dest;
}

char* getSwitchName(char *dest, uint8_t idx, bool defaultOnly)
{
  if (!defaultOnly && g_model.switchHasCustomName(idx)) {
    dest = strAppend(dest, g_model.getSwitchCustomName(idx), LEN_SWITCH_NAME);
  } else {
    dest = strAppend(dest, switchGetDefaultName(idx), LEN_SWITCH_NAME);
  }

  return dest;
}

static const char *_switch_state_str[]{
    " ",
    CHAR_UP,
    "-",
    CHAR_DOWN,
};

const char *getSwitchWarnSymbol(uint8_t pos)
{
  // 0: NONE
  // 1: UP
  // 2: MIDDLE
  // 3: DOWN
  if (pos >= 4) return "";
  return _switch_state_str[pos];
}

const char *getSwitchPositionSymbol(uint8_t pos)
{
  // 0: UP
  // 1: MIDDLE
  // 2: DOWN
  if (pos >= 3) return "";
  return _switch_state_str[pos + 1];
}

char *getSwitchPositionName(char *dest, const SwitchRef& ref)
{
  if (ref.isNone()) {
    return strcpy(dest, STR_EMPTY);
  }

  char *s = dest;
  if (ref.isInverted()) {
    *s++ = '!';
  }

  switch (ref.type) {
    case SWITCH_TYPE_SWITCH: {
      uint16_t idx = ref.index;
      div_t swinfo = div(idx, 3);
      s = getSwitchName(s, swinfo.quot);
      s = strAppend(s, getSwitchPositionSymbol(swinfo.rem), 2);
      *s = '\0';
      break;
    }
    case SWITCH_TYPE_MULTIPOS: {
      div_t swinfo = div(int(ref.index), XPOTS_MULTIPOS_COUNT);
      s = strAppendStringWithIndex(s, getPotLabel(swinfo.quot), swinfo.rem + 1);
      break;
    }
    case SWITCH_TYPE_TRIM: {
      uint16_t idx = ref.index;
      s = strAppend(s, getTrimLabel(idx / 2));
      *s++ = idx & 1 ? '+' : '-';
      *s = '\0';
      break;
    }
    case SWITCH_TYPE_LOGICAL:
      *s++ = 'L';
      strAppendUnsigned(s, ref.index + 1, 2);
      break;
    case SWITCH_TYPE_ON:
      getStringAtIndex(s, STR_ON_ONE_SWITCHES, 0);
      break;
    case SWITCH_TYPE_ONE:
      getStringAtIndex(s, STR_ON_ONE_SWITCHES, 1);
      break;
    case SWITCH_TYPE_FLIGHT_MODE:
      strAppendStringWithIndex(s, STR_FM, ref.index);
      break;
    case SWITCH_TYPE_TELEMETRY:
      strcpy(s, "Tele");
      break;
    case SWITCH_TYPE_RADIO_ACTIVITY:
      strcpy(s, "Act");
      break;
    case SWITCH_TYPE_TRAINER:
      strcpy(s, "Trn");
      break;
    case SWITCH_TYPE_SENSOR:
      strncpy(s, g_model.telemetrySensors[ref.index].label, TELEM_LABEL_LEN);
      s[TELEM_LABEL_LEN] = '\0';
      break;
    default:
      *s = '\0';
      break;
  }

  return dest;
}

char *getSwitchPositionName(const SwitchRef& ref)
{
  return getSwitchPositionName(_static_str_buffer, ref);
}


bool switchCanHaveCustomName(swsrc_t idx)
{
  return (idx >= SWSRC_FIRST_SWITCH && idx <= SWSRC_LAST_SWITCH);
}

uint32_t getSwitchIndex(const char* name, bool all)
{
  bool negate = false;

  if (name[0] == '!') {
    name++;
    negate = true;
  }

  // Iterate by SwitchRefType + index, constructing SwitchRef directly
  struct TypeRange { uint8_t type; uint16_t count; };
  const TypeRange types[] = {
    { SWITCH_TYPE_SWITCH,         (uint16_t)(switchGetMaxAllSwitches() * 3) },
    { SWITCH_TYPE_MULTIPOS,       MAX_XPOTS_POSITIONS },
    { SWITCH_TYPE_TRIM,           (uint16_t)(2 * MAX_TRIMS) },
    { SWITCH_TYPE_LOGICAL,        MAX_LOGICAL_SWITCHES },
    { SWITCH_TYPE_ON,             1 },
    { SWITCH_TYPE_ONE,            1 },
    { SWITCH_TYPE_FLIGHT_MODE,    getFlightModeCount() },
    { SWITCH_TYPE_TELEMETRY,      1 },
    { SWITCH_TYPE_SENSOR,         MAX_TELEMETRY_SENSORS },
    { SWITCH_TYPE_RADIO_ACTIVITY, 1 },
    { SWITCH_TYPE_TRAINER,        1 },
  };

  for (const auto& tr : types) {
    for (uint16_t i = 0; i < tr.count; i++) {
      SwitchRef ref = SwitchRef_(tr.type, i);
      if (all || isSwitchAvailable(ref, ModelCustomFunctionsContext)) {
        bool hasCustom = (tr.type == SWITCH_TYPE_SWITCH);
        char* s;
        if (hasCustom) {
          s = getSwitchPositionName(_static_str_buffer, ref);
          if (!strcasecmp(s, name)) {
            uint8_t flags = negate ? SWITCH_FLAG_INVERTED : 0;
            return SwitchRef_(tr.type, i, flags).toUint32();
          }
        }
        s = getSwitchPositionName(ref);
        if (!strcasecmp(s, name)) {
          uint8_t flags = negate ? SWITCH_FLAG_INVERTED : 0;
          return SwitchRef_(tr.type, i, flags).toUint32();
        }
      }
    }
  }

  return 0;  // Not found (SWITCH_TYPE_NONE packed)
}

const char *getAnalogLabel(uint8_t type, uint8_t idx, bool defaultOnly)
{
  if (!defaultOnly && analogHasCustomLabel(type, idx))
    return analogGetCustomLabel(type, idx);

  if (type == ADC_INPUT_MAIN) {
    // main controls: translated label is stored in "short label"
    return adcGetInputShortLabel(type, idx);
  }

  if (type == ADC_INPUT_FLEX) {
    return adcGetInputLabel(type, idx);
  }

  return analogGetCanonicalName(type, idx);
}

const char *getAnalogShortLabel(uint8_t idx)
{
  auto max = adcGetMaxInputs(ADC_INPUT_MAIN);
  if (idx < max) {
    // main controls: translated label is stored in "short label"
    auto label = adcGetInputShortLabel(ADC_INPUT_MAIN, idx);
    if (label) {
      static char _str_buffer[2];
      _str_buffer[0] = label[0];
      _str_buffer[1] = '\0';
      return _str_buffer;
    } else {
      return "";
    }
  }

  idx -= max;
  max = adcGetMaxInputs(ADC_INPUT_FLEX);

  if (idx < max) {
    return adcGetInputShortLabel(ADC_INPUT_FLEX, idx);
  }

  // we only support short labels
  // on main controls and pots
  return "";
}

const char *getMainControlLabel(uint8_t idx, bool defaultOnly)
{
  return getAnalogLabel(ADC_INPUT_MAIN, idx, defaultOnly);
}

const char *getTrimLabel(uint8_t idx, bool defaultOnly)
{
  if (idx < adcGetMaxInputs(ADC_INPUT_MAIN)) {
    return getMainControlLabel(idx, defaultOnly);
  }

  // TODO: replace with string from HW def
  static char _trim_buffer[4];
  strAppendStringWithIndex(_trim_buffer, "T", idx + 1);
  return _trim_buffer;
}

const char *getTrimSourceLabel(uint16_t src_raw, int8_t trim_src)
{
  if (trim_src < TRIM_ON) {
    return getTrimLabel(-trim_src - 1);
  } else if (trim_src == TRIM_ON && src_raw >= MIXSRC_FIRST_STICK &&
             src_raw <= MIXSRC_LAST_STICK) {
    return STR_OFFON[1];
  } else {
    return STR_OFFON[0];
  }
}

const char *getTrimSourceLabel(const SourceRef& ref, int8_t trim_src)
{
  if (trim_src < TRIM_ON) {
    return getTrimLabel(-trim_src - 1);
  } else if (trim_src == TRIM_ON && ref.type == SOURCE_TYPE_STICK) {
    return STR_OFFON[1];
  } else {
    return STR_OFFON[0];
  }
}

const char *getPotLabel(uint8_t idx, bool defaultOnly)
{
  return getAnalogLabel(ADC_INPUT_FLEX, idx, defaultOnly);
}

// Primary SourceRef-based implementation
template <size_t L>
char *getSourceString(char (&destRef)[L], const SourceRef& ref, bool defaultOnly)
{
  size_t dest_len = L;
  char* dest = destRef;

  if (ref.isInverted()) {
    dest[0] = '!';
    dest += 1;
    dest_len -= 1;
  }

  uint16_t idx = ref.index;

  switch (ref.type) {
    case SOURCE_TYPE_NONE:
      strncpy(dest, STR_EMPTY, dest_len - 1);
      break;

    case SOURCE_TYPE_INPUT:
      static_assert(L > sizeof(CHAR_INPUT) - 1, "dest string too small");
      dest_len -= sizeof(CHAR_INPUT) - 1;
      {
        char *pos = strAppend(dest, CHAR_INPUT, sizeof(CHAR_INPUT) - 1);
        const char* iName = inputName(idx);
        if (!defaultOnly && iName && iName[0] != '\0' &&
            (dest_len > LEN_INPUT_NAME)) {
          memset(pos, 0, LEN_INPUT_NAME + 1);
          size_t input_len =
              std::min(dest_len - 1, (size_t)LEN_INPUT_NAME);
          strncpy(pos, iName, input_len);
          pos[input_len] = '\0';
        } else {
          strAppendUnsigned(pos, idx + 1, 2);
        }
      }
      break;

#if defined(LUA_INPUTS)
    case SOURCE_TYPE_LUA:
    {
#if defined(LUA_MODEL_SCRIPTS)
      div_t qr = div((uint16_t)idx, MAX_SCRIPT_OUTPUTS);
      if (qr.quot < MAX_SCRIPTS &&
          qr.rem < scriptInputsOutputs[qr.quot].outputsCount) {
        static_assert(L > sizeof(CHAR_LUA) - 1, "dest string too small");
        dest_len -= sizeof(CHAR_LUA) - 1;
        char *pos = strAppend(dest, CHAR_LUA, sizeof(CHAR_LUA) - 1);

        if (g_model.scriptsData[qr.quot].name[0] != '\0') {
          pos = strAppend(pos, g_model.scriptsData[qr.quot].name, LEN_SCRIPT_NAME);
        } else {
          pos = strAppendUnsigned(pos, qr.quot + 1);
          pos = strAppend(pos, "-");
          pos = strAppend(pos, g_model.scriptsData[qr.quot].file, LEN_SCRIPT_FILENAME);
        }
        pos = strAppend(pos, "/");
        dest_len = L - (pos - dest);
        strAppend(pos, scriptInputsOutputs[qr.quot].outputs[qr.rem].name, dest_len);
      }
#else
      strncpy(dest, "N/A", dest_len-1);
#endif
      break;
    }
#endif

    case SOURCE_TYPE_STICK:
    {
      char *pos = strAppend(dest, CHAR_STICK, sizeof(CHAR_STICK) - 1);
      dest_len -= sizeof(CHAR_STICK) - 1;
      const char *name = getMainControlLabel(idx, defaultOnly);
      strncpy(pos, name, dest_len - 1);
      pos[dest_len - 1] = '\0';
      break;
    }

    case SOURCE_TYPE_POT:
    {
      char *pos = dest;
      if (IS_SLIDER(idx)) {
        pos = strAppend(pos, CHAR_SLIDER, sizeof(CHAR_SLIDER) - 1);
        dest_len -= sizeof(CHAR_SLIDER) - 1;
      } else {
        pos = strAppend(pos, CHAR_POT, sizeof(CHAR_POT) - 1);
        dest_len -= sizeof(CHAR_POT) - 1;
      }
      const char *name = getPotLabel(idx, defaultOnly);
      strncpy(pos, name, dest_len - 1);
      pos[dest_len - 1] = '\0';
      break;
    }

#if defined(IMU)
    case SOURCE_TYPE_IMU:
      getStringAtIndex(dest, STR_IMU_VSRCRAW, idx);
      break;
#endif

#if defined(PCBHORUS)
    case SOURCE_TYPE_SPACEMOUSE:
      getStringAtIndex(dest, STR_SM_VSRCRAW, idx);
      break;
#endif

    case SOURCE_TYPE_MIN:
      strncpy(dest, STR_MENU_MIN, dest_len - 1);
      break;

    case SOURCE_TYPE_MAX:
      strncpy(dest, STR_MENU_MAX, dest_len - 1);
      break;

#if defined(LUMINOSITY_SENSOR)
    case SOURCE_TYPE_LIGHT:
      strncpy(dest, STR_SRC_LIGHT, dest_len - 1);
      break;
#endif

    case SOURCE_TYPE_HELI:
      getStringAtIndex(dest, STR_CYC_VSRCRAW, idx);
      break;

    case SOURCE_TYPE_TRIM:
    {
      char *pos = strAppend(dest, CHAR_TRIM, sizeof(CHAR_TRIM) - 1);
      strAppend(pos, getTrimLabel(idx, defaultOnly));
      break;
    }

    case SOURCE_TYPE_SWITCH:
    {
      char *pos = strAppend(dest, CHAR_SWITCH, sizeof(CHAR_SWITCH) - 1);
      getSwitchName(pos, idx, defaultOnly);
      break;
    }

#if defined(FUNCTION_SWITCHES)
    case SOURCE_TYPE_CUSTOM_SWITCH_GROUP:
    {
      char *pos = strAppend(dest, CHAR_SWITCH, sizeof(CHAR_SWITCH) - 1);
      getCustomSwitchesGroupName(pos, idx);
      break;
    }
#endif

    case SOURCE_TYPE_LOGICAL_SWITCH:
      getSwitchPositionName(dest, SwitchRef_(SWITCH_TYPE_LOGICAL, (uint16_t)idx));
      break;

    case SOURCE_TYPE_TRAINER:
      strAppendStringWithIndex(dest, STR_PPM_TRAINER, idx + 1);
      break;

    case SOURCE_TYPE_CHANNEL:
      if (!defaultOnly && g_model.limitData[idx].name[0] != '\0') {
        strAppend(dest, g_model.limitData[idx].name, LEN_CHANNEL_NAME);
      } else {
        strAppendStringWithIndex(dest, STR_CH, idx + 1);
      }
      break;

    case SOURCE_TYPE_GVAR:
#if defined(COLORLCD)
    {
      char *s = strAppendStringWithIndex(dest, STR_GV, idx + 1);
      if (!defaultOnly && gvarDataAddress(idx)->name[0]) {
        s = strAppend(s, ":");
        getGVarString(s, idx);
      }
    }
#else
      strAppendStringWithIndex(dest, STR_GV, idx + 1);
#endif
      break;

    case SOURCE_TYPE_TX_VOLTAGE:
      strncpy(dest, STR_SRC_BATT, dest_len - 1);
      break;

    case SOURCE_TYPE_TX_TIME:
      strncpy(dest, STR_SRC_TIME, dest_len - 1);
      break;

    case SOURCE_TYPE_TX_GPS:
      strncpy(dest, STR_SRC_GPS, dest_len - 1);
      break;

    case SOURCE_TYPE_TIMER:
      if (!defaultOnly && g_model.timers[idx].name[0] != '\0') {
        strAppend(dest, g_model.timers[idx].name, LEN_TIMER_NAME);
      } else {
        strAppendStringWithIndex(dest, STR_SRC_TIMER, idx + 1);
      }
      break;

    case SOURCE_TYPE_TELEMETRY:
    {
      div_t qr = div((uint16_t)idx, 3);
      char* pos = strAppend(dest, CHAR_TELEMETRY, 2);
      pos = strAppend(pos, g_model.telemetrySensors[qr.quot].label,
                      sizeof(g_model.telemetrySensors[qr.quot].label));
      if (qr.rem) *pos = (qr.rem == 2 ? '+' : '-');
      *++pos = '\0';
      break;
    }

    default:
      *dest = '\0';
      break;
  }

  destRef[L - 1] = '\0'; // assert the termination
  return destRef;
}

bool sourceCanHaveCustomName(mixsrc_t idx)
{
  return (idx >= MIXSRC_FIRST_INPUT && idx <= MIXSRC_LAST_INPUT) ||
         (idx >= MIXSRC_FIRST_STICK && idx <= MIXSRC_LAST_STICK) ||
         (idx >= MIXSRC_FIRST_POT && idx <= MIXSRC_LAST_POT) ||
         (idx >= MIXSRC_FIRST_TRIM && idx <= MIXSRC_LAST_TRIM) ||
         (idx >= MIXSRC_FIRST_SWITCH && idx <= MIXSRC_LAST_SWITCH) ||
         (idx >= MIXSRC_FIRST_CH && idx <= MIXSRC_LAST_CH) ||
         (idx >= MIXSRC_FIRST_GVAR && idx <= MIXSRC_LAST_GVAR) ||
         (idx >= MIXSRC_FIRST_TIMER && idx <= MIXSRC_LAST_TIMER);
}

static bool matchSource(const char* name, const SourceRef& ref, bool defaultOnly)
{
  char *s = getSourceString(ref, defaultOnly);
  if (strcasecmp(s, name) == 0)
    return true;
  // Check for names starting with CHAR_xxx symbol strings
  if (s[0] == '\302' && (strcasecmp(s + 2, name) == 0))
    return true;
  return false;
}

uint32_t getSourceIndex(const char* name, bool all)
{
  // Iterate by SourceType + index, constructing SourceRef directly
  struct TypeRange { uint8_t type; uint16_t count; bool hasCustomName; };
  const TypeRange types[] = {
    { SOURCE_TYPE_INPUT,          MAX_INPUTS,             true },
#if defined(LUA_INPUTS)
    { SOURCE_TYPE_LUA,            (uint16_t)(MAX_SCRIPTS * MAX_SCRIPT_OUTPUTS), false },
#endif
    { SOURCE_TYPE_STICK,          MAX_STICKS,             true },
    { SOURCE_TYPE_POT,            MAX_POTS,               true },
#if defined(IMU)
    { SOURCE_TYPE_IMU,            2,                      false },
#endif
#if defined(PCBHORUS)
    { SOURCE_TYPE_SPACEMOUSE,     6,                      false },
#endif
    { SOURCE_TYPE_MIN,            1,                      false },
    { SOURCE_TYPE_MAX,            1,                      false },
#if defined(LUMINOSITY_SENSOR)
    { SOURCE_TYPE_LIGHT,          1,                      false },
#endif
    { SOURCE_TYPE_HELI,           3,                      false },
    { SOURCE_TYPE_TRIM,           MAX_TRIMS,              true },
    { SOURCE_TYPE_SWITCH,         MAX_SWITCHES,           true },
#if defined(FUNCTION_SWITCHES)
    { SOURCE_TYPE_CUSTOM_SWITCH_GROUP, NUM_FUNCTIONS_GROUPS, false },
#endif
    { SOURCE_TYPE_LOGICAL_SWITCH, MAX_LOGICAL_SWITCHES,   false },
    { SOURCE_TYPE_TRAINER,        MAX_TRAINER_CHANNELS,   false },
    { SOURCE_TYPE_CHANNEL,        MAX_OUTPUT_CHANNELS,    true },
    { SOURCE_TYPE_GVAR,           getGVarCount(),          true },
    { SOURCE_TYPE_TX_VOLTAGE,     1,                      false },
    { SOURCE_TYPE_TX_TIME,        1,                      false },
    { SOURCE_TYPE_TX_GPS,         1,                      false },
    { SOURCE_TYPE_TIMER,          MAX_TIMERS,             true },
    { SOURCE_TYPE_TELEMETRY,      (uint16_t)(3 * MAX_TELEMETRY_SENSORS), false },
  };

  for (const auto& tr : types) {
    for (uint16_t i = 0; i < tr.count; i++) {
      SourceRef ref = SourceRef_(tr.type, i);
      if (all || isSourceAvailable(ref)) {
        if (tr.hasCustomName) {
          if (matchSource(name, ref, true))
            return ref.toUint32();
        }
        if (matchSource(name, ref, false))
          return ref.toUint32();
      }
    }
  }

  return 0;  // Not found (SOURCE_TYPE_NONE packed)
}

// pre-instantiate for use from external
// all other instantiations are done from this file
template char *getSourceString<16>(char (&dest)[16], const SourceRef& ref, bool defaultOnly);

char *getSourceString(const SourceRef& ref, bool defaultOnly)
{
  return getSourceString(_static_str_buffer, ref, defaultOnly);
}


char *getCurveString(int idx)
{
  return getCurveString(_static_str_buffer, idx);
}

char *getTimerString(int32_t tme, TimerOptions timerOptions)
{
  return getFormattedTimerString(_static_str_buffer, tme, timerOptions);
}

char *getTimerString(char *dest, int32_t tme, TimerOptions timerOptions)
{
  return getFormattedTimerString(dest, tme, timerOptions);
}

char *getGVarString(int idx) { return getGVarString(_static_str_buffer, idx); }

#if defined(COLORLCD)
char *getValueWithUnit(char *dest, size_t len, int32_t val, uint8_t unit,
                       LcdFlags flags)
{
  if (unit == UNIT_CELLS) unit = UNIT_VOLTS;
  if ((flags & NO_UNIT) || (unit == UNIT_RAW)) {
    flags = flags & (~NO_UNIT);
    formatNumberAsString(dest, len, val, flags);
  } else {
    formatNumberAsString(dest, len, val, flags, 0, nullptr,
                         STR_VTELEMUNIT[unit]);
  }

  return dest;
}

template <size_t L>
char *getSensorCustomValueString(char (&dest)[L], uint8_t sensor, int32_t val,
                                 LcdFlags flags)
{
  if (sensor >= MAX_TELEMETRY_SENSORS) {
    return dest;
  }

  TelemetrySensor &telemetrySensor = g_model.telemetrySensors[sensor];

  size_t len = L - 1;
  // TODO: display TEXT sensors?
  if (telemetrySensor.unit == UNIT_DATETIME ||
      telemetrySensor.unit == UNIT_GPS || telemetrySensor.unit == UNIT_TEXT) {
    strAppend(dest, "N/A", len);
    return dest;
  }

  if (telemetrySensor.prec > 0) {
    flags |= (telemetrySensor.prec == 1 ? PREC1 : PREC2);
  }
  getValueWithUnit(dest, len, val, telemetrySensor.unit, flags);

  return dest;
}

template <size_t L>
char *getSourceCustomValueString(char (&dest)[L], const SourceRef& source, int32_t val,
                                 LcdFlags flags)
{
  size_t len = L - 1;
  switch (source.type) {
    case SOURCE_TYPE_TELEMETRY:
      return getSensorCustomValueString(dest, source.index / 3, val, flags);
    case SOURCE_TYPE_TIMER:
    case SOURCE_TYPE_TX_TIME:
    {
      if (L < LEN_TIMER_STRING) return dest;
      if (source.type == SOURCE_TYPE_TX_TIME) flags |= TIMEHOUR;
      TimerOptions timerOptions;
      timerOptions.options = SHOW_TIMER;
      if ((flags & TIMEHOUR) != 0) timerOptions.options = SHOW_TIME;
      return getTimerString(dest, val, timerOptions);
    }
    case SOURCE_TYPE_TX_VOLTAGE:
      formatNumberAsString(dest, len, val, flags | PREC1);
      return dest;
#if defined(INTERNAL_GPS)
    case SOURCE_TYPE_TX_GPS:
      if (gpsData.fix) {
        std::string s = getGPSSensorValue(gpsData.longitude, gpsData.latitude, flags);
        strAppend(dest, s.c_str(), L);
      } else {
        formatNumberAsString(dest, L, gpsData.numSat, flags, len, "sats: ");
      }
      return dest;
#endif
#if defined(GVARS)
    case SOURCE_TYPE_GVAR:
    {
      auto gvar = gvarDataAddress(source.index);
      uint8_t prec = gvar->prec;
      if (prec > 0) {
        flags |= (prec == 1 ? PREC1 : PREC2);
      }
      uint8_t unit = gvar->unit ? UNIT_PERCENT : UNIT_RAW;
      getValueWithUnit(dest, len, val, unit, flags);
      break;
    }
#endif
    case SOURCE_TYPE_CHANNEL:
      if (g_eeGeneral.ppmunit == PPM_PERCENT_PREC1) {
        val = calcRESXto1000(val);
        formatNumberAsString(dest, len, val, flags | PREC1);
      } else {
        val = calcRESXto100(val);
        formatNumberAsString(dest, len, val, flags);
      }
      break;
    case SOURCE_TYPE_INPUT:
    case SOURCE_TYPE_STICK:
    case SOURCE_TYPE_POT:
    case SOURCE_TYPE_MIN:
    case SOURCE_TYPE_MAX:
    case SOURCE_TYPE_HELI:
    case SOURCE_TYPE_TRIM:
    case SOURCE_TYPE_SWITCH:
    case SOURCE_TYPE_TRAINER:
#if defined(LUA_INPUTS)
    case SOURCE_TYPE_LUA:
#endif
      val = calcRESXto100(val);
      formatNumberAsString(dest, len, val, flags);
      break;
    default:
      formatNumberAsString(dest, len, val, flags);
      break;
  }
  return dest;
}

void formatNumberAsString(char *buffer, uint8_t buffer_size, int32_t val,
                          LcdFlags flags, uint8_t len, const char *prefix,
                          const char *suffix)
{
  if (buffer) {
    char str[48 + 1];  // max=16 for the prefix, 16 chars for the number, 16
                       // chars for the suffix
    char *s = str + 32;
    *s = '\0';
    int idx = 0;
    int mode = MODE(flags);
    bool neg = false;
    if (val < 0) {
      val = -val;
      neg = true;
    }
    do {
      *--s = '0' + (val % 10);
      ++idx;
      val /= 10;
      if (mode != 0 && idx == mode) {
        mode = 0;
        *--s = '.';
        if (val == 0) *--s = '0';
      }
    } while (val != 0 || mode > 0 || (mode == MODE(LEADING0) && idx < len));
    if (neg) *--s = '-';

    // TODO needs check on all string lengths ...
    if (prefix) {
      int len = strlen(prefix);
      if (len <= 16) {
        s -= len;
        strncpy(s, prefix, len);
      }
    }
    if (suffix) {
      strncpy(&str[32], suffix, 16);
    }
    strncpy(buffer, s, buffer_size);
  }
}

std::string formatNumberAsString(int32_t val, LcdFlags flags, uint8_t len,
                                 const char *prefix, const char *suffix)
{
  char s[49];
  formatNumberAsString(s, 49, val, flags, len, prefix, suffix);
  return std::string(s);
}

char *getSourceCustomValueString(const SourceRef& source, int32_t val, LcdFlags flags)
{
  return getSourceCustomValueString(_static_str_buffer, source, val, flags);
}

std::string getValueWithUnit(int val, uint8_t unit, LcdFlags flags)
{
  if ((flags & NO_UNIT) || unit == UNIT_RAW)
    return formatNumberAsString(val, flags & (~NO_UNIT));

  return formatNumberAsString(val, flags & (~NO_UNIT), 0, nullptr,
                              STR_VTELEMUNIT[unit]);
}

std::string getGVarValue(uint8_t gvar, gvar_t value, LcdFlags flags)
{
  uint8_t prec = gvarDataAddress(gvar)->prec;
  if (prec > 0) {
    flags |= (prec == 1 ? PREC1 : PREC2);
  }
  return getValueWithUnit(
      value, gvarDataAddress(gvar)->unit ? UNIT_PERCENT : UNIT_RAW, flags);
}

std::string getGPSCoord(int32_t value, const char *direction, bool seconds)
{
  char s[32] = {};
  uint32_t absvalue = abs(value);
  char *tmp = strAppendUnsigned(s, absvalue / 1000000);
  tmp = strAppend(tmp, "°");
  absvalue = absvalue % 1000000;
  absvalue *= 60;
  if (g_eeGeneral.gpsFormat == 0 || !seconds) {
    tmp = strAppendUnsigned(tmp, absvalue / 1000000, 2);
    *tmp++ = '\'';
    if (seconds) {
      absvalue %= 1000000;
      absvalue *= 60;
      absvalue /= 100000;
      tmp = strAppendUnsigned(tmp, absvalue / 10);
      *tmp++ = '.';
      tmp = strAppendUnsigned(tmp, absvalue % 10);
      *tmp++ = '"';
    }
  } else {
    tmp = strAppendUnsigned(tmp, absvalue / 1000000, 2);
    *tmp++ = '.';
    absvalue /= 1000;
    tmp = strAppendUnsigned(tmp, absvalue, 3);
  }
  *tmp++ = direction[value >= 0 ? 0 : 1];
  *tmp = '\0';
  return std::string(s);
}

std::string getGPSSensorValue(int32_t longitude, int32_t latitude, LcdFlags flags)
{
  if (flags & PREC1) {
    return getGPSCoord(latitude, "NS", true) + " " +
           getGPSCoord(longitude, "EW", true);
  } else {
    if (flags & RIGHT)
      return getGPSCoord(longitude, "EW", true) + " " +
             getGPSCoord(latitude, "NS", true);

    return getGPSCoord(latitude, "NS", true) + " " +
           getGPSCoord(longitude, "EW", true);
  }
}

std::string getGPSSensorValue(TelemetryItem &telemetryItem, LcdFlags flags)
{
  return getGPSSensorValue(telemetryItem.gps.longitude, telemetryItem.gps.latitude, flags);
}

std::string getTelemDate(TelemetryItem &telemetryItem)
{
  return formatNumberAsString(telemetryItem.datetime.year, LEADING0 | LEFT, 4) +
         "-" +
         formatNumberAsString(telemetryItem.datetime.month, LEADING0 | LEFT,
                              2) +
         "-" +
         formatNumberAsString(telemetryItem.datetime.day, LEADING0 | LEFT, 2);
}

std::string getTelemTime(TelemetryItem &telemetryItem)
{
  return formatNumberAsString(telemetryItem.datetime.hour, LEADING0 | LEFT, 2) +
         ":" +
         formatNumberAsString(telemetryItem.datetime.min, LEADING0 | LEFT, 2) +
         ":" +
         formatNumberAsString(telemetryItem.datetime.sec, LEADING0 | LEFT, 2);
}

#endif  // defined(COLORLCD)
#endif  // !defined(BOOT)

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
  if (source == nullptr) { *dest = '\0'; return dest; }

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

/** 
 * @brief Count the number of digits in a string.
 * Works with negative numbers, and zero is considered to have 1 digit.
 * @param number Integer whose digits are to be counted.
 * @return The number of digits in the integer.
 */
int countDigits(int number)
{
  number = std::abs(number);  // Handle negative numbers if any
  if (number == 0) return 1;  // Special case for 0
  int count = 0;
  while (number > 0) {
    number /= 10;
    count++;
  }
  return count;
}

// Manage timezones
// For backward compatibility timezone is stored as two separate values:
//   timezone = hour value
//   timezoneMinutes - minute value / 15

int8_t minTimezone() { return -12 * 4; }

int8_t maxTimezone() { return 14 * 4; }

std::string timezoneDisplay(int tz)
{
  char s[16];
  int h = abs(tz / 4);
  int m = abs(tz % 4) * 15;
  sprintf(s, "%s%d:%02d", (tz < 0) ? "-" : "", h, m);
  return std::string(s);
}

int timezoneIndex(int8_t tzHour, int8_t tzMinute)
{
  return (tzHour * 4) + tzMinute;
}

int8_t timezoneHour(int tz) { return tz / 4; }

int8_t timezoneMinute(int tz) { return tz % 4; }

int timezoneOffsetSeconds(int8_t tzHour, int8_t tzMinute)
{
  return (tzHour * 3600) + (tzMinute * 15 * 60);
}
