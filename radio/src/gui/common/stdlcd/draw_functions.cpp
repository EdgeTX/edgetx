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
#include <math.h>

#include "hal/adc_driver.h"
#include "analogs.h"

#if defined(MULTIMODULE)
void lcdDrawMultiProtocolString(coord_t x, coord_t y, uint8_t moduleIdx, uint8_t protocol, LcdFlags flags)
{
  MultiModuleStatus & status = getMultiModuleStatus(moduleIdx);
  if (status.protocolName[0] && status.isValid()) {
    lcdDrawText(x, y, status.protocolName, flags);
  }
  else if (protocol <= MODULE_SUBTYPE_MULTI_LAST) {
    lcdDrawTextAtIndex(x, y, STR_MULTI_PROTOCOLS, protocol, flags);
  }
  else {
    lcdDrawNumber(x, y, protocol + 3, flags); // Convert because of OpenTX FrSky fidling (OpenTX protocol tables and Multiprotocol tables don't match)
  }
}

void lcdDrawMultiSubProtocolString(coord_t x, coord_t y, uint8_t moduleIdx, uint8_t subType, LcdFlags flags)
{
  MultiModuleStatus & status = getMultiModuleStatus(moduleIdx);
  const mm_protocol_definition * pdef = getMultiProtocolDefinition(g_model.moduleData[moduleIdx].multi.rfProtocol);

  if (status.protocolName[0] && status.isValid()) {
    lcdDrawText(x, y, status.protocolSubName, flags);
  }
  else if (subType <= pdef->maxSubtype && pdef->subTypeString != nullptr) {
    lcdDrawTextAtIndex(x, y, pdef->subTypeString, subType, flags);
  }
  else {
    lcdDrawNumber(x, y, subType, flags);
  }
}
#endif

void drawStringWithIndex(coord_t x, coord_t y, const char * str, uint8_t idx, LcdFlags flags)
{
  if (flags & RIGHT) {
    lcdDrawNumber(x, y, idx, flags);
    lcdDrawText(lcdNextPos, y, str, flags & ~LEADING0);
  }
  else {
    lcdDrawText(x, y, str, flags & ~LEADING0);
    lcdDrawNumber(lcdNextPos, y, idx, flags|LEFT, 2);
  }
}

void drawTrimMode(coord_t x, coord_t y, uint8_t flightMode, uint8_t idx, LcdFlags att)
{
  trim_t v = getRawTrimValue(flightMode, idx);
  unsigned int mode = v.mode;
  unsigned int p = mode >> 1;

  if (mode == TRIM_MODE_NONE) {
    lcdDrawText(x, y, "--", att);
  } else if (mode == TRIM_MODE_3POS) {
    lcdDrawText(x, y, "3P", att);
  } else {
    if (mode % 2 == 0)
      lcdDrawChar(x, y, ':', att|FIXEDWIDTH);
    else
      lcdDrawChar(x, y, '+', att|FIXEDWIDTH);
    lcdDrawChar(lcdNextPos, y, '0'+p, att);
  }
}

void drawValueWithUnit(coord_t x, coord_t y, int32_t val, uint8_t unit, LcdFlags att)
{
  // convertUnit(val, unit);
  lcdDrawNumber(x, y, val, att & (~NO_UNIT));
  if (!(att & NO_UNIT) && unit != UNIT_RAW) {
    lcdDrawTextAtIndex(lcdLastRightPos/*+1*/, y, STR_VTELEMUNIT, unit, 0);
  }
}

FlightModesType editFlightModes(coord_t x, coord_t y, event_t event, FlightModesType value, uint8_t attr)
{
  int posHorz = menuHorizontalPosition;

  for (uint8_t p=0; p<MAX_FLIGHT_MODES; p++) {
    LcdFlags flags = 0;
    if (attr) {
      flags |= INVERS;
      if (posHorz==p) flags |= BLINK;
    }
    if (value & (1<<p))
      lcdDrawChar(x, y, ' ', flags|FIXEDWIDTH);
    else
      lcdDrawChar(x, y, '0'+p, flags);
    x += FW;
  }

  if (attr) {
    if (s_editMode && event==EVT_KEY_BREAK(KEY_ENTER)) {
      s_editMode = 0;
      value ^= (1<<posHorz);
      storageDirty(EE_MODEL);
    }
  }

  return value;
}

