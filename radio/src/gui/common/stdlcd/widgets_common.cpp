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

choice_t editChoice(coord_t x, coord_t y, const char * label, const char *const *values, choice_t value, choice_t min, choice_t max, LcdFlags attr, event_t event, coord_t lblX, IsValueAvailable isValueAvailable)
{
  if (label) {
    lcdDrawText(lblX, y, label);
  }
  if (values) lcdDrawTextAtIndex(x, y, values, value-min, attr);
  if (attr & (~RIGHT)) value = checkIncDec(event, value, min, max, (isModelMenuDisplayed()) ? EE_MODEL : EE_GENERAL, isValueAvailable);
  return value;
}

choice_t editChoice(coord_t x, coord_t y, const char * label, const char *const *values, choice_t value, choice_t min, choice_t max, LcdFlags attr, event_t event, coord_t lblX)
{
  return editChoice(x, y, label, values, value, min, max, attr, event, lblX, nullptr);
}

choice_t editChoice(coord_t x, coord_t y, const char * label, const char *const *values, choice_t value, choice_t min, choice_t max, LcdFlags attr, event_t event)
{
  return editChoice(x, y, label, values, value, min, max, attr, event, 0, nullptr);
}

uint8_t editCheckBox(uint8_t value, coord_t x, coord_t y, const char *label, LcdFlags attr, event_t event, coord_t lblX)
{
  drawCheckBox(x, y, value, attr);
  return editChoice(x, y, label, nullptr, value, 0, 1, attr, event, lblX);
}

uint8_t editCheckBox(uint8_t value, coord_t x, coord_t y, const char *label, LcdFlags attr, event_t event)
{
  return editCheckBox(value, x, y, label, attr, event, 0);
}

// Source/switch type iteration tables
static const struct { uint8_t type; uint16_t count; } sourceTypes[] = {
  {SOURCE_TYPE_NONE, 1},
  {SOURCE_TYPE_INPUT, MAX_INPUTS},
#if defined(LUA_INPUTS)
  {SOURCE_TYPE_LUA, MAX_SCRIPTS * MAX_SCRIPT_OUTPUTS},
#endif
  {SOURCE_TYPE_STICK, MAX_STICKS},
  {SOURCE_TYPE_POT, MAX_POTS},
#if defined(IMU)
  {SOURCE_TYPE_IMU, 2},
#endif
#if defined(PCBHORUS)
  {SOURCE_TYPE_SPACEMOUSE, 6},
#endif
  {SOURCE_TYPE_MIN, 1},
  {SOURCE_TYPE_MAX, 1},
#if defined(LUMINOSITY_SENSOR)
  {SOURCE_TYPE_LIGHT, 1},
#endif
#if defined(HELI)
  {SOURCE_TYPE_HELI, 3},
#endif
  {SOURCE_TYPE_TRIM, MAX_TRIMS},
  {SOURCE_TYPE_SWITCH, MAX_SWITCHES},
#if defined(FUNCTION_SWITCHES)
  {SOURCE_TYPE_CUSTOM_SWITCH_GROUP, NUM_FUNCTIONS_GROUPS},
#endif
  {SOURCE_TYPE_LOGICAL_SWITCH, MAX_LOGICAL_SWITCHES},
  {SOURCE_TYPE_TRAINER, MAX_TRAINER_CHANNELS},
  {SOURCE_TYPE_CHANNEL, MAX_OUTPUT_CHANNELS},
#if defined(GVARS)
  {SOURCE_TYPE_GVAR, MAX_GVARS},
#endif
  {SOURCE_TYPE_TX_VOLTAGE, 1},
  {SOURCE_TYPE_TX_TIME, 1},
  {SOURCE_TYPE_TX_GPS, 1},
  {SOURCE_TYPE_TIMER, MAX_TIMERS},
  {SOURCE_TYPE_TELEMETRY, 3 * MAX_TELEMETRY_SENSORS},
};

