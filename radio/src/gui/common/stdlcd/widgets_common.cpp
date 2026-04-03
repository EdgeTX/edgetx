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

swsrc_t editSwitch(coord_t x, coord_t y, swsrc_t value, LcdFlags attr, event_t event)
{
  lcdDrawTextAlignedLeft(y, STR_SWITCH);
  drawSwitch(x,  y, value, attr);
  if (attr & (~RIGHT)) CHECK_INCDEC_MODELSWITCH(event, value, SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES, isSwitchAvailableInMixes);
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
    drawSource(x, y, v.value, attr);
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
