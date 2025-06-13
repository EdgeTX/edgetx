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

void drawStick(coord_t centrex, int16_t xval, int16_t yval)
{
#define BOX_CENTERY   (LCD_H-BOX_WIDTH/2-10)
#define MARKER_WIDTH  5
  lcdDrawSquare(centrex-BOX_WIDTH/2, BOX_CENTERY-BOX_WIDTH/2, BOX_WIDTH);
  lcdDrawSolidVerticalLine(centrex, BOX_CENTERY-1, 3);
  lcdDrawSolidHorizontalLine(centrex-1, BOX_CENTERY, 3);
  lcdDrawSquare(centrex + (xval/((2*RESX)/(BOX_WIDTH-MARKER_WIDTH))) - MARKER_WIDTH/2, BOX_CENTERY - (yval/((2*RESX)/(BOX_WIDTH-MARKER_WIDTH))) - MARKER_WIDTH/2, MARKER_WIDTH, ROUND);
#undef BOX_CENTERY
#undef MARKER_WIDTH
}

void drawColumnHeader(const char * const *headers, uint8_t index)
{
  lcdDrawText(16*FW+3, 0, " ", 0);
  lcdDrawText(17*FW, 0, headers[index], 0);
  lcdDrawText(lcdNextPos, 0, " ", 0);
}

void drawCheckBox(coord_t x, coord_t y, uint8_t value, LcdFlags attr)
{
  if (value)
    lcdDrawChar(x+1, y, '#');
  if (attr)
    lcdDrawFilledRect(x, y, 7, 7);
  else
    lcdDrawSquare(x, y, 7);
}

void drawScreenIndex(uint8_t index, uint8_t count, uint8_t attr)
{
  lcdDrawNumber(LCD_W, 0, count, RIGHT | attr);
  coord_t x = 1+LCD_W-FW*(count>9 ? 3 : 2);
  lcdDrawChar(x, 0, '/', attr);
  lcdDrawNumber(x, 0, index+1, RIGHT | attr);
}

void drawVerticalScrollbar(coord_t x, coord_t y, coord_t h, uint16_t offset, uint16_t count, uint8_t visible)
{
  lcdDrawVerticalLine(x, y, h, DOTTED);
  coord_t yofs = (h * offset) / count;
  coord_t yhgt = (h * visible) / count;
  if (yhgt + yofs > h)
    yhgt = h - yofs;
  lcdDrawVerticalLine(x, y + yofs, yhgt, SOLID, FORCE);
}

void drawGauge(coord_t x, coord_t y, coord_t w, coord_t h, int32_t val, int32_t max)
{
  lcdDrawRect(x, y, w+1, h);
  lcdDrawFilledRect(x+1, y+1, w-1, 4, SOLID, ERASE);
  coord_t len = limit((uint8_t)1, uint8_t((abs(val) * w/2 + max/2) / max), uint8_t(w/2));
  coord_t x0 = (val>0) ? x+w/2 : x+1+w/2-len;
  for (coord_t i=h-2; i>0; i--) {
    lcdDrawSolidHorizontalLine(x0, y+i, len);
  }
}

void title(const char * s)
{
  lcdDrawText(0, 0, s, INVERS);
}

choice_t editChoice(coord_t x, coord_t y, const char * label, const char* const* values, choice_t value, choice_t min, choice_t max, LcdFlags attr, event_t event, coord_t lblX, IsValueAvailable isValueAvailable)
{
  if (label) {
    lcdDrawText(lblX, y, label);
  }
  if (values) lcdDrawTextAtIndex(x, y, values, value-min, attr);
  if (attr & (~RIGHT)) value = checkIncDec(event, value, min, max, (isModelMenuDisplayed()) ? EE_MODEL : EE_GENERAL, isValueAvailable);
  return value;
}

choice_t editChoice(coord_t x, coord_t y, const char * label, const char* const* values, choice_t value, choice_t min, choice_t max, LcdFlags attr, event_t event, coord_t lblX)
{
  return editChoice(x, y, label, values, value, min, max, attr, event, lblX, nullptr);
}

