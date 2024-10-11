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

#include "edgetx.h"
#include "hal/adc_driver.h"
#include "hal/switch_driver.h"

#include "switches.h"
#include "input_mapping.h"

#define BIGSIZE       DBLSIZE
#if defined (PCBTARANIS)
  #define LBOX_CENTERX  (LCD_W/4 + 12)
  #define RBOX_CENTERX  (3*LCD_W/4 - 13)
#else
  #define LBOX_CENTERX  (LCD_W/4 + 10)
  #define RBOX_CENTERX  (3*LCD_W/4 - 10)
#endif
#define MODELNAME_X   (2*FW-2)
#define MODELNAME_Y   (0)
#define PHASE_X       (6*FW-2)
#define PHASE_Y       (2*FH)
#define PHASE_FLAGS   0
#define VBATT_X       (6*FW-1)
#define VBATT_Y       (2*FH)
#define VBATTUNIT_Y   (3*FH)
#define REBOOT_X      (2)
#define BAR_HEIGHT    (BOX_WIDTH-1l) // don't remove the l here to force 16bits maths on 9X
#define TRIM_LEN      21
#define TRIM_LH_X     (TRIM_LEN+4)
#define TRIM_LV_X     3
#define TRIM_RV_X     (LCD_W-4)
#define TRIM_RH_X     (LCD_W-TRIM_LEN-5)
#define TRIM_LH_NEG   (TRIM_LH_X+3*FW+1)
#define TRIM_LH_POS   (TRIM_LH_X-4*FW+3)
#define TRIM_RH_NEG   (TRIM_RH_X+3*FW+1)
#define TRIM_RH_POS   (TRIM_RH_X-4*FW+3)
#define RSSSI_X       (30)
#define RSSSI_Y       (31)
#define RSSI_MAX      105
#define CLOCK_X       53
#define CLOCK_Y       57

#if defined(ASTERISK) || (!defined(USE_WATCHDOG) && !defined(SIMU)) || defined(LOG_TELEMETRY) || \
    defined(LOG_BLUETOOTH) || defined(DEBUG_LATENCY)

static bool isAsteriskDisplayed() {
  return true;
}

#else

#include "hal/abnormal_reboot.h"

static bool isAsteriskDisplayed() {
  return UNEXPECTED_SHUTDOWN();
}
#endif

void drawExternalAntennaAndRSSI()
{
#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
  if (isModuleXJT(INTERNAL_MODULE) && isExternalAntennaEnabled()) {
    lcdDrawText(VBATT_X - 1, VBATT_Y + 8, "E", TINSIZE);
  }
#endif

  if (TELEMETRY_RSSI() > 0) {
    auto warningRSSI = g_model.rfAlarms.warning;
    int8_t value = TELEMETRY_RSSI() - warningRSSI;
    uint8_t step = (RSSI_MAX - warningRSSI) / 4;
    for (uint8_t i = 1; i < 5; i++) {
      if (value > step * (i - 1)) {
        lcdDrawFilledRect(RSSSI_X + i * 4, RSSSI_Y - 2 * i + 1, 3, 2 * i - 1, SOLID, 0);
      }
    }
  }
}

void drawPotsBars()
{
  uint8_t max_pots = adcGetMaxInputs(ADC_INPUT_FLEX);
  uint8_t offset = adcGetInputOffset(ADC_INPUT_FLEX);
  uint8_t configured_pots = 0;

  for (uint8_t i = 0; i < max_pots; i++) {
    if (IS_POT_SLIDER_AVAILABLE(i)) {
      configured_pots ++;
    }
  }

  uint8_t lines = configured_pots > 3 ? 2 : 1;
  uint8_t cols =  configured_pots > 4 ? 3 : configured_pots % 2 ? 3 : 2;
  coord_t xstart =  LCD_W / 2 - (cols % 2 ? 5 : 3);

  for (uint8_t i = 0; i < max_pots; i++) {
    if (IS_POT_SLIDER_AVAILABLE(i)) {
      coord_t x = xstart + (i % cols) * 5;
      coord_t y = lines == 1 ? (LCD_H - 8) : i >= cols ? (LCD_H - 8) : (LCD_H - 8 - BAR_HEIGHT / 2 - 1);
      auto v = calibratedAnalogs[offset + i] + RESX;
      uint8_t len = (v * (BAR_HEIGHT - (lines - 1)) / (RESX * 2 * lines)) + 1l;
      V_BAR(x, y, len);
    }
  }
}

