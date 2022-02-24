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
#include "radio_calibration.h"
#include "sliders.h"
#include "view_main_decoration.h"
#include "hal/adc_driver.h"

#define XPOT_DELTA                     10
#define XPOT_DELAY                     5 /* cycles */

uint8_t menuCalibrationState;

class StickCalibrationWindow: public Window {
  public:
   StickCalibrationWindow(Window *parent, const rect_t &rect, uint8_t stickX,
                          uint8_t stickY) :
       Window(parent, rect, REFRESH_ALWAYS), stickX(stickX), stickY(stickY)
   {
     setLeft(rect.x - calibStickBackground->width() / 2);
     setTop(rect.y - calibStickBackground->height() / 2);
     setWidth(calibStickBackground->width());
     setHeight(calibStickBackground->height());
    }

    void paint(BitmapBuffer * dc) override
    {
      dc->drawBitmap(0, 0, calibStickBackground);
      int16_t x = calibratedAnalogs[CONVERT_MODE(stickX)];
      int16_t y = calibratedAnalogs[CONVERT_MODE(stickY)];
      dc->drawBitmap(width() / 2 - 9 + (bitmapSize / 2 * x) / RESX,
                     height() / 2 - 9 - (bitmapSize / 2 * y) / RESX,
                     calibStick);
    }

  protected:
    static constexpr coord_t bitmapSize = 68;
    uint8_t stickX, stickY;
};

RadioCalibrationPage::RadioCalibrationPage(bool initial):
  Page(ICON_RADIO_CALIBRATION),
  initial(initial)
{
  buildHeader(&header);
  buildBody(&body);
  setFocus(SET_FOCUS_DEFAULT);
}

