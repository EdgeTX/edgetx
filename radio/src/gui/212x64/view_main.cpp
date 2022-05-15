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

#define BIGSIZE       MIDSIZE
#define LBOX_CENTERX  (BOX_WIDTH/2 + 16)
#define RBOX_CENTERX  (LCD_W-LBOX_CENTERX-1)
#define MODELNAME_X   (15)
#define MODELNAME_Y   (11)
#define VBATT_X       (MODELNAME_X+26)
#define VBATT_Y       (FH+3)
#define VBATTUNIT_Y   VBATT_Y
#define BITMAP_X      ((LCD_W-64)/2)
#define BITMAP_Y      (LCD_H/2)
#define PHASE_X       BITMAP_X
#define PHASE_Y       (3*FH)
#define PHASE_FLAGS   (0)
#define TIMERS_X      145
#define TIMERS_Y      20
#define TIMERS_H      25
#define TIMERS_R      193
#define REBOOT_X      (LCD_W-FW)
#define VSWITCH_X(i)  (((i>=MAX_LOGICAL_SWITCHES*3/4) ? BITMAP_X+28 : ((i>=MAX_LOGICAL_SWITCHES/2) ? BITMAP_X+25 : ((i>=MAX_LOGICAL_SWITCHES/4) ? 21 : 18))) + 3*i)
#define VSWITCH_Y     (LCD_H-9)
#define TRIM_LH_X     (32+9)
#define TRIM_LV_X     10
#define TRIM_RV_X     (LCD_W-11)
#define TRIM_RH_X     (LCD_W-32-9)

#define TRIM_LEN 27
#define MARKER_WIDTH  5

const unsigned char logo_taranis[]  = {
#include "logo.lbm"
};

const unsigned char icons[]  = {
#include "icons.lbm"
};

#define ICON_RSSI     0, 9
#define ICON_SPEAKER0 9, 8
#define ICON_SPEAKER1 17, 8
#define ICON_SPEAKER2 25, 8
#define ICON_SPEAKER3 33, 8
#define ICON_SD       41, 11
#define ICON_LOGS     51, 11
#define ICON_TRAINER  61, 11
#define ICON_TRAINEE  71, 11
#define ICON_USB      81, 11
#define ICON_REBOOT   91, 11
#define ICON_ALTITUDE 102, 9

void doMainScreenGraphics()
{
  int16_t calibStickVert = calibratedAnalogs[CONVERT_MODE(1)];
  if (g_model.throttleReversed && CONVERT_MODE(1) == THR_STICK)
    calibStickVert = -calibStickVert;
  drawStick(LBOX_CENTERX, calibratedAnalogs[CONVERT_MODE(0)], calibStickVert);

  calibStickVert = calibratedAnalogs[CONVERT_MODE(2)];
  if (g_model.throttleReversed && CONVERT_MODE(2) == THR_STICK)
    calibStickVert = -calibStickVert;
  drawStick(RBOX_CENTERX, calibratedAnalogs[CONVERT_MODE(3)], calibStickVert);
}