void doMainScreenGraphics()
{
#if defined(SURFACE_RADIO)
  drawWheel(RBOX_CENTERX, calibratedAnalogs[ADC_MAIN_ST]);
  drawThrottle(LBOX_CENTERX, calibratedAnalogs[ADC_MAIN_TH]);
#else
  int16_t calibStickVert = calibratedAnalogs[ADC_MAIN_LV];
  if (g_model.throttleReversed &&
      inputMappingConvertMode(ADC_MAIN_LV) == inputMappingGetThrottle()) {
    calibStickVert = -calibStickVert;
  }
  drawStick(LBOX_CENTERX, calibratedAnalogs[ADC_MAIN_LH], calibStickVert);

  calibStickVert = calibratedAnalogs[ADC_MAIN_RV];
  if (g_model.throttleReversed &&
      inputMappingConvertMode(ADC_MAIN_RV) == inputMappingGetThrottle()) {
    calibStickVert = -calibStickVert;
  }
  drawStick(RBOX_CENTERX, calibratedAnalogs[ADC_MAIN_RH], calibStickVert);
#endif

  drawPotsBars();
}

void displayTrims(uint8_t phase)
{
#if defined(SURFACE_RADIO)
  static uint8_t x[] = {TRIM_RH_X, TRIM_LH_X, TRIM_RV_X, TRIM_LV_X, TRIM_LV_X};
  static uint8_t vert[] = {0, 0, 1, 1, 1};
#else
  static uint8_t x[] = {TRIM_LH_X, TRIM_LV_X, TRIM_RV_X, TRIM_RH_X, TRIM_LH_X, TRIM_LV_X, TRIM_RH_X, TRIM_RV_X};
  static uint8_t vert[] = {0, 1, 1, 0, 0, 1, 0, 1};
#endif

  bool squareMarker = (keysGetMaxTrims() <= 4);

  for (uint8_t i = 0; i < keysGetMaxTrims(); i++) {

    if (getRawTrimValue(phase, i).mode == TRIM_MODE_NONE || getRawTrimValue(phase, i).mode == TRIM_MODE_3POS)
      continue;

    coord_t ym;
    uint8_t stickIndex = inputMappingConvertMode(i);
    coord_t xm = x[stickIndex];
    uint8_t att = ROUND;
    int16_t val = getTrimValue(phase, i);

    int16_t dir = val;
    bool exttrim = false;
    if (val < TRIM_MIN || val > TRIM_MAX) {
      exttrim = true;
    }
    val = (val * TRIM_LEN) / TRIM_MAX;
    if (val < -TRIM_LEN) {
      val = -TRIM_LEN;
    }
    else if (val > TRIM_LEN) {
      val = TRIM_LEN;
    }

    uint8_t nx, ny;
    LcdFlags nFlg = TINSIZE;

    if (vert[i]) {
      ym = 31;
      if (!getPixel(xm, ym))
        lcdDrawSolidVerticalLine(xm, ym - TRIM_LEN, TRIM_LEN * 2 + 1);
      if (squareMarker) {
        if (i != 2 || !g_model.thrTrim) {
          lcdDrawSolidVerticalLine(xm - 1, ym - 1, 3);
          lcdDrawSolidVerticalLine(xm + 1, ym - 1, 3);
        }
        ym -= val;
        lcdDrawFilledRect(xm - 3, ym - 3, 7, 7, SOLID, att | ERASE);
        if (dir >= 0) {
          lcdDrawSolidHorizontalLine(xm - 1, ym - 1, 3);
        }
        if (dir <= 0) {
          lcdDrawSolidHorizontalLine(xm - 1, ym + 1, 3);
        }
        if (exttrim) {
          lcdDrawSolidHorizontalLine(xm - 1, ym, 3);
        }
      }
      else {
        ym -= val;
        if ((i > 3 && xm < LCD_W / 2) || (i < 4 && xm > LCD_W / 2) ) {
          lcdDrawSolidVerticalLine(xm - 1, ym, 1);
          lcdDrawSolidVerticalLine(xm - 2, ym - 1, 3);
          lcdDrawSolidVerticalLine(xm - 3, ym - 2, 5);
        }
        else {
          lcdDrawSolidVerticalLine(xm + 1, ym, 1);
          lcdDrawSolidVerticalLine(xm + 2, ym - 1, 3);
          lcdDrawSolidVerticalLine(xm + 3, ym - 2, 5);
        }
      }
      nx = dir > 0 ? 12 : 52;
      ny = xm - 2;
      nFlg |= VERTICAL;
    }
    else {
      ym = 60;
      if (!getPixel(xm, ym))
        lcdDrawSolidHorizontalLine(xm - TRIM_LEN, ym, TRIM_LEN * 2 + 1);
      if (squareMarker) {
        lcdDrawSolidHorizontalLine(xm - 1, ym - 1, 3);
        lcdDrawSolidHorizontalLine(xm - 1, ym + 1, 3);
        xm += val;
        lcdDrawFilledRect(xm - 3, ym - 3, 7, 7, SOLID, att | ERASE);
        if (dir >= 0) {
          lcdDrawSolidVerticalLine(xm + 1, ym - 1, 3);
        }
        if (dir <= 0) {
          lcdDrawSolidVerticalLine(xm - 1, ym - 1, 3);
        }
        if (exttrim) {
          lcdDrawSolidVerticalLine(xm, ym - 1, 3);
        }
      }
      else {
        xm += val;
        if (i > 3) {
          lcdDrawSolidHorizontalLine(xm, ym + 1, 1);
          lcdDrawSolidHorizontalLine(xm - 1, ym + 2, 3);
          lcdDrawSolidHorizontalLine(xm - 2, ym + 3, 5);
        }
        else {
          lcdDrawSolidHorizontalLine(xm, ym - 1, 1);
          lcdDrawSolidHorizontalLine(xm - 1, ym - 2, 3);
          lcdDrawSolidHorizontalLine(xm - 2, ym - 3, 5);
        }
      }
      nx = xm < LCD_W / 2 ? (dir > 0 ? TRIM_LH_POS : TRIM_LH_NEG)
                          : (dir > 0 ? TRIM_RH_POS : TRIM_RH_NEG);
      ny = ym - 2;
    }
    if (g_model.displayTrims != DISPLAY_TRIMS_NEVER && dir != 0 && i < 4) {
      if (g_model.displayTrims == DISPLAY_TRIMS_ALWAYS ||
          (trimsDisplayTimer > 0 && (trimsDisplayMask & (1 << i)))) {
        lcdDrawNumber(nx, ny, -abs(dir), nFlg | (dir < 0 ? RIGHT : 0));
      }
    }
    if (squareMarker)
      lcdDrawSquare(xm - 3, ym - 3, 7, att);
  }
}

