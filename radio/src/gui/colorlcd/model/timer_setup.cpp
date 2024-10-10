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

#include "edgetx.h"
#include "switchchoice.h"
#include "timeedit.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

TimerWindow::TimerWindow(uint8_t timer) :
  SubPage(ICON_STATS_TIMERS, STR_MENU_MODEL_SETUP, (std::string(STR_TIMER) + std::to_string(timer + 1)).c_str())
{
  body->setFlexLayout();

  TimerData* p_timer = &g_model.timers[timer];

  // Timer name
  setupLine(STR_NAME,
    [=](Window* parent, coord_t x, coord_t y) {
      new ModelTextEdit(parent, {x, y, 0, 0}, p_timer->name, LEN_TIMER_NAME);
    });

  // Timer mode
  setupLine(STR_MODE,
    [=](Window* parent, coord_t x, coord_t y) {
      new Choice(parent, {x, y, 0, 0}, STR_TIMER_MODES, 0, TMRMODE_MAX,
                GET_SET_DEFAULT(p_timer->mode));
    });

  // Timer switch
  setupLine(STR_SWITCH,
    [=](Window* parent, coord_t x, coord_t y) {
      new SwitchChoice(parent, rect_t{x, y, 0, 0}, SWSRC_FIRST, SWSRC_LAST,
                       GET_SET_DEFAULT(p_timer->swtch));
    });

  // Timer start value
  setupLine(STR_START,
    [=](Window* parent, coord_t x, coord_t y) {
      auto timerValue = new TimeEdit(parent, {x, y, 0, 0}, 0, TIMER_MAX,
                                GET_DEFAULT(p_timer->start), [=](int newValue) {
                                  p_timer->start = newValue;
                                  timerSet(timer, newValue);
                                  timerDirLine->show(newValue > 0);
                                  SET_DIRTY();
                                });
      timerValue->setAccelFactor(16);
    });

  // Timer direction
  timerDirLine = setupLine(STR_LIMITS_HEADERS_DIRECTION,
    [=](Window* parent, coord_t x, coord_t y) {
      new Choice(parent, {x, y, 0, 0}, STR_TIMER_DIR, 0, 1,
                 GET_SET_DEFAULT(p_timer->showElapsed));
    });
  timerDirLine->show(p_timer->start> 0);

  // Timer minute beep
  setupLine(STR_MINUTEBEEP,
    [=](Window* parent, coord_t x, coord_t y) {
      new ToggleSwitch(parent, {x, y, 0, 0}, GET_SET_DEFAULT(p_timer->minuteBeep));
    });

  // Timer countdown
  setupLine(STR_BEEPCOUNTDOWN,
    [=](Window* parent, coord_t x, coord_t y) {
      new Choice(
          parent, rect_t{x, y, COUNTDOWN_W, 0}, STR_VBEEPCOUNTDOWN, COUNTDOWN_SILENT, COUNTDOWN_COUNT - 1,
          [=]() -> int {
            int value = p_timer->countdownBeep;
            if (p_timer->extraHaptic) {
              value += (COUNTDOWN_NON_HAPTIC_LAST + 1);
            }
            return (value);
          },
          [=](int value) {
            if (value > COUNTDOWN_NON_HAPTIC_LAST + 1) {
              p_timer->extraHaptic = 1;
              p_timer->countdownBeep = value - (COUNTDOWN_NON_HAPTIC_LAST + 1);
            } else {
              p_timer->extraHaptic = 0;
              p_timer->countdownBeep = value;
            }
            SET_DIRTY();
            TRACE("value=%d\tcountdownBeep = %d\textraHaptic = %d", value,
                  p_timer->countdownBeep, p_timer->extraHaptic);
          });

      new Choice(parent, {x + COUNTDOWN_VAL_XO, y + COUNTDOWN_VAL_YO, 0, 0}, STR_COUNTDOWNVALUES, 0, 3,
                GET_SET_WITH_OFFSET(p_timer->countdownStart, 2));
    }, COUNTDOWN_LBL_YO);

  // Timer persistent
  setupLine(STR_PERSISTENT,
    [=](Window* parent, coord_t x, coord_t y) {
      new Choice(parent, {x, y, 0, 0}, STR_VPERSISTENT, 0, 2,
                GET_SET_DEFAULT(p_timer->persistent));
    });
}
