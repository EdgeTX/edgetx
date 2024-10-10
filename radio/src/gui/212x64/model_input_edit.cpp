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

#define EXPO_ONE_2ND_COLUMN (LCD_W-8*FW-90)

int expoFn(int x)
{
  ExpoData * ed = expoAddress(s_currIdx);
  int16_t anas[MAX_INPUTS] = {0};
  applyExpos(anas, e_perout_mode_inactive_flight_mode, ed->srcRaw, x);
  return anas[ed->chn];
}

enum ExposFields {
  EXPO_FIELD_INPUT_NAME,
  EXPO_FIELD_LINE_NAME,
  EXPO_FIELD_SOURCE,
  EXPO_FIELD_SCALE,
  EXPO_FIELD_WEIGHT,
  EXPO_FIELD_OFFSET,
  EXPO_FIELD_CURVE,
  CASE_FLIGHT_MODES(EXPO_FIELD_FLIGHT_MODES)
  EXPO_FIELD_SWITCH,
  EXPO_FIELD_SIDE,
  EXPO_FIELD_TRIM,
  EXPO_FIELD_MAX
};

#define CURVE_ROWS                     1

uint8_t FM_ROW(uint8_t value)
{
  if (modelFMEnabled())
    return value;
  return HIDDEN_ROW;
}

void menuModelExpoOne(event_t event)
{
  if (event == EVT_KEY_LONG(KEY_MENU)) {
    pushMenu(menuChannelsView);
  }

  ExpoData * ed = expoAddress(s_currIdx);
  drawSource(PSIZE(TR_MENUINPUTS)*FW+FW, 0, MIXSRC_FIRST_INPUT+ed->chn, 0);
  lcdDrawFilledRect(0, 0, LCD_W, FH, SOLID, FILL_WHITE|GREY_DEFAULT);

  uint8_t old_editMode = s_editMode;
  
  SUBMENU(STR_MENUINPUTS, EXPO_FIELD_MAX,
          {0, 0, 0, ed->srcRaw >= MIXSRC_FIRST_TELEM ? (uint8_t)0 : (uint8_t)HIDDEN_ROW, 0, 0, CURVE_ROWS,
           CASE_FLIGHT_MODES(FM_ROW((MAX_FLIGHT_MODES-1) | NAVIGATION_LINE_BY_LINE)) 0 /*, ...*/});

  SET_SCROLLBAR_X(EXPO_ONE_2ND_COLUMN+10*FW);

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
        editSingleName(EXPO_ONE_2ND_COLUMN, y, STR_INPUTNAME,
                       g_model.inputNames[ed->chn], LEN_INPUT_NAME, event, attr,
                       old_editMode);
        break;

      case EXPO_FIELD_LINE_NAME:
        editSingleName(EXPO_ONE_2ND_COLUMN, y, STR_EXPONAME, ed->name,
                       LEN_EXPOMIX_NAME, event, attr, old_editMode);
        break;

      case EXPO_FIELD_SOURCE:
        lcdDrawTextAlignedLeft(y, STR_SOURCE);
        drawSource(EXPO_ONE_2ND_COLUMN, y, ed->srcRaw, STREXPANDED|attr);
        if (attr && menuHorizontalPosition==0)
          ed->srcRaw = checkIncDec(event, ed->srcRaw, INPUTSRC_FIRST, INPUTSRC_LAST, EE_MODEL|INCDEC_SOURCE|INCDEC_SOURCE_INVERT|NO_INCDEC_MARKS, isSourceAvailableInInputs);
        if (abs(ed->srcRaw) >= MIXSRC_FIRST_TELEM) {
          drawSensorCustomValue(EXPO_ONE_2ND_COLUMN+30, y, (abs(ed->srcRaw) - MIXSRC_FIRST_TELEM)/3, getValue(ed->srcRaw), LEFT|(menuHorizontalPosition==1?attr:0));
          if (attr && menuHorizontalPosition == 1) ed->scale = checkIncDec(event, ed->scale, 0, maxTelemValue(abs(ed->srcRaw) - MIXSRC_FIRST_TELEM + 1), EE_MODEL);
        }
        else if (attr) {
          menuHorizontalPosition = 0;
        }
        break;

      case EXPO_FIELD_SCALE:
        lcdDrawTextAlignedLeft(y, STR_SCALE);
        drawSensorCustomValue(EXPO_ONE_2ND_COLUMN, y, (abs(ed->srcRaw) - MIXSRC_FIRST_TELEM)/3, convertTelemValue(abs(ed->srcRaw) - MIXSRC_FIRST_TELEM + 1, ed->scale), LEFT|attr);
        if (attr) ed->scale = checkIncDec(event, ed->scale, 0, maxTelemValue(abs(ed->srcRaw) - MIXSRC_FIRST_TELEM + 1), EE_MODEL);
        break;

      case EXPO_FIELD_WEIGHT:
        ed->weight = editSrcVarFieldValue(EXPO_ONE_2ND_COLUMN, y, STR_WEIGHT, ed->weight,
                        -100, 100, attr, event, isSourceAvailable, MIXSRC_FIRST, INPUTSRC_LAST);
        break;

      case EXPO_FIELD_OFFSET:
        ed->offset = editSrcVarFieldValue(EXPO_ONE_2ND_COLUMN, y, STR_OFFSET, ed->offset,
                        -100, 100, attr, event, isSourceAvailable, MIXSRC_FIRST, INPUTSRC_LAST);
        break;

      case EXPO_FIELD_CURVE:
        lcdDrawTextAlignedLeft(y, STR_CURVE);
        editCurveRef(EXPO_ONE_2ND_COLUMN, y, ed->curve, event, attr, isSourceAvailable, MIXSRC_FIRST, INPUTSRC_LAST);
        break;

#if defined(FLIGHT_MODES)
      case EXPO_FIELD_FLIGHT_MODES:
        lcdDrawTextAlignedLeft(y, STR_FLMODE);
        ed->flightModes = editFlightModes(EXPO_ONE_2ND_COLUMN, y, event, ed->flightModes, attr);
        break;
#endif

      case EXPO_FIELD_SWITCH:
        ed->swtch = editSwitch(EXPO_ONE_2ND_COLUMN, y, ed->swtch, attr, event);
        break;

      case EXPO_FIELD_SIDE:
        ed->mode = 4 - editChoice(EXPO_ONE_2ND_COLUMN, y, STR_SIDE, STR_VCURVEFUNC, 4-ed->mode, 1, 3, attr, event);
        break;

      case EXPO_FIELD_TRIM:
        lcdDrawTextAlignedLeft(y, STR_TRIM);
        {
          const char* trim_str = getTrimSourceLabel(abs(ed->srcRaw), ed->trimSource);
          LcdFlags flags = RIGHT | (menuHorizontalPosition==0 ? attr : 0);
          lcdDrawText(EXPO_ONE_2ND_COLUMN, y, trim_str, flags);

          if (attr) {
            int8_t min = TRIM_ON;
            if (abs(ed->srcRaw) >= MIXSRC_FIRST_STICK && abs(ed->srcRaw) <= MIXSRC_LAST_STICK) {
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
  s_currSrcRaw = ed->srcRaw;
  s_currScale = ed->scale;
  drawCursor(expoFn);
}