uint8_t editCheckBox(uint8_t value, coord_t x, coord_t y, const char *label, LcdFlags attr, event_t event, coord_t lblX)
{
  drawCheckBox(x, y, value, attr);
  return editChoice(x, y, label, nullptr, value, 0, 1, attr, event, lblX);
}

swsrc_t editSwitch(coord_t x, coord_t y, swsrc_t value, LcdFlags attr, event_t event)
{
  lcdDrawTextAlignedLeft(y, STR_SWITCH);
  drawSwitch(x,  y, value, attr);
  if (attr & (~RIGHT)) CHECK_INCDEC_MODELSWITCH(event, value, SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES, isSwitchAvailableInMixes);
  return value;
}

void drawSlider(coord_t x, coord_t y, uint8_t value, uint8_t max, uint8_t attr)
{
  lcdDrawChar(x+(value*4*FW)/max, y, '$');
  lcdDrawSolidHorizontalLine(x, y+3, 5*FW-1, FORCE);
  if (attr && (!(attr & BLINK) || !BLINK_ON_PHASE)) lcdDrawFilledRect(x, y, 5*FW-1, FH-1);
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

#if defined(GVARS)
void drawGVarValue(coord_t x, coord_t y, uint8_t gvar, gvar_t value, LcdFlags flags)
{
  uint8_t prec = g_model.gvars[gvar].prec;
  if (prec > 0) {
    flags |= (prec == 1 ? PREC1 : PREC2);
  }
  drawValueWithUnit(x, y, value, g_model.gvars[gvar].unit ? UNIT_PERCENT : UNIT_RAW, flags);
}

int16_t editGVarFieldValue(coord_t x, coord_t y, int16_t value, int16_t min, int16_t max, LcdFlags attr, uint8_t editflags, event_t event)
{
  bool invers = (attr & INVERS);

  // TRACE("editGVarFieldValue(val=%d min=%d max=%d)", value, min, max);

  if (modelGVEnabled() && invers && event == EVT_KEY_LONG(KEY_ENTER)) {
    killEvents(event);
    s_editMode = !s_editMode;
    if (attr & PREC1) {
      value = (GV_IS_GV_VALUE(value) ? GET_GVAR(value, min, max, mixerCurrentFlightMode)*10 : GV_VALUE_FROM_INDEX(0));
    }
    else {
      value = (GV_IS_GV_VALUE(value) ? GET_GVAR(value, min, max, mixerCurrentFlightMode) : GV_VALUE_FROM_INDEX(0));
    }
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
  return value;
}
#else
int16_t editGVarFieldValue(coord_t x, coord_t y, int16_t value, int16_t min, int16_t max, LcdFlags attr, event_t event)
{
  lcdDrawNumber(x, y, value, attr);
  if (attr&INVERS) value = checkIncDec(event, value, min, max, EE_MODEL);
  return value;
}
#endif

char statusLineMsg[STATUS_LINE_LENGTH];
tmr10ms_t statusLineTime = 0;
uint8_t statusLineHeight = 0;

void showStatusLine()
{
  statusLineTime = get_tmr10ms();
}

#define STATUS_LINE_DELAY (3 * 100) /* 3s */
void drawStatusLine()
{
  if (statusLineTime) {
    if ((tmr10ms_t)(get_tmr10ms() - statusLineTime) <= (tmr10ms_t)STATUS_LINE_DELAY) {
      if (statusLineHeight < FH) statusLineHeight++;
    }
    else if (statusLineHeight) {
      statusLineHeight--;
    }
    else {
      statusLineTime = 0;
    }

    lcdDrawFilledRect(0, LCD_H-statusLineHeight, LCD_W, FH, SOLID, ERASE);
    lcdDrawText(5, LCD_H+1-statusLineHeight, statusLineMsg, 0);
    lcdDrawFilledRect(0, LCD_H-statusLineHeight, LCD_W, FH, SOLID);
  }
}
