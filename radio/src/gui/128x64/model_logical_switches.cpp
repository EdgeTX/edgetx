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
#include "switches.h"

enum LogicalSwitchFields {
  LS_FIELD_FUNCTION,
  LS_FIELD_V1,
  LS_FIELD_V2,
  LS_FIELD_ANDSW,
  LS_FIELD_DURATION,
  LS_FIELD_DELAY,
  LS_FIELD_PERSIST,
  LS_FIELD_COUNT,
  LS_FIELD_LAST = LS_FIELD_COUNT-1
};

#define CSW_1ST_COLUMN  ((3*FW)-1)
#define CSW_2ND_COLUMN  ((7*FW)-1)
#define CSW_3RD_COLUMN  ((12*FW))
#define CSW_4TH_COLUMN  ((21*FW)+3)

void putsEdgeDelayParam(coord_t x, coord_t y, LogicalSwitchData *cs, uint8_t lattr, uint8_t rattr)
{
  lcdDrawChar(x-4, y, '[');
  lcdDrawNumber(x, y, lswTimerValue(cs->v2), LEFT|PREC1|lattr);
  lcdDrawChar(lcdLastRightPos, y, ':');
  if (cs->v3 < 0)
    lcdDrawText(lcdLastRightPos+3, y, "<<", rattr);
  else if (cs->v3 == 0)
    lcdDrawText(lcdLastRightPos+3, y, "--", rattr);
  else
    lcdDrawNumber(lcdLastRightPos+3, y, lswTimerValue(cs->v2+cs->v3), LEFT|PREC1|rattr);
  lcdDrawChar(lcdLastRightPos, y, ']');
}

#define CSWONE_2ND_COLUMN (11*FW)

