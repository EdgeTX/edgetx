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

#include "view_main_decoration.h"
#include "layout.h"

// because of IS_POT_MULTIPOS() and pal
#include "opentx.h"

// these 2 need things from opentx.h
#include "layouts/sliders.h"
#include "layouts/trims.h"

#include "board.h"
#include "hal/adc_driver.h"
#include "themes/etx_lv_theme.h"

static Window* create_layout_box(Window* parent, lv_align_t align,
                                 lv_flex_flow_t flow)
{
  auto w = new Window(parent, rect_t{0, 0, LV_SIZE_CONTENT, LV_SIZE_CONTENT});

  lv_obj_set_align(w->getLvObj(), align);
  lv_obj_set_flex_flow(w->getLvObj(), flow);

  if (_LV_FLEX_COLUMN & flow) {
    lv_obj_set_style_pad_row(w->getLvObj(), 0, 0);
  } else {
    lv_obj_set_style_pad_column(w->getLvObj(), 0, 0);
  }

  return w;
}

ViewMainDecoration::ViewMainDecoration(Window* parent) :
  parent(parent)
{
  memset(sliders, 0, sizeof(sliders));
  memset(trims, 0, sizeof(trims));
  flightMode = nullptr;

  w_ml = create_layout_box(parent, LV_ALIGN_LEFT_MID, LV_FLEX_FLOW_ROW_REVERSE);
  w_mr = create_layout_box(parent, LV_ALIGN_RIGHT_MID, LV_FLEX_FLOW_ROW);
  w_bl = create_layout_box(parent, LV_ALIGN_BOTTOM_LEFT, LV_FLEX_FLOW_COLUMN);
  w_br = create_layout_box(parent, LV_ALIGN_BOTTOM_RIGHT, LV_FLEX_FLOW_COLUMN);

  w_bc = create_layout_box(parent, LV_ALIGN_BOTTOM_MID, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(w_bc->getLvObj(), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

  createTrims(w_ml, w_mr, w_bl, w_br);
  createFlightMode(w_bc);
  createSliders(w_ml, w_mr, w_bl, w_bc, w_br);
}

void ViewMainDecoration::setSlidersVisible(bool visible)
{
  for (int i=0; i < SLIDERS_MAX; i++) {
    if (sliders[i]) {
      sliders[i]->show(visible);
    }
  }
}

void ViewMainDecoration::setTrimsVisible(bool visible)
{
  for (int i=0; i < TRIMS_MAX; i++) {
    if (trims[i]) {
      trims[i]->setVisible(visible);
    }
  }
}

void ViewMainDecoration::setFlightModeVisible(bool visible)
{
  if (flightMode)
    flightMode->show(visible);
}

rect_t ViewMainDecoration::getMainZone() const
{
  // update layout first
  lv_obj_update_layout(parent->getLvObj());

  auto x_left = lv_obj_get_x2(w_ml->getLvObj());
  auto x_right = lv_obj_get_x(w_mr->getLvObj());

  lv_coord_t bottom = LCD_H;
  Window* boxes[] = { w_bl, w_bc, w_br };

  for ( auto box : boxes ) {
    auto obj = box->getLvObj();
    auto y = lv_obj_get_y(obj);
    if (y < bottom) bottom = y;
  }

  return rect_t{ x_left, 0, x_right - x_left, bottom};
}

void ViewMainDecoration::createSliders(Window* ml, Window* mr, Window* bl, Window* bc, Window* br)
{
  int pot = 0, sl = 0;

  // Bottom left horizontal slider
  if (IS_POT_AVAILABLE(pot)) {
    sliders[sl] = new MainViewHorizontalSlider(bl, pot);
    sl += 1;
  }
  pot += 1;

  // Bottom center 6POS
  if (IS_POT_AVAILABLE(pot)) {
    if (IS_POT_MULTIPOS(pot)) {
      // Has 6POS - place bottom center
      sliders[sl] = new MainView6POS(bc, pot);
      pot += 1; sl += 1;
    }
  } else {
    pot += 1;
  }

  // Bottom right horizontal slider
  if (IS_POT_AVAILABLE(pot)) {
    sliders[sl] = new MainViewHorizontalSlider(br, pot);
    sl += 1;
  }
  pot += 1;

  auto max_pots = adcGetMaxInputs(ADC_INPUT_FLEX);
  if (max_pots > pot) {
    // create containers for the sliders, so that they are at the borders of the display
    // on top of each other, when there are two sliders to display per side
    auto leftPots = create_layout_box(ml, LV_ALIGN_LEFT_MID, LV_FLEX_FLOW_COLUMN);
    leftPots->setHeight(VERTICAL_SLIDERS_HEIGHT);

    auto rightPots = create_layout_box(mr, LV_ALIGN_RIGHT_MID, LV_FLEX_FLOW_COLUMN);
    rightPots->setHeight(VERTICAL_SLIDERS_HEIGHT);

    coord_t lsh = (IS_POT_AVAILABLE(pot + 2)) ? VERTICAL_SLIDERS_HEIGHT / 2 : VERTICAL_SLIDERS_HEIGHT;
    coord_t rsh = (IS_POT_AVAILABLE(pot + 3)) ? VERTICAL_SLIDERS_HEIGHT / 2 : VERTICAL_SLIDERS_HEIGHT;

    if (IS_POT_AVAILABLE(pot)) {
      sliders[sl] = new MainViewVerticalSlider(leftPots, rect_t{0, 0, TRIM_SQUARE_SIZE, lsh}, pot);
      sl += 1;
    }
    pot += 1;

    if (IS_POT_AVAILABLE(pot)) {
      sliders[sl] = new MainViewVerticalSlider(rightPots, rect_t{0, 0, TRIM_SQUARE_SIZE, rsh}, pot);
      sl += 1;
    }
    pot += 1;

    if (IS_POT_AVAILABLE(pot)) {
      sliders[sl] = new MainViewVerticalSlider(leftPots, rect_t{0, 0, TRIM_SQUARE_SIZE, lsh}, pot);
      sl += 1;
    }
    pot += 1;

    if (IS_POT_AVAILABLE(pot)) {
      sliders[sl] = new MainViewVerticalSlider(rightPots, rect_t{0, 0, TRIM_SQUARE_SIZE, rsh}, pot);
    }
  }
}

void ViewMainDecoration::createTrims(Window* ml, Window* mr, Window* bl, Window* br)
{
  // Trim order TRIM_LH, TRIM_LV, TRIM_RV, TRIM_RH
  trims[TRIMS_LH] = new MainViewHorizontalTrim(bl, TRIMS_LH);
  trims[TRIMS_RH] = new MainViewHorizontalTrim(br, TRIMS_RH);
  trims[TRIMS_LV] = new MainViewVerticalTrim(ml, TRIMS_LV);
  trims[TRIMS_RV] = new MainViewVerticalTrim(mr, TRIMS_RV);
}

void ViewMainDecoration::createFlightMode(Window* bc)
{
  std::function<std::string()> getFM = []() -> std::string {
      return stringFromNtString(g_model.flightModeData[mixerCurrentFlightMode].name);
  };

  flightMode = new DynamicText(bc, rect_t{}, getFM, COLOR_THEME_SECONDARY1);
}

