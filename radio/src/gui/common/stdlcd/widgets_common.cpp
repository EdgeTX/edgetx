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

static constexpr uint8_t switchTypeOrder[] = {
  SWITCH_TYPE_NONE,
  SWITCH_TYPE_SWITCH,
#if defined(MULTIPOS_SWITCH)
  SWITCH_TYPE_MULTIPOS,
#endif
  SWITCH_TYPE_TRIM,
  SWITCH_TYPE_LOGICAL,
  SWITCH_TYPE_ON,
  SWITCH_TYPE_ONE,
  SWITCH_TYPE_FLIGHT_MODE,
  SWITCH_TYPE_TELEMETRY,
  SWITCH_TYPE_SENSOR,
  SWITCH_TYPE_RADIO_ACTIVITY,
  SWITCH_TYPE_TRAINER,
};

static uint16_t switchTypeCount(uint8_t type)
{
  switch (type) {
    case SWITCH_TYPE_NONE:           return 1;
    case SWITCH_TYPE_SWITCH:         return switchGetMaxSwitches() * 3;
    case SWITCH_TYPE_MULTIPOS:       return XPOTS_MULTIPOS_COUNT * MAX_POTS;
    case SWITCH_TYPE_TRIM:           return MAX_TRIMS * 2;
    case SWITCH_TYPE_LOGICAL:        return MAX_LOGICAL_SWITCHES;
    case SWITCH_TYPE_ON:             return 1;
    case SWITCH_TYPE_ONE:            return 1;
    case SWITCH_TYPE_FLIGHT_MODE:    return MAX_FLIGHT_MODES;
    case SWITCH_TYPE_TELEMETRY:      return 1;
    case SWITCH_TYPE_SENSOR:         return MAX_TELEMETRY_SENSORS;
    case SWITCH_TYPE_RADIO_ACTIVITY: return 1;
    case SWITCH_TYPE_TRAINER:        return 1;
    default:                         return 0;
  }
}

