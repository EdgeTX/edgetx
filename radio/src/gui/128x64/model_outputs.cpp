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

enum MenuModelOutputsItems {
  ITEM_OUTPUTS_OFFSET,
  ITEM_OUTPUTS_MIN,
  ITEM_OUTPUTS_MAX,
  ITEM_OUTPUTS_DIRECTION,
  ITEM_OUTPUTS_CURVE,
#if defined(PPM_LIMITS_SYMETRICAL)
  ITEM_OUTPUTS_SYMETRICAL,
#endif
  ITEM_OUTPUTS_COUNT,
  ITEM_OUTPUTS_MAXROW = ITEM_OUTPUTS_COUNT-1
};

#define LIMITS_OFFSET_POS         8*FW-1

#if defined(PPM_LIMITS_SYMETRICAL)
  #if defined(PPM_CENTER_ADJUSTABLE)
    #define LIMITS_MAX_POS        15*FW+3
    #define LIMITS_REVERT_POS     16*FW-1
    #define LIMITS_PPM_CENTER_POS 20*FW+1
  #else
    #define LIMITS_DIRECTION_POS  12*FW+4
    #define LIMITS_MAX_POS        16*FW+4
    #define LIMITS_REVERT_POS     17*FW
  #endif
#else
  #if defined(PPM_CENTER_ADJUSTABLE)
    #define LIMITS_MAX_POS        16*FW
    #define LIMITS_REVERT_POS     17*FW-2
    #define LIMITS_PPM_CENTER_POS 21*FW+2
  #else
    #define LIMITS_MAX_POS        17*FW
    #define LIMITS_REVERT_POS     18*FW
    #define LIMITS_DIRECTION_POS  12*FW+5
  #endif
#endif

#define LIMITS_CURVE_POS          17*FW+1
#define CONVERT_US_MIN_MAX(x) ((int16_t(x)*128)/25)
#define MIN_MAX_ATTR          attr

enum MenuModelOutputsOneItems {
  ITEM_OUTPUTONE_CH_NAME,
  ITEM_OUTPUTONE_OFFSET,
  ITEM_OUTPUTONE_MIN,
  ITEM_OUTPUTONE_MAX,
  ITEM_OUTPUTONE_DIR,
  ITEM_OUTPUTONE_CURVE,
#if defined(PPM_CENTER_ADJUSTABLE)
  ITEM_OUTPUTONE_PPM_CENTER,
#endif
#if defined(PPM_LIMITS_SYMETRICAL)
  ITEM_OUTPUTONE_SYMETRICAL,
#endif
  ITEM_OUTPUTONE_MAXROW
};

#define LIMITS_ONE_2ND_COLUMN (13*FW)

int32_t gvValDisplay(int32_t val)
{
  if (g_eeGeneral.ppmunit == PPM_US)
    val = CONVERT_US_MIN_MAX(val);
  return val;
}

int32_t gvValEdit(const char* title, int32_t val, int32_t offset, int min, int max, coord_t y, uint8_t attr, event_t event, bool active, LcdFlags flags)
{
  lcdDrawText(0, y, title, flags);
  if (GV_IS_GV_VALUE(val, -GV_RANGELARGE, GV_RANGELARGE) || (attr && event == EVT_KEY_LONG(KEY_ENTER))) {
    if (event == EVT_KEY_LONG(KEY_ENTER))
      killEvents(event);
    return GVAR_MENU_ITEM(LIMITS_ONE_2ND_COLUMN, y, val, -LIMIT_EXT_MAX, LIMIT_EXT_MAX, attr|PREC1|flags, 0, event);
  }
  lcdDrawNumber(LIMITS_ONE_2ND_COLUMN, y, gvValDisplay(val - offset), attr|PREC1|flags);
  if (active) {
    return offset + checkIncDec(event, val - offset, min, max, EE_MODEL, nullptr, stops1000);
  }
  return val;
}