char getNextChar(char c, uint8_t position)
{
  if (c == ' ')
    return (position == 0 ? 'A' : 'a');

  if (c == 'Z' || c == 'z')
    return '0';

  static const char * specialChars = "9_-., ";
  for (uint8_t i = 0; specialChars[i] != 0; i++) {
    if (c == specialChars[i])
      return specialChars[i + 1];
  }

  return c + 1;
}

char getPreviousChar(char c, uint8_t position)
{
  if (c == 'A' || c == 'a')
    return ' ';

  if (c == '0')
    return (position == 0 ? 'Z' : 'z');

  static const char * specialChars = "9_-., ";
  for (uint8_t i = 1; specialChars[i] != 0; i++) {
    if (c == specialChars[i])
      return specialChars[i - 1];
  }

  return c - 1;
}

static const char nameChars[] = " abcdefghijklmnopqrstuvwxyz0123456789_-,.";

static int nameCharIdx(char v)
{
  if (islower(v)) return v - 'a' + 1;
  if (isupper(v)) return v - 'A' + 1;
  if (isdigit(v)) return v - '0' + 27;
  if (v == '_') return 37;
  if (v == '-') return 38;
  if (v == ',') return 39;
  if (v == '.') return 40;
  return 0;
}

void editName(coord_t x, coord_t y, char* name, uint8_t size, event_t event,
              uint8_t active, LcdFlags attr, uint8_t old_editMode)
{
  uint8_t mode = 0;
  if (active) {
    if (s_editMode <= 0)
      mode = INVERS + FIXEDWIDTH;
    else
      mode = FIXEDWIDTH;
  }

  lcdDrawSizedText(x, y, name[0] == '\0' ? "---" : name, size, attr | mode);
  coord_t backupNextPos = lcdNextPos;

  if (active) {
    uint8_t cur = editNameCursorPos;
    if (s_editMode > 0) {
      int8_t c = name[cur];
      int8_t v = c ? c : ' ';

      if (IS_NEXT_EVENT(event) || IS_PREVIOUS_EVENT(event)) {
        bool caps = isupper(v);
        v = nameChars[checkIncDec(event, nameCharIdx(v), 0, DIM(nameChars)-2)];
        if (caps && islower(v)) v = toupper(v);
      }

      switch (event) {
        case EVT_KEY_BREAK(KEY_ENTER):
          if (s_editMode == EDIT_MODIFY_FIELD) {
            s_editMode = EDIT_MODIFY_STRING;
            cur = 0;
          }
          else if (cur < size - 1)
            cur++;
          else
            s_editMode = 0;
          break;

#if defined(NAVIGATION_XLITE) || defined(NAVIGATION_9X)
        case EVT_KEY_BREAK(KEY_LEFT):
          if (cur > 0)
            cur--;
          break;

        case EVT_KEY_BREAK(KEY_RIGHT):
          if (cur < size - 1)
            cur++;
          break;
#endif

#if defined(NAVIGATION_XLITE)
        case EVT_KEY_BREAK(KEY_SHIFT):
#elif defined(NAVIGATION_9X)
        case EVT_KEY_LONG(KEY_LEFT):
        case EVT_KEY_LONG(KEY_RIGHT):
#else
        case EVT_KEY_LONG(KEY_ENTER):
#endif
          killEvents(event);

#if !defined(NAVIGATION_XLITE)
          if (v == ' ') {
            s_editMode = 0;
            break;
          }
          else
#endif
          if (v >= 'A' && v <= 'Z') {
            v = 'a' + v - 'A'; // toggle case
          }
          else if (v >= 'a' && v <= 'z') {
            v = 'A' + v - 'a'; // toggle case
          }

#if defined(NAVIGATION_9X)
          if (event==EVT_KEY_LONG(KEY_LEFT))
#endif
          break;
      }

      if (c != v) {
        name[cur] = v;
        storageDirty(isModelMenuDisplayed() ? EE_MODEL : EE_GENERAL);
      }

      lcdDrawChar(x+editNameCursorPos*FW, y, v, ERASEBG|INVERS|FIXEDWIDTH);
    }
    else {
      cur = 0;
    }
    editNameCursorPos = cur;
    lcdNextPos = backupNextPos;

    if ((old_editMode > 0) && (s_editMode == 0)) {
      bool modified = false;
      for (int8_t i = size - 1; i >= 0; --i) {
        // wipe empty spaces
        if (name[i] == ' ') {
          name[i] = '\0';
          modified = true;

        } else if (name[i] != '\0') {
          // and stop at the first non empty character
          break;
        }
      }

      if (modified) {
        storageDirty(isModelMenuDisplayed() ? EE_MODEL : EE_GENERAL);
      }
    }
  }
}