// Find next valid SourceRef in the enumeration
static SourceRef nextSource(SourceRef cur)
{
  for (unsigned t = 0; t < sourceTypeOrderCount; t++) {
    if (sourceTypeOrder[t] == cur.type) {
      // Try next index in same type
      uint16_t count = sourceTypeCount(cur.type);
      for (uint16_t i = cur.index + 1; i < count; i++) {
        SourceRef next = SourceRef_(cur.type, i);
        if (isSourceAvailable(next)) return next;
      }
      // Move to next type
      for (unsigned nt = t + 1; nt < sourceTypeOrderCount; nt++) {
        uint16_t ntCount = sourceTypeCount(sourceTypeOrder[nt]);
        for (uint16_t i = 0; i < ntCount; i++) {
          SourceRef next = SourceRef_(sourceTypeOrder[nt], i);
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
  for (unsigned t = 0; t < sourceTypeOrderCount; t++) {
    if (sourceTypeOrder[t] == cur.type) {
      // Try previous index in same type
      for (int16_t i = cur.index - 1; i >= 0; i--) {
        SourceRef prev = SourceRef_(cur.type, (uint16_t)i);
        if (isSourceAvailable(prev)) return prev;
      }
      // Move to previous type
      for (int nt = t - 1; nt >= 0; nt--) {
        int16_t ntCount = (int16_t)sourceTypeCount(sourceTypeOrder[nt]);
        for (int16_t i = ntCount - 1; i >= 0; i--) {
          SourceRef prev = SourceRef_(sourceTypeOrder[nt], (uint16_t)i);
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
  for (unsigned t = 0; t < DIM(switchTypeOrder); t++) {
    if (switchTypeOrder[t] == cur.type) {
      uint16_t count = switchTypeCount(cur.type);
      if (cur.index + 1 < count) {
        return SwitchRef_(cur.type, (uint16_t)(cur.index + 1), cur.flags);
      }
      for (unsigned nt = t + 1; nt < DIM(switchTypeOrder); nt++) {
        if (switchTypeCount(switchTypeOrder[nt]) > 0)
          return SwitchRef_(switchTypeOrder[nt], 0, cur.flags);
      }
      return cur;
    }
  }
  return cur;
}

static SwitchRef prevSwitch(SwitchRef cur)
{
  for (unsigned t = 0; t < DIM(switchTypeOrder); t++) {
    if (switchTypeOrder[t] == cur.type) {
      if (cur.index > 0) {
        return SwitchRef_(cur.type, (uint16_t)(cur.index - 1), cur.flags);
      }
      for (int nt = t - 1; nt >= 0; nt--) {
        uint16_t count = switchTypeCount(switchTypeOrder[nt]);
        if (count > 0)
          return SwitchRef_(switchTypeOrder[nt], (uint16_t)(count - 1), cur.flags);
      }
      return cur;
    }
  }
  return cur;
}

// Source popup selection state
static SourceRef s_sourcePopupSelection = {};
static bool s_sourcePopupPending = false;

// Switch popup selection state
static SwitchRef s_switchPopupSelection = {};
static bool s_switchPopupPending = false;

static SourceRef firstAvailableSourceOfType(uint8_t type,
    std::function<bool(SourceRef)> available)
{
  uint16_t count = sourceTypeCount(type);
  for (uint16_t i = 0; i < count; i++) {
    SourceRef ref = SourceRef_(type, i);
    if (!available || available(ref))
      return ref;
  }
  return {};
}

static SwitchRef firstAvailableSwitchOfType(uint8_t type,
    std::function<bool(SwitchRef)> available)
{
  uint16_t count = switchTypeCount(type);
  for (uint16_t i = 0; i < count; i++) {
    SwitchRef ref = SwitchRef_(type, i);
    if (!available || available(ref))
      return ref;
  }
  return {};
}

static std::function<bool(SourceRef)> s_sourceAvailableForPopup;

static void onSourcePopupSelect(const char* result)
{
  auto avail = s_sourceAvailableForPopup;
  if (result == STR_MENU_INPUTS) {
    s_sourcePopupSelection = firstAvailableSourceOfType(SOURCE_TYPE_INPUT, avail);
  }
#if defined(LUA_INPUTS)
  else if (result == STR_MENU_LUA) {
    s_sourcePopupSelection = firstAvailableSourceOfType(SOURCE_TYPE_LUA, avail);
  }
#endif
  else if (result == STR_MENU_STICKS) {
    s_sourcePopupSelection = SourceRef_(SOURCE_TYPE_STICK, 0);
  }
  else if (result == STR_MENU_POTS) {
    s_sourcePopupSelection = SourceRef_(SOURCE_TYPE_POT, 0);
  }
  else if (result == STR_MENU_MIN) {
    s_sourcePopupSelection = SourceRef_(SOURCE_TYPE_MIN, 0);
  }
  else if (result == STR_MENU_MAX) {
    s_sourcePopupSelection = SourceRef_(SOURCE_TYPE_MAX, 0);
  }
#if defined(HELI)
  else if (result == STR_MENU_HELI) {
    s_sourcePopupSelection = SourceRef_(SOURCE_TYPE_HELI, 0);
  }
#endif
  else if (result == STR_MENU_TRIMS) {
    s_sourcePopupSelection = SourceRef_(SOURCE_TYPE_TRIM, 0);
  }
  else if (result == STR_MENU_SWITCHES) {
    s_sourcePopupSelection = SourceRef_(SOURCE_TYPE_SWITCH, 0);
  }
  else if (result == STR_MENU_TRAINER) {
    s_sourcePopupSelection = SourceRef_(SOURCE_TYPE_TRAINER, 0);
  }
  else if (result == STR_MENU_CHANNELS) {
    s_sourcePopupSelection = firstAvailableSourceOfType(SOURCE_TYPE_CHANNEL, avail);
  }
#if defined(GVARS)
  else if (result == STR_MENU_GVARS) {
    s_sourcePopupSelection = SourceRef_(SOURCE_TYPE_GVAR, 0);
  }
#endif
  else if (result == STR_MENU_TELEMETRY) {
    for (int i = 0; i < (int)getSensorCount(); i++) {
      if (sensorAddress(i)->isAvailable()) {
        s_sourcePopupSelection = SourceRef_(SOURCE_TYPE_TELEMETRY, (uint16_t)(3 * i));
        break;
      }
    }
  }
  if (!s_sourcePopupSelection.isNone()) {
    s_sourcePopupPending = true;
  }
}

static void onSwitchPopupSelect(const char* result)
{
  if (result == STR_MENU_SWITCHES) {
    s_switchPopupSelection = SwitchRef_(SWITCH_TYPE_SWITCH, 0);
  }
  else if (result == STR_MENU_TRIMS) {
    s_switchPopupSelection = SwitchRef_(SWITCH_TYPE_TRIM, 0);
  }
  else if (result == STR_MENU_LOGICAL_SWITCHES) {
    s_switchPopupSelection = firstAvailableSwitchOfType(SWITCH_TYPE_LOGICAL, nullptr);
  }
  else if (result == STR_MENU_OTHER) {
    s_switchPopupSelection = SwitchRef_(SWITCH_TYPE_ON, 0);
  }
  else if (result == STR_MENU_INVERT) {
    // Special handling: invert current value
    s_switchPopupSelection = SwitchRef_(SWITCH_TYPE_NONE, 0, SWITCH_FLAG_INVERTED);
  }
  if (s_switchPopupSelection.type != SWITCH_TYPE_NONE ||
      s_switchPopupSelection.flags != 0) {
    s_switchPopupPending = true;
  }
}

// Source type to popup menu item mapping
struct SourcePopupEntry {
  uint8_t type;
  const char* menuItem;
};

static const SourcePopupEntry sourcePopupEntries[] = {
  {SOURCE_TYPE_INPUT, STR_MENU_INPUTS},
#if defined(LUA_INPUTS)
  {SOURCE_TYPE_LUA, STR_MENU_LUA},
#endif
  {SOURCE_TYPE_STICK, STR_MENU_STICKS},
  {SOURCE_TYPE_POT, STR_MENU_POTS},
  {SOURCE_TYPE_MIN, STR_MENU_MIN},
  {SOURCE_TYPE_MAX, STR_MENU_MAX},
#if defined(HELI)
  {SOURCE_TYPE_HELI, STR_MENU_HELI},
#endif
  {SOURCE_TYPE_TRIM, STR_MENU_TRIMS},
  {SOURCE_TYPE_SWITCH, STR_MENU_SWITCHES},
  {SOURCE_TYPE_TRAINER, STR_MENU_TRAINER},
  {SOURCE_TYPE_CHANNEL, STR_MENU_CHANNELS},
#if defined(GVARS)
  {SOURCE_TYPE_GVAR, STR_MENU_GVARS},
#endif
};

SwitchRef checkIncDecSwitch(event_t event, SwitchRef value,
                           SwitchTypeMask allowedTypes, unsigned int flags,
                           std::function<bool(SwitchRef)> available)
{
  if (s_editMode > 0) {
    SwitchRef newValue = value;

    // Long ENTER: show popup menu
    if (event == EVT_KEY_LONG(KEY_ENTER)) {
      killEvents(event);
      s_switchPopupSelection = {};
      s_switchPopupPending = false;
      if (allowedTypes & SW_TYPE_BIT(SWITCH_TYPE_SWITCH))
        POPUP_MENU_ADD_ITEM(STR_MENU_SWITCHES);
      if (allowedTypes & SW_TYPE_BIT(SWITCH_TYPE_TRIM))
        POPUP_MENU_ADD_ITEM(STR_MENU_TRIMS);
      if (allowedTypes & SW_TYPE_BIT(SWITCH_TYPE_LOGICAL)) {
        SwitchRef first = firstAvailableSwitchOfType(SWITCH_TYPE_LOGICAL, available);
        if (!first.isNone())
          POPUP_MENU_ADD_ITEM(STR_MENU_LOGICAL_SWITCHES);
      }
      if (allowedTypes & SW_TYPE_BIT(SWITCH_TYPE_ON))
        POPUP_MENU_ADD_ITEM(STR_MENU_OTHER);
      POPUP_MENU_ADD_ITEM(STR_MENU_INVERT);
      POPUP_MENU_START(onSwitchPopupSelect);
      s_editMode = EDIT_MODIFY_FIELD;
    }

    // Consume popup selection
    if (s_switchPopupPending) {
      if (s_switchPopupSelection.flags & SWITCH_FLAG_INVERTED) {
        // Invert current value
        newValue.flags ^= SWITCH_FLAG_INVERTED;
      } else {
        newValue = s_switchPopupSelection;
      }
      s_switchPopupPending = false;
      s_editMode = EDIT_MODIFY_FIELD;
    }

#if defined(AUTOSWITCH)
    {
      SwitchRef moved = getMovedSwitch();
      if (!moved.isNone() && (allowedTypes & SW_TYPE_BIT(moved.type)) &&
          (!available || available(moved))) {
        newValue = moved;
      }
    }
#endif

    bool forward = (event == EVT_ROTARY_RIGHT || event == EVT_KEY_FIRST(KEY_PLUS) ||
                    event == EVT_KEY_REPT(KEY_PLUS));
    bool backward = (event == EVT_ROTARY_LEFT || event == EVT_KEY_FIRST(KEY_MINUS) ||
                     event == EVT_KEY_REPT(KEY_MINUS));
    if (forward || backward) {
      auto step = [&](SwitchRef cur) {
        return forward ? nextSwitch(cur) : prevSwitch(cur);
      };
      SwitchRef candidate = step(value);
      int limit = 500;
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

    // Long ENTER: show source category popup
    if (event == EVT_KEY_LONG(KEY_ENTER)) {
      killEvents(event);
      s_sourcePopupSelection = {};
      s_sourcePopupPending = false;
      s_sourceAvailableForPopup = available;

      for (unsigned i = 0; i < DIM(sourcePopupEntries); i++) {
        if (allowedTypes & SRC_TYPE_BIT(sourcePopupEntries[i].type)) {
          SourceRef first = firstAvailableSourceOfType(sourcePopupEntries[i].type, available);
          if (!first.isNone())
            POPUP_MENU_ADD_ITEM(sourcePopupEntries[i].menuItem);
        }
      }
      if ((allowedTypes & SRC_TYPE_BIT(SOURCE_TYPE_TELEMETRY)) && modelTelemetryEnabled()) {
        for (int i = 0; i < (int)getSensorCount(); i++) {
          if (sensorAddress(i)->isAvailable()) {
            POPUP_MENU_ADD_ITEM(STR_MENU_TELEMETRY);
            break;
          }
        }
      }
      POPUP_MENU_START(onSourcePopupSelect);
    }

    // Consume popup selection
    if (s_sourcePopupPending) {
      newValue = s_sourcePopupSelection;
      s_sourcePopupPending = false;
      s_editMode = EDIT_MODIFY_FIELD;
    }

#if defined(AUTOSOURCE)
    {
      SourceRef moved = getMovedSource();
      if (!moved.isNone() && (allowedTypes & SRC_TYPE_BIT(moved.type)) &&
          (!available || available(moved))) {
        newValue = moved;
      }
#if defined(AUTOSWITCH)
      else {
        SwitchRef swtch = getMovedSwitch();
        if (!swtch.isNone() && swtch.type == SWITCH_TYPE_SWITCH &&
            (allowedTypes & SRC_TYPE_BIT(SOURCE_TYPE_SWITCH))) {
          newValue = SourceRef_(SOURCE_TYPE_SWITCH, (uint16_t)(swtch.index / 3));
        }
      }
#endif
    }
#endif

    bool forward = (event == EVT_ROTARY_RIGHT || event == EVT_KEY_FIRST(KEY_PLUS) ||
                    event == EVT_KEY_REPT(KEY_PLUS));
    bool backward = (event == EVT_ROTARY_LEFT || event == EVT_KEY_FIRST(KEY_MINUS) ||
                     event == EVT_KEY_REPT(KEY_MINUS));
    if (forward || backward) {
      auto step = [&](SourceRef cur) {
        return forward ? nextSource(cur) : prevSource(cur);
      };
      SourceRef candidate = step(value);
      int limit = 1000;
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


void editValueOrSource(coord_t x, coord_t y, const char* title, ValueOrSource* vos,
                       int16_t min, int16_t max, LcdFlags attr, event_t event)
{
  if (title)
    lcdDrawTextAlignedLeft(y, title);
  if (vos->isSource) {
    SourceRef ref = vos->toSourceRef();
    if (ref.type == SOURCE_TYPE_GVAR) {
      int8_t idx = ref.isInverted() ? -(int8_t)ref.index - 1 : (int8_t)ref.index;
      drawGVarName(x, y, idx, attr);
    } else {
      drawSource(x, y, ref, attr);
    }
    if (attr & (~RIGHT)) {
      vos->setSource(checkIncDecSource(event, vos->toSourceRef(), SRCMASK_ALL, isSourceAvailable));
    }
  } else {
    lcdDrawNumber(x, y, vos->numericValue(), attr);
    if (attr & (~RIGHT)) {
      int16_t val = vos->numericValue();
      CHECK_INCDEC_MODELVAR(event, val, min, max);
      vos->setNumeric(val);
    }
  }
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
      value = (GV_IS_GV_VALUE(value) ? GV_ENCODE(GET_GVAR(value, min, max, mixerCurrentFlightMode)*10) : GV_VALUE_FROM_INDEX(0));
    else
      value = (GV_IS_GV_VALUE(value) ? GV_ENCODE(GET_GVAR(value, min, max, mixerCurrentFlightMode)) : GV_VALUE_FROM_INDEX(0));
    storageDirty(EE_MODEL);
  }

  if (GV_IS_GV_VALUE(value)) {
    attr &= ~PREC1;
    int8_t idx = (int16_t)GV_INDEX_FROM_VALUE(value);
    if (invers) {
      CHECK_INCDEC_MODELVAR(event, idx, -getGVarCount(), getGVarCount()-1);
    }
    value = (int16_t)GV_VALUE_FROM_INDEX(idx);
    drawGVarName(x, y, idx, attr);
  }
  else {
    lcdDrawNumber(x, y, GV_DECODE(value), attr);
    if (invers) value = GV_ENCODE(checkIncDec(event, GV_DECODE(value), min, max, EE_MODEL | editflags));
  }
#else
  lcdDrawNumber(x, y, GV_DECODE(value), attr);
  if (attr&INVERS) value = GV_ENCODE(checkIncDec(event, GV_DECODE(value), min, max, EE_MODEL));
#endif
  return value;
}

#if defined(GVARS)
void editGVarValue(coord_t x, coord_t y, event_t event, uint8_t gvar, uint8_t flightMode, LcdFlags flags)
{
  gvar_t * v = &GVAR_VALUE(gvar, flightMode);
  int16_t vmin, vmax;
  if (*v > GVAR_MAX) {
    uint8_t fm = *v - GVAR_MAX - 1;
    if (fm >= flightMode) fm++;
    drawFlightMode(x, y, fm + 1, flags&(~LEFT));
    vmin = GVAR_MAX + 1;
    vmax = GVAR_MAX + MAX_FLIGHT_MODES - 1;
  }
  else {
    vmin = GVAR_MIN + gvarDataAddress(gvar)->min;
    vmax = GVAR_MAX - gvarDataAddress(gvar)->max;
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