void displayTrims(uint8_t phase)
{
  for (unsigned int i=0; i<NUM_STICKS; i++) {
    coord_t x[4] = { TRIM_LH_X, TRIM_LV_X, TRIM_RV_X, TRIM_RH_X };
    uint8_t vert[4] = { 0, 1, 1, 0 };
    coord_t xm, ym;
    unsigned int stickIndex = CONVERT_MODE(i);
    xm = x[stickIndex];

    uint32_t att = ROUND;
    int32_t trim = getTrimValue(phase, i);
    int32_t val = trim;
    bool exttrim = false;

    if(getRawTrimValue(phase, i).mode == TRIM_MODE_NONE)
      continue;

    if (val < TRIM_MIN || val > TRIM_MAX) {
      exttrim = true;
    }
    if (val < -(TRIM_LEN+1)*4) {
      val = -(TRIM_LEN+1);
    }
    else if (val > (TRIM_LEN+1)*4) {
      val = TRIM_LEN+1;
    }
    else {
      val /= 4;
    }

    if (vert[i]) {
      ym = 31;
      lcdDrawSolidVerticalLine(xm, ym-TRIM_LEN, TRIM_LEN*2);
      if (i!=2 || !g_model.thrTrim) {
        lcdDrawSolidVerticalLine(xm-1, ym-1,  3);
        lcdDrawSolidVerticalLine(xm+1, ym-1,  3);
      }
      ym -= val;
      lcdDrawFilledRect(xm-3, ym-3, 7, 7, SOLID, att|ERASE);
      if (trim >= 0) {
        lcdDrawSolidHorizontalLine(xm-1, ym-1,  3);
      }
      if (trim <= 0) {
        lcdDrawSolidHorizontalLine(xm-1, ym+1,  3);
      }
      if (exttrim) {
        lcdDrawSolidHorizontalLine(xm-1, ym,  3);
      }
      if (g_model.displayTrims != DISPLAY_TRIMS_NEVER && trim != 0) {
        if (g_model.displayTrims == DISPLAY_TRIMS_ALWAYS || (trimsDisplayTimer > 0 && (trimsDisplayMask & (1<<i)))) {
          lcdDrawNumber(trim>0 ? 22 : 54, xm-2, -abs(trim), RIGHT|TINSIZE|VERTICAL);
        }
      }
    }
    else {
      ym = 60;
      lcdDrawSolidHorizontalLine(xm-TRIM_LEN, ym, TRIM_LEN*2);
      lcdDrawSolidHorizontalLine(xm-1, ym-1,  3);
      lcdDrawSolidHorizontalLine(xm-1, ym+1,  3);
      xm += val;
      lcdDrawFilledRect(xm-3, ym-3, 7, 7, SOLID, att|ERASE);
      if (trim >= 0) {
        lcdDrawSolidVerticalLine(xm+1, ym-1,  3);
      }
      if (trim <= 0) {
        lcdDrawSolidVerticalLine(xm-1, ym-1,  3);
      }
      if (exttrim) {
        lcdDrawSolidVerticalLine(xm, ym-1,  3);
      }
      if (g_model.displayTrims != DISPLAY_TRIMS_NEVER && trim != 0) {
        if (g_model.displayTrims == DISPLAY_TRIMS_ALWAYS || (trimsDisplayTimer > 0 && (trimsDisplayMask & (1<<i)))) {
          lcdDrawNumber((stickIndex==0 ? TRIM_LH_X : TRIM_RH_X)+(trim>0 ? -11 : 20), ym-2, -abs(trim), RIGHT|TINSIZE);
        }
      }
    }
    lcdDrawSquare(xm-3, ym-3, 7, att);
  }
}

void drawSliders()
{
  for (uint8_t i = NUM_STICKS; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
#if defined(PCBX9E)
    if (i < SLIDER1)
      continue;  // TODO change and display more values
    coord_t x = ((i==SLIDER1 || i==SLIDER3) ? 3 : LCD_W-5);
    int8_t y = (i<SLIDER3 ? LCD_H/2+1 : 1);
#else
    if (i == POT3)
      continue;
    coord_t x = ((i==POT1 || i==SLIDER1) ? 3 : LCD_W-5);
    int8_t y = (i>=SLIDER1 ? LCD_H/2+1 : 1);
#endif
    lcdDrawSolidVerticalLine(x, y, LCD_H/2-2);
    lcdDrawSolidVerticalLine(x+1, y, LCD_H/2-2);
    y += LCD_H / 2 - 4;
    y -= ((calibratedAnalogs[i]+RESX)*(LCD_H/2-4)/(RESX*2));  // calculate once per loop
    lcdDrawSolidVerticalLine(x-1, y, 2);
    lcdDrawSolidVerticalLine(x+2, y, 2);
  }
}

#define BAR_X        14
#define BAR_Y        1
#define BAR_W        184
#define BAR_H        9
#define BAR_NOTIFS_X BAR_X+133
#define BAR_VOLUME_X BAR_X+147
#define BAR_TIME_X   BAR_X+159

