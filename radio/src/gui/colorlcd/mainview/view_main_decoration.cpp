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

// because of IS_POT_MULTIPOS() and pal
#include "edgetx.h"

// these 2 need things from edgetx.h
#include "sliders.h"
#include "trims.h"

#include "hal/adc_driver.h"

ViewMainDecoration::ViewMainDecoration(Window* parent, bool showTrims, bool showSliders, bool showFM) :
  parent(parent), showTrims(showTrims), showSliders(showSliders), showFM(showFM)
{
  w_ml = layoutBox(parent, LV_ALIGN_LEFT_MID, LV_FLEX_FLOW_ROW_REVERSE);
  w_mr = layoutBox(parent, LV_ALIGN_RIGHT_MID, LV_FLEX_FLOW_ROW);
  w_bl = layoutBox(parent, LV_ALIGN_BOTTOM_LEFT, LV_FLEX_FLOW_COLUMN);
  w_br = layoutBox(parent, LV_ALIGN_BOTTOM_RIGHT, LV_FLEX_FLOW_COLUMN);

  w_bc = layoutBox(parent, LV_ALIGN_BOTTOM_MID, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(w_bc->getLvObj(), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

  if (this->showTrims)
    createTrims(w_ml, w_mr, w_bl, w_br);
  if (this->showFM)
    createFlightMode(w_bc);
  if (this->showSliders)
    createSliders(w_ml, w_mr, w_bl, w_bc, w_br);
}

Window* ViewMainDecoration::layoutBox(Window* parent, lv_align_t align,
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

void ViewMainDecoration::setSlidersVisible(bool visible)
{
  showSliders = visible;
  for (int i=0; i < SLIDERS_MAX; i++) {
    if (sliders[i]) {
      sliders[i]->show(visible);
    }
  }
}

void ViewMainDecoration::setTrimsVisible(bool visible)
{
  showTrims = visible;
  for (int i=0; i < TRIMS_MAX; i++) {
    if (trims[i]) {
      trims[i]->setVisible(visible);
    }
  }
}

void ViewMainDecoration::setFlightModeVisible(bool visible)
{
  showFM = visible;
  if (flightMode)
    flightMode->show(visible);
}

// Check if trim may be visible in one or more flight modes
static bool canTrimShow(int idx)
{
  idx = inputMappingConvertMode(idx);
  for (int i = 0; i < MAX_FLIGHT_MODES; i += 1) {
    if (i == 0 || g_model.flightModeData[i].swtch != SWITCH_NONE) {
      trim_t v = getRawTrimValue(i, idx);
      if (v.mode != TRIM_MODE_NONE && v.mode != TRIM_MODE_3POS)
        return true;
    }
  }
  return false;
}

rect_t ViewMainDecoration::getMainZone() const
{
  coord_t x = 0, w = LCD_W, h = LCD_H;
  coord_t bh = 0;

  if (showSliders) {
    if (hasVerticalSliders) {
      x += MainViewSlider::SLIDER_BAR_SIZE;
      w -= 2 * MainViewSlider::SLIDER_BAR_SIZE;
    }
    bh = MainViewSlider::SLIDER_BAR_SIZE;
  }

  if (showTrims) {
    if (canTrimShow(TRIMS_LV)) {
      x += MainViewSlider::SLIDER_BAR_SIZE;
      w -= MainViewSlider::SLIDER_BAR_SIZE;
    }
    if (canTrimShow(TRIMS_RV)) {
      w -= MainViewSlider::SLIDER_BAR_SIZE;
    }
    if (showFM && (has6POS || !showSliders))
      bh += EdgeTxStyles::STD_FONT_HEIGHT;
    else if (canTrimShow(TRIMS_LH) || canTrimShow(TRIMS_RH))
      bh += MainViewSlider::SLIDER_BAR_SIZE;
  } else if (showFM) {
    bh += EdgeTxStyles::STD_FONT_HEIGHT;
    if (!has6POS && showSliders)
      bh -= MainViewSlider::SLIDER_BAR_SIZE;
  }

  h -= bh;

  return rect_t{x, 0, w, h};
}

void ViewMainDecoration::createSliders(Window* ml, Window* mr, Window* bl, Window* bc, Window* br)
{
  int pot = 0;

  // Bottom left horizontal slider
  if (IS_POT_AVAILABLE(pot)) {
    sliders[pot] = new MainViewHorizontalSlider(bl, pot);
  }
#if defined(RADIO_PL18U)
  pot += 2;
#else
  pot += 1;
#endif

  // Bottom center 6POS
  if (IS_POT_AVAILABLE(pot)) {
#if defined(RADIO_PL18) || defined(RADIO_PL18EV) || defined(RADIO_PL18U)
    has6POS = true;
    sliders[pot] = new MainViewHorizontalSlider(bc, pot);
    pot += 1;
#else
    if (IS_POT_MULTIPOS(pot)) {
      has6POS = true;
      // Has 6POS - place bottom center
      sliders[pot] = new MainView6POS(bc, pot);
      pot += 1;
    }
#endif
  } else {
    pot += 1;
  }

  // Bottom right horizontal slider

#if defined(RADIO_PL18U)
  pot -= 2;
#endif
  if (IS_POT_AVAILABLE(pot)) {
    sliders[pot] = new MainViewHorizontalSlider(br, pot);
  }
#if defined(RADIO_PL18U)
  pot += 2;
#else
  pot += 1;
#endif

  auto max_pots = adcGetMaxInputs(ADC_INPUT_FLEX);
  if (max_pots > pot) {
    hasVerticalSliders = true;

    // create containers for the sliders, so that they are at the borders of the display
    // on top of each other, when there are two sliders to display per side
    auto leftPots = layoutBox(ml, LV_ALIGN_LEFT_MID, LV_FLEX_FLOW_COLUMN);
    leftPots->setHeight(MainViewSlider::VERTICAL_SLIDERS_HEIGHT);

    auto rightPots = layoutBox(mr, LV_ALIGN_RIGHT_MID, LV_FLEX_FLOW_COLUMN);
    rightPots->setHeight(MainViewSlider::VERTICAL_SLIDERS_HEIGHT);

    coord_t lsh = (IS_POT_AVAILABLE(pot + 2)) ? MainViewSlider::VERTICAL_SLIDERS_HEIGHT / 2 : MainViewSlider::VERTICAL_SLIDERS_HEIGHT;
    coord_t rsh = (IS_POT_AVAILABLE(pot + 3)) ? MainViewSlider::VERTICAL_SLIDERS_HEIGHT / 2 : MainViewSlider::VERTICAL_SLIDERS_HEIGHT;

    if (IS_POT_AVAILABLE(pot)) {
      sliders[pot] = new MainViewVerticalSlider(leftPots, rect_t{0, 0, MainViewSlider::SLIDER_BAR_SIZE, lsh}, pot);
    }
    pot += 1;

    if (IS_POT_AVAILABLE(pot)) {
      sliders[pot] = new MainViewVerticalSlider(rightPots, rect_t{0, 0, MainViewSlider::SLIDER_BAR_SIZE, rsh}, pot);
    }
    pot += 1;

    if (IS_POT_AVAILABLE(pot)) {
      sliders[pot] = new MainViewVerticalSlider(leftPots, rect_t{0, 0, MainViewSlider::SLIDER_BAR_SIZE, lsh}, pot);
    }
    pot += 1;

    if (IS_POT_AVAILABLE(pot)) {
      sliders[pot] = new MainViewVerticalSlider(rightPots, rect_t{0, 0, MainViewSlider::SLIDER_BAR_SIZE, rsh}, pot);
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

  flightMode = new DynamicText(bc, rect_t{}, getFM, COLOR_THEME_SECONDARY1_INDEX);
}
