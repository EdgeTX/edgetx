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

#include "opentx.h"
#include "widgets_container_impl.h"

class TimerWidget: public Widget
{
  public:
    TimerWidget(const WidgetFactory * factory, FormGroup * parent, const rect_t & rect, Widget::PersistentData * persistentData):
      Widget(factory, parent, rect, persistentData)
    {
    }

    void refresh(BitmapBuffer * dc) override
    {
      uint32_t index = persistentData->options[0].value.unsignedValue;
      TimerData & timerData = g_model.timers[index];
      TimerState & timerState = timersStates[index];

      if (width() >= 180 && height() >= 70) {
        if (timerState.val >= 0 || !(timerState.val % 2)) {
          dc->drawBitmapPattern(0, 0, LBM_TIMER_BACKGROUND, MAINVIEW_PANES_COLOR);
        }
        else {
          dc->drawBitmapPattern(0, 0, LBM_TIMER_BACKGROUND, MENU_BGCOLOR);
        }
        if (timerData.start && timerState.val >= 0) {
          dc->drawBitmapPatternPie(
            2, 3, LBM_RSCALE, MAINVIEW_GRAPHICS_COLOR, 0,
            timerState.val <= 0 ? 360 : 360 * (timerData.start - timerState.val) / timerData.start);
        }
        else {
          dc->drawBitmapPattern(3, 4, LBM_TIMER, MAINVIEW_GRAPHICS_COLOR);
        }
        if (abs(timerState.val) >= 3600) {
          drawTimer(dc, 70, 31, abs(timerState.val), DEFAULT_COLOR | FONT(STD) | LEFT | TIMEHOUR);
        }
        else {
          drawTimer(dc,76, 31, abs(timerState.val), DEFAULT_COLOR | FONT(XL) | LEFT);
        }
        if (ZLEN(timerData.name) > 0) {
          dc->drawSizedText(78, 20, timerData.name, LEN_TIMER_NAME, FONT(XS) | DEFAULT_COLOR);
        }
        else {
          drawStringWithIndex(dc, 137, 17, "TMR", index + 1, FONT(XS) | DEFAULT_COLOR);
        }
      }
      else {
        if (timerState.val < 0 && timerState.val % 2) {
          dc->drawSolidFilledRect(0, 0, width(), height(), HEADER_ICON_BGCOLOR);
        }
        drawStringWithIndex(dc, 2, 0, "TMR", index + 1, FONT(XS) | FOCUS_COLOR);
        if (width() > 100 && height() > 40) {
          if (abs(timerState.val) >= 3600) {
            drawTimer(dc,3, 16, abs(timerState.val), FOCUS_COLOR | LEFT | TIMEHOUR);
          }
          else {
            drawTimer(dc, 3, 16, abs(timerState.val), FOCUS_COLOR | LEFT | FONT(STD));
          }
        }
        else {
          if (abs(timerState.val) >= 3600) {
            drawTimer(dc,3, 14, abs(timerState.val), FOCUS_COLOR | LEFT | FONT(XS) | TIMEHOUR);
          }
          else {
            drawTimer(dc, 3, 14, abs(timerState.val), FOCUS_COLOR | LEFT);
          }
        }
      }
    }

    void checkEvents() override
    {
      Widget::checkEvents();
      auto newValue = timersStates[persistentData->options[0].value.unsignedValue].val;
      if (lastValue != newValue) {
        lastValue = newValue;
        invalidate();
      }
    }

    static const ZoneOption options[];
    tmrval_t lastValue = 0;
};

const ZoneOption TimerWidget::options[] = {
  { STR_TIMER_SOURCE, ZoneOption::Timer, OPTION_VALUE_UNSIGNED(0) },
  { nullptr, ZoneOption::Bool }
};


BaseWidgetFactory<TimerWidget> timerWidget("Timer", TimerWidget::options);