void menuModelLimitsOne(event_t event)
{
  title(STR_MENULIMITS);
  LimitData * ld = limitAddress(s_currIdx);

  putsChn(11*FW, 0, s_currIdx+1, 0);
  lcdDrawNumber(19*FW, 0, PPM_CH_CENTER(s_currIdx)+channelOutputs[s_currIdx]/2, RIGHT);
  lcdDrawText(19*FW, 0, STR_US);

  int chanVal = calcRESXto100(ex_chans[s_currIdx]);

  uint8_t old_editMode = s_editMode;

  SUBMENU_NOTITLE(ITEM_OUTPUTONE_MAXROW, { 0, 0, 0, 0, 0, 0 , 0  /*, 0...*/ });

  int8_t sub = menuVerticalPosition;

  for (uint8_t k=0; k<LCD_LINES-1; k++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + k*FH;
    uint8_t i = k + menuVerticalOffset;
    uint8_t attr = (sub==i ? (s_editMode>0 ? BLINK|INVERS : INVERS) : 0);
    bool active = (attr && s_editMode > 0);
    int limit = (g_model.extendedLimits ? LIMIT_EXT_MAX : LIMIT_STD_MAX);

    switch (i) {
      case ITEM_OUTPUTONE_CH_NAME:
        editSingleName(LIMITS_ONE_2ND_COLUMN, y, STR_NAME, ld->name,
                       sizeof(ld->name), event, attr, old_editMode);
        break;

      case ITEM_OUTPUTONE_OFFSET:
        ld->offset = gvValEdit(TR_LIMITS_HEADERS_SUBTRIM, ld->offset, 0, -1000, 1000, y, attr, event, active, 0);
        break;

      case ITEM_OUTPUTONE_MIN:
        ld->min = gvValEdit(STR_MIN, ld->min, LIMITS_MIN_MAX_OFFSET, -limit, 0, y, attr, event, active, (chanVal < 0) ? BOLD : 0);
        break;

      case ITEM_OUTPUTONE_MAX:
        ld->max = gvValEdit(STR_MAX, ld->max, -LIMITS_MIN_MAX_OFFSET, 0, limit, y, attr, event, active, (chanVal > 0) ? BOLD : 0);
        break;

      case ITEM_OUTPUTONE_DIR:
      {
        lcdDrawTextAlignedLeft(y, STR_LIMITS_HEADERS_DIRECTION);
        lcdDrawTextAtIndex(LIMITS_ONE_2ND_COLUMN, y, STR_MMMINV, ld->revert, attr);
        if (active) {
          CHECK_INCDEC_MODELVAR_ZERO(event, ld->revert, 1);
        }
        break;
      }

      case ITEM_OUTPUTONE_CURVE:
        lcdDrawTextAlignedLeft(y, STR_CURVE);
        drawCurveName(LIMITS_ONE_2ND_COLUMN, y, ld->curve, attr);
        if (active) {
          CHECK_INCDEC_MODELVAR(event, ld->curve, -MAX_CURVES, +MAX_CURVES);
        }
        break;

#if defined(PPM_CENTER_ADJUSTABLE)
      case ITEM_OUTPUTONE_PPM_CENTER:
        lcdDrawTextAlignedLeft(y, TR_LIMITS_HEADERS_PPMCENTER);
        lcdDrawNumber(LIMITS_ONE_2ND_COLUMN, y, PPM_CENTER+ld->ppmCenter, attr);
        if (active) {
          CHECK_INCDEC_MODELVAR(event, ld->ppmCenter, -PPM_CENTER_MAX, +PPM_CENTER_MAX);
        }
        break;
#endif

#if defined(PPM_LIMITS_SYMETRICAL)
      case ITEM_OUTPUTONE_SYMETRICAL:
        lcdDrawTextAlignedLeft(y, TR_LIMITS_HEADERS_SUBTRIMMODE);
        lcdDrawSizedText(LIMITS_ONE_2ND_COLUMN, y,
                         ld->symetrical ? "=" : STR_CHAR_DELTA, 2, attr);
        if (active) {
          CHECK_INCDEC_MODELVAR_ZERO(event, ld->symetrical, 1);
        }
        break;
#endif
    }
  }
}

void onLimitsMenu(const char *result)
{
  s_currIdx = menuVerticalPosition - HEADER_LINE;
  if (result == STR_RESET) {
    LimitData *ld = limitAddress(s_currIdx);
    ld->min = 0;
    ld->max = 0;
    ld->offset = 0;
    ld->ppmCenter = 0;
    ld->revert = false;
    ld->curve = 0;
    storageDirty(EE_MODEL);
  }
  else if (result == STR_COPY_STICKS_TO_OFS) {
    copySticksToOffset(s_currIdx);
    storageDirty(EE_MODEL);
  }
  else if (result == STR_COPY_TRIMS_TO_OFS) {
    copyTrimsToOffset(s_currIdx);
    storageDirty(EE_MODEL);
  }
  else if (result == STR_EDIT) {
    pushMenu(menuModelLimitsOne);
  }
  else if (result == STR_COPY_MIN_MAX_TO_OUTPUTS) {
    copyMinMaxToOutputs(s_currIdx);
  }
}

