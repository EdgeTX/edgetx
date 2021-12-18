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

#if defined(HARDWARE_NO_TRIMS)
struct {
  int8_t preStickIdx = -1;
  int8_t curStickIdx = -1;
  tmr10ms_t preEnterTime;
  bool preEnterValid = false;
} trimSelection;
#endif

#define BIGSIZE       DBLSIZE
#if defined (PCBTARANIS)
  #define LBOX_CENTERX  (LCD_W/4 + 14)
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
#define REBOOT_X      (20*FW-3)
#define BAR_HEIGHT    (BOX_WIDTH-1l) // don't remove the l here to force 16bits maths on 9X
#define TRIM_LH_X     (LCD_W*1/4+2)
#define TRIM_LV_X     3
#define TRIM_RV_X     (LCD_W-4)
#define TRIM_RH_X     (LCD_W*3/4-2)
#define TRIM_LH_NEG   (TRIM_LH_X+1*FW)
#define TRIM_LH_POS   (TRIM_LH_X-4*FW)
#define TRIM_RH_NEG   (TRIM_RH_X+1*FW)
#define TRIM_RH_POS   (TRIM_RH_X-4*FW)
#define RSSSI_X       (30)
#define RSSSI_Y       (31)
#define RSSI_MAX      105
#define TRIM_LEN      23

void drawExternalAntennaAndRSSI()
{
#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
  if (isModuleXJT(INTERNAL_MODULE) && isExternalAntennaEnabled()) {
    lcdDrawText(VBATT_X - 1, VBATT_Y + 8, "E", TINSIZE);
  }
#endif

  if (TELEMETRY_RSSI() > 0) {
    auto warningRSSI = g_model.rssiAlarms.getWarningRssi();
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
  // Optimization by Mike Blandford
  for (uint8_t x = LCD_W / 2 - (NUM_POTS + NUM_SLIDERS - 1) * 5 / 2, i = NUM_STICKS; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; x += 5, i++) {
    if (IS_POT_SLIDER_AVAILABLE(i)) {
      uint8_t len = ((calibratedAnalogs[i] + RESX) * BAR_HEIGHT / (RESX * 2)) + 1l;  // calculate once per loop
      V_BAR(x, LCD_H - 8, len);
    }
  }
}

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
#if defined(HARDWARE_POT1)
  drawPotsBars();
#endif
}

#if defined(HARDWARE_NO_TRIMS)
void doMainScreenGraphics(uint8_t views, uint32_t ptr)
{
  int16_t * calibStickValPtr = nullptr;
  int16_t calibStickVert = 0;

  if (ptr)
    calibStickValPtr = (int16_t *)(ptr);
  else
    calibStickValPtr = calibratedAnalogs;
  calibStickVert = calibStickValPtr[CONVERT_MODE(1)];

  if (views & MAINSCREEN_GRAPHICS_STICKS) {
    if (g_model.throttleReversed && CONVERT_MODE(1) == THR_STICK)
      calibStickVert = -calibStickVert;
    drawStick(LBOX_CENTERX, calibStickValPtr[CONVERT_MODE(0)], calibStickVert);

    calibStickVert = calibStickValPtr[CONVERT_MODE(2)];
    if (g_model.throttleReversed && CONVERT_MODE(2) == THR_STICK)
      calibStickVert = -calibStickVert;
    drawStick(RBOX_CENTERX, calibStickValPtr[CONVERT_MODE(3)], calibStickVert);
  }

  if (views & MAINSCREEN_GRAPHICS_POTS) {
    drawPotsBars();
  }
}
#endif

