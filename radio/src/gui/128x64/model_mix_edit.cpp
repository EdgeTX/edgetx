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
#include "mixes.h"

enum MixFields {
  MIX_FIELD_NAME,
  MIX_FIELD_SOURCE,
  MIX_FIELD_WEIGHT,
  MIX_FIELD_OFFSET,
  MIX_FIELD_TRIM,
  MIX_FIELD_CURVE,
  CASE_FLIGHT_MODES(MIX_FIELD_FLIGHT_MODE)
  MIX_FIELD_SWITCH,
  MIX_FIELD_WARNING,
  MIX_FIELD_MLTPX,
  MIX_FIELD_DELAY_PREC,
  MIX_FIELD_DELAY_UP,
  MIX_FIELD_DELAY_DOWN,
  MIX_FIELD_SLOW_PREC,
  MIX_FIELD_SLOW_UP,
  MIX_FIELD_SLOW_DOWN,
  MIX_FIELD_COUNT
};

extern uint8_t FM_ROW(uint8_t);

extern int32_t getSourceNumFieldValue(int16_t val, int16_t min, int16_t max);

void drawOffsetBar(uint8_t x, uint8_t y, MixData * md)
{
  const int gaugeWidth = 33;
  const int gaugeHeight = 6;

  int offset = getSourceNumFieldValue(md->offset, MIX_OFFSET_MIN, MIX_OFFSET_MAX) / 10;
  int weight = getSourceNumFieldValue(md->weight, MIX_WEIGHT_MIN, MIX_WEIGHT_MAX) / 10;
  int barMin = offset - weight;
  int barMax = offset + weight;
  if (y > 15) {
    lcdDrawNumber(x-((barMin >= 0) ? 2 : 3), y-6, barMin, TINSIZE|LEFT);
    lcdDrawNumber(x+gaugeWidth+1, y-6, barMax, TINSIZE|RIGHT);
  }
  if (weight < 0) {
    barMin = -barMin;
    barMax = -barMax;
  }
  if (barMin < -101)
    barMin = -101;
  if (barMax > 101)
    barMax = 101;
  lcdDrawHorizontalLine(x-2, y, gaugeWidth+2, DOTTED);
  lcdDrawHorizontalLine(x-2, y+gaugeHeight, gaugeWidth+2, DOTTED);
  lcdDrawSolidVerticalLine(x-2, y+1, gaugeHeight-1);
  lcdDrawSolidVerticalLine(x+gaugeWidth-1, y+1, gaugeHeight-1);
  if (barMin <= barMax) {
    int8_t right = (barMax * gaugeWidth) / 200;
    int8_t left = ((barMin * gaugeWidth) / 200)-1;
    lcdDrawSolidFilledRect(x+gaugeWidth/2+left, y+2, right-left, gaugeHeight-3);
  }
  lcdDrawSolidVerticalLine(x+gaugeWidth/2-1, y, gaugeHeight+1);
  if (barMin == -101) {
    for (uint8_t i=0; i<3; ++i) {
      lcdDrawPoint(x+i, y+4-i);
      lcdDrawPoint(x+3+i, y+4-i);
    }
  }
  if (barMax == 101) {
    for (uint8_t i=0; i<3; ++i) {
      lcdDrawPoint(x+gaugeWidth-8+i, y+4-i);
      lcdDrawPoint(x+gaugeWidth-5+i, y+4-i);
    }
  }
}