void menuModelLogicalSwitchOne(event_t event)
{
  title(STR_MENULOGICALSWITCH);

  LogicalSwitchData * cs = lswAddress(s_currIdx);

  uint8_t sw = SWSRC_FIRST_LOGICAL_SWITCH+s_currIdx;
  uint8_t cstate = lswFamily(cs->func);

  drawSwitch(14*FW, 0, sw, (getSwitch(sw) ? BOLD : 0));

  SUBMENU_NOTITLE(LS_FIELD_COUNT, {
          0,
          0,
          uint8_t(cstate == LS_FAMILY_EDGE ? 1 : 0),
          0,
          0,
          uint8_t(cstate == LS_FAMILY_EDGE ? HIDDEN_ROW : 0),
          uint8_t(cstate != LS_FAMILY_STICKY ? HIDDEN_ROW : 0),
        });

  int8_t sub = menuVerticalPosition;

  coord_t y = MENU_HEADER_HEIGHT + 1;

  for (uint8_t k=0; k<LCD_LINES-1; k++) {
    uint8_t i = k + menuVerticalOffset;
    for (int j=0; j<=i; ++j) {
      if (j<(int)DIM(mstate_tab) && mstate_tab[j] == HIDDEN_ROW) {
        ++i;
      }
    }
    uint8_t attr = (sub==i ? (s_editMode>0 ? BLINK|INVERS : INVERS) : 0);

    switch (i) {
      case LS_FIELD_FUNCTION:
        lcdDrawTextAlignedLeft(y, STR_FUNC);
        lcdDrawTextAtIndex(CSWONE_2ND_COLUMN, y, STR_VCSWFUNC, cs->func, attr);
        if (attr) {
          cs->func = checkIncDec(event, cs->func, 0, LS_FUNC_MAX, EE_MODEL);
          uint8_t new_cstate = lswFamily(cs->func);
          if (cstate != new_cstate) {
            cstate = new_cstate;
            if (cstate == LS_FAMILY_TIMER) {
              cs->v1 = cs->v2 = -119;
            }
            else if (cstate == LS_FAMILY_EDGE) {
              cs->v1 = 0; cs->v2 = -129; cs->v3 = 0;
            }
            else {
              cs->v1 = cs->v2 = 0;
            }
          }
        }
        break;

      case LS_FIELD_V1:
      {
        INCDEC_DECLARE_VARS(EE_MODEL);
        lcdDrawTextAlignedLeft(y, STR_V1);
        int v1_min=0, v1_max=MIXSRC_LAST_TELEM;
        if (cstate == LS_FAMILY_BOOL || cstate == LS_FAMILY_STICKY || cstate == LS_FAMILY_EDGE) {
          drawSwitch(CSWONE_2ND_COLUMN, y, cs->v1, attr);
          v1_min = SWSRC_FIRST_IN_LOGICAL_SWITCHES; v1_max = SWSRC_LAST_IN_LOGICAL_SWITCHES;
          INCDEC_SET_FLAG(EE_MODEL | INCDEC_SWITCH);
          INCDEC_ENABLE_CHECK(isSwitchAvailableInLogicalSwitches);
        }
        else if (cstate == LS_FAMILY_TIMER) {
          lcdDrawNumber(CSWONE_2ND_COLUMN, y, lswTimerValue(cs->v1), LEFT|PREC1|attr);
          v1_min = -128;
          v1_max = 122;
        }
        else {
          drawSource(CSWONE_2ND_COLUMN, y, cs->v1, attr);
          INCDEC_SET_FLAG(EE_MODEL | INCDEC_SOURCE | INCDEC_SOURCE_INVERT);
          INCDEC_ENABLE_CHECK(isSourceAvailable);
        }
        if (attr) {
          cs->v1 = CHECK_INCDEC_PARAM(event, cs->v1, v1_min, v1_max);
        }
        break;
      }

      case LS_FIELD_V2:
      {
        INCDEC_DECLARE_VARS(EE_MODEL);
        lcdDrawTextAlignedLeft(y, STR_V2);
        int16_t v2_min = 0, v2_max = MIXSRC_LAST_TELEM;
        if (cstate == LS_FAMILY_BOOL || cstate == LS_FAMILY_STICKY) {
          drawSwitch(CSWONE_2ND_COLUMN, y, cs->v2, attr);
          v2_min = SWSRC_FIRST_IN_LOGICAL_SWITCHES; v2_max = SWSRC_LAST_IN_LOGICAL_SWITCHES;
          INCDEC_SET_FLAG(EE_MODEL | INCDEC_SWITCH);
          INCDEC_ENABLE_CHECK(isSwitchAvailableInLogicalSwitches);
        }
        else if (cstate == LS_FAMILY_TIMER) {
          lcdDrawNumber(CSWONE_2ND_COLUMN, y, lswTimerValue(cs->v2), LEFT|PREC1|attr);
          v2_min = -128;
          v2_max = 122;
        }
        else if (cstate == LS_FAMILY_EDGE) {
          putsEdgeDelayParam(CSWONE_2ND_COLUMN+5, y, cs, menuHorizontalPosition==0 ? attr : 0, menuHorizontalPosition==1 ? attr : 0);
          if (attr && menuHorizontalPosition==1) {
            CHECK_INCDEC_MODELVAR(event, cs->v3, -1, 222 - cs->v2);
            break;
          }
          v2_min = -129; v2_max = 122;
        }
        else if (cstate == LS_FAMILY_COMP) {
          drawSource(CSWONE_2ND_COLUMN, y, cs->v2, attr);
          INCDEC_SET_FLAG(EE_MODEL | INCDEC_SOURCE | INCDEC_SOURCE_INVERT);
          INCDEC_ENABLE_CHECK(isSourceAvailable);
        }
        else {
          if (abs(cs->v1) >= MIXSRC_FIRST_TELEM) {
            drawSourceCustomValue(CSWONE_2ND_COLUMN, y, cs->v1, convertLswTelemValue(cs), attr|LEFT);
            v2_max = maxTelemValue(cs->v1 - MIXSRC_FIRST_TELEM + 1);
            if ((cs->func == LS_FUNC_APOS) || (cs->func == LS_FUNC_ANEG) || (cs->func == LS_FUNC_ADIFFEGREATER))
              v2_min = 0;
            else
              v2_min = -v2_max;
            INCDEC_SET_FLAG(EE_MODEL | INCDEC_REP10 | NO_INCDEC_MARKS);
            if (cs->v2 < v2_min || cs->v2 > v2_max) {
              cs->v2 = 0;
              storageDirty(EE_MODEL);
            }
          }
          else
          {
            LcdFlags lf = attr | LEFT;
            getMixSrcRange(cs->v1, v2_min, v2_max, &lf);
            if ((cs->func == LS_FUNC_APOS) || (cs->func == LS_FUNC_ANEG) || (cs->func == LS_FUNC_ADIFFEGREATER))
              v2_min = 0;
            drawSourceCustomValue(CSWONE_2ND_COLUMN, y, cs->v1, (abs(cs->v1) <= MIXSRC_LAST_CH ? calc100toRESX(cs->v2) : cs->v2), lf);
          }
        }

        if (attr) {
          cs->v2 = CHECK_INCDEC_PARAM(event, cs->v2, v2_min, v2_max);
        }
        break;
      }

      case LS_FIELD_ANDSW:
        lcdDrawTextAlignedLeft(y, STR_AND_SWITCH);
        drawSwitch(CSWONE_2ND_COLUMN, y, cs->andsw, attr);
        if (attr) {
          CHECK_INCDEC_MODELSWITCH(event, cs->andsw, -MAX_LS_ANDSW, MAX_LS_ANDSW, isSwitchAvailableInLogicalSwitches);
        }
        break;

      case LS_FIELD_DURATION:
        lcdDrawTextAlignedLeft(y, STR_DURATION);
        if (cs->duration > 0)
          lcdDrawNumber(CSWONE_2ND_COLUMN, y, cs->duration, attr|PREC1|LEFT);
        else
          lcdDrawMMM(CSWONE_2ND_COLUMN, y, attr);
        if (attr) {
          CHECK_INCDEC_MODELVAR_ZERO(event, cs->duration, MAX_LS_DURATION);
        }
        break;

      case LS_FIELD_DELAY:
        lcdDrawTextAlignedLeft(y, STR_DELAY);
        if (cs->delay > 0)
          lcdDrawNumber(CSWONE_2ND_COLUMN, y, cs->delay, attr|PREC1|LEFT);
        else
          lcdDrawMMM(CSWONE_2ND_COLUMN, y, attr);
        if (attr) {
          CHECK_INCDEC_MODELVAR_ZERO(event, cs->delay, MAX_LS_DELAY);
        }
        break;

      case LS_FIELD_PERSIST:
        lcdDrawTextAlignedLeft(y, STR_PERSISTENT);
        drawCheckBox(CSWONE_2ND_COLUMN, y, cs->lsPersist, attr);
        if (attr) cs->lsPersist = checkIncDecModel(event, cs->lsPersist, 0, 1);
        break;
    }

    y += FH;
  }
}

