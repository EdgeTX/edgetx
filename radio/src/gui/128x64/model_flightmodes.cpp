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

void displayFlightModes(coord_t x, coord_t y, FlightModesType value)
{
  uint8_t p = MAX_FLIGHT_MODES;
  do {
    --p;
    if (!(value & (1<<p)))
      lcdDrawChar(x, y, '0'+p);
    x -= FWNUM;
  } while (p!=0);
}

enum MenuModelFlightModeItems {
  ITEM_MODEL_FLIGHT_MODE_NAME,
  ITEM_MODEL_FLIGHT_MODE_SWITCH,
  ITEM_MODEL_FLIGHT_MODE_TRIMS,
  ITEM_MODEL_FLIGHT_MODE_TRIMS2,
  ITEM_MODEL_FLIGHT_MODE_FADE_IN,
  ITEM_MODEL_FLIGHT_MODE_FADE_OUT,
#if defined(GVARS)
  ITEM_MODEL_FLIGHT_MODE_GVARS_LABEL,
  ITEM_MODEL_FLIGHT_MODE_GV1,
  ITEM_MODEL_FLIGHT_MODE_GV2,
  ITEM_MODEL_FLIGHT_MODE_GV3,
  ITEM_MODEL_FLIGHT_MODE_GV4,
  ITEM_MODEL_FLIGHT_MODE_GV5,
  ITEM_MODEL_FLIGHT_MODE_GV6,
  ITEM_MODEL_FLIGHT_MODE_GV7,
  ITEM_MODEL_FLIGHT_MODE_GV8,
  ITEM_MODEL_FLIGHT_MODE_GV9,
#endif
  ITEM_MODEL_FLIGHT_MODE_MAX
};

bool isTrimModeAvailable(int mode)
{
  if (mode < 0 || mode == TRIM_MODE_3POS) return true;
  if (s_currIdx == 0) return mode == 0;
  return (mode%2) == 0 || (mode/2) != s_currIdx;
}

static void showTrims(uint8_t cnt, uint8_t first, coord_t y, LcdFlags attr, event_t event, FlightModeData *fm)
{
  for (uint8_t t = 0; t < cnt; t += 1) {
    drawTrimMode(MIXES_2ND_COLUMN + (t * 2 * FW), y, s_currIdx, t + first,
                  menuHorizontalPosition == t ? attr : 0);
    if (attr && menuHorizontalPosition == t)
      drawSource(LCD_W, 0, t + first + MIXSRC_FIRST_TRIM, RIGHT);
    if (s_editMode > 0 && attr && menuHorizontalPosition == t) {
      trim_t& v = fm->trim[t + first];
      v.mode = checkIncDec(event, v.mode == TRIM_MODE_NONE ? -1 : v.mode,
                            -1, 2 * MAX_FLIGHT_MODES,
                            EE_MODEL, isTrimModeAvailable);
    }
  }
}

