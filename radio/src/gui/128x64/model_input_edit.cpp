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

extern mixsrc_t sourceRefToMixSrc(const SourceRef& ref);
extern swsrc_t switchRefToSwSrc(const SwitchRef& ref);
extern gvar_t valueOrSourceToLegacy(const ValueOrSource& vos);
extern int32_t getSourceNumFieldValue(const ValueOrSource& vos, int16_t min, int16_t max);

static SourceRef mixSrcToSourceRef(mixsrc_t src);
static SwitchRef swSrcToSwitchRef(swsrc_t src);
static ValueOrSource legacyToValueOrSource(int32_t rawValue);

#define EXPO_ONE_2ND_COLUMN (6*FW+1)

int expoFn(int x)
{
  ExpoData * ed = expoAddress(s_currIdx);
  int16_t anas[MAX_INPUTS] = {0};
  applyExpos(anas, e_perout_mode_inactive_flight_mode, sourceRefToMixSrc(ed->srcRaw), x);
  return anas[ed->chn];
}

enum ExposFields {
  EXPO_FIELD_INPUT_NAME,
  EXPO_FIELD_LINE_NAME,
  EXPO_FIELD_SOURCE,
  EXPO_FIELD_SCALE,
  EXPO_FIELD_WEIGHT,
  EXPO_FIELD_OFFSET,
  EXPO_FIELD_CURVE_LABEL,
  EXPO_FIELD_CURVE,
  CASE_FLIGHT_MODES(EXPO_FIELD_FLIGHT_MODES_LABEL)
  CASE_FLIGHT_MODES(EXPO_FIELD_FLIGHT_MODES)
  EXPO_FIELD_SWITCH,
  EXPO_FIELD_SIDE,
  EXPO_FIELD_TRIM,
  EXPO_FIELD_MAX
};

uint8_t FM_ROW(uint8_t value)
{
  if (modelFMEnabled())
    return value;
  return HIDDEN_ROW;
}

