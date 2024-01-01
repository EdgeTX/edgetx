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

#include "view_statistics.h"

#include "opentx.h"
#include "draw_functions.h"

#include "tasks.h"
#include "tasks/mixer_task.h"

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};

#if LCD_W > LCD_H
static const lv_coord_t dbg_4col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
                                         LV_GRID_FR(3), LV_GRID_FR(3),
                                         LV_GRID_TEMPLATE_LAST};
#define DBG_COL_CNT 4
#else
static const lv_coord_t dbg_2col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                         LV_GRID_TEMPLATE_LAST};
static const lv_coord_t dbg_3col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                         LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
#define DBG_COL_CNT 3
#endif

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

#if LCD_W > LCD_H
#define CV_SCALE 3
#define DBG_B_WIDTH (LCD_W - 20) / 4
#else
#define CV_SCALE 4
#define DBG_B_WIDTH (LCD_W - 20) / 2
#endif
#define CV_WIDTH MAXTRACE
#define CV_HEIGHT (CV_SCALE * 32 + 5)

#define DBG_B_HEIGHT 20

template <class T>
class DebugInfoNumber : public Window
{
 public:
  DebugInfoNumber(Window* parent, const rect_t& rect,
                  std::function<T()> numberHandler, LcdFlags textFlags = 0,
                  const char* prefix = nullptr, const char* suffix = nullptr) :
      Window(parent, rect, 0, textFlags), prefix(prefix), suffix(suffix)
  {
    coord_t prefixSize = 0;
    if (prefix != nullptr) {
      prefixSize = getTextWidth(prefix, 0, COLOR_THEME_PRIMARY1 | FONT(XS));
      new StaticText(this, {0, 1, prefixSize, rect.h}, prefix, 0,
                     COLOR_THEME_PRIMARY1 | FONT(XS));
    }
    new DynamicNumber<uint32_t>(this,
                                {prefixSize, 0, rect.w - prefixSize, rect.h},
                                numberHandler, COLOR_THEME_PRIMARY1);
  }

 protected:
  const char* prefix;
  const char* suffix;
};

StatisticsViewPageGroup::StatisticsViewPageGroup() : TabsGroup(ICON_STATS)
{
  addTab(new StatisticsViewPage());
  addTab(new DebugViewPage());
}

class ThrottleCurveWindow : public Window
{
 public:
  ThrottleCurveWindow(Window* parent, const rect_t& rect) : Window(parent, rect)
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

  void paint(BitmapBuffer* dc) override
  {
    // Axis
    dc->drawHorizontalLine(0, height() - 3, width(), SOLID,
                           COLOR_THEME_SECONDARY1);
    dc->drawVerticalLine(0, 0, height() - 3, SOLID, COLOR_THEME_SECONDARY1);
    for (coord_t i = 0; i < width(); i += 6) {
      dc->drawVerticalLine(i, height() - 6, 3, SOLID, COLOR_THEME_SECONDARY1);
    }

    // Curve
    uint16_t traceRd = s_traceWr > width() ? s_traceWr - width() : 0;
    coord_t prev_yv = (coord_t)-1;
    for (coord_t i = 1; i <= width() && traceRd < s_traceWr; i++, traceRd++) {
      uint8_t h = s_traceBuf[traceRd % width()];
      coord_t yv = height() - 5 - CV_SCALE * h;
      if (prev_yv != (coord_t)-1) {
        if (prev_yv < yv) {
          for (int y = prev_yv; y <= yv; y++) {
            dc->drawBitmapPattern(i, y, LBM_POINT, COLOR_THEME_SECONDARY1);
          }
        } else {
          for (int y = yv; y <= prev_yv; y++) {
            dc->drawBitmapPattern(i, y, LBM_POINT, COLOR_THEME_SECONDARY1);
          }
        }
      } else {
        dc->drawBitmapPattern(i, yv, LBM_POINT, COLOR_THEME_SECONDARY1);
      }
      prev_yv = yv;
    }
  };

 protected:
  unsigned previousTraceWr = 0;
};

