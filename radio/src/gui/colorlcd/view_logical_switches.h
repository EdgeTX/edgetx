/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#pragma once

#include "edgetx.h"
#include "tabsgroup.h"

constexpr coord_t CSW_1ST_COLUMN = 50;
constexpr coord_t CSW_2ND_COLUMN = 120;
constexpr coord_t CSW_3RD_COLUMN = 200;
constexpr coord_t CSW_4TH_COLUMN = 280;
constexpr coord_t CSW_5TH_COLUMN = 340;
constexpr coord_t CSW_6TH_COLUMN = 390;

void putsEdgeDelayParam(BitmapBuffer * dc, coord_t x, coord_t y, LogicalSwitchData * ls, LcdFlags flags);

class LogicalSwitchDisplayFooter: public Window {
  public:
    explicit LogicalSwitchDisplayFooter(Window * parent, rect_t rect):
      Window(parent,  rect, OPAQUE),
      rect(rect)
    {
    }

    void setIndex(unsigned value)
    {
      index = value;
    }

    void paint(BitmapBuffer * dc) override
    {
      dc->clear(TITLE_BGCOLOR);
      LogicalSwitchData * cs = lswAddress(index);
      dc->drawTextAtIndex(10, 1, STR_VCSWFUNC, cs->func, MENU_COLOR);
      // CSW params
      unsigned int cstate = lswFamily(cs->func);

      if (cstate == LS_FAMILY_BOOL || cstate == LS_FAMILY_STICKY) {
        drawSwitch(dc, CSW_2ND_COLUMN, 1, cs->v1, MENU_COLOR);
        drawSwitch(dc, CSW_3RD_COLUMN, 1, cs->v2, MENU_COLOR);
      }
      else if (cstate == LS_FAMILY_EDGE) {
        drawSwitch(dc, CSW_2ND_COLUMN, 1, cs->v1, MENU_COLOR);
        putsEdgeDelayParam(dc, CSW_3RD_COLUMN, 1, cs, MENU_COLOR);
      }
      else if (cstate == LS_FAMILY_COMP) {
        drawSource(dc, CSW_2ND_COLUMN, 1, cs->v1, MENU_COLOR);
        drawSource(dc, CSW_3RD_COLUMN, 1, cs->v2, MENU_COLOR);
      }
      else if (cstate == LS_FAMILY_TIMER) {
        dc->drawNumber(CSW_2ND_COLUMN, 1, lswTimerValue(cs->v1), LEFT | PREC1 | MENU_COLOR);
        dc->drawNumber(CSW_3RD_COLUMN, 1, lswTimerValue(cs->v2), LEFT | PREC1 | MENU_COLOR);
      }
      else {
        drawSource(dc, CSW_2ND_COLUMN, 1, cs->v1, MENU_COLOR);
        drawSourceCustomValue(dc, CSW_3RD_COLUMN, 1, cs->v1, cs->v1 <= MIXSRC_LAST_CH ? calc100toRESX(cs->v2) : cs->v2, LEFT | MENU_COLOR);
      }

      // CSW AND switch
      drawSwitch(dc, CSW_4TH_COLUMN, 1, cs->andsw, MENU_COLOR);

      // CSW duration
      if (cs->duration > 0)
        dc->drawNumber(CSW_5TH_COLUMN, 1, cs->duration, PREC1 | LEFT | MENU_COLOR);
      else
        dc->drawTextAtIndex(CSW_5TH_COLUMN, 1, STR_MMMINV, 0, MENU_COLOR);

      // CSW delay
      if (cstate == LS_FAMILY_EDGE) {
        dc->drawText(CSW_6TH_COLUMN, 1, STR_NA, MENU_COLOR);
      }
      else if (cs->delay > 0) {
        dc->drawNumber(CSW_6TH_COLUMN, 1, cs->delay, PREC1 | LEFT | MENU_COLOR);
      }
      else {
        dc->drawTextAtIndex(CSW_6TH_COLUMN, 1, STR_MMMINV, 0, MENU_COLOR);
      }
    }

  protected:
    rect_t rect;
    unsigned index = 0;
};


class LogicalSwitchesViewPage : public PageTab
{
  public:
    LogicalSwitchesViewPage() :
      PageTab(STR_MONITOR_SWITCHES, ICON_MONITOR_LOGICAL_SWITCHES)
    {
    }

  protected:
    void build(FormWindow * window) override;
    LogicalSwitchDisplayFooter * footer = nullptr;
};