static const struct { uint8_t type; uint16_t count; } switchTypes[] = {
  {SWITCH_TYPE_NONE, 1},
  {SWITCH_TYPE_SWITCH, switchGetMaxSwitches() * 3},
#if defined(MULTIPOS_SWITCH)
  {SWITCH_TYPE_MULTIPOS, XPOTS_MULTIPOS_COUNT * MAX_POTS},
#endif
  {SWITCH_TYPE_TRIM, MAX_TRIMS * 2},
  {SWITCH_TYPE_LOGICAL, MAX_LOGICAL_SWITCHES},
  {SWITCH_TYPE_ON, 1},
  {SWITCH_TYPE_ONE, 1},
  {SWITCH_TYPE_FLIGHT_MODE, MAX_FLIGHT_MODES},
  {SWITCH_TYPE_TELEMETRY, 1},
  {SWITCH_TYPE_SENSOR, MAX_TELEMETRY_SENSORS},
  {SWITCH_TYPE_RADIO_ACTIVITY, 1},
  {SWITCH_TYPE_TRAINER, 1},
};

// Find next valid SourceRef in the enumeration
static SourceRef nextSource(SourceRef cur)
{
  // Find current position in sourceTypes table
  for (unsigned t = 0; t < DIM(sourceTypes); t++) {
    if (sourceTypes[t].type == cur.type) {
      // Try next index in same type
      if (cur.index + 1 < sourceTypes[t].count) {
        SourceRef next = {cur.type, 0, (uint16_t)(cur.index + 1)};
        if (isSourceAvailable(next)) return next;
        // Skip unavailable, try further
        for (uint16_t i = cur.index + 2; i < sourceTypes[t].count; i++) {
          next.index = i;
          if (isSourceAvailable(next)) return next;
        }
      }
      // Move to next type
      for (unsigned nt = t + 1; nt < DIM(sourceTypes); nt++) {
        for (uint16_t i = 0; i < sourceTypes[nt].count; i++) {
          SourceRef next = {sourceTypes[nt].type, 0, i};
          if (isSourceAvailable(next)) return next;
        }
      }
      return cur; // at end, don't wrap
    }
  }
  return cur;
}

// Find previous valid SourceRef in the enumeration
static SourceRef prevSource(SourceRef cur)
{
  for (unsigned t = 0; t < DIM(sourceTypes); t++) {
    if (sourceTypes[t].type == cur.type) {
      // Try previous index in same type
      if (cur.index > 0) {
        for (int16_t i = cur.index - 1; i >= 0; i--) {
          SourceRef prev = {cur.type, 0, (uint16_t)i};
          if (isSourceAvailable(prev)) return prev;
        }
      }
      // Move to previous type
      for (int nt = t - 1; nt >= 0; nt--) {
        for (int16_t i = sourceTypes[nt].count - 1; i >= 0; i--) {
          SourceRef prev = {sourceTypes[nt].type, 0, (uint16_t)i};
          if (isSourceAvailable(prev)) return prev;
        }
      }
      return cur; // at beginning, don't wrap
    }
  }
  return cur;
}

// Find next valid SwitchRef in the enumeration
static SwitchRef nextSwitch(SwitchRef cur)
{
  for (unsigned t = 0; t < DIM(switchTypes); t++) {
    if (switchTypes[t].type == cur.type) {
      if (cur.index + 1 < switchTypes[t].count) {
        return {cur.type, cur.flags, (uint16_t)(cur.index + 1)};
      }
      for (unsigned nt = t + 1; nt < DIM(switchTypes); nt++) {
        if (switchTypes[nt].count > 0)
          return {switchTypes[nt].type, cur.flags, 0};
      }
      return cur;
    }
  }
  return cur;
}

static SwitchRef prevSwitch(SwitchRef cur)
{
  for (unsigned t = 0; t < DIM(switchTypes); t++) {
    if (switchTypes[t].type == cur.type) {
      if (cur.index > 0) {
        return {cur.type, cur.flags, (uint16_t)(cur.index - 1)};
      }
      for (int nt = t - 1; nt >= 0; nt--) {
        if (switchTypes[nt].count > 0)
          return {switchTypes[nt].type, cur.flags, (uint16_t)(switchTypes[nt].count - 1)};
      }
      return cur;
    }
  }
  return cur;
}

