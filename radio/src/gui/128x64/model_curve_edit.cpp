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
#include "hal/rotary_encoder.h"

void runPopupCurvePreset(event_t event)
{
  warningResult = false;

  drawMessageBox(warningText);

  lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y+2*FH, STR_POPUPS_ENTER_EXIT);

  switch (event) {
    case EVT_KEY_BREAK(KEY_ENTER):
      warningResult = true;
      // no break

    case EVT_KEY_BREAK(KEY_EXIT):
      warningText = nullptr;
      warningType = WARNING_TYPE_ASTERISK;
      break;

    default:
      s_editMode = EDIT_MODIFY_FIELD;
      reusableBuffer.curveEdit.preset = checkIncDec(event, reusableBuffer.curveEdit.preset, -4, 4);
      s_editMode = EDIT_SELECT_FIELD;
      break;
  }

  lcdDrawNumber(WARNING_LINE_X+FW*7, WARNING_LINE_Y, 45 * reusableBuffer.curveEdit.preset / 4, LEFT|INVERS);
  lcdDrawChar(lcdLastRightPos, WARNING_LINE_Y, STR_CHAR_BW_DEGREE, INVERS);

  if (warningResult) {
    warningResult = 0;
    CurveHeader & crv = g_model.curves[s_currIdxSubMenu];
    int8_t * points = curveAddress(s_currIdxSubMenu);
    int k = 25 * reusableBuffer.curveEdit.preset;
    int dx = 2000 / (5+crv.points-1);
    for (uint8_t i=0; i<5+crv.points; i++) {
      int x = -1000 + i * dx;
      points[i] = divRoundClosest(divRoundClosest(k * x, 100), 10);
    }
    if (crv.type == CURVE_TYPE_CUSTOM) {
      resetCustomCurveX(points, 5+crv.points);
    }
  }
}

void onCurveOneMenu(const char * result)
{
  if (result == STR_CURVE_PRESET) {
    reusableBuffer.curveEdit.preset = 4; // 45°
    POPUP_INPUT(STR_PRESET, runPopupCurvePreset);
  }
  else if (result == STR_MIRROR) {
    curveMirror(s_currIdxSubMenu);
    storageDirty(EE_MODEL);
  }
  else if (result == STR_CLEAR) {
    curveClear(s_currIdxSubMenu);
    storageDirty(EE_MODEL);
  }
}