void StatisticsViewPage::build(FormWindow* window)
{
  window->padAll(0);

  auto form = new FormWindow(window, rect_t{});
  form->setFlexLayout();
  form->padAll(0);
  form->padLeft(4);
  form->padRight(4);

  FlexGridLayout grid(col_dsc, row_dsc, 0);

  auto line = form->newLine(&grid);
  line->padAll(0);

  // Session data
  new StaticText(line, rect_t{}, STR_SESSION, 0, COLOR_THEME_PRIMARY1);
  new DynamicText(
      line, rect_t{}, [] { return getTimerString(sessionTimer); },
      COLOR_THEME_PRIMARY1);

  // Battery data
  new StaticText(line, rect_t{}, STR_BATT_LABEL, 0, COLOR_THEME_PRIMARY1);
  new DynamicText(
      line, rect_t{},
      [] { return getTimerString(g_eeGeneral.globalTimer + sessionTimer); },
      COLOR_THEME_PRIMARY1);

  line = form->newLine(&grid);
  line->padAll(0);

  // Throttle
  new StaticText(line, rect_t{}, STR_THROTTLE_LABEL, 0, COLOR_THEME_PRIMARY1);
  new DynamicText(
      line, rect_t{}, [] { return getTimerString(s_timeCumThr); },
      COLOR_THEME_PRIMARY1);

  // Throttle %  data
  new StaticText(line, rect_t{}, STR_THROTTLE_PERCENT_LABEL, 0,
                 COLOR_THEME_PRIMARY1);
  new DynamicText(
      line, rect_t{}, [] { return getTimerString(s_timeCum16ThrP / 16); },
      COLOR_THEME_PRIMARY1);

  line = form->newLine(&grid);
  line->padAll(0);

  // Timers
  new StaticText(line, rect_t{}, STR_TIMER_LABEL, 0, COLOR_THEME_PRIMARY1);
  new DynamicText(
      line, rect_t{}, [] { return getTimerString(timersStates[0].val); },
      COLOR_THEME_PRIMARY1);
  new DynamicText(
      line, rect_t{}, [] { return getTimerString(timersStates[1].val); },
      COLOR_THEME_PRIMARY1);
  new DynamicText(
      line, rect_t{}, [] { return getTimerString(timersStates[2].val); },
      COLOR_THEME_PRIMARY1);

  line = form->newLine(&grid);
  line->padAll(0);
  line->padTop(3);

  // Throttle curve
  auto curve = new ThrottleCurveWindow(line, {0, 0, CV_WIDTH, CV_HEIGHT});

  lv_obj_set_grid_cell(curve->getLvObj(), LV_GRID_ALIGN_CENTER, 0, 4,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  line = form->newLine(&grid);
  line->padAll(0);
  line->padTop(3);

  // Reset
  auto btn = new TextButton(line, rect_t{0, 0, 0, 24}, STR_MENUTORESET,
                            [=]() -> uint8_t {
                              g_eeGeneral.globalTimer = 0;
                              storageDirty(EE_GENERAL);
                              sessionTimer = 0;
                              s_timeCumThr = 0;
                              s_timeCum16ThrP = 0;
                              s_traceWr = 0;
                              curve->invalidate();
                              return 0;
                            });

  lv_obj_set_grid_cell(btn->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 4,
                       LV_GRID_ALIGN_START, 0, 1);
}

void DebugViewPage::build(FormWindow* window)
{
  window->padAll(4);

  auto form = new FormWindow(window, rect_t{});
  form->setFlexLayout();
  form->padAll(0);

#if LCD_W > LCD_H
  FlexGridLayout grid(dbg_4col_dsc, row_dsc, 0);
  FlexGridLayout grid2(dbg_4col_dsc, row_dsc, 0);
#else
  FlexGridLayout grid(dbg_2col_dsc, row_dsc, 0);
  FlexGridLayout grid2(dbg_3col_dsc, row_dsc, 0);
#endif

  auto line = form->newLine(&grid);
  line->padAll(2);

  // Mixer data
  static std::string pad_STR_MS = " " + std::string(STR_MS);
  new StaticText(line, rect_t{}, STR_TMIXMAXMS, 0, COLOR_THEME_PRIMARY1);
  new DynamicNumber<uint16_t>(
      line, rect_t{}, [] { return DURATION_MS_PREC2(maxMixerDuration); },
      PREC2 | COLOR_THEME_PRIMARY1, nullptr, pad_STR_MS.c_str());

  line = form->newLine(&grid);
  line->padAll(2);

  // Free mem
  static std::string pad_STR_BYTES = " " + std::string(STR_BYTES);
  new StaticText(line, rect_t{}, STR_FREE_MEM_LABEL, 0, COLOR_THEME_PRIMARY1);
  new DynamicNumber<int32_t>(
      line, rect_t{}, [] { return availableMemory(); }, COLOR_THEME_PRIMARY1, 
      nullptr, pad_STR_BYTES.c_str());

#if defined(LUA)
  line = form->newLine(&grid);
  line->padAll(2);

  // LUA timing data
  new StaticText(line, rect_t{}, STR_LUA_SCRIPTS_LABEL, 0,
                 COLOR_THEME_PRIMARY1);
#if LCD_H > LCD_W
  line = form->newLine(&grid);
  line->padAll(0);
  line->padLeft(10);
#endif
  new DebugInfoNumber<uint16_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return 10 * maxLuaDuration; }, COLOR_THEME_PRIMARY1, STR_DURATION_MS,
      nullptr);
  new DebugInfoNumber<uint16_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return 10 * maxLuaInterval; }, COLOR_THEME_PRIMARY1, STR_INTERVAL_MS,
      nullptr);

  line = form->newLine(&grid);
  line->padAll(0);