void onLogicalSwitchesMenu(const char *result)
{
  int8_t sub = menuVerticalPosition - HEADER_LINE;
  LogicalSwitchData * cs = lswAddress(sub);

  if (result == STR_EDIT) {
    s_currIdx = sub;
    pushMenu(menuModelLogicalSwitchOne);
  }
  else if (result == STR_COPY) {
    clipboard.type = CLIPBOARD_TYPE_CUSTOM_SWITCH;
    clipboard.data.csw = *cs;
  }
  else if (result == STR_PASTE) {
    *cs = clipboard.data.csw;
    storageDirty(EE_MODEL);
  }
  else if (result == STR_CLEAR) {
    memset(cs, 0, sizeof(LogicalSwitchData));
    storageDirty(EE_MODEL);
  }
}

void menuModelLogicalSwitches(event_t event)
{
  SIMPLE_MENU(STR_MENULOGICALSWITCHES, menuTabModel, MENU_MODEL_LOGICAL_SWITCHES, HEADER_LINE+MAX_LOGICAL_SWITCHES);

  coord_t y = 0;
  uint8_t k = 0;
  int8_t sub = menuVerticalPosition - HEADER_LINE;

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    LogicalSwitchData * cs = lswAddress(sub);
    if (cs->func)
      s_currIdx = sub;
    if (sub >= 0)
      POPUP_MENU_ADD_ITEM(STR_EDIT);
    if (cs->func || cs->v1 || cs->v2 || cs->delay || cs->duration || cs->andsw)
      POPUP_MENU_ADD_ITEM(STR_COPY);
    if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_SWITCH)
      POPUP_MENU_ADD_ITEM(STR_PASTE);
    if (cs->func || cs->v1 || cs->v2 || cs->delay || cs->duration || cs->andsw)
      POPUP_MENU_ADD_ITEM(STR_CLEAR);
    if (popupMenuItemsCount == 1) {
      popupMenuItemsCount = 0;
      s_currIdx = sub;
      pushMenu(menuModelLogicalSwitchOne);
    }
    else {
      s_editMode = 0; // Was set in 'check' function.
      POPUP_MENU_START(onLogicalSwitchesMenu);
    }
  }

  for (uint8_t i=0; i<LCD_LINES-1; i++) {
    y = 1 + (i+1)*FH;
    k = i + menuVerticalOffset;
    LogicalSwitchData * cs = lswAddress(k);

    // CSW name
    uint8_t sw = SWSRC_FIRST_LOGICAL_SWITCH+k;

    drawSwitch(0, y, sw, (sub==k ? INVERS : 0) | (getSwitch(sw) ? BOLD : 0));

    if (cs->func > 0) {
      // CSW params
      uint8_t cstate = lswFamily(cs->func);

      // CSW func
      LcdFlags flags = 0;
      if (cstate == LS_FAMILY_STICKY && getLSStickyState(k))
        flags = BOLD;
      lcdDrawTextAtIndex(CSW_1ST_COLUMN, y, STR_VCSWFUNC, cs->func, flags);

      if (cstate == LS_FAMILY_BOOL || cstate == LS_FAMILY_STICKY) {
        drawSwitch(CSW_2ND_COLUMN, y, cs->v1, 0);
        drawSwitch(CSW_3RD_COLUMN, y, cs->v2, 0);
      }
      else if (cstate == LS_FAMILY_COMP) {
        drawSource(CSW_2ND_COLUMN, y, cs->v1, 0);
        drawSource(CSW_3RD_COLUMN, y, cs->v2, 0);
      }
      else if (cstate == LS_FAMILY_EDGE) {
        drawSwitch(CSW_2ND_COLUMN, y, cs->v1, 0);
        putsEdgeDelayParam(CSW_3RD_COLUMN, y, cs, 0, 0);
      }
      else if (cstate == LS_FAMILY_TIMER) {
        lcdDrawNumber(CSW_2ND_COLUMN, y, lswTimerValue(cs->v1), LEFT|PREC1);
        lcdDrawNumber(CSW_3RD_COLUMN, y, lswTimerValue(cs->v2), LEFT|PREC1);
      }
      else {
        mixsrc_t v1 = cs->v1;
        drawSource(CSW_2ND_COLUMN, y, v1, 0);
        if (v1 >= MIXSRC_FIRST_TELEM) {
          drawSourceCustomValue(CSW_3RD_COLUMN, y, v1, convertLswTelemValue(cs), LEFT);
        }
        else if (v1 <= MIXSRC_LAST_CH) {
          drawSourceCustomValue(CSW_3RD_COLUMN, y, v1, calc100toRESX(cs->v2), LEFT);
        }
        else {
          drawSourceCustomValue(CSW_3RD_COLUMN, y, v1, cs->v2, LEFT | (v1 != MIXSRC_TX_TIME ? TIMEHOUR : 0));
        }
      }

      // CSW and switch
      drawSwitch(CSW_4TH_COLUMN, y, cs->andsw, RIGHT);
    }
  }
}