SwitchRef checkIncDecSwitch(event_t event, SwitchRef value,
                           SwitchTypeMask allowedTypes, unsigned int flags,
                           std::function<bool(SwitchRef)> available)
{
  if (s_editMode > 0) {
    SwitchRef newValue = value;
    bool forward = (event == EVT_ROTARY_RIGHT || event == EVT_KEY_FIRST(KEY_PLUS) ||
                    event == EVT_KEY_REPT(KEY_PLUS));
    bool backward = (event == EVT_ROTARY_LEFT || event == EVT_KEY_FIRST(KEY_MINUS) ||
                     event == EVT_KEY_REPT(KEY_MINUS));
    if (forward || backward) {
      // Iterate through allowed switch types/indices
      auto step = [&](SwitchRef cur) {
        return forward ? nextSwitch(cur) : prevSwitch(cur);
      };
      SwitchRef candidate = step(value);
      int limit = 500;  // safety bound
      while (candidate != value && --limit > 0) {
        if ((allowedTypes & SW_TYPE_BIT(candidate.type)) &&
            (!available || available(candidate))) {
          newValue = candidate;
          break;
        }
        candidate = step(candidate);
      }
    }
    if (newValue != value) {
      storageDirty(flags & EE_GENERAL ? EE_GENERAL : EE_MODEL);
      AUDIO_KEY_PRESS();
      value = newValue;
    }
  }
  return value;
}

SourceRef checkIncDecSource(event_t event, SourceRef value,
                            SourceTypeMask allowedTypes,
                            std::function<bool(SourceRef)> available)
{
  if (s_editMode > 0) {
    SourceRef newValue = value;
    bool forward = (event == EVT_ROTARY_RIGHT || event == EVT_KEY_FIRST(KEY_PLUS) ||
                    event == EVT_KEY_REPT(KEY_PLUS));
    bool backward = (event == EVT_ROTARY_LEFT || event == EVT_KEY_FIRST(KEY_MINUS) ||
                     event == EVT_KEY_REPT(KEY_MINUS));
    if (forward || backward) {
      auto step = [&](SourceRef cur) {
        return forward ? nextSource(cur) : prevSource(cur);
      };
      SourceRef candidate = step(value);
      int limit = 1000;  // safety bound
      while (candidate != value && --limit > 0) {
        if ((allowedTypes & SRC_TYPE_BIT(candidate.type)) &&
            (!available || available(candidate))) {
          newValue = candidate;
          break;
        }
        candidate = step(candidate);
      }
    }
    if (newValue != value) {
      storageDirty(EE_MODEL);
      AUDIO_KEY_PRESS();
      value = newValue;
    }
  }
  return value;
}

SwitchRef editSwitch(coord_t x, coord_t y, SwitchRef value, LcdFlags attr, event_t event)
{
  lcdDrawTextAlignedLeft(y, STR_SWITCH);
  drawSwitch(x, y, value, attr);
  if (attr & (~RIGHT)) {
    value = checkIncDecSwitch(event, value, SWMASK_ALL, EE_MODEL,
                              isSwitchAvailableInMixes);
  }
  return value;
}

SourceRef editSource(coord_t x, coord_t y, const char* label, SourceRef value,
                     SourceTypeMask allowedTypes, LcdFlags attr, event_t event)
{
  lcdDrawTextAlignedLeft(y, label);
  drawSource(x, y, value, attr);
  if (attr) {
    value = checkIncDecSource(event, value, allowedTypes);
  }
  return value;
}

uint16_t editSrcVarFieldValue(coord_t x, coord_t y, const char* title, uint16_t value,
                              int16_t min, int16_t max, LcdFlags attr, event_t event,
                              IsValueAvailable isValueAvailable, int16_t sourceMin, int16_t sourceMax)
{
  if (title)
    lcdDrawTextAlignedLeft(y, title);
  SourceNumVal v;
  v.rawValue = value;
  if (v.isSource) {
    drawSource(x, y, mixSrcToSourceRef(v.value), attr);
    if (attr & (~RIGHT)) {
      value = checkIncDec(event, value, sourceMin, sourceMax,
                EE_MODEL|INCDEC_SOURCE|INCDEC_SOURCE_VALUE|INCDEC_SOURCE_INVERT|NO_INCDEC_MARKS, isValueAvailable);
    }
  } else {
    lcdDrawNumber(x, y, v.value, attr);
    if (attr & (~RIGHT)) {
      value = checkIncDec(event, value, min, max, sourceMin, sourceMax,
                EE_MODEL|INCDEC_SOURCE_VALUE|NO_INCDEC_MARKS|INCDEC_SKIP_VAL_CHECK_FUNC,
                isValueAvailable);
    }
  }
  return value;
}