void menuModelFlightModeOne(event_t event)
{
  FlightModeData * fm = flightModeAddress(s_currIdx);
  drawFlightMode(13*FW, 0, s_currIdx+1, (getFlightMode()==s_currIdx ? BOLD : 0));

  uint8_t old_editMode = s_editMode;

  uint8_t trim_count = keysGetMaxTrims();
  uint8_t trim_lines = (trim_count <= 6) ? 1 : 2;

  SUBMENU(STR_MENUFLIGHTMODE, ITEM_MODEL_FLIGHT_MODE_MAX, {
    0,
    (uint8_t)(s_currIdx == 0 ? HIDDEN_ROW : 0),
    (uint8_t)((trim_lines == 1) ? trim_count - 1 : 3),
    (uint8_t)((trim_lines == 2) ? trim_count - 5 : HIDDEN_ROW),
    0, 0,
#if defined(GVARS)
    READONLY_ROW,
    (uint8_t)(s_currIdx == 0 ? 1 : 2),  // Same value used for all GV rows
#endif
  });

  int8_t sub = menuVerticalPosition;
  int8_t editMode = s_editMode;

  coord_t y = MENU_HEADER_HEIGHT + 1;

  for (uint8_t k=0; k<min(NUM_BODY_LINES, (int)ITEM_MODEL_FLIGHT_MODE_MAX); k++) {
    int8_t i = k + menuVerticalOffset;
    for (int j=0; j<=i; ++j) {
      if (j<(int)DIM(mstate_tab) && mstate_tab[j] == HIDDEN_ROW) {
        ++i;
      }
    }
    uint8_t attr = (sub==i ? (editMode>0 ? BLINK|INVERS : INVERS) : 0);

    switch (i) {
      case ITEM_MODEL_FLIGHT_MODE_NAME:
        editSingleName(MIXES_2ND_COLUMN, y, STR_PHASENAME, fm->name,
                       sizeof(fm->name), event, attr, old_editMode);
        break;

      case ITEM_MODEL_FLIGHT_MODE_SWITCH:
        fm->swtch = editSwitch(MIXES_2ND_COLUMN, y, fm->swtch, attr, event);
        break;

      case ITEM_MODEL_FLIGHT_MODE_TRIMS:
        lcdDrawTextAlignedLeft(y, STR_TRIMS);
        showTrims(trim_lines == 1 ? trim_count : 4, 0, y, attr, event, fm);
        break;

      case ITEM_MODEL_FLIGHT_MODE_TRIMS2:
        if (trim_lines == 2) 
          showTrims(trim_count - 4, 4, y, attr, event, fm);
        break;

      case ITEM_MODEL_FLIGHT_MODE_FADE_IN:
        fm->fadeIn = editDelay(y, event, attr, STR_FADEIN, fm->fadeIn, PREC1);
        break;

      case ITEM_MODEL_FLIGHT_MODE_FADE_OUT:
        fm->fadeOut = editDelay(y, event, attr, STR_FADEOUT, fm->fadeOut, PREC1);
        break;

#if defined(GVARS)
      case ITEM_MODEL_FLIGHT_MODE_GVARS_LABEL:
        lcdDrawTextAlignedLeft(y, STR_GLOBAL_VARS);
        break;

      default:
      {
        uint8_t idx = i-ITEM_MODEL_FLIGHT_MODE_GV1;
        uint8_t posHorz = menuHorizontalPosition;

        if (attr && posHorz > 0 && s_currIdx==0) posHorz++;

        drawStringWithIndex(INDENT_WIDTH, y, STR_GV, idx+1, posHorz==0 ? attr : 0);
        lcdDrawSizedText(4*FW, y,g_model.gvars[idx].name, LEN_GVAR_NAME, 0);
        if (attr && editMode>0 && posHorz==0) {
          s_currIdxSubMenu = sub - ITEM_MODEL_FLIGHT_MODE_GV1;
          editMode = 0;
          pushMenu(menuModelGVarOne);
        }
        int16_t v = fm->gvars[idx];
        if (v > GVAR_MAX) {
          uint8_t p = v - GVAR_MAX - 1;
          if (p >= s_currIdx) p++;
          drawFlightMode(9*FW, y, p+1, posHorz==1 ? attr : 0);
        }
        else {
          lcdDrawText(9*FW, y, STR_OWN, posHorz==1 ? attr : 0);
        }
        if (attr && s_currIdx>0 && posHorz==1 && editMode > 0) {
          if (v < GVAR_MAX) v = GVAR_MAX;
          v = checkIncDec(event, v, GVAR_MAX, GVAR_MAX+MAX_FLIGHT_MODES-1, EE_MODEL);
          if (checkIncDec_Ret) {
            if (v == GVAR_MAX) v = 0;
            fm->gvars[idx] = v;
          }
        }
        editGVarValue(17*FW, y, event, idx, getGVarFlightMode(s_currIdx, idx), posHorz==2 ? attr : 0);
        break;
      }
#endif
    }
    y += FH;
  }
}

#define NAME_POS    11
#define SWITCH_POS  49
#define TRIMS_POS   74

void menuModelFlightModesAll(event_t event)
{
  SIMPLE_MENU(STR_MENUFLIGHTMODES, menuTabModel, MENU_MODEL_FLIGHT_MODES,
              HEADER_LINE+MAX_FLIGHT_MODES+1);

  int8_t sub = menuVerticalPosition - HEADER_LINE;

  // "Check trims" button
  if (sub == MAX_FLIGHT_MODES && event == EVT_KEY_BREAK(KEY_ENTER)) {
    s_editMode = 0;
    trimsCheckTimer = 200;  // 2 seconds
  }

  // Flight mode lines
  if (sub >= 0 && sub < MAX_FLIGHT_MODES &&
      (event == EVT_KEY_BREAK(KEY_ENTER) || event == EVT_KEY_FIRST(KEY_RIGHT))) {
    s_currIdx = sub;
    pushMenu(menuModelFlightModeOne);
  }

  uint8_t att;
  for (uint8_t i = 0; i < MAX_FLIGHT_MODES; i++) {
    int8_t y = 1 + (1 + i - menuVerticalOffset) * FH;
    if (y < 1 * FH + 1 || y > (LCD_LINES - 1) * FH + 1) continue;
    att = (i == sub ? INVERS : 0);
    FlightModeData* p = flightModeAddress(i);
    lcdDrawChar(0, y, ' ', att);
    lcdDrawChar(3, y, '1' + i, att | (getFlightMode() == i ? BOLD : 0));
    lcdDrawSizedText(NAME_POS, y, p->name, sizeof(p->name), 0);
    auto trims = min(keysGetMaxTrims(), (uint8_t)MAX_STICKS);
    if (i > 0) drawSwitch(SWITCH_POS, y, p->swtch, 0);
    for (uint8_t t = 0; t < trims; t++) {
      drawTrimMode(TRIMS_POS + t * FW * 2, y, i, t, 0);
    }

    if (p->fadeIn || p->fadeOut) {
      lcdDrawChar(LCD_W - FW + ((p->fadeIn && !p->fadeOut) ? 1 : 0), y,
                  (p->fadeIn && p->fadeOut) ? '*' : (p->fadeIn ? 'I' : 'O'));
    }
  }

  if (menuVerticalOffset != MAX_FLIGHT_MODES-(LCD_LINES-2)) return;

  lcdDrawText(CENTER_OFS, (LCD_LINES-1)*FH+1, STR_CHECKTRIMS);
  drawFlightMode(OFS_CHECKTRIMS, (LCD_LINES-1)*FH+1, mixerCurrentFlightMode+1);
  if (sub==MAX_FLIGHT_MODES && !trimsCheckTimer) {
    lcdInvertLastLine();
  }
}
