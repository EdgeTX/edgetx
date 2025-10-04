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

static int numFlightModes() { return modelFMEnabled() ? MAX_FLIGHT_MODES : 1; }

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
    drawGVarValue(x, y, gvar, *v, flags);
    vmin = GVAR_MIN + g_model.gvars[gvar].min;
    vmax = GVAR_MAX - g_model.gvars[gvar].max;
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

enum GVarFields {
  GVAR_FIELD_NAME,
  GVAR_FIELD_UNIT,
  GVAR_FIELD_PREC,
  GVAR_FIELD_MIN,
  GVAR_FIELD_MAX,
  GVAR_FIELD_POPUP,
  GVAR_FIELD_FM0,
  GVAR_FIELD_LAST=GVAR_FIELD_FM0+MAX_FLIGHT_MODES
};

#define GVAR_2ND_COLUMN                (12*FW)

static const char* const _str_units[] = { "-", "%" };

void menuModelGVarOne(event_t event)
{
  GVarData * gvar = &g_model.gvars[s_currIdx];

  drawStringWithIndex(strlen(STR_GVARS)*FW+FW, 0, STR_GV, s_currIdx+1, 0);
  drawGVarValue(32*FW, 0, s_currIdx, getGVarValue(s_currIdx, getFlightMode()));
  lcdDrawFilledRect(0, 0, LCD_W, FH, SOLID, FILL_WHITE|GREY_DEFAULT);

  uint8_t old_editMode = s_editMode;
  
  SIMPLE_SUBMENU(STR_GVARS, modelFMEnabled() ? GVAR_FIELD_LAST : GVAR_FIELD_FM0+1);

  for (int i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i * FH;
    int k = i + menuVerticalOffset;
    LcdFlags attr = (menuVerticalPosition == k ? (s_editMode > 0 ? BLINK | INVERS : INVERS) : 0);

    switch (k) {
      case GVAR_FIELD_NAME:
        editSingleName(GVAR_2ND_COLUMN, y, STR_NAME, gvar->name, LEN_GVAR_NAME, event, attr, old_editMode);
        break;

      case GVAR_FIELD_UNIT:
        gvar->unit = editChoice(GVAR_2ND_COLUMN, y, STR_UNIT, _str_units, gvar->unit, 0, 1, attr, event);
        break;

      case GVAR_FIELD_PREC:
        gvar->prec = editChoice(GVAR_2ND_COLUMN, y, STR_PRECISION, STR_VPREC, gvar->prec, 0, 1, attr, event);
        break;

      case GVAR_FIELD_MIN:
        lcdDrawText(0, y, STR_MIN);
        drawGVarValue(GVAR_2ND_COLUMN, y, s_currIdx, GVAR_MIN+gvar->min, LEFT|attr);
        if (attr) gvar->min = checkIncDec(event, GVAR_MIN+gvar->min, GVAR_MIN, GVAR_MAX-gvar->max, EE_MODEL) - GVAR_MIN;
        break;

      case GVAR_FIELD_MAX:
        lcdDrawText(0, y, STR_MAX);
        drawGVarValue(GVAR_2ND_COLUMN, y, s_currIdx, GVAR_MAX-gvar->max, LEFT|attr);
        if (attr) gvar->max = GVAR_MAX - checkIncDec(event, GVAR_MAX-gvar->max, GVAR_MIN+gvar->min, GVAR_MAX, EE_MODEL);
        break;

      case GVAR_FIELD_POPUP:
        gvar->popup = editCheckBox(gvar->popup, GVAR_2ND_COLUMN, y, STR_POPUP, attr, event);
        break;

      default:
        if (modelFMEnabled())
          drawStringWithIndex(0, y, STR_FM, k-GVAR_FIELD_FM0);
        else
          lcdDrawText(0, y, STR_VALUE);
        editGVarValue(GVAR_2ND_COLUMN, y, event, s_currIdx, k-GVAR_FIELD_FM0, LEFT|attr);
        break;
    }
  }
}

void onGVARSMenu(const char * result)
{
  int sub = menuVerticalPosition;

  if (result == STR_EDIT) {
    s_currIdx = sub;
    pushMenu(menuModelGVarOne);
  }
  else if (result == STR_CLEAR) {
    for (int i=0; i<MAX_FLIGHT_MODES; i++) {
      g_model.flightModeData[i].gvars[sub] = 0;
    }
    storageDirty(EE_MODEL);
  }
}

#define GVARS_FM_COLUMN(p)             (7*FW - 7 + (p)*20)

uint8_t colCount() {
  if (modelFMEnabled())
    return NAVIGATION_LINE_BY_LINE | (MAX_FLIGHT_MODES - 1);
  return 0;
}

void menuModelGVars(event_t event)
{
  tmr10ms_t tmr10ms = get_tmr10ms();
  const char * menuTitle;
  bool after2seconds = modelFMEnabled() && (tmr10ms - menuEntryTime > 200); /*2 seconds*/

  if (after2seconds) {
    menuTitle = STR_GVARS;
    for (int i=0; i<MAX_GVARS; i++) {
      drawStringWithIndex(GVARS_FM_COLUMN(i), 1, STR_FM, i, SMLSIZE|(getFlightMode()==i ? INVERS : 0));
    }
  }
  else {
    menuTitle = STR_MENU_GLOBAL_VARS;
  }

 MENU_FLAGS(menuTitle, menuTabModel, MENU_MODEL_GVARS, after2seconds ? CHECK_FLAG_NO_SCREEN_INDEX : 0, MAX_GVARS,
           { colCount(), colCount(), colCount(), colCount(), colCount(), colCount(), colCount(), colCount(), colCount() });

  int sub = menuVerticalPosition;

  for (int l=0; l<NUM_BODY_LINES; l++) {
    int i = l + menuVerticalOffset;
    coord_t y = MENU_HEADER_HEIGHT + 1 + l*FH;

    drawGVarName(0, y, i, (sub==i && menuHorizontalPosition<0) ? INVERS : 0);

    for (int j=0; j<numFlightModes(); j++) {
      FlightModeData * fm = &g_model.flightModeData[j];
      gvar_t v = fm->gvars[i];

      LcdFlags attr = ((sub == i && menuHorizontalPosition == j) ? (s_editMode > 0 ? BLINK | INVERS : INVERS) : 0);
      coord_t x = GVARS_FM_COLUMN(j);
      coord_t yval = y;
      if (v > GVAR_MAX) {
        attr |= SMLSIZE;
      }
      else if (g_model.gvars[i].prec > 0 || abs(v) >= 100) {
        attr |= TINSIZE | NO_UNIT;
        ++yval;
      }
      else {
        attr |= SMLSIZE | NO_UNIT;
      }
      editGVarValue(x, yval, event, i, j, attr);
    }
  }

  if ((menuHorizontalPosition<0 || !modelFMEnabled()) && event==EVT_KEY_LONG(KEY_ENTER)) {
    POPUP_MENU_START(onGVARSMenu, 2, STR_EDIT, STR_CLEAR);
  }
}
