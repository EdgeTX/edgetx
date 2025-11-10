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
#include "hw_inputs.h"

#include <memory>

uint8_t menuCalibrationState;

static const uint8_t stick_pointer[] = {
#include "bmp_radio_stick_pointer.lbm"
};
static const uint8_t stick_background[] = {
#include "bmp_radio_stick_background.lbm"
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

  static LAYOUT_VAL_SCALED(CAL_CTR, 9)
  static LAYOUT_VAL_SCALED(CAL_SIZ, 68)

 protected:
  uint8_t stickX, stickY;
  StaticLZ4Image *calibStick = nullptr;
};

RadioCalibrationPage::RadioCalibrationPage() :
    Page(ICON_RADIO_CALIBRATION)
{
  buildHeader(header);
  buildBody(body);
}

void RadioCalibrationPage::buildHeader(Window *window)
{
  header->setTitle(STR_MENUCALIBRATION);
  title2 = header->setTitle2("");
  etx_font(title2->getLvObj(), FONT_BOLD_INDEX);
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

  axisBtn = new TextButton(window, {AXIS_X, PAD_LARGE, AXIS_W, 0}, STR_STICKS,
                 [=]() -> uint8_t {
                   new HWInputDialog<HWSticks>(STR_STICKS);
                   return 0;
                 });

  potsBtn = new TextButton(window, {POTS_X, PAD_LARGE, POTS_W, 0}, STR_POTS,
                 [=]() -> uint8_t {
                   new HWInputDialog<HWPots>(STR_POTS, HWPots::POTS_WINDOW_WIDTH);
                   return 0;
                 });

  nxtBtn = new TextButton(window, {NXT_X, PAD_LARGE, NXT_W, 0}, "",
                 [=]() -> uint8_t {
                   nextStep();
                   return 0;
                 });

  setState();
}

void RadioCalibrationPage::setState()
{
  axisBtn->hide();
  potsBtn->hide();

  switch (menuCalibrationState) {
    case CALIB_START:
      title2->setText("");
      nxtBtn->setText(STR_START);
      break;
    case CALIB_SET_MIDPOINT:
      title2->setText(STR_SETMIDPOINT);
      nxtBtn->setText(STR_NEXT);
      break;
    case CALIB_MOVE_STICKS:
      title2->setText(STR_MOVESTICKSPOTS);
      nxtBtn->setText(STR_NEXT);
      axisBtn->show();
      potsBtn->show();
      break;
    case CALIB_STORE:
      title2->setText(STR_CALIB_DONE);
      nxtBtn->setText(STR_EXIT);
      break;
    case CALIB_FINISHED:
      break;
  }
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

void RadioCalibrationPage::onCancel()
{
  if (menuCalibrationState != CALIB_START &&
      menuCalibrationState != CALIB_STORE) {
    menuCalibrationState = CALIB_START;
    setState();
  } else {
    Page::onCancel();
  }
}

void RadioCalibrationPage::nextStep()
{
  menuCalibrationState++;

  if (menuCalibrationState == CALIB_FINISHED)
    deleteLater();

  if (menuCalibrationState == CALIB_STORE)
    adcCalibStore();

  setState();
}

void startCalibration() { new RadioCalibrationPage(); }
