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
#include <math.h>

#include <memory>

uint8_t menuCalibrationState;

class StickCalibrationWindow : public Window
{
 public:
  StickCalibrationWindow(Window *parent, const rect_t &rect, uint8_t stickX,
                         uint8_t stickY) :
      Window(parent, rect), stickX(stickX), stickY(stickY)
  {
    lv_obj_t* bg = lv_obj_create(lvobj);
    etx_obj_add_style(bg, styles->rounded, LV_PART_MAIN);
    etx_obj_add_style(bg, styles->border, LV_PART_MAIN);
    etx_obj_add_style(bg, styles->border_color[COLOR_BLACK_INDEX], LV_PART_MAIN);
    lv_obj_set_size(bg, CAL_SIZ + PAD_BORDER * 2, CAL_SIZ + PAD_BORDER * 2);
    lv_obj_add_flag(bg, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    axis[0] = {PAD_SMALL, CAL_SIZ / 2};
    axis[1] = {CAL_SIZ - PAD_SMALL, CAL_SIZ / 2};
    addLine(bg, COLOR_GREY_INDEX, &axis[0]);

    axis[2] = {CAL_SIZ / 2, PAD_SMALL};
    axis[3] = {CAL_SIZ / 2, CAL_SIZ - PAD_SMALL};
    addLine(bg, COLOR_GREY_INDEX, &axis[2]);

    lv_obj_t* bgCircle = lv_obj_create(bg);
    etx_obj_add_style(bgCircle, styles->circle, LV_PART_MAIN);
    etx_obj_add_style(bgCircle, styles->border, LV_PART_MAIN);
    etx_obj_add_style(bgCircle, styles->border_color[COLOR_GREY_INDEX], LV_PART_MAIN);
    lv_obj_set_style_border_width(bgCircle, CIRC_W, LV_PART_MAIN);
    lv_obj_set_size(bgCircle, CAL_SIZ - PAD_TINY * 2, CAL_SIZ - PAD_TINY * 2);
    lv_obj_set_pos(bgCircle, PAD_TINY, PAD_TINY);

    for (int i = 0; i < MKR_CNT; i += 1) {
      float angle = (i * 360.0f / MKR_CNT) / 180.0f * M_PI;
      float x = cos(angle);
      float y = sin(angle);
      float r1 = (CAL_SIZ - PAD_SMALL) / 2;
      float r2 = r1 - CIRC_W;
      mkr[i * 2] = {(lv_coord_t)(x * r1) + CAL_SIZ / 2, (lv_coord_t)(y * r1) + CAL_SIZ / 2};
      mkr[i * 2 + 1] = {(lv_coord_t)(x * r2) + CAL_SIZ / 2, (lv_coord_t)(y * r2) + CAL_SIZ / 2};
      addLine(bg, COLOR_WHITE_INDEX, &mkr[i * 2]);
    }

    calibShadow = lv_obj_create(bg);
    etx_solid_bg(calibShadow, COLOR_BLACK_INDEX);
    etx_obj_add_style(calibShadow, styles->circle, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(calibShadow, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_size(calibShadow, CAL_PTR_W, CAL_PTR_W);

    calibStick = lv_obj_create(bg);
    etx_solid_bg(calibStick, COLOR_WHITE_INDEX);
    etx_obj_add_style(calibStick, styles->circle, LV_PART_MAIN);
    etx_obj_add_style(calibStick, styles->border, LV_PART_MAIN);
    etx_obj_add_style(calibStick, styles->border_color[COLOR_RED_INDEX], LV_PART_MAIN);
    lv_obj_set_style_border_width(calibStick, PAD_SMALL, LV_PART_MAIN);
    lv_obj_set_size(calibStick, CAL_PTR_W, CAL_PTR_W);

    checkEvents();
  }

  void addLine(lv_obj_t* parent, LcdColorIndex color, lv_point_t* points)
  {
    lv_obj_t* lin = lv_line_create(parent);
    etx_obj_add_style(lin, styles->line_color[color], LV_PART_MAIN);
    lv_obj_set_style_line_opa(lin, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_line_width(lin, 2, LV_PART_MAIN);
    lv_line_set_points(lin, points, 2);
  }

  void checkEvents() override
  {
    int32_t x = calibratedAnalogs[stickX];
    int32_t y = calibratedAnalogs[stickY];
    coord_t w = (CAL_SIZ - CAL_PTR_W) / 2;
    coord_t h = (CAL_SIZ - CAL_PTR_W) / 2;
    coord_t dx = w + (w * x) / RESX;
    coord_t dy = h - (h * y) / RESX;
    lv_obj_set_pos(calibStick, dx, dy);
    lv_obj_set_pos(calibShadow, dx + 1, dy + 1);
  }

  static LAYOUT_VAL_SCALED_EVEN(CAL_PTR_W, 18);
  static LAYOUT_VAL_SCALED_EVEN(CAL_SIZ, 120)
  static LAYOUT_VAL_SCALED_EVEN(CIRC_W, 10)
  static constexpr int MKR_CNT = 24;

 protected:
  uint8_t stickX, stickY;
  lv_obj_t* calibStick = nullptr;
  lv_obj_t* calibShadow = nullptr;
  lv_point_t axis[4];
  lv_point_t mkr[MKR_CNT * 2];
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

  coord_t w = StickCalibrationWindow::CAL_SIZ + PAD_BORDER * 2;
  coord_t x = (window->width() - w * 2) / 3;
  coord_t y = (window->height() - w) / 2;

  new StickCalibrationWindow(window, {x, y, w, w}, 0, 1);

  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  if (max_sticks > 2) {
    new StickCalibrationWindow(window, {x * 2 + w, y, w, w}, 3, 2);
  }

  new ViewMainDecoration(window, true);

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