void menuModelExpoOne(event_t event)
{
  if (EVT_KEY_OPEN_CHAN_VIEW(event)) {
    pushMenu(menuChannelsView);
  }

  ExpoData * ed = expoAddress(s_currIdx);
  drawSource(strlen(STR_MENUINPUTS)*FW+FW, 0, MIXSRC_FIRST_INPUT+ed->chn, 0);

  uint8_t old_editMode = s_editMode;

  SUBMENU(STR_MENUINPUTS, EXPO_FIELD_MAX,
          {0, 0, 0, abs(sourceRefToMixSrc(ed->srcRaw)) >= MIXSRC_FIRST_TELEM ? (uint8_t)0 : (uint8_t)HIDDEN_ROW, 0, 0, LABEL(Curve), 1,
           CASE_FLIGHT_MODES(FM_ROW(LABEL(Flight Mode))) CASE_FLIGHT_MODES(FM_ROW((MAX_FLIGHT_MODES-1) | NAVIGATION_LINE_BY_LINE)) 0 /*, ...*/});

  int8_t sub = menuVerticalPosition;

  coord_t y = MENU_HEADER_HEIGHT + 1;

  for (uint8_t k=0; k<NUM_BODY_LINES; k++) {
    int i = k + menuVerticalOffset;
    for (int j=0; j<=i; ++j) {
      if (j<(int)DIM(mstate_tab) && mstate_tab[j] == HIDDEN_ROW) {
        ++i;
      }
    }
    LcdFlags attr = (sub==i ? (s_editMode>0 ? BLINK|INVERS : INVERS) : 0);

    switch (i) {
      case EXPO_FIELD_INPUT_NAME:
        editSingleName(EXPO_ONE_2ND_COLUMN, y,
                       STR_INPUTNAME, g_model.inputNames[ed->chn],
                       LEN_INPUT_NAME, event, (attr != 0), old_editMode);
        break;

      case EXPO_FIELD_LINE_NAME:
        editSingleName(EXPO_ONE_2ND_COLUMN, y,
                       STR_EXPONAME, ed->name, LEN_EXPOMIX_NAME, event,
                       (attr != 0), old_editMode);
        break;

      case EXPO_FIELD_SOURCE:
        {
          lcdDrawTextAlignedLeft(y, STR_SOURCE);
          mixsrc_t srcRawVal = sourceRefToMixSrc(ed->srcRaw);
          drawSource(EXPO_ONE_2ND_COLUMN, y, srcRawVal, STREXPANDED|attr);
          if (attr) {
            srcRawVal = checkIncDec(event, srcRawVal, INPUTSRC_FIRST, INPUTSRC_LAST, EE_MODEL|INCDEC_SOURCE|INCDEC_SOURCE_INVERT|NO_INCDEC_MARKS, isSourceAvailable);
            ed->srcRaw = mixSrcToSourceRef(srcRawVal);
          }
        }
        break;

      case EXPO_FIELD_SCALE:
        {
          lcdDrawTextAlignedLeft(y, STR_SCALE);
          mixsrc_t absSrc = abs(sourceRefToMixSrc(ed->srcRaw));
          drawSensorCustomValue(EXPO_ONE_2ND_COLUMN, y, (absSrc - MIXSRC_FIRST_TELEM)/3, convertTelemValue(absSrc - MIXSRC_FIRST_TELEM + 1, ed->scale),  attr);
          if (attr)
            ed->scale = checkIncDec(event, ed->scale, 0, maxTelemValue(absSrc - MIXSRC_FIRST_TELEM + 1), EE_MODEL);
        }
        break;

      case EXPO_FIELD_WEIGHT:
        ed->weight = legacyToValueOrSource(editSrcVarFieldValue(EXPO_ONE_2ND_COLUMN, y, STR_WEIGHT, valueOrSourceToLegacy(ed->weight),
                        -100, 100, attr, event, isSourceAvailable, MIXSRC_FIRST, INPUTSRC_LAST));
        break;

      case EXPO_FIELD_OFFSET:
        ed->offset = legacyToValueOrSource(editSrcVarFieldValue(EXPO_ONE_2ND_COLUMN, y, STR_OFFSET, valueOrSourceToLegacy(ed->offset),
                        -100, 100, attr, event, isSourceAvailable, MIXSRC_FIRST, INPUTSRC_LAST));
        break;

      case EXPO_FIELD_CURVE_LABEL:
        lcdDrawTextAlignedLeft(y, STR_CURVE);
        break;

      case EXPO_FIELD_CURVE:
        editCurveRef(FW + 1, y, ed->curve, event, attr, isSourceAvailable, MIXSRC_FIRST, INPUTSRC_LAST);
        break;

#if defined(FLIGHT_MODES)
      case EXPO_FIELD_FLIGHT_MODES_LABEL:
        lcdDrawTextAlignedLeft(y, STR_FLMODE);
        break;

      case EXPO_FIELD_FLIGHT_MODES:
        ed->flightModes = editFlightModes(EXPO_ONE_2ND_COLUMN-5*FW-2, y, event, ed->flightModes, attr);
        break;
#endif

      case EXPO_FIELD_SWITCH:
        ed->swtch = swSrcToSwitchRef(editSwitch(EXPO_ONE_2ND_COLUMN, y, switchRefToSwSrc(ed->swtch), attr, event));
        break;

      case EXPO_FIELD_SIDE:
        ed->mode = 4 - editChoice(EXPO_ONE_2ND_COLUMN, y, STR_SIDE, STR_VCURVEFUNC, 4-ed->mode, 1, 3, attr, event);
        break;

      case EXPO_FIELD_TRIM:
        lcdDrawTextAlignedLeft(y, STR_TRIM);
        {
          mixsrc_t absSrc = abs(sourceRefToMixSrc(ed->srcRaw));
          const char* trim_str = getTrimSourceLabel(absSrc, ed->trimSource);
          LcdFlags flags = (menuHorizontalPosition==0 ? attr : 0);
          lcdDrawText(EXPO_ONE_2ND_COLUMN, y, trim_str, flags);

          if (attr) {
            int8_t min = TRIM_ON;
            if (absSrc >= MIXSRC_FIRST_STICK && absSrc <= MIXSRC_LAST_STICK) {
              min = -TRIM_OFF;
            }
            ed->trimSource = -checkIncDecModel(event, -ed->trimSource, min, keysGetMaxTrims());
          }
        }
        break;
    }
    y += FH;
  }

  drawFunction(expoFn);
  // those parameters are global so that they can be reused in the curve edit screen
  s_currSrcRaw = sourceRefToMixSrc(ed->srcRaw);
  s_currScale = ed->scale;
  drawCursor(expoFn);
}