void displayBattVoltage()
{
#if defined(BATTGRAPH)
  putsVBat(VBATT_X - 8, VBATT_Y + 1, RIGHT);
  lcdDrawSolidFilledRect(VBATT_X - 25, VBATT_Y + 9, 21, 5);
  lcdDrawSolidVerticalLine(VBATT_X - 4, VBATT_Y + 10, 3);
  uint8_t count = GET_TXBATT_BARS(20);

  #if defined(HARDWARE_CHARGING_STATE)
  if (IS_CHARGING_STATE()) {
    count = (get_tmr10ms() & 127u) * count / 128;
  }
#endif
  for (uint8_t i = 0; i < count; i += 2) {
#if defined(USB_CHARGER)
    if ((i >= count - 2) && usbChargerLed() && BLINK_ON_PHASE)  // Blink last segment on charge
      continue;
#endif
    lcdDrawSolidVerticalLine(VBATT_X - 24 + i, VBATT_Y + 10, 3);
  }
  if (!IS_TXBATT_WARNING() || BLINK_ON_PHASE)
    lcdDrawSolidFilledRect(VBATT_X - 26, VBATT_Y, 24, 15);
#else
  LcdFlags att = (IS_TXBATT_WARNING() ? BLINK|INVERS : 0) | BIGSIZE;
  putsVBat(VBATT_X-1, VBATT_Y, att|NO_UNIT);
  lcdDrawChar(VBATT_X, VBATTUNIT_Y, 'V');
#endif
}