int16_t editGVarFieldValue(coord_t x, coord_t y, int16_t value, int16_t min, int16_t max, LcdFlags attr, uint8_t editflags, event_t event)
{
#if defined(GVARS)
  bool invers = (attr & INVERS);

  // TRACE("editGVarFieldValue(val=%d min=%d max=%d)", value, min, max);

  if (modelGVEnabled() && invers && event == EVT_KEY_LONG(KEY_ENTER)) {
    killEvents(event);
    s_editMode = !s_editMode;
    if (attr & PREC1)
      value = (GV_IS_GV_VALUE(value) ? GET_GVAR(value, min, max, mixerCurrentFlightMode)*10 : GV_VALUE_FROM_INDEX(0));
    else
      value = (GV_IS_GV_VALUE(value) ? GET_GVAR(value, min, max, mixerCurrentFlightMode) : GV_VALUE_FROM_INDEX(0));
    storageDirty(EE_MODEL);
  }

  if (GV_IS_GV_VALUE(value)) {
    attr &= ~PREC1;
    int8_t idx = (int16_t)GV_INDEX_FROM_VALUE(value);
    if (invers) {
      CHECK_INCDEC_MODELVAR(event, idx, -MAX_GVARS, MAX_GVARS-1);
    }
    value = (int16_t)GV_VALUE_FROM_INDEX(idx);
    drawGVarName(x, y, idx, attr);
  }
  else {
    lcdDrawNumber(x, y, value, attr);
    if (invers) value = checkIncDec(event, value, min, max, EE_MODEL | editflags);
  }
#else
  lcdDrawNumber(x, y, value, attr);
  if (attr&INVERS) value = checkIncDec(event, value, min, max, EE_MODEL);
#endif
  return value;
}

#if defined(GVARS)
void editGVarValue(coord_t x, coord_t y, event_t event, uint8_t gvar, uint8_t flightMode, LcdFlags flags)
{
  FlightModeData * fm = &g_model.flightModeData[flightMode];
  gvar_t * v = &fm->gvars[gvar];
  int16_t vmin, vmax;
  if (*v > GVAR_MAX) {
    uint8_t fm = *v - GVAR_MAX - 1;
    if (fm >= flightMode) fm++;
    drawFlightMode(x, y, fm + 1, flags&(~LEFT));
    vmin = GVAR_MAX + 1;
    vmax = GVAR_MAX + MAX_FLIGHT_MODES - 1;
  }
  else {
    vmin = GVAR_MIN + g_model.gvars[gvar].min;
    vmax = GVAR_MAX - g_model.gvars[gvar].max;
    if (*v < vmin) {
      *v = vmin;
      storageDirty(EE_MODEL);
    } else if (*v > vmax) {
      *v = vmax;
      storageDirty(EE_MODEL);
    }
    drawGVarValue(x, y, gvar, *v, flags);
  }

  if (flags & INVERS) {
    if (event == EVT_KEY_LONG(KEY_ENTER) && flightMode > 0) {
      killEvents(event);
      *v = (*v > GVAR_MAX ? 0 : GVAR_MAX+1);
      storageDirty(EE_MODEL);
    }
    else if (s_editMode > 0) {
      *v = checkIncDec(event, *v, vmin, vmax, EE_MODEL);
    }
  }
}
#endif

void editSingleName(coord_t x, coord_t y, const char * label, char *name, uint8_t size, event_t event, uint8_t active, uint8_t old_editMode, coord_t lblX)
{
  lcdDrawText(lblX, y, label);
  editName(x, y, name, size, event, active, 0, old_editMode);
}

uint8_t editDelay(coord_t y, event_t event, uint8_t attr, const char * str, uint8_t delay, uint8_t prec)
{
  lcdDrawTextAlignedLeft(y, str);
  lcdDrawNumber(MIXES_2ND_COLUMN, y, delay, attr|prec|LEFT);
  if (attr) CHECK_INCDEC_MODELVAR_ZERO(event, delay, DELAY_MAX);
  return delay;
}

int editNumberField(const char* name, coord_t lx, coord_t vx, coord_t y, int val,
                    int min, int max, LcdFlags attr, event_t event, const char* zeroStr, int ofst)
{
  lcdDrawText(lx, y, name);
  if (zeroStr && val == 0)
    lcdDrawText(vx, y, zeroStr, attr & (~PREC1));
  else
    lcdDrawNumber(vx, y, val + ofst, LEFT|attr);
  if (attr & INVERS) CHECK_INCDEC_MODELVAR(event, val, min, max);
  return val;
}