void menuModelMixOne(event_t event)
{
#if defined(NAVIGATION_XLITE)
  if (event == EVT_KEY_FIRST(KEY_ENTER) && keysGetState(KEY_SHIFT)) {
    pushMenu(menuChannelsView);
  }
#else
  if (event == EVT_KEY_BREAK(KEY_MODEL) || event == EVT_KEY_BREAK(KEY_MENU)) {
    pushMenu(menuChannelsView);
  }
#endif
  MixData * md2 = mixAddress(s_currIdx) ;
  putsChn(PSIZE(TR_MIXES)*FW+FW, 0, md2->destCh+1,0);

  uint8_t old_editMode = s_editMode;
  
  SUBMENU(STR_MIXES, MIX_FIELD_COUNT,
          {0, 0, 0, 0, 0, 1, CASE_FLIGHT_MODES(FM_ROW((MAX_FLIGHT_MODES-1) | NAVIGATION_LINE_BY_LINE)) 0, 0,
           (uint8_t)((s_currIdx > 0 && mixAddress(s_currIdx - 1)->destCh == md2->destCh) ? 0 : HIDDEN_ROW),
           0 /*, ...*/
          });
  
  int8_t sub = menuVerticalPosition;
  int8_t editMode = s_editMode;

  coord_t y = MENU_HEADER_HEIGHT + 1;

  for (int k=0; k<NUM_BODY_LINES; k++) {
    int i = k + menuVerticalOffset;
    for (int j=0; j<=i; ++j) {
      if (j<(int)DIM(mstate_tab) && mstate_tab[j] == HIDDEN_ROW) {
        ++i;
      }
    }
    LcdFlags attr = (sub==i ? (editMode>0 ? BLINK|INVERS : INVERS) : 0);

    switch(i) {
      case MIX_FIELD_NAME:
        editSingleName(MIXES_2ND_COLUMN, y, STR_MIXNAME, md2->name,
                       sizeof(md2->name), event, (attr != 0),
                       old_editMode);
        break;

      case MIX_FIELD_SOURCE:
        lcdDrawTextAlignedLeft(y, STR_SOURCE);
        drawSource(MIXES_2ND_COLUMN, y, md2->srcRaw, STREXPANDED|attr);
        if (attr)
          md2->srcRaw = checkIncDec(event, md2->srcRaw, 1, MIXSRC_LAST, EE_MODEL|INCDEC_SOURCE|INCDEC_SOURCE_INVERT|NO_INCDEC_MARKS, isSourceAvailable);
        break;

      case MIX_FIELD_WEIGHT:
        md2->weight = editSrcVarFieldValue(MIXES_2ND_COLUMN, y, STR_WEIGHT, md2->weight, 
                        MIX_WEIGHT_MIN, MIX_WEIGHT_MAX, attr, event, isSourceAvailable, 1, MIXSRC_LAST);
        break;

      case MIX_FIELD_OFFSET:
        md2->offset = editSrcVarFieldValue(MIXES_2ND_COLUMN, y, STR_OFFSET, md2->offset,
                        MIX_OFFSET_MIN, MIX_OFFSET_MAX, attr, event, isSourceAvailable, 1, MIXSRC_LAST);
        drawOffsetBar(LCD_W - 33, y, md2);
        break;

      case MIX_FIELD_TRIM:
        lcdDrawTextAlignedLeft(y, STR_TRIM);
        drawCheckBox(MIXES_2ND_COLUMN, y, !md2->carryTrim, attr);
        if (attr) md2->carryTrim = !checkIncDecModel(event, !md2->carryTrim, 0, 1);
        break;

      case MIX_FIELD_CURVE:
        lcdDrawTextAlignedLeft(y, STR_CURVE);
        s_currSrcRaw = md2->srcRaw;
        s_currScale = 0;
        editCurveRef(MIXES_2ND_COLUMN, y, md2->curve, event, attr, isSourceAvailable, 1, MIXSRC_LAST);
        break;

#if defined(FLIGHT_MODES)
      case MIX_FIELD_FLIGHT_MODE:
        lcdDrawTextAlignedLeft(y, STR_FLMODE);
        md2->flightModes = editFlightModes(MIXES_2ND_COLUMN, y, event, md2->flightModes, attr);
        break;
#endif

      case MIX_FIELD_SWITCH:
        md2->swtch = editSwitch(MIXES_2ND_COLUMN, y, md2->swtch, attr, event);
        break;

      case MIX_FIELD_WARNING:
        lcdDrawTextAlignedLeft(y, STR_MIXWARNING);
        if (md2->mixWarn)
          lcdDrawNumber(MIXES_2ND_COLUMN, y, md2->mixWarn, attr|LEFT);
        else
          lcdDrawText(MIXES_2ND_COLUMN, y, STR_OFF, attr);
        if (attr) CHECK_INCDEC_MODELVAR_ZERO(event, md2->mixWarn, 3);
        break;

      case MIX_FIELD_MLTPX:
        md2->mltpx = editChoice(MIXES_2ND_COLUMN, y, STR_MULTPX, STR_VMLTPX, md2->mltpx, 0, 2, attr, event);
        break;

      case MIX_FIELD_DELAY_PREC:
        md2->delayPrec = editChoice(MIXES_2ND_COLUMN, y, STR_MIX_DELAY_PREC, &STR_VPREC[1], md2->delayPrec, 0, 1, attr, event);
        break;

      case MIX_FIELD_DELAY_UP:
        md2->delayUp = EDIT_DELAY(0, y, event, attr, STR_DELAYUP, md2->delayUp, (md2->delayPrec ? PREC2 : PREC1));
        break;

      case MIX_FIELD_DELAY_DOWN:
        md2->delayDown = EDIT_DELAY(0, y, event, attr, STR_DELAYDOWN, md2->delayDown, (md2->delayPrec ? PREC2 : PREC1));
        break;

      case MIX_FIELD_SLOW_PREC:
        md2->speedPrec = editChoice(MIXES_2ND_COLUMN, y, STR_MIX_SLOW_PREC, &STR_VPREC[1], md2->speedPrec, 0, 1, attr, event);
        break;

      case MIX_FIELD_SLOW_UP:
        md2->speedUp = EDIT_DELAY(0, y, event, attr, STR_SLOWUP, md2->speedUp, (md2->speedPrec ? PREC2 : PREC1));
        break;

      case MIX_FIELD_SLOW_DOWN:
        md2->speedDown = EDIT_DELAY(0, y, event, attr, STR_SLOWDOWN, md2->speedDown, (md2->speedPrec ? PREC2 : PREC1));
        break;
    }
    y += FH;
  }
}
