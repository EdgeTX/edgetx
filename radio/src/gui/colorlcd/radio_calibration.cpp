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
      int16_t x = calibratedAnalogs[stickX];
      int16_t y = calibratedAnalogs[stickY];
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
}

void RadioCalibrationPage::buildHeader(Window * window)
{
  header.setTitle(STR_MENUCALIBRATION);
  text = header.setTitle2(STR_MENUTOSTART);
}

void RadioCalibrationPage::buildBody(FormWindow * window)
{
  menuCalibrationState = CALIB_START;

  // The two sticks

  //TODO: dynamic placing
  new StickCalibrationWindow(
      window, {window->width() / 3, window->height() / 2, 0, 0}, 0, 1);

  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  if (max_sticks > 2) {
      new StickCalibrationWindow(
          window, {(2 * window->width()) / 3, window->height() / 2, 0, 0}, 3,
          2);
  }

  std::unique_ptr<ViewMainDecoration> deco(new ViewMainDecoration(window));
  deco->setTrimsVisible(false);
  deco->setSlidersVisible(true);
  deco->setFlightModeVisible(false);

#if defined(PCBNV14)
  new TextButton(window, {LCD_W - 120, LCD_H - 140, 90, 40}, "Next",
                    [=]() -> uint8_t {
                        nextStep();
                        return 0;
                    }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);
#endif
}

void RadioCalibrationPage::checkEvents()
{
  Page::checkEvents();

  // Get min / max values
  adcCalibMinMax();

  if (menuCalibrationState == CALIB_SET_MIDPOINT) {
    adcCalibSetMidPoint();
  }
  else if (menuCalibrationState == CALIB_MOVE_STICKS) {
    adcCalibSetMinMax();
    adcCalibSetXPot();
  }
}

void RadioCalibrationPage::onClicked()
{
  nextStep();
}

void RadioCalibrationPage::onCancel()
{
  if (menuCalibrationState != CALIB_START &&
      menuCalibrationState != CALIB_FINISHED) {
    menuCalibrationState = CALIB_START;
    text->setText(STR_MENUTOSTART);
  } else {
    Page::onCancel();
  }
}

void RadioCalibrationPage::nextStep()
{
  if (menuCalibrationState == CALIB_FINISHED)
    deleteLater();

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
      adcCalibStore();
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