void displayTopBarGauge(coord_t x, int count, bool blinking=false)
{
  if (!blinking || BLINK_ON_PHASE)
    lcdDrawFilledRect(x+1, BAR_Y+2, 11, 5, SOLID, ERASE);
  for (int i=0; i<count; i+=2)
    lcdDrawSolidVerticalLine(x+2+i, BAR_Y+3, 3);
}

#define LCD_NOTIF_ICON(x, icon) \
 lcdDrawRleBitmap(x, BAR_Y, icons, icon); \
 lcdDrawSolidHorizontalLine(x, BAR_Y+8, 11)

void displayTopBar()
{
  uint8_t batt_icon_x;
  uint8_t altitude_icon_x;

  /* Tx voltage */
  putsVBat(BAR_X+2, BAR_Y+1, LEFT);
  batt_icon_x = lcdLastRightPos;
  lcdDrawRect(batt_icon_x+FW, BAR_Y+1, 13, 7);
  lcdDrawSolidVerticalLine(batt_icon_x+FW+13, BAR_Y+2, 5);

  if (TELEMETRY_STREAMING()) {
    /* RSSI */
    LCD_ICON(batt_icon_x+3*FW+3, BAR_Y, ICON_RSSI);
    lcdDrawRect(batt_icon_x+5*FW, BAR_Y+1, 13, 7);

    /* Rx voltage */
    altitude_icon_x = batt_icon_x+7*FW+3;
    if (g_model.voltsSource) {
      uint8_t item = g_model.voltsSource-1;
      if (item < MAX_TELEMETRY_SENSORS) {
        TelemetryItem & voltsItem = telemetryItems[item];
        if (voltsItem.isAvailable()) {
          drawSensorCustomValue(batt_icon_x+7*FW+2, BAR_Y+1, item, voltsItem.value, LEFT);
          altitude_icon_x = lcdLastRightPos+1;
        }
      }
    }

    /* Altitude */
    if (g_model.altitudeSource && !IS_FAI_ENABLED()) {
      uint8_t item = g_model.altitudeSource - 1;
      if (item < MAX_TELEMETRY_SENSORS) {
        TelemetryItem & altitudeItem = telemetryItems[item];
        if (altitudeItem.isAvailable()) {
          LCD_ICON(altitude_icon_x, BAR_Y, ICON_ALTITUDE);
          int32_t value = altitudeItem.value / g_model.telemetrySensors[item].getPrecDivisor();
          drawValueWithUnit(altitude_icon_x+2*FW-1, BAR_Y+1, value, g_model.telemetrySensors[item].unit, LEFT);
        }
      }
    }
  }

  /* Notifs icons */
  coord_t x = BAR_NOTIFS_X;
  if (isAsteriskDisplayed()) {
    LCD_NOTIF_ICON(x, ICON_REBOOT);
    x -= 12;
  }

  if (usbPlugged()) {
    LCD_NOTIF_ICON(x, ICON_USB);
    x -= 12;
  }

  if (SLAVE_MODE()) {
    if (TRAINER_CONNECTED()) {
      LCD_NOTIF_ICON(x, ICON_TRAINEE);
      x -= 12;
    }
  }
  else if (IS_TRAINER_INPUT_VALID()) {
    LCD_NOTIF_ICON(x, ICON_TRAINER);
    x -= 12;
  }

  if (isFunctionActive(FUNCTION_LOGS)) {
    LCD_NOTIF_ICON(x, ICON_LOGS);
    x -= 12;
  }

  /* Audio volume */
  if (requiredSpeakerVolume == 0 || g_eeGeneral.beepMode == e_mode_quiet)
    LCD_ICON(BAR_VOLUME_X, BAR_Y, ICON_SPEAKER0);
  else if (requiredSpeakerVolume <= 6)
    LCD_ICON(BAR_VOLUME_X, BAR_Y, ICON_SPEAKER1);
  else if (requiredSpeakerVolume <= 12)
    LCD_ICON(BAR_VOLUME_X, BAR_Y, ICON_SPEAKER2);
  else if (requiredSpeakerVolume <= 18)
    LCD_ICON(BAR_VOLUME_X, BAR_Y, ICON_SPEAKER2);
  else
    LCD_ICON(BAR_VOLUME_X, BAR_Y, ICON_SPEAKER3);

  /* RTC time */
  drawRtcTime(BAR_TIME_X, BAR_Y+1, LEFT|TIMEBLINK);

  /* The background */
  lcdDrawFilledRect(BAR_X, BAR_Y, BAR_W, BAR_H, SOLID, FILL_WHITE|GREY(12)|ROUND);

  /* The inside of the Batt gauge */
  displayTopBarGauge(batt_icon_x+FW, GET_TXBATT_BARS(10), IS_TXBATT_WARNING());

  /* The inside of the RSSI gauge */
  if (TELEMETRY_RSSI() > 0) {
    displayTopBarGauge(batt_icon_x+5*FW, TELEMETRY_RSSI() / 10, TELEMETRY_RSSI() < g_model.rssiAlarms.getWarningRssi());
  }
}