void drawGVarName(coord_t x, coord_t y, int8_t idx, LcdFlags flags)
{
  char s[8];
  getGVarString(s, idx);
  lcdDrawText(x, y, s, flags);
}

void editStickHardwareSettings(coord_t x, coord_t y, int idx, event_t event,
                               LcdFlags flags, uint8_t old_editMode)
{
  lcdDrawTextIndented(y, STR_CHAR_STICK);
  lcdDrawText(lcdNextPos, y, analogGetCanonicalName(ADC_INPUT_MAIN, idx), 0);

  if (analogHasCustomLabel(ADC_INPUT_MAIN, idx) || (flags && s_editMode > 0))
    editName(x, y, (char*)analogGetCustomLabel(ADC_INPUT_MAIN, idx),
             LEN_ANA_NAME, event, (flags != 0), flags, old_editMode);
  else
    lcdDrawMMM(x, y, flags);
}

bool isSwitchAvailableInCustomFunctions(int swtch)
{
  if (menuHandlers[menuLevel] == menuModelSpecialFunctions)
    return isSwitchAvailable(swtch, ModelCustomFunctionsContext);
  else
    return isSwitchAvailable(swtch, GeneralCustomFunctionsContext);
}

void drawPower(coord_t x, coord_t y, int8_t dBm, LcdFlags att)
{
  float power_W_PREC1 = powf(10.0, (dBm - 30.0) / 10.0) * 10;
  if (dBm >= 30) {
    lcdDrawNumber(x, y, power_W_PREC1, PREC1 | att);
    lcdDrawText(lcdNextPos, y, "W", att);
  }
  else if (dBm < 10) {
    uint16_t power_MW_PREC1 = roundf(power_W_PREC1 * 1000);
    lcdDrawNumber(x, y, power_MW_PREC1, PREC1 | att);
    lcdDrawText(lcdNextPos, y, "mW", att);
  }
  else {
    uint16_t power_MW = roundf(power_W_PREC1 * 100);
    if (power_MW >= 50) {
      power_MW = (power_MW / 5) * 5;
      lcdDrawNumber(x, y, power_MW, att);
      lcdDrawText(lcdNextPos, y, "mW", att);
    }
    else {
      lcdDrawNumber(x, y, power_MW, att);
      lcdDrawText(lcdNextPos, y, "mW", att);
    }
  }
}

void drawReceiverName(coord_t x, coord_t y, uint8_t moduleIdx, uint8_t receiverIdx, LcdFlags flags)
{
  if (isModulePXX2(moduleIdx)) {
    if (g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx][0] != '\0')
      lcdDrawSizedText(x, y, g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], effectiveLen(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], PXX2_LEN_RX_NAME), flags);
    else
      lcdDrawText(x, y, "---", flags);
  }
#if defined(HARDWARE_INTERNAL_MODULE)
  else if (moduleIdx == INTERNAL_MODULE) {
    lcdDrawText(x, y, "Internal", flags);
  }
#endif
  else {
    lcdDrawText(x, y, "External", flags);
  }
}

void lcdDrawMMM(coord_t x, coord_t y, LcdFlags flags)
{
  lcdDrawTextAtIndex(x, y, STR_MMMINV, 0, flags);
}

#if defined(FLIGHT_MODES)
void drawFlightMode(coord_t x, coord_t y, int8_t idx, LcdFlags att)
{
  if (idx==0) {
    lcdDrawMMM(x, y, att);
    return;
  }
  // TODO this code was not included in Taranis! and used with abs(...) on Horus
  if (idx < 0) {
    lcdDrawChar(x-2, y, '!', att);
    idx = -idx;
  }
#if defined(CONDENSED)
  if (att & CONDENSED) {
    lcdDrawNumber(x+FW*1, y, idx-1, (att & ~CONDENSED), 1);
    return;
  }
#endif
  drawStringWithIndex(x, y, STR_FM, idx-1, att);
}
#endif

void drawCurveRef(coord_t x, coord_t y, CurveRef & curve, LcdFlags att)
{
  if (curve.value != 0) {
    switch (curve.type) {
      case CURVE_REF_DIFF:
        lcdDrawText(x, y, "D", att);
        editSrcVarFieldValue(lcdNextPos, y, nullptr, curve.value, -100, 100, LEFT|att, 0, 0, MIXSRC_FIRST, INPUTSRC_LAST);
        break;

      case CURVE_REF_EXPO:
        lcdDrawText(x, y, "E", att);
        editSrcVarFieldValue(lcdNextPos, y, nullptr, curve.value, -100, 100, LEFT|att, 0, 0, MIXSRC_FIRST, INPUTSRC_LAST);
        break;

      case CURVE_REF_FUNC:
        lcdDrawTextAtIndex(x, y, STR_VCURVEFUNC, curve.value, att);
        break;

      case CURVE_REF_CUSTOM:
        drawCurveName(x, y, curve.value, att);
        break;
    }
  }
}

