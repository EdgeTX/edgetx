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

#include "button.h"
#include "edgetx.h"
#include "lua/lua_states.h"
#include "mixer_scheduler.h"
#include "os/task.h"
#include "quick_menu.h"
#include "static.h"
#include "tasks.h"
#include "tasks/mixer_task.h"

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};

#if LANDSCAPE
static const lv_coord_t dbg_4col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
                                          LV_GRID_FR(3), LV_GRID_FR(3),
                                          LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t dbg_2col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t dbg_3col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
#endif

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

static LAYOUT_ORIENTATION(DBG_COL_CNT, 4, 3)
static LAYOUT_ORIENTATION(DBG_B_WIDTH, (LCD_W - 20) / 4, (LCD_W - 20) / 2)
static LAYOUT_VAL_SCALED(DBG_B_HEIGHT, 20)
static LAYOUT_ORIENTATION(CV_SCALE, 3, 4)
#define CV_WIDTH MAXTRACE
#define CV_HEIGHT (CV_SCALE * EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_SMALL + 1)
static LAYOUT_VAL_SCALED(RST_BTN_H, 24)

template <class T>
class DebugInfoNumber : public Window
{
 public:
  DebugInfoNumber(Window* parent, const rect_t& rect,
                  std::function<T()> numberHandler,
                  const char* prefix = nullptr) :
      Window(parent, rect)
  {
    coord_t prefixSize = 0;
    if (prefix != nullptr) {
      prefixSize = getTextWidth(prefix, 0, FONT(XS));
      new StaticText(this, {0, 1, prefixSize, rect.h}, prefix, COLOR_THEME_PRIMARY1_INDEX, FONT(XS));
    }
    new DynamicNumber<uint32_t>(this,
                                {prefixSize, 0, rect.w - prefixSize, rect.h},
                                numberHandler);
  }
};

class ThrottleCurveWindow : public Window
{
 public:
  ThrottleCurveWindow(Window* parent, const rect_t& rect) : Window(parent, rect)
  {
    lv_coord_t x, h = height();
    int i;

    axis[0] = {0, 0};
    axis[1] = {0, (lv_coord_t)(h - PAD_THREE)};
    axis[2] = {(lv_coord_t)width(), (lv_coord_t)(h - PAD_THREE)};
    auto axisLine = lv_line_create(lvobj);
    etx_obj_add_style(axisLine, styles->div_line_black, LV_PART_MAIN);
    lv_line_set_points(axisLine, axis, 3);

    for (x = 0, i = 0; x < width(); x += 6, i += 2) {
      ticks[i] = {x, (lv_coord_t)(h - PAD_SMALL - 1)};
      ticks[i + 1] = {x, h};
      auto tick = lv_line_create(lvobj);
      lv_line_set_points(tick, &ticks[i], 2);
      etx_obj_add_style(tick, styles->div_line_black, LV_PART_MAIN);
    }

    line = lv_line_create(lvobj);
    etx_obj_add_style(line, styles->graph_line, LV_PART_MAIN);
  }

  void checkEvents() override
  {
    Window::checkEvents();
    if (previousTraceWr != s_traceWr) {
      previousTraceWr = s_traceWr;

      graphSize = 0;
      uint16_t traceRd = s_traceWr > width() ? s_traceWr - width() : 0;
      for (lv_coord_t x = 0; x < width() && traceRd < s_traceWr;
           x += 1, traceRd += 1) {
        uint8_t h = s_traceBuf[traceRd % width()];
        lv_coord_t y = height() - PAD_THREE - CV_SCALE * h;
        graph[x] = {x, y};
        graphSize += 1;
      }
      lv_line_set_points(line, graph, graphSize);
    }
  }

 protected:
  unsigned previousTraceWr = 0;
  lv_point_t graph[MAXTRACE];
  lv_point_t axis[3];
  lv_point_t ticks[MAXTRACE * 2 / 6];
  lv_obj_t* line = nullptr;
  int16_t graphSize = 0;
};

StatisticsViewPage::StatisticsViewPage(const PageDef& pageDef) :
    PageGroupItem(pageDef)
{
}

