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

#include "timer_setup.h"
#include "opentx.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

// TODO: translation
const char* STR_TIMER_MODES[] = {"OFF",      "ON",         "Start",
                                 "Throttle", "Throttle %", "Throttle Start"};

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

static std::function<void(uint32_t)> timerValueUpdater(uint8_t timer)
{
    return [=](uint32_t value) {
        TimerData* p_timer = &g_model.timers[timer];
        p_timer->start = value;
        timerSet(timer, value);
        SET_DIRTY();
    };
}

TimerWindow::TimerWindow(uint8_t timer) : Page(ICON_STATS_TIMERS)
{
  std::string title = std::string(STR_TIMER) + std::to_string(timer + 1);
  new StaticText(&header,
                 {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                  PAGE_LINE_HEIGHT},
                 title, 0, COLOR_THEME_PRIMARY2);

  body.setFlexLayout();
  FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

  TimerData* p_timer = &g_model.timers[timer];

  // Timer name
  auto line = body.newLine(&grid);
  new StaticText(line, rect_t{}, STR_NAME, 0, COLOR_THEME_PRIMARY1);
  new ModelTextEdit(line, rect_t{}, p_timer->name, LEN_TIMER_NAME);

  // Timer mode
  line = body.newLine(&grid);
  new StaticText(line, rect_t{}, STR_MODE, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_TIMER_MODES, 0, TMRMODE_MAX,
             GET_SET_DEFAULT(p_timer->mode));

  // Timer switch
  line = body.newLine(&grid);
  new StaticText(line, rect_t{}, STR_SWITCH, 0, COLOR_THEME_PRIMARY1);
  new SwitchChoice(line, rect_t{}, SWSRC_FIRST, SWSRC_LAST,
                   GET_SET_DEFAULT(p_timer->swtch));

  // Timer start value
  line = body.newLine(&grid);
  new StaticText(line, rect_t{}, STR_START, 0, COLOR_THEME_PRIMARY1);
  new TimeEdit(line, rect_t{}, 0, TIMER_MAX, GET_DEFAULT(p_timer->start),
               timerValueUpdater(timer));

  // Timer minute beep
  line = body.newLine(&grid);
  new StaticText(line, rect_t{}, STR_MINUTEBEEP, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_DEFAULT(p_timer->minuteBeep));

  // Timer countdown
  line = body.newLine(&grid);
  new StaticText(line, rect_t{}, STR_BEEPCOUNTDOWN, 0, COLOR_THEME_PRIMARY1);

  auto box = new FormGroup::Line(line, lv_obj_create(line->getLvObj()), nullptr, &body);
  lv_obj_set_layout(box->getLvObj(), LV_LAYOUT_FLEX);
  new Choice(box, rect_t{}, STR_VBEEPCOUNTDOWN, COUNTDOWN_SILENT,
             COUNTDOWN_COUNT - 1, GET_SET_DEFAULT(p_timer->countdownBeep));
  new Choice(box, rect_t{}, STR_COUNTDOWNVALUES, 0, 3,
             GET_SET_WITH_OFFSET(p_timer->countdownStart, 2));

  // Timer persistent
  line = body.newLine(&grid);
  new StaticText(line, rect_t{}, STR_PERSISTENT, 0, COLOR_THEME_PRIMARY1);

  new Choice(line, rect_t{}, STR_VPERSISTENT, 0, 2,
             GET_SET_DEFAULT(p_timer->persistent));

  body.updateSize();
}