#if LCD_H > LCD_W
  line->padLeft(10);
#else
  grid.nextCell();
#endif

  // lUA memory data
  new DebugInfoNumber<uint32_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return luaGetMemUsed(lsScripts); }, COLOR_THEME_PRIMARY1,
      STR_MEM_USED_SCRIPT, nullptr);
  new DebugInfoNumber<uint32_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return luaGetMemUsed(lsWidgets); }, COLOR_THEME_PRIMARY1,
      STR_MEM_USED_WIDGET, nullptr);

#if LCD_H > LCD_W
  line = form->newLine(&grid);
  line->padAll(0);
  line->padLeft(10);
#endif

  new DebugInfoNumber<uint32_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return luaExtraMemoryUsage; }, COLOR_THEME_PRIMARY1,
      STR_MEM_USED_EXTRA, nullptr);
#endif

  line = form->newLine(&grid);
  line->padAll(2);

  // Stacks data
  new StaticText(line, rect_t{}, STR_FREE_STACK, 0, COLOR_THEME_PRIMARY1);
#if LCD_H > LCD_W
  line = form->newLine(&grid2);
  line->padAll(0);
  line->padLeft(10);
#endif
  new DebugInfoNumber<uint32_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return menusStack.available(); }, COLOR_THEME_PRIMARY1,
      STR_STACK_MENU, nullptr);
  new DebugInfoNumber<uint32_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return mixerStack.available(); }, COLOR_THEME_PRIMARY1,
      STR_STACK_MIX, nullptr);
  new DebugInfoNumber<uint32_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return audioStack.available(); }, COLOR_THEME_PRIMARY1,
      STR_STACK_AUDIO, nullptr);

#if defined(DEBUG_LATENCY)
  line = form->newLine(&grid2);
  line->padAll(2);

  new StaticText(line, rect_t{}, STR_HEARTBEAT_LABEL, 0, COLOR_THEME_PRIMARY1);
  if (heartbeatCapture.valid)
    new DebugInfoNumber<uint16_t>(
        line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
        [] { return heartbeatCapture.count; }, COLOR_THEME_PRIMARY1);
  else
    new StaticText(window, grid.getFieldSlot(), "---", 0, COLOR_THEME_PRIMARY1);
#endif

#if defined(INTERNAL_GPS)
  if (serialGetModePort(UART_MODE_GPS) >= 0) {
    line = form->newLine(&grid);
    line->padAll(2);

    new StaticText(line, rect_t{}, STR_INT_GPS_LABEL, 0, COLOR_THEME_PRIMARY1);
#if LCD_H > LCD_W
    line = form->newLine(&grid2);
    line->padAll(0);
    line->padLeft(10);
#endif
    new DynamicText(
        line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
        [] { return std::string(gpsData.fix ? STR_GPS_FIX_YES : STR_GPS_FIX_NO); },
        COLOR_THEME_PRIMARY1);
    new DebugInfoNumber<uint8_t>(
        line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
        [] { return gpsData.numSat; }, COLOR_THEME_PRIMARY1, STR_GPS_SATS,
        nullptr);
    new DebugInfoNumber<uint16_t>(
        line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
        [] { return gpsData.hdop; }, COLOR_THEME_PRIMARY1, STR_GPS_HDOP, nullptr);
  }
#endif

  line = form->newLine(&grid2);
  line->padAll(4);

  // Reset
  auto btn = new TextButton(line, rect_t{0, 0, 0, 24}, STR_MENUTORESET,
                            [=]() -> uint8_t {
                              maxMixerDuration = 0;
#if defined(LUA)
                              maxLuaInterval = 0;
                              maxLuaDuration = 0;
#endif
                              return 0;
                            });

  lv_obj_set_grid_cell(btn->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, DBG_COL_CNT,
                       LV_GRID_ALIGN_CENTER, 0, 1);
}
