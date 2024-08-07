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

#include "hal/adc_driver.h"
#include "edgetx.h"
#include "tasks.h"

#define STATS_1ST_COLUMN               FW/2
#define STATS_2ND_COLUMN               12*FW+FW/2
#define STATS_3RD_COLUMN               24*FW+FW/2
#define STATS_LABEL_WIDTH              4*FW

void menuStatisticsView(event_t event)
{
  title(STR_MENUSTAT);

  switch(event) {
    case EVT_KEY_BREAK(KEY_PLUS):
    case EVT_KEY_BREAK(KEY_PAGEDN):
      chainMenu(menuStatisticsDebug);
      break;

    case EVT_KEY_BREAK(KEY_MINUS):
    case EVT_KEY_BREAK(KEY_PAGEUP):
#if defined(DEBUG_TRACE_BUFFER)
      chainMenu(menuTraceBuffer);
#else
      chainMenu(menuStatisticsDebug2);
#endif
      break;

    case EVT_KEY_LONG(KEY_MENU):  // historical
    case EVT_KEY_LONG(KEY_ENTER):
      killEvents(event);
      g_eeGeneral.globalTimer = 0;
      storageDirty(EE_GENERAL);
      sessionTimer = 0;
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      chainMenu(menuMainView);
      break;
  }

  // Session and Total timers
  lcdDrawText(STATS_1ST_COLUMN, FH*1+1, "SES", BOLD);
  drawTimer(STATS_1ST_COLUMN + STATS_LABEL_WIDTH, FH*1+1, sessionTimer);
  lcdDrawText(STATS_1ST_COLUMN, FH*2+1, "TOT", BOLD);
  drawTimer(STATS_1ST_COLUMN + STATS_LABEL_WIDTH, FH*2+1, g_eeGeneral.globalTimer + sessionTimer, TIMEHOUR, 0);

  // Throttle special timers
  lcdDrawText(STATS_2ND_COLUMN, FH*0+1, "THR", BOLD);
  drawTimer(STATS_2ND_COLUMN + STATS_LABEL_WIDTH, FH*0+1, s_timeCumThr);
  lcdDrawText(STATS_2ND_COLUMN, FH*1+1, "TH%", BOLD);
  drawTimer(STATS_2ND_COLUMN + STATS_LABEL_WIDTH, FH*1+1, s_timeCum16ThrP/16);

  // Timers
  for (int i=0; i<TIMERS; i++) {
    drawStringWithIndex(STATS_3RD_COLUMN, FH*i+1, "TM", i+1, BOLD);
    if (timersStates[i].val > 3600)
      drawTimer(STATS_3RD_COLUMN + STATS_LABEL_WIDTH, FH*i+1, timersStates[i].val, TIMEHOUR, 0);
    else
      drawTimer(STATS_3RD_COLUMN + STATS_LABEL_WIDTH, FH*i+1, timersStates[i].val);
  }

#if defined(THRTRACE)
  const coord_t x = 5;
  const coord_t y = 60;
  lcdDrawSolidHorizontalLine(x-3, y, MAXTRACE+3+3);
  lcdDrawSolidVerticalLine(x, y-32, 32+3);
  for (coord_t i=0; i<MAXTRACE; i+=6) {
    lcdDrawSolidVerticalLine(x+i+6, y-1, 3);
  }

  uint16_t traceRd = s_traceWr > MAXTRACE ? s_traceWr - MAXTRACE : 0;
  for (coord_t i=1; i<=MAXTRACE && traceRd<s_traceWr; i++, traceRd++) {
    uint8_t h = s_traceBuf[traceRd % MAXTRACE];
    lcdDrawSolidVerticalLine(x+i, y-h, h);
  }
#endif
}

#define MENU_DEBUG_COL1_OFS   (11*FW-2)
#define MENU_DEBUG_ROW1       (2*FH-3)
#define MENU_DEBUG_ROW2       (3*FH-2)
#define MENU_DEBUG_ROW3       (4*FH-1)
#define MENU_DEBUG_ROW4       (5*FH)
#define MENU_DEBUG_ROW5       (6*FH)