static SourceRef mixSrcToSourceRef(mixsrc_t src)
{
  SourceRef ref = {};
  if (src == MIXSRC_NONE) return ref;

  bool inverted = (src < 0);
  mixsrc_t absSrc = inverted ? -src : src;

  struct Range { mixsrc_t first; mixsrc_t last; uint8_t type; };
  static const Range ranges[] = {
    {MIXSRC_FIRST_INPUT, MIXSRC_LAST_INPUT, SOURCE_TYPE_INPUT},
    {MIXSRC_FIRST_LUA, MIXSRC_LAST_LUA, SOURCE_TYPE_LUA},
    {MIXSRC_FIRST_STICK, MIXSRC_LAST_STICK, SOURCE_TYPE_STICK},
    {MIXSRC_FIRST_POT, MIXSRC_LAST_POT, SOURCE_TYPE_POT},
    {MIXSRC_FIRST_TRIM, MIXSRC_LAST_TRIM, SOURCE_TYPE_TRIM},
    {MIXSRC_FIRST_SWITCH, MIXSRC_LAST_SWITCH, SOURCE_TYPE_SWITCH},
    {MIXSRC_FIRST_LOGICAL_SWITCH, MIXSRC_LAST_LOGICAL_SWITCH, SOURCE_TYPE_LOGICAL_SWITCH},
    {MIXSRC_FIRST_TRAINER, MIXSRC_LAST_TRAINER, SOURCE_TYPE_TRAINER},
    {MIXSRC_FIRST_CH, MIXSRC_LAST_CH, SOURCE_TYPE_CHANNEL},
    {MIXSRC_FIRST_GVAR, MIXSRC_LAST_GVAR, SOURCE_TYPE_GVAR},
    {MIXSRC_FIRST_TIMER, MIXSRC_LAST_TIMER, SOURCE_TYPE_TIMER},
    {MIXSRC_FIRST_TELEM, MIXSRC_LAST_TELEM, SOURCE_TYPE_TELEMETRY},
    {MIXSRC_FIRST_HELI, MIXSRC_LAST_HELI, SOURCE_TYPE_HELI},
  };

  if (absSrc == MIXSRC_MIN) { ref.type = SOURCE_TYPE_MIN; }
  else if (absSrc == MIXSRC_MAX) { ref.type = SOURCE_TYPE_MAX; }
  else if (absSrc == MIXSRC_TX_VOLTAGE) { ref.type = SOURCE_TYPE_TX_VOLTAGE; }
  else if (absSrc == MIXSRC_TX_TIME) { ref.type = SOURCE_TYPE_TX_TIME; }
  else {
    for (const auto& r : ranges) {
      if (absSrc >= r.first && absSrc <= r.last) {
        ref.type = r.type;
        ref.index = absSrc - r.first;
        break;
      }
    }
  }

  if (inverted) ref.flags = SOURCE_FLAG_INVERTED;
  return ref;
}

static SwitchRef swSrcToSwitchRef(swsrc_t src)
{
  SwitchRef ref = {};
  if (src == SWSRC_NONE) return ref;

  bool inverted = (src < 0);
  swsrc_t absSrc = inverted ? -src : src;

  struct Range { swsrc_t first; swsrc_t last; uint8_t type; };
  static const Range ranges[] = {
    {SWSRC_FIRST_SWITCH, SWSRC_LAST_SWITCH, SWITCH_TYPE_SWITCH},
    {SWSRC_FIRST_MULTIPOS_SWITCH, SWSRC_LAST_MULTIPOS_SWITCH, SWITCH_TYPE_MULTIPOS},
    {SWSRC_FIRST_TRIM, SWSRC_LAST_TRIM, SWITCH_TYPE_TRIM},
    {SWSRC_FIRST_LOGICAL_SWITCH, SWSRC_LAST_LOGICAL_SWITCH, SWITCH_TYPE_LOGICAL},
    {SWSRC_FIRST_FLIGHT_MODE, SWSRC_LAST_FLIGHT_MODE, SWITCH_TYPE_FLIGHT_MODE},
    {SWSRC_FIRST_SENSOR, SWSRC_LAST_SENSOR, SWITCH_TYPE_SENSOR},
  };

  if (absSrc == SWSRC_ON) { ref.type = SWITCH_TYPE_ON; }
  else if (absSrc == SWSRC_ONE) { ref.type = SWITCH_TYPE_ONE; }
  else if (absSrc == SWSRC_TELEMETRY_STREAMING) { ref.type = SWITCH_TYPE_TELEMETRY; }
  else if (absSrc == SWSRC_RADIO_ACTIVITY) { ref.type = SWITCH_TYPE_RADIO_ACTIVITY; }
  else if (absSrc == SWSRC_TRAINER_CONNECTED) { ref.type = SWITCH_TYPE_TRAINER; }
  else {
    for (const auto& r : ranges) {
      if (absSrc >= r.first && absSrc <= r.last) {
        ref.type = r.type;
        ref.index = absSrc - r.first;
        break;
      }
    }
  }

  if (inverted) ref.flags = SWITCH_FLAG_INVERTED;
  return ref;
}

static ValueOrSource legacyToValueOrSource(int32_t rawValue)
{
  ValueOrSource vos = {};
  SourceNumVal v;
  v.rawValue = rawValue;
  if (v.isSource) {
    vos.isSource = 1;
    mixsrc_t src = v.value;
    if (src >= MIXSRC_FIRST_GVAR && src <= MIXSRC_LAST_GVAR) {
      vos.srcType = SOURCE_TYPE_GVAR;
      vos.value = src - MIXSRC_FIRST_GVAR;
    } else if (src >= MIXSRC_FIRST_INPUT && src <= MIXSRC_LAST_INPUT) {
      vos.srcType = SOURCE_TYPE_INPUT;
      vos.value = src - MIXSRC_FIRST_INPUT;
    } else if (src >= MIXSRC_FIRST_STICK && src <= MIXSRC_LAST_STICK) {
      vos.srcType = SOURCE_TYPE_STICK;
      vos.value = src - MIXSRC_FIRST_STICK;
    } else if (src >= MIXSRC_FIRST_CH && src <= MIXSRC_LAST_CH) {
      vos.srcType = SOURCE_TYPE_CHANNEL;
      vos.value = src - MIXSRC_FIRST_CH;
    } else {
      vos.srcType = 0;
      vos.value = src;
    }
  } else {
    vos.setNumeric(v.value);
  }
  return vos;
}