void RadioCalibrationPage::buildHeader(Window * window)
{
  new StaticText(window,
                 {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                  PAGE_LINE_HEIGHT},
                 STR_MENUCALIBRATION, 0, COLOR_THEME_PRIMARY2);

  text = new StaticText(window,
                        {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
                         LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                        STR_MENUTOSTART, 0, COLOR_THEME_PRIMARY2);
}

void RadioCalibrationPage::buildBody(FormWindow * window)
{
  menuCalibrationState = CALIB_START;

  // The two sticks

  //TODO: dynamic placing
  new StickCalibrationWindow(window,
                             {window->width() / 3, window->height() / 2, 0, 0},
                             STICK1, STICK2);

  new StickCalibrationWindow(window,
                             {(2 * window->width()) / 3, window->height() / 2, 0, 0},
                             STICK4, STICK3);

  rect_t r = { 0, 0, window->width(), window->height() };
  auto deco = new ViewMainDecoration(window, r);
  deco->setTrimsVisible(false);
  deco->setSlidersVisible(true);
  deco->setFlightModeVisible(false);
  deco->adjustDecoration();

#if defined(PCBNV14) || defined(PCBPL18)
  new TextButton(window, {LCD_W - 120, LCD_H - 140, 90, 40}, "Next",
                    [=]() -> uint8_t {
                        nextStep();
                        return 0;
                    }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);
#endif
}
bool checkEventsRan = false;
void RadioCalibrationPage::checkEvents()
{
  checkEventsRan=true;
  Page::checkEvents();

  for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS + NUM_MOUSE_ANALOGS; i++) { // get low and high vals for sticks and trims
    int16_t vt = i < TX_VOLTAGE ? anaIn(i) : anaIn(i + 1);
    reusableBuffer.calib.loVals[i] = min(vt, reusableBuffer.calib.loVals[i]);
    reusableBuffer.calib.hiVals[i] = max(vt, reusableBuffer.calib.hiVals[i]);
    if (i >= POT1 && i <= POT_LAST) {
      if (IS_POT_WITHOUT_DETENT(i)) {
        reusableBuffer.calib.midVals[i] = (reusableBuffer.calib.hiVals[i] + reusableBuffer.calib.loVals[i]) / 2;
      }
#if NUM_XPOTS > 0
      uint8_t idx = i - POT1;
      int count = reusableBuffer.calib.xpotsCalib[idx].stepsCount;
      if (IS_POT_MULTIPOS(i) && count <= XPOTS_MULTIPOS_COUNT) {
        // use raw analog value for multipos calibraton, anaIn() already has multipos decoded value
        vt = getAnalogValue(i) >> 1;
        if (reusableBuffer.calib.xpotsCalib[idx].lastCount == 0 || vt < reusableBuffer.calib.xpotsCalib[idx].lastPosition - XPOT_DELTA ||
            vt > reusableBuffer.calib.xpotsCalib[idx].lastPosition + XPOT_DELTA) {
          reusableBuffer.calib.xpotsCalib[idx].lastPosition = vt;
          reusableBuffer.calib.xpotsCalib[idx].lastCount = 1;
        }
        else {
          if (reusableBuffer.calib.xpotsCalib[idx].lastCount < 255) {
            reusableBuffer.calib.xpotsCalib[idx].lastCount++;
          }
        }
        if (reusableBuffer.calib.xpotsCalib[idx].lastCount == XPOT_DELAY) {
          int16_t position = reusableBuffer.calib.xpotsCalib[idx].lastPosition;
          bool found = false;
          for (int j = 0; j < count; j++) {
            int16_t step = reusableBuffer.calib.xpotsCalib[idx].steps[j];
            if (position >= step - XPOT_DELTA && position <= step + XPOT_DELTA) {
              found = true;
              break;
            }
          }
          if (!found) {
            if (count < XPOTS_MULTIPOS_COUNT) {
              reusableBuffer.calib.xpotsCalib[idx].steps[count] = position;
            }
            reusableBuffer.calib.xpotsCalib[idx].stepsCount += 1;
          }
        }
      }
#endif
    }
  }

  if (menuCalibrationState == CALIB_SET_MIDPOINT) {
    for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS + NUM_MOUSE_ANALOGS; i++) {
      reusableBuffer.calib.loVals[i] = 15000;
      reusableBuffer.calib.hiVals[i] = -15000;
      reusableBuffer.calib.midVals[i] = i < TX_VOLTAGE ? anaIn(i) : anaIn(i + 1);
#if NUM_XPOTS > 0
      if (i < NUM_XPOTS) {
        reusableBuffer.calib.xpotsCalib[i].stepsCount = 0;
        reusableBuffer.calib.xpotsCalib[i].lastCount = 0;
      }
#endif
    }
  }
  else if (menuCalibrationState == CALIB_MOVE_STICKS) {
    for (uint8_t i = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS + NUM_MOUSE_ANALOGS; i++) {
      if (abs(reusableBuffer.calib.loVals[i] - reusableBuffer.calib.hiVals[i]) > 50) {
        g_eeGeneral.calib[i].mid = reusableBuffer.calib.midVals[i];
        int16_t v = reusableBuffer.calib.midVals[i] - reusableBuffer.calib.loVals[i];
        g_eeGeneral.calib[i].spanNeg = v - v / STICK_TOLERANCE;
        v = reusableBuffer.calib.hiVals[i] - reusableBuffer.calib.midVals[i];
        g_eeGeneral.calib[i].spanPos = v - v / STICK_TOLERANCE;
      }
    }
#if NUM_XPOTS > 0
    for (int i = POT1; i <= POT_LAST; i++) {
      int idx = i - POT1;
      int count = reusableBuffer.calib.xpotsCalib[idx].stepsCount;
      if (IS_POT_MULTIPOS(i)) {
        if (count > 1 && count <= XPOTS_MULTIPOS_COUNT) {
          for (int j = 0; j < count; j++) {
            for (int k = j + 1; k < count; k++) {
              if (reusableBuffer.calib.xpotsCalib[idx].steps[k] < reusableBuffer.calib.xpotsCalib[idx].steps[j]) {
                SWAP(reusableBuffer.calib.xpotsCalib[idx].steps[j], reusableBuffer.calib.xpotsCalib[idx].steps[k]);
              }
            }
          }
          StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[i];
          calib->count = count - 1;
          for (int j = 0; j < calib->count; j++) {
            calib->steps[j] = (reusableBuffer.calib.xpotsCalib[idx].steps[j + 1] + reusableBuffer.calib.xpotsCalib[idx].steps[j]) >> 5;
          }
        }
        else {
          // g_eeGeneral.potsConfig &= ~(0x03<<(2*idx));
        }
      }
    }
#endif
  }
}

#if defined(HARDWARE_KEYS)
void RadioCalibrationPage::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    nextStep();
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT) && menuCalibrationState != CALIB_START) {
    menuCalibrationState = CALIB_START;
    text->setText(STR_MENUTOSTART);

  }
  else {
    Page::onEvent(event);
  }
}
#endif

void RadioCalibrationPage::nextStep()
{
  if(!checkEventsRan)
    return;
  checkEventsRan=false;
  menuCalibrationState++;

  switch (menuCalibrationState) {
    case CALIB_SET_MIDPOINT:
      text->setText(STR_SETMIDPOINT);
      break;

    case CALIB_MOVE_STICKS:
      text->setText(STR_MOVESTICKSPOTS);
      break;

    case CALIB_STORE:
      text->setText(STR_CALIB_DONE);
      g_eeGeneral.chkSum = evalChkSum();
      storageDirty(EE_GENERAL);
      menuCalibrationState = CALIB_FINISHED;

      // initial calibration completed
      // -> exit
      if (initial)
        deleteLater();
      break;

    default:
      text->setText(STR_MENUTOSTART);
      menuCalibrationState = CALIB_START;
      break;
  }
}

void startCalibration()
{
  new RadioCalibrationPage(true);
}