void drawSensorCustomValue(coord_t x, coord_t y, uint8_t sensor, int32_t value, LcdFlags flags)
{
  if (sensor >= MAX_TELEMETRY_SENSORS) {
    // Lua luaLcdDrawChannel() can call us with a bad value
    return;
  }

  if (IS_FAI_FORBIDDEN(MIXSRC_FIRST_TELEM + 3 * sensor)) {
    lcdDrawText(x, y, "FAI mode", flags);
    return;
  }

  TelemetryItem & telemetryItem = telemetryItems[sensor];
  TelemetrySensor & telemetrySensor = g_model.telemetrySensors[sensor];

  if (telemetrySensor.unit == UNIT_DATETIME) {
    drawDate(x, y, telemetryItem, flags);
  }
  else if (telemetrySensor.unit == UNIT_GPS) {
    drawGPSSensorValue(x, y, telemetryItem, flags);
  }
  else if (telemetrySensor.unit == UNIT_TEXT) {
    lcdDrawSizedText(x, flags & DBLSIZE ? y + 1 : y, telemetryItem.text,
                     sizeof(telemetryItem.text), flags & ~DBLSIZE);
  } else {
    if (telemetrySensor.prec > 0) {
      flags |= (telemetrySensor.prec == 1 ? PREC1 : PREC2);
    }
    drawValueWithUnit(x, y, value,
        telemetrySensor.unit == UNIT_CELLS ? UNIT_VOLTS : telemetrySensor.unit,
        flags);
  }
}

void drawSourceCustomValue(coord_t x, coord_t y, mixsrc_t source, int32_t value, LcdFlags flags)
{
  source = abs(source);

  if (source >= MIXSRC_FIRST_TELEM) {
    source = (source-MIXSRC_FIRST_TELEM) / 3;
    drawSensorCustomValue(x, y, source, value, flags);
  }
  else if (source >= MIXSRC_FIRST_TIMER || source == MIXSRC_TX_TIME) {
    if (value < 0) flags |= BLINK|INVERS;
    drawTimer(x, y, value, flags);
  }
  else if (source == MIXSRC_TX_VOLTAGE) {
    lcdDrawNumber(x, y, value, flags|PREC1);
  }
#if defined(INTERNAL_GPS)
    else if (source == MIXSRC_TX_GPS) {
    if (gpsData.fix) {
      drawGPSPosition(x, y, gpsData.longitude, gpsData.latitude, flags);
    }
    else {
      lcdDrawText(x, y, "sats: ", flags);
      lcdDrawNumber(lcdNextPos, y, gpsData.numSat, flags);
    }
  }
#endif
#if defined(GVARS)
  else if (source >= MIXSRC_FIRST_GVAR && source <= MIXSRC_LAST_GVAR) {
    drawGVarValue(x, y, source - MIXSRC_FIRST_GVAR, value, flags);
  }
#endif
  else if (source < MIXSRC_FIRST_CH) {
    lcdDrawNumber(x, y, calcRESXto100(value), flags);
  }
  else if (source <= MIXSRC_LAST_CH) {
    if (g_eeGeneral.ppmunit == PPM_PERCENT_PREC1) {
      lcdDrawNumber(x, y, calcRESXto1000(value), flags|PREC1);
    } else {
      lcdDrawNumber(x, y, calcRESXto100(value), flags);
    }
  }
  else {
    lcdDrawNumber(x, y, value, flags);
  }
}

void drawSourceValue(coord_t x, coord_t y, source_t source, LcdFlags flags)
{
  getvalue_t value = getValue(source);
  drawSourceCustomValue(x, y, source, value, flags);
}

void drawFatalErrorScreen(const char * message)
{
  lcdClear();
  lcdDrawText((LCD_W - getTextWidth(message, 0, DBLSIZE)) / 2,
              LCD_H/2 - FH, message, DBLSIZE);
  WDG_RESET();
  lcdRefresh();
  lcdRefreshWait();
}