void menuModelLimits(event_t event)
{
  uint8_t sub = menuVerticalPosition - HEADER_LINE;

  if (sub < MAX_OUTPUT_CHANNELS) {
    lcdDrawNumber(13*FW, 0, PPM_CH_CENTER(sub)+channelOutputs[sub]/2, RIGHT);
    lcdDrawText(13*FW, 0, STR_US);
  }

  SIMPLE_MENU(STR_MENULIMITS, menuTabModel, MENU_MODEL_OUTPUTS, HEADER_LINE+MAX_OUTPUT_CHANNELS+1);

  for (uint8_t i=0; i<LCD_LINES-1; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i+menuVerticalOffset;
    LcdFlags attr = (sub==MAX_OUTPUT_CHANNELS) ? INVERS : 0;

    if (sub == k &&
        (event == EVT_KEY_BREAK(KEY_ENTER)) &&
        (k != MAX_OUTPUT_CHANNELS)) {
      s_editMode = 0;
      POPUP_MENU_START(onLimitsMenu, 5, STR_EDIT, STR_RESET,
                       STR_COPY_TRIMS_TO_OFS, STR_COPY_STICKS_TO_OFS,
                       STR_COPY_MIN_MAX_TO_OUTPUTS);
    }

    if (k == MAX_OUTPUT_CHANNELS) {
      // last line available - add the "copy trim menu" line
      lcdDrawText(CENTER_OFS, y, STR_TRIMS2OFFSETS, NO_HIGHLIGHT() ? 0 : attr);
      if (attr) {
        s_editMode = 0;
        if (event == EVT_KEY_LONG(KEY_ENTER)) {
          killEvents(event);
          START_NO_HIGHLIGHT();
          moveTrimsToOffsets(); // if highlighted and menu pressed - move trims to offsets
        }
      }
      return;
    }

    LimitData * ld = limitAddress(k);

    if (ld->name[0] == 0) {
      putsChn(0, y, k+1, (sub==k) ? INVERS : 0);
    } else {
      lcdDrawSizedText(0, y, ld->name, sizeof(ld->name), ((sub==k) ? INVERS : 0) | LEFT);
    }

    int16_t v = (ld->revert) ? -LIMIT_OFS(ld) : LIMIT_OFS(ld);
    char swVal = ' ';  // ' ', '<', '>'
    if ((channelOutputs[k] - v) > 50) swVal = (ld->revert ? 127 : 126); // Switch to raw inputs?  - remove trim!
    if ((channelOutputs[k] - v) < -50) swVal = (ld->revert ? 126 : 127);
    lcdDrawChar(71, y, swVal);

    coord_t limitsMinPos = (g_eeGeneral.ppmunit == PPM_US) ? 12*FW+1 : 12*FW-2;
    int precThreshold = (g_eeGeneral.ppmunit == PPM_US) ? 804 : 0;

    for (uint8_t j=0; j<ITEM_OUTPUTS_COUNT; j++) {
      switch (j) {
        case ITEM_OUTPUTS_OFFSET:
#if defined(GVARS)
          if (GV_IS_GV_VALUE(ld->offset, -GV_RANGELARGE, GV_RANGELARGE)) {
            drawGVarName(LIMITS_OFFSET_POS, y, ld->offset, attr|PREC1|RIGHT);
            break;
          }
#endif
          lcdDrawNumber(LIMITS_OFFSET_POS, y, gvValDisplay(ld->offset), PREC1|RIGHT);
          break;

        case ITEM_OUTPUTS_MIN:
#if defined(GVARS)
          if (GV_IS_GV_VALUE(ld->min, -GV_RANGELARGE, GV_RANGELARGE)) {
            drawGVarName(limitsMinPos, y, ld->min, attr|PREC1|RIGHT);
            break;
          }
#endif
          if (ld->min <= precThreshold) {
            lcdDrawNumber(limitsMinPos, y, gvValDisplay(ld->min-LIMITS_MIN_MAX_OFFSET)/10, RIGHT);
          }
          else {
            lcdDrawNumber(limitsMinPos, y, gvValDisplay(ld->min-LIMITS_MIN_MAX_OFFSET), PREC1|RIGHT);
          }
          break;

        case ITEM_OUTPUTS_MAX:
#if defined(GVARS)
          if (GV_IS_GV_VALUE(ld->max, -GV_RANGELARGE, GV_RANGELARGE)) {
            drawGVarName(LIMITS_MAX_POS, y, ld->max, attr|PREC1|RIGHT);
            break;
          }
#endif
          if (ld->max >= -precThreshold) {
            lcdDrawNumber(LIMITS_MAX_POS, y, gvValDisplay(ld->max+LIMITS_MIN_MAX_OFFSET)/10, RIGHT);
          }
          else {
            lcdDrawNumber(LIMITS_MAX_POS, y, gvValDisplay(ld->max+LIMITS_MIN_MAX_OFFSET), PREC1|RIGHT);
          }
          break;

        case ITEM_OUTPUTS_DIRECTION:
          lcdDrawChar(LIMITS_REVERT_POS, y, ld->revert ? 127 : 126, 0);
          break;

        case ITEM_OUTPUTS_CURVE:
          drawCurveName(LIMITS_CURVE_POS, y, ld->curve, 0);
          break;

        case ITEM_OUTPUTS_SYMETRICAL:
          lcdDrawSizedText(LCD_W-FW, y, ld->symetrical ? "=" : STR_CHAR_DELTA, 2, 0);
          break;
      }
    }
  }
}