void StatisticsViewPage::build(Window* window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);
  window->padLeft(PAD_SMALL);
  window->padRight(PAD_SMALL);

  FlexGridLayout grid(col_dsc, row_dsc, PAD_ZERO);

  auto line = window->newLine(grid);
  line->padAll(PAD_ZERO);

  // Session data
  new StaticText(line, rect_t{}, STR_SESSION);
  new DynamicText(
      line, rect_t{}, [] { return getTimerString(sessionTimer); });

  // Battery data
  new StaticText(line, rect_t{}, STR_BATT_LABEL);
  new DynamicText(
      line, rect_t{},
      [] { return getTimerString(g_eeGeneral.globalTimer + sessionTimer); });

  line = window->newLine(grid);
  line->padAll(PAD_ZERO);

  // Throttle
  new StaticText(line, rect_t{}, STR_THROTTLE_LABEL);
  new DynamicText(
      line, rect_t{}, [] { return getTimerString(s_timeCumThr); });

  // Throttle %  data
  new StaticText(line, rect_t{}, STR_THROTTLE_PERCENT_LABEL);
  new DynamicText(
      line, rect_t{}, [] { return getTimerString(s_timeCum16ThrP / 16); });

  line = window->newLine(grid);
  line->padAll(PAD_ZERO);

  // Timers
  new StaticText(line, rect_t{}, STR_TIMER_LABEL);
  for (int i = 0; i < TIMERS; i += 1)
    new DynamicText(
        line, rect_t{}, [=] { return getTimerString(timersStates[i].val); });

  line = window->newLine(grid);
  line->padAll(PAD_ZERO);
  line->padTop(3);

  // Throttle curve
  auto curve = new ThrottleCurveWindow(line, {0, 0, CV_WIDTH, CV_HEIGHT});

  lv_obj_set_grid_cell(curve->getLvObj(), LV_GRID_ALIGN_CENTER, 0, 4,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  line = window->newLine(grid);
  line->padAll(PAD_SMALL);

  // Reset
  auto btn = new TextButton(line, rect_t{0, 0, 0, RST_BTN_H}, STR_MENUTORESET,
                            [=]() -> uint8_t {
                              g_eeGeneral.globalTimer = 0;
                              storageDirty(EE_GENERAL);
                              sessionTimer = 0;
                              s_timeCumThr = 0;
                              s_timeCum16ThrP = 0;
                              s_traceWr = 0;
                              return 0;
                            });

  lv_obj_set_grid_cell(btn->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 4,
                       LV_GRID_ALIGN_START, 0, 1);
}

DebugViewPage::DebugViewPage(const PageDef& pageDef) :
    PageGroupItem(pageDef)
{
}

void DebugViewPage::build(Window* window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

#if LANDSCAPE
  FlexGridLayout grid(dbg_4col_dsc, row_dsc, PAD_ZERO);
  FlexGridLayout grid2(dbg_4col_dsc, row_dsc, PAD_ZERO);
#else
  FlexGridLayout grid(dbg_2col_dsc, row_dsc, PAD_ZERO);
  FlexGridLayout grid2(dbg_3col_dsc, row_dsc, PAD_ZERO);
#endif

  auto line = window->newLine(grid);
  line->padAll(PAD_TINY);

  // Mixer data
  static std::string pad_STR_MS = " " + std::string(STR_MS);
  static std::string pad_STR_PERIOD = std::string(STR_PERIOD) + ": ";
  new StaticText(line, rect_t{}, STR_TMIXMAXMS);
  new DynamicNumber<uint16_t>(
      line, rect_t{}, [] { return DURATION_MS_PREC2(maxMixerDuration); },
      COLOR_THEME_PRIMARY1_INDEX, PREC2, nullptr, pad_STR_MS.c_str());

  new DynamicNumber<uint16_t>(
          line, rect_t{}, [] { return getMixerSchedulerPeriod() / 1000; },
          COLOR_THEME_PRIMARY1_INDEX, 0, pad_STR_PERIOD.c_str(), pad_STR_MS.c_str());

  line = window->newLine(grid);
  line->padAll(PAD_TINY);

  // Free mem
  static std::string pad_STR_BYTES = " " + std::string(STR_BYTES);
  new StaticText(line, rect_t{}, STR_FREE_MEM_LABEL);
  new DynamicNumber<int32_t>(
      line, rect_t{}, [] { return availableMemory(); }, COLOR_THEME_PRIMARY1_INDEX, 0,
      nullptr, pad_STR_BYTES.c_str());

#if defined(LUA)
  line = window->newLine(grid);
  line->padAll(PAD_TINY);

  // LUA timing data
  new StaticText(line, rect_t{}, STR_LUA_SCRIPTS_LABEL);
#if PORTRAIT
  line = window->newLine(grid);
  line->padAll(PAD_ZERO);
  line->padLeft(PAD_LARGE);
#endif
  new DebugInfoNumber<uint16_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return 10 * maxLuaDuration; }, STR_DURATION_MS);
  new DebugInfoNumber<uint16_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return 10 * maxLuaInterval; }, STR_INTERVAL_MS);

  line = window->newLine(grid);
  line->padAll(PAD_ZERO);
