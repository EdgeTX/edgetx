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

#include "edgetx.h"
#include "libopenui.h"
#include "view_statistics.h"

StatisticsViewPageGroup::StatisticsViewPageGroup():
  TabsGroup(ICON_STATS)
{
  addTab(new StatisticsViewPage());
  addTab(new DebugViewPage());
}

class ThrottleCurveWindow : public Window
{
  public:
    ThrottleCurveWindow(Window * parent, const rect_t &rect) :
      Window(parent, rect)
    {
    }

    void checkEvents() override
    {
      Window::checkEvents();
      if (previousTraceWr != s_traceWr) {
        previousTraceWr = s_traceWr;
        invalidate();
      }
    }
    
    void paint(BitmapBuffer * dc) override
    {
      // Axis
      dc->drawHorizontalLine(0, height() - 2, width(), SOLID | DEFAULT_COLOR);
      dc->drawVerticalLine(0, 0, height(), SOLID, DEFAULT_COLOR);
      for (coord_t i=0; i < width(); i+=6) {
        dc->drawVerticalLine(i,height() - 4, 3, SOLID | DEFAULT_COLOR);
      }

      // Curve
      uint16_t traceRd = s_traceWr > width() ? s_traceWr - width() : 0;
      coord_t prev_yv = (coord_t)-1;
      for (coord_t i=1; i<=width() && traceRd<s_traceWr; i++, traceRd++) {
        uint8_t h = s_traceBuf[traceRd % width()];
        coord_t yv = height() - 2 - 3*h;
        if (prev_yv != (coord_t)-1) {
          if (prev_yv < yv) {
            for (int y=prev_yv; y<=yv; y++) {
              dc->drawBitmapPattern(i, y, LBM_POINT, DEFAULT_COLOR);
            }
          }
          else {
            for (int y=yv; y<=prev_yv; y++) {
              dc->drawBitmapPattern(i, y, LBM_POINT, DEFAULT_COLOR);
            }
          }
        }
        else {
          dc->drawBitmapPattern(i, yv, LBM_POINT, DEFAULT_COLOR);
        }
        prev_yv = yv;
      }
    };

  protected:
    unsigned previousTraceWr = 0;
};

void StatisticsViewPage::build(FormWindow * window)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);
  grid.setLabelWidth(LCD_W / 4);

  // Session data
  new StaticText(window, grid.getLabelSlot(), STR_SESSION);
  new DynamicText(window, grid.getFieldSlot(3, 0), [] {
      return getTimerString(sessionTimer, TIMEHOUR);
  });

  // Battery data
  new StaticText(window, grid.getFieldSlot(3, 1), STR_BATT_LABEL);
  new DynamicText(window, grid.getFieldSlot(3, 2), [] {
      return getTimerString(g_eeGeneral.globalTimer + sessionTimer, TIMEHOUR);
  });
  grid.nextLine();

  // Throttle
  new StaticText(window, grid.getLabelSlot(), STR_THROTTLE_LABEL);
  new DynamicText(window, grid.getFieldSlot(3, 0), [] {
      return getTimerString(s_timeCumThr, TIMEHOUR);
  });

  // Throttle %  data
  new StaticText(window, grid.getFieldSlot(3, 1), STR_THROTTLE_PERCENT_LABEL);
  new DynamicText(window, grid.getFieldSlot(3, 2), [] {
      return getTimerString(s_timeCum16ThrP/16, TIMEHOUR);
  });
  grid.nextLine();

  // Timers
  new StaticText(window, grid.getLabelSlot(), STR_TIMER_LABEL);
  new DynamicText(window, grid.getFieldSlot(3, 0), [] {
      return getTimerString(timersStates[0].val, TIMEHOUR);
  }, 0);
  new DynamicText(window, grid.getFieldSlot(3, 1), [] {
      return getTimerString(timersStates[1].val, TIMEHOUR);
  }, 0);
  new DynamicText(window, grid.getFieldSlot(3, 2), [] {
      return getTimerString(timersStates[2].val, TIMEHOUR);
  }, 0);
  grid.nextLine();

  // Throttle curve
  auto curve = new ThrottleCurveWindow(window, {5, grid.getWindowHeight(), window->width() - 10, window->height() / 2});
  grid.spacer(curve->height() + 3);

  // Reset
  new TextButton (window, grid.getLineSlot(), STR_MENUTORESET,
                  [=]() -> uint8_t {
                      g_eeGeneral.globalTimer = 0;
                      storageDirty(EE_GENERAL);
                      sessionTimer = 0;
                      return 0;
                  }, BUTTON_BACKGROUND | NO_FOCUS);
}