void displayTimers()
{
  // Main and Second timer
  for (unsigned int i=0; i<2; i++) {
    if (g_model.timers[i].mode) {
      TimerState & timerState = timersStates[i];
      TimerData & timerData = g_model.timers[i];
      uint8_t y = TIMERS_Y + i*TIMERS_H;
      if (ZLEN(timerData.name) > 0) {
        lcdDrawSizedText(TIMERS_X, y-7, timerData.name, LEN_TIMER_NAME, ZCHAR|SMLSIZE);
      }
      else {
        lcdDrawTextAtIndex(TIMERS_X, y-7, STR_VTMRMODES, timerData.mode, SMLSIZE);
      }
      int val = timerState.val;
      if (timerData.start && timerData.showElapsed &&
          timerData.start != timerState.val)
        val = (int)timerData.start - (int)timerState.val;
      drawTimer(TIMERS_X, y, val, TIMEHOUR|MIDSIZE|LEFT, TIMEHOUR|MIDSIZE|LEFT);
      if (timerData.persistent) {
        lcdDrawChar(TIMERS_R, y+1, 'P', SMLSIZE);
      }
      if (timerState.val < 0) {
        if (BLINK_ON_PHASE) {
          lcdDrawFilledRect(TIMERS_X-7, y-8, 60, 20);
        }
      }
    }
  }
}

void menuMainViewChannelsMonitor(event_t event)
{
  switch(event) {
    case EVT_KEY_BREAK(KEY_PAGE):
    case EVT_KEY_BREAK(KEY_EXIT):
      chainMenu(menuMainView);
      event = 0;
      break;
  }

  return menuChannelsView(event);
}

void onMainViewMenu(const char * result)
{
  if (result == STR_RESET_TIMER1) {
    timerReset(0);
  }
  else if (result == STR_RESET_TIMER2) {
    timerReset(1);
  }
#if TIMERS > 2
  else if (result == STR_RESET_TIMER3) {
    timerReset(2);
  }
#endif
  else if (result == STR_VIEW_NOTES) {
    pushModelNotes();
  }
  else if (result == STR_RESET_SUBMENU) {
    POPUP_MENU_ADD_ITEM(STR_RESET_FLIGHT);
    POPUP_MENU_ADD_ITEM(STR_RESET_TIMER1);
    POPUP_MENU_ADD_ITEM(STR_RESET_TIMER2);
    POPUP_MENU_ADD_ITEM(STR_RESET_TIMER3);
    POPUP_MENU_ADD_ITEM(STR_RESET_TELEMETRY);
    POPUP_MENU_START(onMainViewMenu);
  }
  else if (result == STR_RESET_TELEMETRY) {
    telemetryReset();
  }
  else if (result == STR_RESET_FLIGHT) {
    flightReset();
  }
  else if (result == STR_STATISTICS) {
    chainMenu(menuStatisticsView);
  }
  else if (result == STR_ABOUT_US) {
    chainMenu(menuAboutView);
  }
}

