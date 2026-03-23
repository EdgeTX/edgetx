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
  LS_FIELD_V3,
  LS_FIELD_ANDSW,
  LS_FIELD_DURATION,
  LS_FIELD_DELAY,
  LS_FIELD_PERSIST,
  LS_FIELD_COUNT,
  LS_FIELD_LAST = LS_FIELD_COUNT-1
};

#define CSW_1ST_COLUMN  (4*FW-3)
#define CSW_2ND_COLUMN  (8*FW+2+FW/2)
#define CSW_3RD_COLUMN  (14*FW+1+FW/2)
#define CSW_4TH_COLUMN  (21*FW)
#define CSW_5TH_COLUMN  (25*FW+3)
#define CSW_6TH_COLUMN  (29*FW+3)
#define CSW_7TH_COLUMN  (33*FW+3)

void putsEdgeDelayParam(coord_t x, coord_t y, LogicalSwitchData * cs, uint8_t lattr, uint8_t rattr)
{
  lcdDrawChar(x-4, y, '[');
  lcdDrawNumber(x, y, lswTimerValue(cs->v2.value), LEFT|PREC1|lattr);
  lcdDrawChar(lcdLastRightPos, y, ':');
  if (cs->v3 < 0)
    lcdDrawText(lcdLastRightPos+3, y, "<<", rattr);
  else if (cs->v3 == 0)
    lcdDrawText(lcdLastRightPos+3, y, "--", rattr);
  else
    lcdDrawNumber(lcdLastRightPos+3, y, lswTimerValue(cs->v2.value+cs->v3), LEFT|PREC1|rattr);
  lcdDrawChar(lcdLastRightPos, y, ']');
}