void menuStatisticsDebug(event_t event)
{
  title(STR_MENUDEBUG);

#if defined(WATCHDOG_TEST)
  if (warningResult) {
    warningResult = 0;
    // do a user requested watchdog test
    TRACE("Performing watchdog test");
    pulsesStop();
  }
#endif

  switch (event) {
    case EVT_KEY_LONG(KEY_ENTER):
      killEvents(event);
      g_eeGeneral.globalTimer = 0;
      sessionTimer = 0;
      storageDirty(EE_GENERAL);
      break;

    case EVT_KEY_BREAK(KEY_ENTER):
#if defined(LUA)
      maxLuaInterval = 0;
      maxLuaDuration = 0;
#endif
      maxMixerDuration  = 0;
      break;

    case EVT_KEY_BREAK(KEY_PLUS):
    case EVT_KEY_BREAK(KEY_PAGEDN):
      disableVBatBridge();
      chainMenu(menuStatisticsDebug2);
      break;

    case EVT_KEY_BREAK(KEY_MINUS):
    case EVT_KEY_BREAK(KEY_PAGEUP):
      disableVBatBridge();
      chainMenu(menuStatisticsView);
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      disableVBatBridge();
      chainMenu(menuMainView);
      break;

#if defined(WATCHDOG_TEST)
    case EVT_KEY_LONG(KEY_MENU):
      {
        killEvents(event);
        POPUP_CONFIRMATION("Test the watchdog?", nullptr);
        const char * w = "The radio will reset!";
        SET_WARNING_INFO(w, strlen(w), 0);
      }
      break;
#endif
  }

  uint8_t y = FH + 1;

#if defined(STM32)
  lcdDrawTextAlignedLeft(y, STR_FREE_MEM_LABEL);
  lcdDrawNumber(MENU_DEBUG_COL1_OFS, y, availableMemory(), LEFT);
  lcdDrawText(lcdLastRightPos+FW, y, STR_BYTES);
  y += FH;
#endif

#if defined(LUA)
  lcdDrawTextAlignedLeft(y, STR_LUA_SCRIPTS_LABEL);
  lcdDrawText(MENU_DEBUG_COL1_OFS, y+1, STR_DURATION_MS, SMLSIZE);
  lcdDrawNumber(lcdLastRightPos, y, 10*maxLuaDuration, LEFT);
  lcdDrawText(lcdLastRightPos+2, y+1, STR_INTERVAL_MS, SMLSIZE);
  lcdDrawNumber(lcdLastRightPos, y, 10*maxLuaInterval, LEFT);
  y += FH;
#endif

  lcdDrawTextAlignedLeft(y, STR_TMIXMAXMS);
  lcdDrawNumber(MENU_DEBUG_COL1_OFS, y, DURATION_MS_PREC2(maxMixerDuration), PREC2|LEFT);
  lcdDrawText(lcdLastRightPos, y, STR_MS);
  y += FH;

  lcdDrawTextAlignedLeft(y, STR_FREE_STACK);
  lcdDrawText(MENU_DEBUG_COL1_OFS, y+1, "[M]", SMLSIZE);
  lcdDrawNumber(lcdLastRightPos, y, menusStack.available(), LEFT);
  lcdDrawText(lcdLastRightPos+2, y+1, "[X]", SMLSIZE);
  lcdDrawNumber(lcdLastRightPos, y, mixerStack.available(), LEFT);
#if defined(AUDIO)
  lcdDrawText(lcdLastRightPos+2, y+1, "[A]", SMLSIZE);
  lcdDrawNumber(lcdLastRightPos, y, audioStack.available(), LEFT);
#endif
  lcdDrawText(lcdLastRightPos+2, y+1, "[I]", SMLSIZE);
  lcdDrawNumber(lcdLastRightPos, y, mainStackAvailable(), LEFT);
  y += FH;

#if defined(DEBUG_LATENCY)
  lcdDrawTextAlignedLeft(y, STR_HEARTBEAT_LABEL);
  if (heartbeatCapture.valid)
    lcdDrawNumber(MENU_DEBUG_COL1_OFS, y, heartbeatCapture.count, LEFT);
  else
    lcdDrawText(MENU_DEBUG_COL1_OFS, y, "---");
  y += FH;
#endif

  lcdDrawText(LCD_W/2, 7*FH+1, STR_MENUTORESET, CENTERED);
  lcdInvertLastLine();
}