void displaySwitch(coord_t x, coord_t y, int width, unsigned int index)
{
  if (SWITCH_EXISTS(index)) {
    int val = getValue(MIXSRC_FIRST_SWITCH+index);

    if (val >= 0) {
      lcdDrawSolidHorizontalLine(x, y, width);
      lcdDrawSolidHorizontalLine(x, y+2, width);
      y += 4;
      if (val > 0) {
        lcdDrawSolidHorizontalLine(x, y, width);
        lcdDrawSolidHorizontalLine(x, y+2, width);
        y += 4;
      }
    }

    lcdDrawChar(width==5 ? x+1 : x, y, 'A'+index, TINSIZE);
    y += 6;

    if (val <= 0) {
      lcdDrawSolidHorizontalLine(x, y, width);
      lcdDrawSolidHorizontalLine(x, y+2, width);
      if (val < 0) {
        lcdDrawSolidHorizontalLine(x, y+4, width);
        lcdDrawSolidHorizontalLine(x, y+6, width);
      }
    }
  }
}

int getSwitchCount()
{
  int count = 0;
  for (int i=0; i<NUM_SWITCHES; ++i) {
    if (SWITCH_EXISTS(i)) {
      ++count;
    }
  }
  return count;
}

void menuMainView(event_t event)
{
  static bool secondPage = false;

  switch(event) {
    case EVT_ENTRY:
      killEvents(KEY_EXIT);
      killEvents(KEY_UP);
      killEvents(KEY_DOWN);
      // no break

    case EVT_ENTRY_UP:
      LOAD_MODEL_BITMAP();
      break;

    case EVT_KEY_LONG(KEY_ENTER):
      killEvents(event);
      if (modelHasNotes()) {
        POPUP_MENU_ADD_ITEM(STR_VIEW_NOTES);
      }
      POPUP_MENU_ADD_ITEM(STR_RESET_SUBMENU);
      POPUP_MENU_ADD_ITEM(STR_STATISTICS);
      POPUP_MENU_ADD_ITEM(STR_ABOUT_US);
      POPUP_MENU_START(onMainViewMenu);
      break;

    case EVT_KEY_BREAK(KEY_MENU):
      pushMenu(menuModelSelect);
      break;

    case EVT_KEY_LONG(KEY_MENU):
      pushMenu(menuTabGeneral[0]);
      killEvents(event);
      break;

    case EVT_KEY_BREAK(KEY_PAGE):
      storageDirty(EE_MODEL);
      g_model.view += 1;
      if (g_model.view >= VIEW_COUNT) {
        g_model.view = 0;
        chainMenu(menuMainViewChannelsMonitor);
      }
      break;

    case EVT_KEY_LONG(KEY_PAGE):
      chainMenu(menuViewTelemetry);
      killEvents(event);
      break;

    case EVT_KEY_FIRST(KEY_EXIT):
#if defined(GVARS)
      if (gvarDisplayTimer > 0) {
        gvarDisplayTimer = 0;
      }
#endif
      break;

    case EVT_KEY_FIRST(KEY_RIGHT):
    case EVT_KEY_FIRST(KEY_LEFT):
#if defined(ROTARY_ENCODER_NAVIGATION)
    case EVT_ROTARY_LEFT:
    case EVT_ROTARY_RIGHT:
#endif
      secondPage = !secondPage;
      break;
  }

  // Flight Mode Name
  int mode = mixerCurrentFlightMode;
  lcdDrawSizedText(PHASE_X, PHASE_Y, g_model.flightModeData[mode].name, sizeof(g_model.flightModeData[mode].name), ZCHAR|PHASE_FLAGS);

  // Model Name
  drawModelName(MODELNAME_X, MODELNAME_Y, g_model.header.name, g_eeGeneral.currModel, BIGSIZE);

  // Trims sliders
  displayTrims(mode);

  // Top bar
  displayTopBar();

  // Sliders (Pots / Sliders)
  drawSliders();

  lcdDrawBitmap(BITMAP_X, BITMAP_Y, modelBitmap);

  // Switches
  if (getSwitchCount() > 8) {
    for (int i=0; i<NUM_SWITCHES; ++i) {
      div_t qr = div(i, 9);
      if (g_model.view == VIEW_INPUTS) {
        div_t qr2 = div(qr.rem, 5);
        if (i >= 14) qr2.rem += 1;
        const coord_t x[4] = { 50, 142 };
        const coord_t y[4] = { 25, 42, 25, 42 };
        displaySwitch(x[qr.quot]+qr2.rem*4, y[qr2.quot], 3, i);
      }
      else {
        displaySwitch(17+qr.rem*6, 25+qr.quot*17, 5, i);
      }
    }
  }
  else {
    int index = 0;
    for (int i=0; i<NUM_SWITCHES; ++i) {
      if (SWITCH_EXISTS(i)) {
        getvalue_t val = getValue(MIXSRC_FIRST_SWITCH+i);
        getvalue_t sw = ((val < 0) ? 3*i+1 : ((val == 0) ? 3*i+2 : 3*i+3));
        drawSwitch((g_model.view == VIEW_INPUTS) ? (index<4 ? 8*FW+1 : 23*FW+2) : (index<4 ? 3*FW+1 : 8*FW-2), (index%4)*FH+3*FH, sw, 0, false);
        index++;
      }
    }
  }

  if (g_model.view == VIEW_TIMERS) {
    displayTimers();
  }
  else if (g_model.view == VIEW_INPUTS) {
    // Sticks
    doMainScreenGraphics();
  }
  else {
    // Logical Switches
    int sw = (secondPage && MAX_LOGICAL_SWITCHES > 32 ? 32 : 0);
    const int end = sw + 32;
    uint8_t y = 6*FH-1;
    lcdDrawText(TRIM_RH_X - TRIM_LEN/2 + 1, y, "LS");
    lcdDrawNumber(lcdLastRightPos + 1, y, sw + 1, LEFT|LEADING0, 2);
    lcdDrawText(lcdLastRightPos, y, "-");
    lcdDrawNumber(lcdLastRightPos, y, end, LEFT);
    for ( ; sw < end; ++sw) {
      const div_t qr = div(sw + 32 - end, 10);
      const uint8_t x = TRIM_RH_X - TRIM_LEN + qr.rem*5 + (qr.rem >= 5 ? 3 : 0);
      y = 13 + 11 * qr.quot;
      LogicalSwitchData * cs = lswAddress(sw);
      if (cs->func == LS_FUNC_NONE) {
        lcdDrawSolidHorizontalLine(x, y+6, 4);
        lcdDrawSolidHorizontalLine(x, y+7, 4);
      }
      else if (getSwitch(SWSRC_SW1+sw)) {
        lcdDrawFilledRect(x, y, 4, 8);
      }
      else {
        lcdDrawRect(x, y, 4, 8);
      }
    }
  }

#if defined(GVARS)
  if (gvarDisplayTimer > 0) {
    gvarDisplayTimer--;
    lcdDrawFilledRect(BITMAP_X, BITMAP_Y, 64, 32, SOLID, ERASE);
    lcdDrawRect(BITMAP_X, BITMAP_Y, 64, 32);
    drawStringWithIndex(BITMAP_X+FW, BITMAP_Y+FH-1, STR_GV, gvarLastChanged+1);
    lcdDrawSizedText(BITMAP_X+4*FW+FW/2, BITMAP_Y+FH-1, g_model.gvars[gvarLastChanged].name, LEN_GVAR_NAME, ZCHAR);
    lcdDrawText(BITMAP_X+FW, BITMAP_Y+2*FH+3, "[", BOLD);
    drawGVarValue(BITMAP_X+2*FW, BITMAP_Y+2*FH+3, gvarLastChanged, GVAR_VALUE(gvarLastChanged, getGVarFlightMode(mixerCurrentFlightMode, gvarLastChanged)), LEFT|BOLD);
    lcdDrawText(lcdLastRightPos, BITMAP_Y+2*FH+3, "]", BOLD);
  }
#endif
}
