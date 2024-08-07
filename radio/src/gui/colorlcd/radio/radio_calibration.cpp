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

#include "radio_calibration.h"

#include "hal/adc_driver.h"
#include "edgetx.h"
#include "sliders.h"
#include "view_main_decoration.h"

#include <memory>

uint8_t menuCalibrationState;

static const uint8_t stick_pointer[] = {
#include "alpha_stick_pointer.lbm"
};
static const uint8_t stick_background[] = {
#include "alpha_stick_background.lbm"
};

class StickCalibrationWindow : public Window
{
 public:
  StickCalibrationWindow(Window *parent, const rect_t &rect, uint8_t stickX,
                         uint8_t stickY) :
      Window(parent, rect), stickX(stickX), stickY(stickY)
  {
    new StaticLZ4Image(this, 0, 0, (LZ4Bitmap *)stick_background);
    calibStick = new StaticLZ4Image(this, 0, 0, (LZ4Bitmap *)stick_pointer);
    checkEvents();
  }

  void checkEvents() override
  {
    int32_t x = calibratedAnalogs[stickX];
    int32_t y = calibratedAnalogs[stickY];
    coord_t dx = width() / 2 - CAL_CTR + (CAL_SIZ / 2 * x) / RESX;
    coord_t dy = height() / 2 - CAL_CTR - (CAL_SIZ / 2 * y) / RESX;
    lv_obj_set_pos(calibStick->getLvObj(), dx, dy);
  }

  static LAYOUT_VAL(CAL_CTR, 9, 9)
  static LAYOUT_VAL(CAL_SIZ, 68, 68)

 protected:
  uint8_t stickX, stickY;
  StaticLZ4Image *calibStick = nullptr;
};

RadioCalibrationPage::RadioCalibrationPage(bool initial) :
    Page(ICON_RADIO_CALIBRATION), initial(initial)
{
  buildHeader(header);
  buildBody(body);
}

void RadioCalibrationPage::buildHeader(Window *window)
{
  header->setTitle(STR_MENUCALIBRATION);
  text = header->setTitle2(STR_MENUTOSTART);
}

void RadioCalibrationPage::buildBody(Window *window)
{
  window->padAll(PAD_ZERO);

  menuCalibrationState = CALIB_START;

  // The two sticks

  LZ4Bitmap *bg = (LZ4Bitmap *)stick_background;

  new StickCalibrationWindow(
      window,
      {window->width() / 3 - bg->width / 2,
       window->height() / 2 - bg->height / 2, bg->width, bg->height},
      0, 1);

  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  if (max_sticks > 2) {
    new StickCalibrationWindow(
        window,
        {window->width() * 2 / 3 - bg->width / 2,
         window->height() / 2 - bg->height / 2, bg->width, bg->height},
        3, 2);
  }

  std::unique_ptr<ViewMainDecoration> deco(new ViewMainDecoration(window, false, true, false));

#if defined(PCBNV14) || defined(PCBPL18)
  new TextButton(window, {LCD_W - 120, LCD_H - 140, 90, 40}, "Next",
                 [=]() -> uint8_t {
                   nextStep();
                   return 0;
                 });
#endif
}

void RadioCalibrationPage::checkEvents()
{
  Page::checkEvents();

  if (menuCalibrationState == CALIB_SET_MIDPOINT) {
    adcCalibSetMidPoint();
  } else if (menuCalibrationState == CALIB_MOVE_STICKS) {
    adcCalibSetMinMax();
  }
}

void RadioCalibrationPage::onClicked() { nextStep(); }

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
  if (menuCalibrationState == CALIB_FINISHED) deleteLater();

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
      if (initial) deleteLater();
      break;

    default:
      text->setText(STR_MENUTOSTART);
      menuCalibrationState = CALIB_START;
      break;
  }
}

void startCalibration() { new RadioCalibrationPage(true); }