void onLogicalSwitchesMenu(const char *result)
{
  int8_t sub = menuVerticalPosition;
  LogicalSwitchData * cs = lswAddress(sub);

  if (result == STR_COPY) {
    clipboard.type = CLIPBOARD_TYPE_CUSTOM_SWITCH;
    clipboard.data.csw = *cs;
  }
  else if (result == STR_PASTE) {
    cs = lswAllocAt(sub);
    if (!cs) return;
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
  MENU(STR_MENULOGICALSWITCHES, menuTabModel, MENU_MODEL_LOGICAL_SWITCHES, MAX_LOGICAL_SWITCHES, { NAVIGATION_LINE_BY_LINE|LS_FIELD_LAST/*repeated...*/ });

  int k = 0;
  int sub = menuVerticalPosition;
  horzpos_t horz = menuHorizontalPosition;
  bool showHeader = true;

  if (horz<0 && event==EVT_KEY_LONG(KEY_ENTER)) {
    LogicalSwitchData * cs = lswAddress(sub);
    if (cs->func)
      POPUP_MENU_ADD_ITEM(STR_COPY);
    if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_SWITCH)
      POPUP_MENU_ADD_ITEM(STR_PASTE);
    if (cs->func || !cs->v1.isZero() || !cs->v2.isZero() || cs->delay || cs->duration || !cs->andsw.isNone())
      POPUP_MENU_ADD_ITEM(STR_CLEAR);
    POPUP_MENU_START(onLogicalSwitchesMenu);
  }

  for (uint8_t i=0; i<NUM_BODY_LINES; ++i) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    k = i+menuVerticalOffset;
    LcdFlags attr = (sub==k ? ((s_editMode>0) ? BLINK|INVERS : INVERS)  : 0);
    LcdFlags attr1 = (horz==1 ? attr : 0);
    LcdFlags attr2 = (horz==2 ? attr : 0);
    LogicalSwitchData * cs = lswAddress(k);

    // CSW name
    SwitchRef sw = SwitchRef_(SWITCH_TYPE_LOGICAL, (uint16_t)k);
    drawSwitch(0, y, sw, (getSwitch(sw) ? BOLD : 0) | ((sub==k && CURSOR_ON_LINE()) ? INVERS : 0));

    // CSW params
    unsigned int cstate = lswFamily(cs->func);
    int16_t v1_min = 0, v1_max = MIXSRC_LAST_TELEM;
    int16_t v2_min = 0, v2_max = MIXSRC_LAST_TELEM;
    int16_t v3_min =-1, v3_max = 100;

    // CSW func
    LcdFlags flags = 0;
    if (cstate == LS_FAMILY_STICKY && getLSStickyState(k))
      flags = BOLD;
    lcdDrawTextAtIndex(CSW_1ST_COLUMN, y, STR_VCSWFUNC, cs->func, (horz==0 ? attr : 0) | flags);

    if (cstate == LS_FAMILY_BOOL || cstate == LS_FAMILY_STICKY) {
      drawSwitch(CSW_2ND_COLUMN, y, cs->v1.swtch, attr1);
      drawSwitch(CSW_3RD_COLUMN, y, cs->v2.swtch, attr2);
    }
    else if (cstate == LS_FAMILY_EDGE) {
      drawSwitch(CSW_2ND_COLUMN, y, cs->v1.swtch, attr1);
      putsEdgeDelayParam(CSW_3RD_COLUMN, y, cs, attr2, horz==LS_FIELD_V3 ? attr : 0);
      v2_min=-129; v2_max = 122;
      v3_max = 222 - cs->v2.value;
    }
    else if (cstate == LS_FAMILY_COMP) {
      drawSource(CSW_2ND_COLUMN, y, cs->v1.source, attr1);
      drawSource(CSW_3RD_COLUMN, y, cs->v2.source, attr2);
    }
    else if (cstate == LS_FAMILY_TIMER) {
      lcdDrawNumber(CSW_2ND_COLUMN, y, lswTimerValue(cs->v1.value), LEFT|PREC1|attr1);
      lcdDrawNumber(CSW_3RD_COLUMN, y, lswTimerValue(cs->v2.value), LEFT|PREC1|attr2);
      v1_min = v2_min = -128;
      v1_max = v2_max = 122;
    }
    else {
      drawSource(CSW_2ND_COLUMN, y, cs->v1.source, attr1);
      LcdFlags lf = attr2 | LEFT;
      if (validateLSV2Range(cs, v2_min, v2_max, &lf)) storageDirty(EE_MODEL);
      int32_t dispVal = (cs->v1.source.type <= SOURCE_TYPE_CHANNEL) ? calc100toRESX(cs->v2.value) : cs->v2.value;
      drawSourceCustomValue(CSW_3RD_COLUMN, y, cs->v1.source, dispVal, lf);
    }

    // CSW AND switch
    drawSwitch(CSW_4TH_COLUMN, y, cs->andsw, horz==LS_FIELD_ANDSW ? attr : 0);

    // CSW duration
    if (cs->duration > 0)
      lcdDrawNumber(CSW_5TH_COLUMN, y, cs->duration, (horz==LS_FIELD_DURATION ? attr : 0)|PREC1|LEFT);
    else
      lcdDrawMMM(CSW_5TH_COLUMN, y, horz==LS_FIELD_DURATION ? attr : 0);

    // CSW delay
    if (cstate == LS_FAMILY_EDGE) {
      lcdDrawText(CSW_6TH_COLUMN, y, STR_NA);
      if (attr && horz == LS_FIELD_DELAY) {
        repeatLastCursorMove(event);
      }
    }
    else if (cs->delay > 0) {
      lcdDrawNumber(CSW_6TH_COLUMN, y, cs->delay, (horz==LS_FIELD_DELAY ? attr : 0)|PREC1|LEFT);
    }
    else {
      lcdDrawMMM(CSW_6TH_COLUMN, y, horz==LS_FIELD_DELAY ? attr : 0);
    }

    if (attr && horz == LS_FIELD_V3 && cstate != LS_FAMILY_EDGE) {
      repeatLastCursorMove(event);
      showHeader = false;
    }

    // Persistent
    if (cstate == LS_FAMILY_STICKY) {
      drawCheckBox(CSW_7TH_COLUMN, y, cs->lsPersist, horz == LS_FIELD_PERSIST ? attr : 0);
    } else if (attr && horz == LS_FIELD_PERSIST) {
      repeatLastCursorMove(event);
      showHeader = false;
    }

    if (s_editMode>0 && attr) {
      switch (horz) {
        case LS_FIELD_FUNCTION:
        {
          cs->func = checkIncDec(event, cs->func, 0, LS_FUNC_MAX, EE_MODEL);
          uint8_t new_cstate = lswFamily(cs->func);
          if (cstate != new_cstate) {
            unsigned int save_func = cs->func;
            memset(cs, 0, sizeof(LogicalSwitchData));
            cs->func = save_func;
            if (new_cstate == LS_FAMILY_TIMER) {
              cs->v1.value = cs->v2.value = -119;
            }
            else if (new_cstate == LS_FAMILY_EDGE) {
              cs->v2.value = -129;
            }
          }
          break;
        }
        case LS_FIELD_V1:
          if (cstate == LS_FAMILY_BOOL || cstate == LS_FAMILY_STICKY || cstate == LS_FAMILY_EDGE) {
            cs->v1.swtch = checkIncDecSwitch(event, cs->v1.swtch, SWMASK_LOGICAL_AND, EE_MODEL,
                                             isSwitchAvailableInLogicalSwitches);
          }
          else if (cstate == LS_FAMILY_TIMER) {
            CHECK_INCDEC_MODELVAR(event, cs->v1.value, v1_min, v1_max);
          }
          else {
            cs->v1.source = checkIncDecSource(event, cs->v1.source, SRCMASK_ALL,
                                              isSourceAvailable);
          }
          break;
        case LS_FIELD_V2:
          if (cstate == LS_FAMILY_BOOL || cstate == LS_FAMILY_STICKY) {
            cs->v2.swtch = checkIncDecSwitch(event, cs->v2.swtch, SWMASK_LOGICAL_AND, EE_MODEL,
                                             isSwitchAvailableInLogicalSwitches);
          }
          else if (cstate == LS_FAMILY_COMP) {
            cs->v2.source = checkIncDecSource(event, cs->v2.source, SRCMASK_ALL,
                                              isSourceAvailable);
          } else {
            if (cs->v1.source.type >= SOURCE_TYPE_TIMER) {
              cs->v2.value = checkIncDec(event, cs->v2.value, v2_min, v2_max, EE_MODEL | INCDEC_REP10 | NO_INCDEC_MARKS);
            } else {
              CHECK_INCDEC_MODELVAR(event, cs->v2.value, v2_min, v2_max);
            }
            if (cstate==LS_FAMILY_OFS && !cs->v1.isZero() && event==EVT_KEY_LONG(KEY_ENTER)) {
              killEvents(event);
              getvalue_t x = getValue(cs->v1.source);
              if (cs->v1.source.type <= SOURCE_TYPE_CHANNEL) {
                cs->v2.value = calcRESXto100(x);
              }
              storageDirty(EE_MODEL);
            }
          }
          break;
        case LS_FIELD_V3:
          CHECK_INCDEC_MODELVAR(event, cs->v3, v3_min, v3_max);
          break;
        case LS_FIELD_ANDSW:
          cs->andsw = checkIncDecSwitch(event, cs->andsw, SWMASK_LOGICAL_AND, EE_MODEL, isSwitchAvailableInLogicalSwitches);
          break;
        case LS_FIELD_DURATION:
          CHECK_INCDEC_MODELVAR_ZERO(event, cs->duration, MAX_LS_DURATION);
          break;
        case LS_FIELD_DELAY:
          CHECK_INCDEC_MODELVAR_ZERO(event, cs->delay, MAX_LS_DELAY);
          break;
        case LS_FIELD_PERSIST:
          cs->lsPersist = checkIncDecModel(event, cs->lsPersist, 0, 1);
          break;
      }
    }
  }

  if (showHeader && menuHorizontalPosition>=0) {
    drawColumnHeader(STR_LSW_HEADERS, menuHorizontalPosition);
  }
}