void DebugViewPage::build(FormWindow * window)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);
  grid.setLabelWidth(LCD_W / 4);

  // Mixer data
  new StaticText(window, grid.getLabelSlot(), STR_TMIXMAXMS);
  new DynamicNumber<uint16_t>(window, grid.getFieldSlot(), [] {
      return DURATION_MS_PREC2(maxMixerDuration);
  }, PREC2, nullptr, "ms");
  grid.nextLine();

  // Free mem
  new StaticText(window, grid.getLabelSlot(), STR_FREE_MEM_LABEL);
  new DynamicNumber<int>(window, grid.getFieldSlot(), [] {
      return availableMemory();
  }, 0, nullptr, "b");
  grid.nextLine();

#if defined(LUA)
  // LUA timing data
  new StaticText(window, grid.getLabelSlot(), STR_LUA_SCRIPTS_LABEL);
  new DebugInfoNumber<uint16_t>(window, grid.getFieldSlot(3, 0), [] {
      return 10 * maxLuaDuration;
  }, 0, "[Dur] ", "ms");
  new DebugInfoNumber<uint16_t>(window, grid.getFieldSlot(3, 1), [] {
      return 10 * maxLuaInterval;
  }, 0, "[Int] ", "ms");
  grid.nextLine();

  // lUA memory data
  new DebugInfoNumber<uint32_t>(window, grid.getFieldSlot(3, 0), [] {
      return 10 * luaGetMemUsed(lsScripts);
  }, 0, "[S] ", nullptr);
  new DebugInfoNumber<uint32_t>(window, grid.getFieldSlot(3, 1), [] {
      return 10 * luaGetMemUsed(lsWidgets);
  }, 0, "[W] ", nullptr);
  new DebugInfoNumber<uint32_t>(window, grid.getFieldSlot(3, 2), [] {
      return luaExtraMemoryUsage;
  }, 0, "[B] ", nullptr);
  grid.nextLine();
#endif

  // Stacks data
  new StaticText(window, grid.getLabelSlot(), STR_FREE_STACK);
  new DebugInfoNumber<uint32_t>(window, grid.getFieldSlot(3, 0), [] {
      return menusStack.available();
  }, 0, "[Menu] ", nullptr);
  new DebugInfoNumber<uint32_t>(window, grid.getFieldSlot(3, 1), [] {
      return  mixerStack.available();
  }, 0, "[Mix] ", nullptr);
  new DebugInfoNumber<uint32_t>(window, grid.getFieldSlot(3, 2), [] {
      return audioStack.available();
  }, 0, "[Audio] ", nullptr);
  grid.nextLine();

#if defined(DEBUG_LATENCY)
  new StaticText(window, grid.getLabelSlot(), STR_HEARTBEAT_LABEL);
  if (heartbeatCapture.valid)
    new DebugInfoNumber<uint16_t>(window, grid.getFieldSlot(3, 0), [] {
      return heartbeatCapture.count;
  });
  else
    new StaticText(window, grid.getFieldSlot(),"---");
#endif

#if defined(INTERNAL_GPS)
  new StaticText(window, grid.getLabelSlot(), STR_INT_GPS_LABEL);
  new DynamicText(window, grid.getFieldSlot(3, 0), [] {
      return std::string(gpsData.fix ? "[Fix] Yes" : "[Fix] No");
  });
  new DebugInfoNumber<uint8_t>(window, grid.getFieldSlot(3, 1), [] {
      return  gpsData.numSat;
  }, 0, "[Sats] ", nullptr);
  new DebugInfoNumber<uint16_t>(window, grid.getFieldSlot(3, 2), [] {
      return gpsData.hdop;
  }, 0, "[Hdop] ", nullptr);
  grid.nextLine();
#endif

  // Reset
  grid.nextLine();
  new TextButton (window, grid.getLineSlot(), STR_MENUTORESET,
     [=]() -> uint8_t {
         maxMixerDuration  = 0;
#if defined(LUA)
         maxLuaInterval = 0;
         maxLuaDuration = 0;
#endif
         return 0;
     }, BUTTON_BACKGROUND | NO_FOCUS);
}
