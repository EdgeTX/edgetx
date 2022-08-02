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

#include "opentx.h"
#include "widgets_container_impl.h"

const uint8_t _LBM_TIMER[] = {
#include "mask_timer.lbm"
};

const uint8_t _LBM_RSCALE[] = {
#include "mask_rscale.lbm"
};

const uint8_t _LBM_TIMER_BACKGROUND[] = {
#include "mask_timer_bg.lbm"
};

STATIC_LZ4_BITMAP(LBM_TIMER);
STATIC_LZ4_BITMAP(LBM_RSCALE);
STATIC_LZ4_BITMAP(LBM_TIMER_BACKGROUND);

class TimerWidget : public Widget
{
 public:
  TimerWidget(const WidgetFactory* factory, Window* parent, const rect_t& rect,
              Widget::PersistentData* persistentData) :
      Widget(factory, parent, rect, persistentData)
  {
  }

  void refresh(BitmapBuffer* dc) override
  {
    uint32_t index = persistentData->options[0].value.unsignedValue;
    TimerData& timerData = g_model.timers[index];
    TimerState& timerState = timersStates[index];
    char sDigitGroup0[LEN_TIMER_STRING];
    char sDigitGroup1[LEN_TIMER_STRING];
    char sUnit0[] = "M";
    char sUnit1[] = "S";
    LcdFlags colorBack;  // background color
    LcdFlags colorFore;  // foreground color

    // Middle size widget
    if (width() >= 180 && height() >= 70) {
      colorBack = (timerState.val >= 0 || !(timerState.val % 2))
                      ? COLOR_THEME_PRIMARY2
                      : COLOR_THEME_WARNING;
      colorFore = (timerState.val >= 0 || !(timerState.val % 2))
                      ? COLOR_THEME_SECONDARY1
                      : COLOR_THEME_SECONDARY2;

      // background
      dc->drawBitmapPattern(0, 0, LBM_TIMER_BACKGROUND, colorBack);

      if (timerData.start && timerState.val >= 0) {
        dc->drawBitmapPatternPie(
            2, 3, LBM_RSCALE, colorFore, 0,
            timerState.val <= 0
                ? 360
                : 360 * (timerData.start - timerState.val) / timerData.start);
      } else {
        dc->drawBitmapPattern(3, 4, LBM_TIMER, colorFore);
      }
      // value
      int val = timerState.val;
      if (timerData.start && timerData.showElapsed &&
          timerData.start != timerState.val)
        val = (int)timerData.start - (int)timerState.val;
      splitTimer(sDigitGroup0, sDigitGroup1, sUnit0, sUnit1, abs(val), false);

      dc->drawSizedText(76, 31, sDigitGroup0, ZLEN(sDigitGroup0),
                        FONT(XL) | colorFore);
      dc->drawSizedText(76 + 35, 33, sUnit0, ZLEN(sUnit0),
                        FONT(STD) | colorFore);
      dc->drawSizedText(76 + 50, 31, sDigitGroup1, ZLEN(sDigitGroup1),
                        FONT(XL) | colorFore);
      dc->drawSizedText(76 + 85, 33, sUnit1, ZLEN(sUnit1),
                        FONT(STD) | colorFore);
      // name
      if (ZLEN(timerData.name) > 0) {  // user name exist
        dc->drawSizedText(78, 20, timerData.name, LEN_TIMER_NAME,
                          FONT(XS) | colorFore);
      } else {  // user name not exist "TMRn"
        drawStringWithIndex(dc, 137, 17, "TMR", index + 1,
                            FONT(XS) | colorFore);
      }
    }
    // Small size widget
    else {
      // background
      if (timerState.val < 0 && timerState.val % 2) {
        dc->drawSolidFilledRect(0, 0, width(), height(), COLOR_THEME_WARNING);
      }
      // name
      if (ZLEN(timerData.name) > 0) {  // user name exist
        dc->drawText(2, 0, timerData.name, FONT(XS) | COLOR_THEME_PRIMARY2);
      } else {  // user name not exist "TMRn"
        drawStringWithIndex(dc, 2, 0, "TMR", index + 1,
                            FONT(XS) | COLOR_THEME_PRIMARY2);
      }
      // value
      int val = timerState.val;
      if (timerData.start && timerData.showElapsed &&
          timerData.start != timerState.val)
        val = (int)timerData.start - (int)timerState.val;
      if (width() > 100 && height() > 40) {
        if (abs(val) >= 3600) {
          drawTimer(dc, 3, 20, abs(val),
                    COLOR_THEME_PRIMARY2 | LEFT | TIMEHOUR);
        } else {
          drawTimer(dc, 3, 18, abs(val),
                    COLOR_THEME_PRIMARY2 | LEFT | FONT(STD));
        }
      }
      // very small size
      else {
        if (abs(timerState.val) >= 3600) {
          drawTimer(dc, 3, 20, abs(timerState.val),
                    COLOR_THEME_PRIMARY2 | LEFT | FONT(XS) | TIMEHOUR);
        } else {
          // value
          int val = timerState.val;
          if (timerData.start && timerData.showElapsed &&
              timerData.start != timerState.val)
            val = (int)timerData.start - (int)timerState.val;
          drawTimer(dc, 3, 18, abs(val), COLOR_THEME_PRIMARY2 | LEFT);
        }
      }
    }
  }

  void checkEvents() override
  {
    Widget::checkEvents();
    auto newValue =
        timersStates[persistentData->options[0].value.unsignedValue].val;
    if (lastValue != newValue) {
      lastValue = newValue;
      invalidate();
    }
  }

  static const ZoneOption options[];
  tmrval_t lastValue = 0;
};

const ZoneOption TimerWidget::options[] = {
    {STR_TIMER_SOURCE, ZoneOption::Timer, OPTION_VALUE_UNSIGNED(0)},
    {nullptr, ZoneOption::Bool}};

BaseWidgetFactory<TimerWidget> timerWidget("Timer", TimerWidget::options);