#define displayVoltageOrAlarm() displayBattVoltage()

#define EVT_KEY_CONTEXT_MENU           EVT_KEY_LONG(KEY_ENTER)

#if defined(RADIO_T8) || defined(RADIO_COMMANDO8)
#define EVT_KEY_PREVIOUS_VIEW          EVT_KEY_BREAK(KEY_PAGEUP)
#define EVT_KEY_NEXT_VIEW              EVT_KEY_BREAK(KEY_PAGEDN)
#define EVT_KEY_NEXT_PAGE              EVT_KEY_BREAK(KEY_PLUS)
#define EVT_KEY_PREVIOUS_PAGE          EVT_KEY_BREAK(KEY_MINUS)
#define EVT_KEY_MODEL_MENU             EVT_KEY_BREAK(KEY_MODEL)
#define EVT_KEY_GENERAL_MENU           EVT_KEY_BREAK(KEY_SYS)
#define EVT_KEY_TELEMETRY              EVT_KEY_LONG(KEY_PAGEUP)
#elif defined(NAVIGATION_X7_TX12)
#define EVT_KEY_PREVIOUS_VIEW          EVT_KEY_BREAK(KEY_PAGEUP)
#define EVT_KEY_NEXT_VIEW              EVT_KEY_BREAK(KEY_PAGEDN)
#define EVT_KEY_NEXT_PAGE              EVT_ROTARY_RIGHT
#define EVT_KEY_PREVIOUS_PAGE          EVT_ROTARY_LEFT
#define EVT_KEY_MODEL_MENU             EVT_KEY_BREAK(KEY_MODEL)
#define EVT_KEY_GENERAL_MENU           EVT_KEY_BREAK(KEY_SYS)
#define EVT_KEY_TELEMETRY              EVT_KEY_BREAK(KEY_TELE)
#elif defined(NAVIGATION_X7) || defined(NAVIGATION_TBS)
#define EVT_KEY_NEXT_VIEW              EVT_KEY_BREAK(KEY_PAGEDN)
#define EVT_KEY_NEXT_PAGE              EVT_ROTARY_RIGHT
#define EVT_KEY_PREVIOUS_PAGE          EVT_ROTARY_LEFT
#define EVT_KEY_MODEL_MENU             EVT_KEY_BREAK(KEY_MENU)
#define EVT_KEY_GENERAL_MENU           EVT_KEY_LONG(KEY_MENU)
#define EVT_KEY_TELEMETRY              EVT_KEY_BREAK(KEY_PAGEUP)
#else
#define EVT_KEY_PREVIOUS_VIEW          EVT_KEY_BREAK(KEY_UP)
#define EVT_KEY_NEXT_VIEW              EVT_KEY_BREAK(KEY_DOWN)
#define EVT_KEY_NEXT_PAGE              EVT_KEY_BREAK(KEY_RIGHT)
#define EVT_KEY_PREVIOUS_PAGE          EVT_KEY_BREAK(KEY_LEFT)
#define EVT_KEY_MODEL_MENU             EVT_KEY_LONG(KEY_RIGHT)
#define EVT_KEY_GENERAL_MENU           EVT_KEY_LONG(KEY_LEFT)
#define EVT_KEY_TELEMETRY              EVT_KEY_LONG(KEY_DOWN)
#define EVT_KEY_STATISTICS             EVT_KEY_LONG(KEY_UP)
#endif

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
    POPUP_MENU_START(onMainViewMenu, 5, STR_RESET_FLIGHT, STR_RESET_TIMER1, STR_RESET_TIMER2, STR_RESET_TIMER3, STR_RESET_TELEMETRY);
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