void runFatalErrorScreen(const char * message)
{
  while (true) {
    backlightFullOn();
    drawFatalErrorScreen(message);

    uint8_t refresh = false;
    while (true) {
      uint32_t pwr_check = pwrCheck();
      if (pwr_check == e_power_off) {
        boardOff();
        return;  // only happens in SIMU, required for proper shutdown
      }
      else if (pwr_check == e_power_press) {
        refresh = true;
      }
      else if (pwr_check == e_power_on && refresh) {
        break;
      }
      WDG_RESET();
    }
  }
}

void drawSource(coord_t x, coord_t y, mixsrc_t idx, LcdFlags att)
{
  uint16_t aidx = abs(idx);
  bool inverted = idx < 0;

  if (aidx == MIXSRC_NONE) {
    lcdDrawText(x, y, STR_EMPTY, att);
  }
  else if (aidx <= MIXSRC_LAST_INPUT) {
    if (att & RIGHT) {
      if (g_model.inputNames[aidx-MIXSRC_FIRST_INPUT][0])
        lcdDrawSizedText(x, y, g_model.inputNames[aidx-MIXSRC_FIRST_INPUT], LEN_INPUT_NAME, att);
      else
        lcdDrawNumber(x, y, aidx, att|LEADING0, 2);
      x = lcdLastLeftPos - 5;
      if (inverted)
        lcdDrawChar(x-5, y, '-');
      lcdDrawChar(x, y+1, CHR_INPUT, RIGHT|TINSIZE);
      lcdDrawSolidFilledRect(x-1, y, 5, 7);
    } else {
      if (inverted) {
        lcdDrawChar(x-1, y, '-');
        x += 3;
      }
      lcdDrawChar(x+1, y+1, CHR_INPUT, TINSIZE);
      lcdDrawSolidFilledRect(x, y, 5, 7);
      if (g_model.inputNames[aidx-MIXSRC_FIRST_INPUT][0])
        lcdDrawSizedText(x+6, y, g_model.inputNames[aidx-MIXSRC_FIRST_INPUT], LEN_INPUT_NAME, att);
      else
        lcdDrawNumber(x+6, y, aidx, att|LEADING0, 2);
    }
  }
#if defined(LUA_INPUTS)
  else if (aidx <= MIXSRC_LAST_LUA) {
    div_t qr = div((uint16_t)(aidx-MIXSRC_FIRST_LUA), MAX_SCRIPT_OUTPUTS);
    if (att & RIGHT) {
#if defined(LUA_MODEL_SCRIPTS)
      if (qr.quot < MAX_SCRIPTS && qr.rem < scriptInputsOutputs[qr.quot].outputsCount) {
        lcdDrawSizedText(x, y, scriptInputsOutputs[qr.quot].outputs[qr.rem].name, att & STREXPANDED ? 9 : 4, att);
        x = lcdLastLeftPos - 4;
        if (inverted)
          lcdDrawChar(x-5, y, '-');
        lcdDrawChar(x, y+1, '1'+qr.quot, TINSIZE);
        lcdDrawFilledRect(x-1, y, 5, 7, SOLID);
      }
      else
#endif
      {
        lcdDrawChar(x, y, 'a' + qr.rem, att);
        drawStringWithIndex(lcdLastLeftPos, y, "LUA", qr.quot+1, att);
#if defined(LUA_MODEL_SCRIPTS)
        if (inverted)
          lcdDrawChar(lcdLastLeftPos, y, '-', att);
#endif
      }
    } else {
#if defined(LUA_MODEL_SCRIPTS)
      if (inverted) {
        lcdDrawChar(x-1, y, '-');
        x += 3;
      }
      if (qr.quot < MAX_SCRIPTS && qr.rem < scriptInputsOutputs[qr.quot].outputsCount) {
        lcdDrawChar(x+1, y+1, '1'+qr.quot, TINSIZE);
        lcdDrawFilledRect(x, y, 5, 7, SOLID);
        lcdDrawSizedText(x+5, y, scriptInputsOutputs[qr.quot].outputs[qr.rem].name, att & STREXPANDED ? 9 : 4, att);
      }
      else
#endif
      {
        drawStringWithIndex(x, y, "LUA", qr.quot+1, att);
        lcdDrawChar(lcdLastRightPos, y, 'a' + qr.rem, att);
      }
    }
  }
#endif
  else {
    const char* s = getSourceString(idx);
#if LCD_W < 212
    if (idx >= MIXSRC_FIRST_TELEM && idx <= MIXSRC_LAST_TELEM)
      s += strlen(STR_CHAR_TELEMETRY);
#endif
    lcdDrawText(x, y, s, att);
  }
}