void displayTrims(uint8_t phase)
{
  for (uint8_t i = 0; i < 4; i++) {
    static coord_t x[4] = {TRIM_LH_X, TRIM_LV_X, TRIM_RV_X, TRIM_RH_X};
    static uint8_t vert[4] = {0, 1, 1, 0};
    coord_t xm, ym;
    uint8_t stickIndex = CONVERT_MODE(i);
    xm = x[stickIndex];
    uint8_t att = ROUND;
    int16_t val = getTrimValue(phase, i);

    if (getRawTrimValue(phase, i).mode == TRIM_MODE_NONE)
      continue;

    int16_t dir = val;
    bool exttrim = false;
    if (val < TRIM_MIN || val > TRIM_MAX) {
      exttrim = true;
    }
    if (val < -(TRIM_LEN + 1) * 4) {
      val = -(TRIM_LEN + 1);
    }
    else if (val > (TRIM_LEN + 1) * 4) {
      val = TRIM_LEN + 1;
    }
    else {
      val /= 4;
    }

    if (vert[i]) {
#if defined(HARDWARE_NO_TRIMS)
      ym = 61;
      if (trimSelection.curStickIdx == i) {
        lcdDrawSolidVerticalLine(xm, ym - TRIM_LEN, TRIM_LEN * 2);
        if (i != 2 || !g_model.thrTrim) {
          lcdDrawSolidVerticalLine(xm - 1, ym - TRIM_LEN, TRIM_LEN * 2);
          lcdDrawSolidVerticalLine(xm + 1, ym - TRIM_LEN, TRIM_LEN * 2);
        }
      }
      else {
        lcdDrawSolidVerticalLine(xm, ym - TRIM_LEN, TRIM_LEN * 2);
        if (i != 2 || !g_model.thrTrim) {
          lcdDrawSolidVerticalLine(xm - 1, ym - 1, 3);
          lcdDrawSolidVerticalLine(xm + 1, ym - 1, 3);
        }
      }
#else
      ym = 31;
      lcdDrawSolidVerticalLine(xm, ym - TRIM_LEN, TRIM_LEN * 2);
      if (i != 2 || !g_model.thrTrim) {
        lcdDrawSolidVerticalLine(xm - 1, ym - 1, 3);
        lcdDrawSolidVerticalLine(xm + 1, ym - 1, 3);
      }
#endif
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
      if (g_model.displayTrims != DISPLAY_TRIMS_NEVER && dir != 0) {
        if (g_model.displayTrims == DISPLAY_TRIMS_ALWAYS || (trimsDisplayTimer > 0 && (trimsDisplayMask & (1 << i)))) {
          lcdDrawNumber(dir > 0 ? 12 : 40, xm - 2, -abs(dir), TINSIZE | VERTICAL);
        }
      }
    }
    else {
#if defined(HARDWARE_NO_TRIMS)
      ym = 92;
      if (trimSelection.curStickIdx == i) {
        lcdDrawSolidHorizontalLine(xm - TRIM_LEN, ym,   TRIM_LEN * 2);
        lcdDrawSolidHorizontalLine(xm - TRIM_LEN, ym - 1, TRIM_LEN * 2);
        lcdDrawSolidHorizontalLine(xm - TRIM_LEN, ym + 1, TRIM_LEN * 2);
      }
      else {
        lcdDrawSolidHorizontalLine(xm - TRIM_LEN, ym, TRIM_LEN * 2);
        lcdDrawSolidHorizontalLine(xm - 1, ym - 1, 3);
        lcdDrawSolidHorizontalLine(xm - 1, ym + 1, 3);
      }
#else
      ym = 60;
      lcdDrawSolidHorizontalLine(xm - TRIM_LEN, ym, TRIM_LEN * 2);
      lcdDrawSolidHorizontalLine(xm - 1, ym - 1, 3);
      lcdDrawSolidHorizontalLine(xm - 1, ym + 1, 3);
#endif
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
      if (g_model.displayTrims != DISPLAY_TRIMS_NEVER && dir != 0) {
        if (g_model.displayTrims == DISPLAY_TRIMS_ALWAYS || (trimsDisplayTimer > 0 && (trimsDisplayMask & (1 << i)))) {
          lcdDrawNumber((stickIndex == 0 ? (dir > 0 ? TRIM_LH_POS : TRIM_LH_NEG) : (dir > 0 ? TRIM_RH_POS : TRIM_RH_NEG)), ym - 2, -abs(dir), TINSIZE);
        }
      }
    }
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

#if defined(PCBSKY9X)
void displayVoltageOrAlarm()
{
  if (g_eeGeneral.mAhWarn && (g_eeGeneral.mAhUsed + Current_used * (488 + g_eeGeneral.txCurrentCalibration)/8192/36) / 500 >= g_eeGeneral.mAhWarn) {
    drawValueWithUnit(7*FW-1, 2*FH, (g_eeGeneral.mAhUsed + Current_used*(488 + g_eeGeneral.txCurrentCalibration)/8192/36)/10, UNIT_MAH,
                      BLINK|INVERS|DBLSIZE|RIGHT);
  }
  else {
    displayBattVoltage();
  }
}
#else
#define displayVoltageOrAlarm() displayBattVoltage()
#endif

#if defined(RADIO_T8)
#define EVT_KEY_CONTEXT_MENU           EVT_KEY_LONG(KEY_ENTER)
#define EVT_KEY_PREVIOUS_VIEW          EVT_KEY_BREAK(KEY_PAGEUP)
#define EVT_KEY_NEXT_VIEW              EVT_KEY_FIRST(KEY_PAGEDN)
#define EVT_KEY_NEXT_PAGE              EVT_KEY_FIRST(KEY_PLUS)
#define EVT_KEY_PREVIOUS_PAGE          EVT_KEY_FIRST(KEY_MINUS)
#define EVT_KEY_MODEL_MENU             EVT_KEY_LONG(KEY_MODEL)
#define EVT_KEY_GENERAL_MENU           EVT_KEY_LONG(KEY_SYS)
#define EVT_KEY_TELEMETRY              EVT_KEY_LONG(KEY_PAGEUP)
#elif defined(NAVIGATION_X7_TX12)
#define EVT_KEY_CONTEXT_MENU           EVT_KEY_LONG(KEY_ENTER)
#define EVT_KEY_PREVIOUS_VIEW          EVT_KEY_FIRST(KEY_PAGEUP)
#define EVT_KEY_NEXT_VIEW              EVT_KEY_FIRST(KEY_PAGEDN)
#define EVT_KEY_NEXT_PAGE              EVT_ROTARY_RIGHT
#define EVT_KEY_PREVIOUS_PAGE          EVT_ROTARY_LEFT
#define EVT_KEY_MODEL_MENU             EVT_KEY_LONG(KEY_MODEL)
#define EVT_KEY_GENERAL_MENU           EVT_KEY_LONG(KEY_SYS)
#define EVT_KEY_TELEMETRY              EVT_KEY_FIRST(KEY_TELE)
#elif defined(NAVIGATION_X7)  || defined(NAVIGATION_TBS)
#define EVT_KEY_CONTEXT_MENU           EVT_KEY_LONG(KEY_ENTER)
#define EVT_KEY_NEXT_VIEW              EVT_KEY_BREAK(KEY_PAGE)
#define EVT_KEY_NEXT_PAGE              EVT_ROTARY_RIGHT
#define EVT_KEY_PREVIOUS_PAGE          EVT_ROTARY_LEFT
#define EVT_KEY_MODEL_MENU             EVT_KEY_BREAK(KEY_MENU)
#define EVT_KEY_GENERAL_MENU           EVT_KEY_LONG(KEY_MENU)
#define EVT_KEY_TELEMETRY              EVT_KEY_LONG(KEY_PAGE)
#elif defined(NAVIGATION_XLITE)
#define EVT_KEY_CONTEXT_MENU           EVT_KEY_LONG(KEY_ENTER)
#define EVT_KEY_PREVIOUS_VIEW          EVT_KEY_BREAK(KEY_UP)
#define EVT_KEY_NEXT_VIEW              EVT_KEY_BREAK(KEY_DOWN)
#define EVT_KEY_NEXT_PAGE              EVT_KEY_BREAK(KEY_RIGHT)
#define EVT_KEY_PREVIOUS_PAGE          EVT_KEY_BREAK(KEY_LEFT)
#define EVT_KEY_MODEL_MENU             EVT_KEY_LONG(KEY_RIGHT)
#define EVT_KEY_GENERAL_MENU           EVT_KEY_LONG(KEY_LEFT)
#define EVT_KEY_TELEMETRY              EVT_KEY_LONG(KEY_DOWN)
#define EVT_KEY_STATISTICS             EVT_KEY_LONG(KEY_UP)
#else
#define EVT_KEY_CONTEXT_MENU           EVT_KEY_BREAK(KEY_MENU)
#define EVT_KEY_PREVIOUS_VIEW          EVT_KEY_BREAK(KEY_UP)
#define EVT_KEY_NEXT_VIEW              EVT_KEY_BREAK(KEY_DOWN)
#define EVT_KEY_NEXT_PAGE              EVT_KEY_BREAK(KEY_RIGHT)
#define EVT_KEY_PREVIOUS_PAGE          EVT_KEY_BREAK(KEY_LEFT)
#define EVT_KEY_MODEL_MENU             EVT_KEY_LONG(KEY_RIGHT)
#define EVT_KEY_GENERAL_MENU           EVT_KEY_LONG(KEY_LEFT)
#define EVT_KEY_LAST_MENU              EVT_KEY_LONG(KEY_MENU)
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
#if defined(HARDWARE_NO_TRIMS)
      if (g_trimEditMode == EDIT_TRIM_DISABLED) {
        if (view_base == VIEW_INPUTS)
          g_eeGeneral.view ^= ALTERNATE_VIEW;
        else
          g_eeGeneral.view = (g_eeGeneral.view + (4 * ALTERNATE_VIEW) + ((event == EVT_KEY_PREVIOUS_PAGE) ? -ALTERNATE_VIEW : ALTERNATE_VIEW)) % (4 * ALTERNATE_VIEW);
      }
      break;
#else
      if (view_base == VIEW_INPUTS)
        g_eeGeneral.view ^= ALTERNATE_VIEW;
      else
        g_eeGeneral.view = (g_eeGeneral.view + (4 * ALTERNATE_VIEW) + ((event == EVT_KEY_PREVIOUS_PAGE) ? -ALTERNATE_VIEW : ALTERNATE_VIEW)) % (4 * ALTERNATE_VIEW);
      break;
#endif

    case EVT_KEY_CONTEXT_MENU:
      killEvents(event);

      if (modelHasNotes()) {
        POPUP_MENU_ADD_ITEM(STR_VIEW_NOTES);
      }

      POPUP_MENU_ADD_ITEM(STR_RESET_SUBMENU);

      POPUP_MENU_ADD_ITEM(STR_STATISTICS);
      POPUP_MENU_ADD_ITEM(STR_ABOUT_US);
      POPUP_MENU_START(onMainViewMenu);
      break;

#if defined(EVT_KEY_LAST_MENU)
    case EVT_KEY_LAST_MENU:
      pushMenu(lastPopMenu());
      killEvents(event);
      break;
#endif

    case EVT_KEY_MODEL_MENU:
      pushMenu(menuModelSelect);
      killEvents(event);
      break;

    case EVT_KEY_GENERAL_MENU:
      pushMenu(menuTabGeneral[0]);
      killEvents(event);
      break;

#if defined(HARDWARE_NO_TRIMS)
      case EVT_KEY_FIRST(KEY_ENTER):
        if (!trimSelection.preEnterValid) {
          trimSelection.preEnterValid = true;
          trimSelection.preEnterTime = get_tmr10ms();
        }
        else {
          trimSelection.preEnterValid = false;
          if (++g_trimEditMode > EDIT_TRIM_MAX) {
            g_trimEditMode = EDIT_TRIM_1;
          }

          trimSelection.curStickIdx = CONVERT_MODE_TRIMS(g_trimEditMode - 1);

          if (trimSelection.preStickIdx != trimSelection.curStickIdx) {
            if (trimSelection.curStickIdx == RUD_STICK) {
              AUDIO_RUDDER_TRIM();
            }
            else if (trimSelection.curStickIdx == ELE_STICK) {
              AUDIO_ELEVATOR_TRIM();
            }
            else if (trimSelection.curStickIdx == THR_STICK) {
              AUDIO_THROTTLE_TRIM();
            }
            else if (trimSelection.curStickIdx == AIL_STICK) {
              AUDIO_AILERON_TRIM();
            }
            trimSelection.preStickIdx = trimSelection.curStickIdx;
          }
        }
        break;
#endif
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
      killEvents(event);
      break;
#endif

    case EVT_KEY_TELEMETRY:
      chainMenu(menuViewTelemetry);
      killEvents(event);
      break;


    case EVT_KEY_FIRST(KEY_EXIT):
#if defined(GVARS)
      if (gvarDisplayTimer > 0) {
        gvarDisplayTimer = 0;
      }
#endif
#if defined(HARDWARE_NO_TRIMS)
      if (g_trimEditMode != EDIT_TRIM_DISABLED) {
        g_trimEditMode = EDIT_TRIM_DISABLED;
        AUDIO_MAIN_MENU();
        trimSelection.curStickIdx = -1;
        trimSelection.preStickIdx = -1;
      }
#endif
      break;
  }
#if defined(HARDWARE_NO_TRIMS)
  if (trimSelection.preEnterValid && (get_tmr10ms() - trimSelection.preEnterTime) > 50) {
    trimSelection.preEnterValid = false;
  }
#endif

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
#if defined(PPM_UNIT_US)
          lcdDrawNumber(x0 + 4 * FW, y0, PPM_CH_CENTER(chan) + val / 2, RIGHT);
#elif defined(PPM_UNIT_PERCENT_PREC1)
          lcdDrawNumber(x0 + 4 * FW, y0, calcRESXto1000(val), RIGHT | PREC1);
#else
          lcdDrawNumber(x0+4*FW , y0, calcRESXto1000(val)/10, RIGHT); // G: Don't like the decimal part*
#endif
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
#if defined(PCBX9LITES)
        static const uint8_t x[NUM_SWITCHES-2] = {2*FW-2, 2*FW-2, 17*FW+1, 2*FW-2, 17*FW+1};
        static const uint8_t y[NUM_SWITCHES-2] = {4*FH+1, 5*FH+1, 5*FH+1, 6*FH+1, 6*FH+1};
        for (int i=0; i<NUM_SWITCHES - 2; ++i) {
          if (SWITCH_EXISTS(i)) {
            getvalue_t val = getValue(MIXSRC_FIRST_SWITCH + i);
            getvalue_t sw = ((val < 0) ? 3 * i + 1 : ((val == 0) ? 3 * i + 2 : 3 * i + 3));
            drawSwitch(x[i], y[i], sw, 0, false);
          }
        }
        drawSmallSwitch(29, 5*FH+1, 4, SW_SF);
        drawSmallSwitch(16*FW+1, 5*FH+1, 4, SW_SG);
#elif defined(PCBX9LITE)
        static const uint8_t x[NUM_SWITCHES] = {2 * FW - 2, 2 * FW - 2, 16 * FW + 1, 2 * FW - 2, 16 * FW + 1};
        static const uint8_t y[NUM_SWITCHES] = {4 * FH + 1, 5 * FH + 1, 5 * FH + 1, 6 * FH + 1, 6 * FH + 1};
        for (int i = 0; i < NUM_SWITCHES; ++i) {
          if (SWITCH_EXISTS(i)) {
            getvalue_t val = getValue(MIXSRC_FIRST_SWITCH + i);
            getvalue_t sw = ((val < 0) ? 3 * i + 1 : ((val == 0) ? 3 * i + 2 : 3 * i + 3));
            drawSwitch(x[i], y[i], sw, 0, false);
          }
        }
#elif defined(PCBXLITES)
        static const uint8_t x[NUM_SWITCHES] = {2*FW-2, 16*FW+1, 2*FW-2, 16*FW+1, 2*FW-2, 16*FW+1};
        static const uint8_t y[NUM_SWITCHES] = {4*FH+1, 4*FH+1, 6*FH+1, 6*FH+1, 5*FH+1, 5*FH+1};
        for (int i=0; i<NUM_SWITCHES; ++i) {
          if (SWITCH_EXISTS(i)) {
            getvalue_t val = getValue(MIXSRC_FIRST_SWITCH + i);
            getvalue_t sw = ((val < 0) ? 3 * i + 1 : ((val == 0) ? 3 * i + 2 : 3 * i + 3));
            drawSwitch(x[i], y[i], sw, 0, false);
          }
        }
#elif defined(PCBTARANIS)
        uint8_t switches = min(NUM_SWITCHES- NUM_FUNCTIONS_SWITCHES, 6);
        for (int i = 0; i < switches; ++i) {
          if (SWITCH_EXISTS(i)) {
            uint8_t x = 2 * FW - 2, y = 4 * FH + i * FH + 1;
            if (i >= switches / 2) {
              x = 16 * FW + 1;
              y -= (switches / 2) * FH;
            }
            getvalue_t val = getValue(MIXSRC_FIRST_SWITCH + i);
            getvalue_t sw = ((val < 0) ? 3 * i + 1 : ((val == 0) ? 3 * i + 2 : 3 * i + 3));
            drawSwitch(x, y, sw, 0, false);
          }
        }
#elif defined(PCBTANGO)
        int sw_i;
        for (int i = 0; i < NUM_SWITCHES; ++i) {
          if (SWITCH_EXISTS(i)) {
            uint8_t x = 2 * FW - 2, y = 4 * FH + i * FH + 20;
            if (i >= NUM_SWITCHES / 2) {
              x = 16 * FW + 6;
              y -= (NUM_SWITCHES / 2) * FH;
            }
            //TDOD : move switchesReorder definition in board.h
            static const uint8_t switchesReorder[] = {0, 1, 5, 3, 2, 4};

            // re-arrange order according to physical layout
            i = switchesReorder[i];
            getvalue_t val = getValue(MIXSRC_FIRST_SWITCH + sw_i);
            getvalue_t sw = ((val < 0) ? 3 * sw_i + 1 : ((val == 0) ? 3 * sw_i + 2 : 3 * sw_i + 3));
            drawSwitch(x, y, sw, 0);
          }
        }
#elif defined(PCBMAMBO)
        for (int i = 0; i < NUM_SWITCHES; ++i) {
          if (SWITCH_EXISTS(i)) {
            uint8_t x = 2 * FW - 2, y = 4 * FH + i * FH + 1;
            if (i >= NUM_SWITCHES / 2) {
              x = 16 * FW + 6;
              y -= (NUM_SWITCHES / 2) * FH;
            }
            getvalue_t val = getValue(MIXSRC_FIRST_SWITCH + i);
            getvalue_t sw = ((val < 0) ? 3 * i + 1 : ((val == 0) ? 3 * i + 2 : 3 * i + 3));
            drawSwitch(x, y, sw, 0);
          }
        }
#else
        // The ID0 3-POS switch is merged with the TRN switch
        for (uint8_t i = SWSRC_THR; i <= SWSRC_TRN; i++) {
          int8_t sw = (i == SWSRC_TRN ? (switchState(SW_ID0) ? SWSRC_ID0 : (switchState(SW_ID1) ? SWSRC_ID1 : SWSRC_ID2)) : i);
          uint8_t x = 2 * FW - 2, y = i * FH + 1;
          if (i >= SWSRC_AIL) {
            x = 17 * FW - 1;
            y -= 3 * FH;
          }
          drawSwitch(x, y, sw, getSwitch(i) ? INVERS : 0, false);
        }
#endif
      }
      else {
        // Logical Switches
        uint8_t index = 0;
        uint8_t y = LCD_H - 20;
        for (uint8_t line = 0; line < 2; line++) {
          for (uint8_t column = 0; column < MAX_LOGICAL_SWITCHES / 2; column++) {
            int8_t len = getSwitch(SWSRC_SW1 + index) ? 10 : 1;
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
      lcdDrawChar(REBOOT_X, 0 * FH, '!', INVERS);
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