void menuStatisticsDebug2(event_t event)
{
  title(STR_MENUDEBUG);

  switch(event) {
    case EVT_KEY_BREAK(KEY_PLUS):
    case EVT_KEY_BREAK(KEY_PAGEDN):
#if defined(DEBUG_TRACE_BUFFER)
      chainMenu(menuTraceBuffer);
#else
      chainMenu(menuStatisticsView);
#endif
      return;

    case EVT_KEY_BREAK(KEY_MINUS):
    case EVT_KEY_BREAK(KEY_PAGEUP):
      chainMenu(menuStatisticsDebug);
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      chainMenu(menuMainView);
      break;
  }

  // UART statistics
  // lcdDrawTextAlignedLeft(MENU_DEBUG_ROW1, "Tlm RX Err");
  // lcdDrawNumber(MENU_DEBUG_COL1_OFS, MENU_DEBUG_ROW1, telemetryErrors, RIGHT);

  lcdDrawText(LCD_W/2, 7*FH+1, STR_MENUTORESET, CENTERED);
  lcdInvertLastLine();
}

#if defined(DEBUG_TRACE_BUFFER)
void menuTraceBuffer(event_t event)
{
  switch(event)
  {
    case EVT_KEY_LONG(KEY_ENTER):
      killEvents(event);
      dumpTraceBuffer();
      break;

    case EVT_KEY_BREAK(KEY_MINUS):
    case EVT_KEY_BREAK(KEY_PAGEUP):
      chainMenu(menuStatisticsDebug2);
      break;

    case EVT_KEY_BREAK(KEY_PLUS):
    case EVT_KEY_BREAK(KEY_PAGEDN):
      chainMenu(menuStatisticsView);
      return;
  }

  SIMPLE_SUBMENU("Trace Buffer", TRACE_BUFFER_LEN);
  /* RTC time */
  drawRtcTime(LCD_W+1, 0, LEFT|TIMEBLINK);

  uint8_t y = 0;
  uint8_t k = 0;
  int8_t sub = menuVerticalPosition;

  lcdDrawChar(0, FH, '#');
  lcdDrawText(4*FW, FH, TR_TIME);
  lcdDrawText(14*FW, FH, "Event");
  lcdDrawText(20*FW, FH, "Data");

  for (uint8_t i=0; i<LCD_LINES-2; i++) {
    y = 1 + (i+2)*FH;
    k = i+menuVerticalOffset;

    //item
    lcdDrawNumber(0, y, k, LEFT | (sub==k ? INVERS : 0));

    const struct TraceElement * te = getTraceElement(k);
    if (te) {
      //time
      int32_t tme = te->time % SECS_PER_DAY;
      drawTimer(4*FW, y, tme, TIMEHOUR|LEFT, TIMEHOUR|LEFT);
      //event
      lcdDrawNumber(14*FW, y, te->event, LEADING0|LEFT, 3);
      //data
      lcdDrawSizedText  (20*FW, y, "0x", 2);
      lcdDrawHexNumber(22*FW-2, y, (uint16_t)(te->data >> 16));
      lcdDrawHexNumber(25*FW, y, (uint16_t)(te->data & 0xFFFF));
    }

  }


}
#endif //#if defined(DEBUG_TRACE_BUFFER)