void menuModelCurveOne(event_t event)
{
  CurveHeader & crv = g_model.curves[s_currIdxSubMenu];
  int8_t * points = curveAddress(s_currIdxSubMenu);

  drawStringWithIndex(PSIZE(TR_MENUCURVES)*FW+FW, 0, STR_CV, s_currIdxSubMenu+1);

  uint8_t old_editMode = s_editMode;
  
  SIMPLE_SUBMENU(STR_MENUCURVES, 4 + 5+crv.points + (crv.type==CURVE_TYPE_CUSTOM ? 5+crv.points-2 : 0));

  // Curve name
  lcdDrawTextAlignedLeft(FH + 1, STR_NAME);
  editName(INDENT_WIDTH, 2 * FH + 1, crv.name, sizeof(crv.name), event,
           menuVerticalPosition == 0, 0, old_editMode);

  // Curve type
  lcdDrawTextAlignedLeft(3 * FH + 1, STR_TYPE);
  LcdFlags attr = (menuVerticalPosition == 1 ? (s_editMode > 0 ? INVERS | BLINK : INVERS) : 0);
  lcdDrawTextAtIndex(INDENT_WIDTH, 4 * FH + 1, STR_CURVE_TYPES, crv.type, attr);
  if (attr) {
    uint8_t newType = checkIncDecModelZero(event, crv.type, CURVE_TYPE_LAST);
    if (newType != crv.type) {
      for (int i = 1; i < 4 + crv.points; i++) {
        points[i] = calcRESXto100(applyCustomCurve(calc100toRESX(getCurveX(5 + crv.points, i)), s_currIdxSubMenu));
      }
      if (moveCurve(s_currIdxSubMenu, checkIncDec_Ret > 0 ? 3 + crv.points : -3 - crv.points)) {
        if (newType == CURVE_TYPE_CUSTOM) {
          resetCustomCurveX(points, 5 + crv.points);
        }
        crv.type = newType;
      }
    }
  }

  // Curve points
  attr = (menuVerticalPosition==2 ? (s_editMode>0 ? INVERS|BLINK : INVERS) : 0);
  lcdDrawTextAlignedLeft(5*FH+1, STR_COUNT);
  lcdDrawNumber(INDENT_WIDTH, 6*FH+1, 5+crv.points, LEFT|attr);
  lcdDrawText(lcdLastRightPos, 6*FH+1, STR_PTS, attr);
  if (attr) {
    rotaryEncoderResetAccel();
    int8_t count = checkIncDecModel(event, crv.points, -3, 12); // 2pts - 17pts
    if (checkIncDec_Ret) {
      int8_t newPoints[MAX_POINTS_PER_CURVE];
      newPoints[0] = points[0];
      newPoints[4 + count] = points[4 + crv.points];
      for (int i = 1; i < 4 + count; i++)
        newPoints[i] = calcRESXto100(applyCustomCurve(calc100toRESX(getCurveX(5 + count, i)), s_currIdxSubMenu));
      if (moveCurve(s_currIdxSubMenu, checkIncDec_Ret * (crv.type == CURVE_TYPE_CUSTOM ? 2 : 1))) {
        for (int i = 0; i < 5 + count; i++) {
          points[i] = newPoints[i];
          if (crv.type == CURVE_TYPE_CUSTOM && i != 0 && i != 4 + count)
            points[5 + count + i - 1] = getCurveX(5 + count, i);
        }
        crv.points = count;
      }
    }
  }

  // Curve smooth
  lcdDrawTextAlignedLeft(7*FH+1, STR_SMOOTH);
  drawCheckBox(7 * FW, 7 * FH + 1, crv.smooth, menuVerticalPosition == 3 ? INVERS : 0);
  if (menuVerticalPosition==3) crv.smooth = checkIncDecModel(event, crv.smooth, 0, 1);

  switch (event) {
    case EVT_ENTRY:
      break;

    case EVT_KEY_LONG(KEY_ENTER):
      if (menuVerticalPosition > 1) {
        POPUP_MENU_START(onCurveOneMenu, 3, STR_CURVE_PRESET, STR_MIRROR, STR_CLEAR);
      }
      break;

#if defined(NAVIGATION_XLITE)
    case EVT_KEY_FIRST(KEY_ENTER):
      if (!keysGetState(KEY_SHIFT))
        break;
#else
    case EVT_KEY_BREAK(KEY_MODEL):
    case EVT_KEY_BREAK(KEY_MENU):
#endif
      pushMenu(menuChannelsView);
      break;
  }

  drawCurve();
  if (s_currSrcRaw != MIXSRC_NONE)
    drawCursor(applyCurrentCurve);

  attr = (s_editMode > 0 ? INVERS|BLINK : INVERS);
  for (uint8_t i=0; i<5+crv.points; i++) {
    point_t point = getPoint(i);
    uint8_t selectionMode = 0;
    if (crv.type==CURVE_TYPE_CUSTOM) {
      if (menuVerticalPosition==4+2*i || (i==5+crv.points-1 && menuVerticalPosition==4+5+crv.points+5+crv.points-2-1))
        selectionMode = 2;
      else if (i>0 && menuVerticalPosition==3+2*i)
        selectionMode = 1;
    }
    else if (menuVerticalPosition == 4+i) {
      selectionMode = 2;
    }
    
    if (selectionMode > 0) {
      int8_t x = getCurveX(5+crv.points, i);
      if (crv.type==CURVE_TYPE_CUSTOM && i>0 && i<5+crv.points-1) {
        x = points[5+crv.points+i-1];
      }
  
      // Selection X / Y
      lcdDrawFilledRect(3, 2*FH+4, 7*FW-2, 4*FH-2, SOLID, ERASE);
      lcdDrawRect(3, 2*FH+4, 7*FW-2, 4*FH-2);
      drawStringWithIndex(7, 3*FH, STR_PT, i+1, LEFT);
      lcdDrawText(7, 4*FH, "x=");
      lcdDrawNumber(7+2*FW+1, 4*FH, x, LEFT|(selectionMode==1?attr:0));
      lcdDrawText(7, 5*FH, "y=");
      lcdDrawNumber(7+2*FW+1, 5*FH, points[i], LEFT|(selectionMode==2?attr:0));
      
      // Selection square
      lcdDrawFilledRect(point.x - 2, point.y - 2, 5, 5, SOLID, FORCE);
      lcdDrawFilledRect(point.x - 1, point.y - 1, 3, 3, SOLID);
      
      if (s_editMode > 0) {
        if (selectionMode == 1)
          CHECK_INCDEC_MODELVAR(event, points[5+crv.points+i-1], i==1 ? -100 : points[5+crv.points+i-2], i==5+crv.points-2 ? 100 : points[5+crv.points+i]);  // edit X
        else if (selectionMode == 2)
          CHECK_INCDEC_MODELVAR(event, points[i], -100, 100);
      }
    }
  }
}