#if PORTRAIT
  line->padLeft(PAD_LARGE);
#else
  grid.nextCell();
#endif

  // lUA memory data
  new DebugInfoNumber<uint32_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return luaGetMemUsed(lsScripts); }, STR_MEM_USED_SCRIPT);
  new DebugInfoNumber<uint32_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return luaGetMemUsed(lsWidgets); }, STR_MEM_USED_WIDGET);

#if PORTRAIT
  line = window->newLine(grid);
  line->padAll(PAD_ZERO);
  line->padLeft(PAD_LARGE);
#endif

  new DebugInfoNumber<uint32_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return luaExtraMemoryUsage; }, STR_MEM_USED_EXTRA);
#endif

  line = window->newLine(grid);
  line->padAll(PAD_TINY);

  // Stacks data
  new StaticText(line, rect_t{}, STR_FREE_STACK);
#if PORTRAIT
  line = window->newLine(grid2);
  line->padAll(PAD_ZERO);
  line->padLeft(PAD_LARGE);
#endif
  new DebugInfoNumber<uint32_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return task_get_stack_usage(&menusTaskId); }, STR_STACK_MENU);
  new DebugInfoNumber<uint32_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return task_get_stack_usage(&mixerTaskId); }, STR_STACK_MIX);
#if defined(AUDIO)
  new DebugInfoNumber<uint32_t>(
      line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
      [] { return task_get_stack_usage(&audioTaskId); }, STR_STACK_AUDIO);
#endif

#if defined(DEBUG_LATENCY)
  line = window->newLine(grid2);
  line->padAll(PAD_TINY);

  new StaticText(line, rect_t{}, STR_HEARTBEAT_LABEL);
  if (heartbeatCapture.valid)
    new DebugInfoNumber<uint16_t>(
        line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
        [] { return heartbeatCapture.count; }, COLOR_THEME_PRIMARY1);
  else
    new StaticText(window, grid.getFieldSlot(), "---");
#endif

#if defined(INTERNAL_GPS)
  if (serialGetModePort(UART_MODE_GPS) >= 0) {
    line = window->newLine(grid);
    line->padAll(PAD_TINY);

    new StaticText(line, rect_t{}, STR_INT_GPS_LABEL);
#if PORTRAIT
    line = window->newLine(grid2);
    line->padAll(PAD_ZERO);
    line->padLeft(PAD_LARGE);
#endif
    new DynamicText(
        line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
        [] {
          return std::string(gpsData.fix ? STR_GPS_FIX_YES : STR_GPS_FIX_NO);
        });
    new DebugInfoNumber<uint8_t>(
        line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
        [] { return gpsData.numSat; }, STR_GPS_SATS);
    new DebugInfoNumber<uint16_t>(
        line, rect_t{0, 0, DBG_B_WIDTH, DBG_B_HEIGHT},
        [] { return gpsData.hdop; }, STR_GPS_HDOP);
  }
#endif

  line = window->newLine(grid2);
  line->padAll(PAD_SMALL);

  // Reset
  auto btn = new TextButton(line, rect_t{0, 0, 0, RST_BTN_H}, STR_MENUTORESET,
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