void drawSmallSwitch(coord_t x, coord_t y, int width, unsigned int index)
{
  if (SWITCH_EXISTS(index)) {
    int val = getValue(MIXSRC_FIRST_SWITCH + index);

    if (val >= 0) {
      lcdDrawSolidHorizontalLine(x, y, width);
      lcdDrawSolidHorizontalLine(x, y + 2, width);
      y += 4;
      if (val > 0) {
        lcdDrawSolidHorizontalLine(x, y, width);
        lcdDrawSolidHorizontalLine(x, y + 2, width);
        y += 4;
      }
    }

    lcdDrawChar(width == 5 ? x + 1 : x, y, 'A' + index, SMLSIZE);
    y += 7;

    if (val <= 0) {
      lcdDrawSolidHorizontalLine(x, y, width);
      lcdDrawSolidHorizontalLine(x, y + 2, width);
      if (val < 0) {
        lcdDrawSolidHorizontalLine(x, y + 4, width);
        lcdDrawSolidHorizontalLine(x, y + 6, width);
      }
    }
  }
}

void menuMainView(event_t event)
{
  uint8_t view = g_eeGeneral.view;
  uint8_t view_base = view & 0x0f;

  switch (event) {
    case EVT_ENTRY:
      killEvents(KEY_EXIT);
      killEvents(KEY_UP);
      killEvents(KEY_DOWN);
      break;

      /* TODO if timer2 is OFF, it's possible to use this timer2 as in er9x...
      case EVT_KEY_BREAK(KEY_MENU):
        if (view_base == VIEW_TIMER2) {
          Timer2_running = !Timer2_running;
          AUDIO_KEY_PRESS();
        }
      break;
      */
    case EVT_KEY_NEXT_PAGE:
    case EVT_KEY_PREVIOUS_PAGE:
      if (view_base == VIEW_INPUTS)
        g_eeGeneral.view ^= ALTERNATE_VIEW;
      else
        g_eeGeneral.view = (g_eeGeneral.view + (4 * ALTERNATE_VIEW) + ((event == EVT_KEY_PREVIOUS_PAGE) ? -ALTERNATE_VIEW : ALTERNATE_VIEW)) % (4 * ALTERNATE_VIEW);
      break;

    case EVT_KEY_CONTEXT_MENU:
      if (modelHasNotes()) {
        POPUP_MENU_ADD_ITEM(STR_VIEW_NOTES);
      }

      POPUP_MENU_START(onMainViewMenu, 3, STR_RESET_SUBMENU, STR_STATISTICS, STR_ABOUT_US);
      break;

#if defined(EVT_KEY_LAST_MENU)
    case EVT_KEY_LAST_MENU:
      pushMenu(lastPopMenu());
      break;
#endif

    case EVT_KEY_MODEL_MENU:
      pushMenu(menuModelSelect);
      break;

    case EVT_KEY_GENERAL_MENU:
      pushMenu(menuTabGeneral[0].menuFunc);
      break;

#if defined(EVT_KEY_PREVIOUS_VIEW)
      // TODO try to split those 2 cases on 9X
    case EVT_KEY_PREVIOUS_VIEW:
    case EVT_KEY_NEXT_VIEW:
      // TODO try to split those 2 cases on 9X
      g_eeGeneral.view = (event == EVT_KEY_PREVIOUS_VIEW ? (view_base == VIEW_COUNT - 1 ? 0 : view_base + 1) : (view_base == 0 ? VIEW_COUNT - 1 : view_base -
                                                                                                                                                  1));
      storageDirty(EE_GENERAL);
      break;
#else
    case EVT_KEY_NEXT_VIEW:
      g_eeGeneral.view = (view_base == 0 ? VIEW_COUNT - 1 : view_base - 1);
      storageDirty(EE_GENERAL);
      break;
#endif

#if defined(EVT_KEY_STATISTICS)
    case EVT_KEY_STATISTICS:
      chainMenu(menuStatisticsView);
      break;
#endif

    case EVT_KEY_TELEMETRY:
      chainMenu(menuViewTelemetry);
      break;


    case EVT_KEY_FIRST(KEY_EXIT):
#if defined(GVARS)
      if (gvarDisplayTimer > 0) {
        gvarDisplayTimer = 0;
      }
#endif
      break;
  }

  switch (view_base) {
    case VIEW_CHAN_MONITOR:
      menuChannelsViewCommon(event);
      break;

    case VIEW_OUTPUTS_VALUES:
    case VIEW_OUTPUTS_BARS:
      // scroll bar
      lcdDrawHorizontalLine(38, 34, 54, DOTTED);
      lcdDrawSolidHorizontalLine(38 + (g_eeGeneral.view / ALTERNATE_VIEW) * 13, 34, 13, SOLID);
      for (uint8_t i = 0; i < 8; i++) {
        uint8_t x0, y0;
        uint8_t chan = 8 * (g_eeGeneral.view / ALTERNATE_VIEW) + i;
        int16_t val = channelOutputs[chan];

        if (view_base == VIEW_OUTPUTS_VALUES) {
          x0 = (i % 4 * 9 + 3) * FW / 2;
#if LCD_H >= 96
          y0 = i / 4 * FH * 2 + 50;
#else
          y0 = i / 4 * FH + 40;
#endif
          if (g_eeGeneral.ppmunit == PPM_US) {
            lcdDrawNumber(x0 + 4 * FW, y0, PPM_CH_CENTER(chan) + val / 2, RIGHT);
          } else if (g_eeGeneral.ppmunit == PPM_PERCENT_PREC1) {
            lcdDrawNumber(x0 + 4 * FW, y0, calcRESXto1000(val), RIGHT | PREC1);
          } else {
            lcdDrawNumber(x0+4*FW , y0, calcRESXto1000(val)/10, RIGHT); // G: Don't like the decimal part*
          }
        }
        else {
          constexpr coord_t WBAR2 = (50 / 2);
          x0 = i < 4 ? LCD_W / 4 + 2 : LCD_W * 3 / 4 - 2;
#if LCD_H >= 96
          y0 = 45 + (i % 4) * 10;
#else
          y0 = 38 + (i % 4) * 5;
#endif

          const uint16_t lim = (g_model.extendedLimits ? (512 * (long) LIMIT_EXT_PERCENT / 100) : 512) * 2;
          int8_t len = (abs(val) * WBAR2 + lim / 2) / lim;

          if (len > WBAR2)
            len = WBAR2; // prevent bars from going over the end - comment for debugging
          lcdDrawHorizontalLine(x0 - WBAR2, y0, WBAR2 * 2 + 1, DOTTED);
          lcdDrawSolidVerticalLine(x0, y0 - 2, 5);
          if (val > 0)
            x0 += 1;
          else
            x0 -= len;
          lcdDrawSolidHorizontalLine(x0, y0 + 1, len);
          lcdDrawSolidHorizontalLine(x0, y0 - 1, len);
        }
      }
      break;

    case VIEW_TIMER2:
      drawTimerWithMode(87, 5 * FH, 1, RIGHT | DBLSIZE);
      break;

    case VIEW_INPUTS:
      if (view == VIEW_INPUTS) {
        // Sticks + Pots
        doMainScreenGraphics();
        
        // Switches
        // -> 2 columns: one for each side
        // -> 4 slots on each side (3 normal / 1 small)
        uint8_t switches = switchGetMaxSwitches();
        uint8_t configured_switches = 0;

        for (uint8_t i = 0; i < switches; i++) {
          if (SWITCH_EXISTS(i) && !switchIsFlex(i)) {
            configured_switches ++;
          }
        }

        if (configured_switches < 7) {
          for (int i = 0; i < switches; ++i) {
            if (SWITCH_EXISTS(i) && !switchIsFlex(i)) {
              auto switch_display = switchGetDisplayPosition(i);
              coord_t x = switch_display.col == 0 ? 3 * FW + 3 : 18 * FW + 1;
              coord_t y = 33 + switch_display.row * FH;
              getvalue_t val = getValue(MIXSRC_FIRST_SWITCH + i);
              if (val == 0) x -= 1;
              getvalue_t sw =
                  ((val < 0) ? 3 * i + 1
                              : ((val == 0) ? 3 * i + 2 : 3 * i + 3));
              drawSwitch(x, y, sw, CENTERED, false);
            }
          }
        }
        else {
          for (int i = 0; i < switches; ++i) {
            if (SWITCH_EXISTS(i) && !switchIsFlex(i)) {
              auto switch_display = switchGetDisplayPosition(i);
              coord_t x = (switch_display.col == 0 ? 8 : 96) + switch_display.row * 5;
              if (configured_switches < 9) x += 3;
              drawSmallSwitch(x, 5 * FH + 1, 4, i);
            }
          }
        }
      }
      else {
        // Logical Switches
        uint8_t index = 0;
        uint8_t y = LCD_H - 20;
        for (uint8_t line = 0; line < 2; line++) {
          for (uint8_t column = 0; column < MAX_LOGICAL_SWITCHES / 2; column++) {
            int8_t len = getSwitch(SWSRC_FIRST_LOGICAL_SWITCH + index) ? 10 : 1;
            uint8_t x = (16 + 3 * column);
            lcdDrawSolidVerticalLine(x - 1, y - len, len);
            lcdDrawSolidVerticalLine(x, y - len, len);
            index++;
          }
          y += 12;
        }
      }
      break;
  }

  if (view_base != VIEW_CHAN_MONITOR) {
    // Flight Mode Name
    uint8_t mode = mixerCurrentFlightMode;
    lcdDrawSizedText(PHASE_X, PHASE_Y, g_model.flightModeData[mode].name, sizeof(g_model.flightModeData[mode].name), PHASE_FLAGS);

    // Model Name
    drawModelName(MODELNAME_X, MODELNAME_Y, g_model.header.name, g_eeGeneral.currModel, BIGSIZE);

    // Main Voltage (or alarm if any)
    displayVoltageOrAlarm();

    // Timer 1
    drawTimerWithMode(125, 2 * FH, 0, RIGHT | DBLSIZE);

    // Trims sliders
    displayTrims(mode);

    // RSSI gauge / external antenna
    drawExternalAntennaAndRSSI();

    // And ! in case of unexpected shutdown
    if (isAsteriskDisplayed()) {
      lcdDrawChar(REBOOT_X, 1, '!', INVERS|BLINK);
    }
  }

#if defined(GVARS)
  if (gvarDisplayTimer > 0) {
    gvarDisplayTimer--;
    warningText = STR_GLOBAL_VAR;
    drawMessageBox(warningText);
    lcdDrawSizedText(16, 5 * FH, g_model.gvars[gvarLastChanged].name, LEN_GVAR_NAME, 0);
    lcdDrawText(16 + 6 * FW, 5 * FH, "[", BOLD);
    drawGVarValue(lcdLastRightPos, 5 * FH, gvarLastChanged, GVAR_VALUE(gvarLastChanged, getGVarFlightMode(mixerCurrentFlightMode, gvarLastChanged)),
                  LEFT | BOLD);
    if (g_model.gvars[gvarLastChanged].unit) {
      lcdDrawText(lcdLastRightPos, 5 * FH, "%", BOLD);
    }
    lcdDrawText(lcdLastRightPos, 5 * FH, "]", BOLD);
    warningText = nullptr;
  }
#endif

#if defined(DSM2)
  if (moduleState[0].mode == MODULE_MODE_BIND) {
    // Issue 98
    lcdDrawText(15 * FW, 0, "BIND", 0);
  }
#if defined(RTCLOCK)
  else if (view_base != VIEW_CHAN_MONITOR && rtcIsValid()) {
    drawRtcTime(CLOCK_X, CLOCK_Y, LEFT|TIMEBLINK);
  }
#endif
#else
#if defined(RTCLOCK)
  if (view_base != VIEW_CHAN_MONITOR && rtcIsValid()) {
    drawRtcTime(CLOCK_X, CLOCK_Y, LEFT|TIMEBLINK);
  }
#endif
#endif
}

#undef EVT_KEY_CONTEXT_MENU
#undef EVT_KEY_PREVIOUS_VIEW
#undef EVT_KEY_NEXT_VIEW
#undef EVT_KEY_NEXT_PAGE
#undef EVT_KEY_PREVIOUS_PAGE
#undef EVT_KEY_MODEL_MENU
#undef EVT_KEY_GENERAL_MENU
#undef EVT_KEY_LAST_MENU
#undef EVT_KEY_TELEMETRY
#undef EVT_KEY_STATISTICS
