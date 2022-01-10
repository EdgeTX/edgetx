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

void ViewMainDecoration::setSlidersVisible(bool visible)
{
  //
  // Horizontal Sliders
  //
  sliders[SLIDERS_POT1]->setHeight(visible ? TRIM_SQUARE_SIZE : 0);

  if (IS_POT_MULTIPOS(POT2)) {
    sliders[SLIDERS_POT2]->setHeight(visible ? MULTIPOS_H : 0);
  }
  else if (IS_POT(POT2)) {
#if !defined(HARDWARE_POT3) || (defined(HARDWARE_POT3) && (LCD_W > 400))
    // If pot3 is present, show pot2 only on screens with width of higher than 400 pixels
    sliders[SLIDERS_POT2]->setHeight(visible ? TRIM_SQUARE_SIZE : 0);
#endif
  }

#if defined(HARDWARE_POT3)
  sliders[SLIDERS_POT3]->setHeight(visible ? TRIM_SQUARE_SIZE : 0);
#endif

#if NUM_SLIDERS > 0
  //
  // Vertical sliders
  //
  sliders[SLIDERS_REAR_LEFT]->setWidth(visible ? TRIM_SQUARE_SIZE : 0);

#if defined(HARDWARE_EXT1) || defined(PCBX12S)
  if (IS_POT_SLIDER_AVAILABLE(EXT1)) {
    sliders[SLIDERS_EXT1]->setWidth(visible ? TRIM_SQUARE_SIZE : 0);
  }
#endif

  sliders[SLIDERS_REAR_RIGHT]->setWidth(visible ? TRIM_SQUARE_SIZE : 0);
    
#if defined(HARDWARE_EXT2) || defined(PCBX12S)
  if (IS_POT_SLIDER_AVAILABLE(EXT2)) {
    sliders[SLIDERS_EXT2]->setWidth(visible ? TRIM_SQUARE_SIZE : 0);
  }
#endif

#endif

  if (visible) {
    visibilityMask |= VM_SLIDERS;
  }
  else {
    visibilityMask &= ~VM_SLIDERS;
  }
}

void ViewMainDecoration::setTrimsVisible(bool visible)
{
  trims[TRIMS_LH]->setHeight(visible ? TRIM_SQUARE_SIZE : 0);
  trims[TRIMS_RH]->setHeight(visible ? TRIM_SQUARE_SIZE : 0);

  trims[TRIMS_LV]->setWidth(visible ? TRIM_SQUARE_SIZE : 0);
  trims[TRIMS_RV]->setWidth(visible ? TRIM_SQUARE_SIZE : 0);

  if (visible) {
    visibilityMask |= VM_TRIMS;
  }
  else {
    visibilityMask &= ~VM_TRIMS;
  }
}

void ViewMainDecoration::setFlightModeVisible(bool visible)
{
  flightMode->setHeight(visible ? FM_LABEL_HEIGHT : 0);

  if (visible) {
    visibilityMask |= VM_FM;
  }
  else {
    visibilityMask &= ~VM_FM;
  }
}

void ViewMainDecoration::adjustDecoration()
{
  // Sliders are closer to the edge and must be computed first

  // These are located on the bottom
  auto pos = bottom() - sliders[SLIDERS_POT1]->height();
  sliders[SLIDERS_POT1]->setTop(pos);

  if (sliders[SLIDERS_POT2]) {
    auto sl = sliders[SLIDERS_POT2];
    sl->setTop(pos);
    if (IS_POT_MULTIPOS(POT2)) {
      sl->setWidth(MULTIPOS_W);
	    sl->setLeft((width() - sl->width()) / 2);  // This is the position of middle multipos pot
    }
    else { // if !IS_POT(POT2) -> sliders[SLIDERS_POT2] == nullptr
      sl->setWidth(HORIZONTAL_SLIDERS_WIDTH);
			sl->setLeft(width() - sl->width() - HMARGIN);  // Normal pot position, align to right
    }
  }

#if defined(HARDWARE_POT3)
  sliders[SLIDERS_POT3]->setTop(pos);
#endif

  // Horizontal trims are on top of horizontal sliders
  pos -= trims[TRIMS_LH]->height();
  trims[TRIMS_LH]->setTop(pos);
  trims[TRIMS_RH]->setTop(pos);

  // Vertical trims/slider are on top of horizontal sliders with a small margin
  auto vertTop = pos - HMARGIN - VERTICAL_SLIDERS_HEIGHT;

#if NUM_SLIDERS > 0
  // Left side (vertical)
  pos = left();
  sliders[SLIDERS_REAR_LEFT]->setLeft(pos);
  sliders[SLIDERS_REAR_LEFT]->setTop(vertTop);

#if defined(HARDWARE_EXT1) || defined(PCBX12S) // TODO: define HARDWARE_EXT1 for X12S
  sliders[SLIDERS_EXT1]->setLeft(pos);
  if (IS_POT_SLIDER_AVAILABLE(EXT1)) {
    auto rl = sliders[SLIDERS_REAR_LEFT];
    auto e1 = sliders[SLIDERS_EXT1];

    // If EXT1 is configured as a slider,
    // place it bellow rear-left slider (and make them smaller)
    rl->setHeight(VERTICAL_SLIDERS_HEIGHT / 2);
    e1->setTop(vertTop + rl->height() + HMARGIN/2);
    e1->setHeight(rl->height());
  }
  else {
    auto rl = sliders[SLIDERS_REAR_LEFT];
    auto e1 = sliders[SLIDERS_EXT1];

    // Otherwise hide the extra slider and make rear-left fullsize
    rl->setHeight(VERTICAL_SLIDERS_HEIGHT);
    e1->setHeight(0);
  }
#endif

  // Right side (vertical)
  pos = right() - sliders[SLIDERS_REAR_RIGHT]->width();
  sliders[SLIDERS_REAR_RIGHT]->setLeft(pos);
  sliders[SLIDERS_REAR_RIGHT]->setTop(vertTop);

#if defined(HARDWARE_EXT2) || defined(PCBX12S)
  sliders[SLIDERS_EXT2]->setLeft(pos);
  if (IS_POT_SLIDER_AVAILABLE(EXT2)) {
    auto rr = sliders[SLIDERS_REAR_RIGHT];
    auto e2 = sliders[SLIDERS_EXT2];

    // If EXT2 is configured as a slider,
    // place it bellow rear-left slider (and make them smaller)
    rr->setHeight(VERTICAL_SLIDERS_HEIGHT / 2);
    e2->setTop(vertTop + rr->height() + HMARGIN/2);
    e2->setHeight(rr->height());
  }
  else {
    auto rr = sliders[SLIDERS_REAR_RIGHT];
    auto e2 = sliders[SLIDERS_EXT2];

    // Otherwise hide the extra slider and make rear-left fullsize
    rr->setHeight(VERTICAL_SLIDERS_HEIGHT);
    e2->setHeight(0);
  }
#endif

  // Finally place the vertical trims further from the edge
  trims[TRIMS_LV]->setLeft(sliders[SLIDERS_REAR_LEFT]->right());
  trims[TRIMS_LV]->setTop(vertTop);
  trims[TRIMS_RV]->setLeft(sliders[SLIDERS_REAR_RIGHT]->left() - trims[TRIMS_RV]->width());
  trims[TRIMS_RV]->setTop(vertTop);
#else
  // No sliders: place the vertical trims on the edge
  trims[TRIMS_LV]->setLeft(left());
  trims[TRIMS_LV]->setTop(vertTop);
  trims[TRIMS_RV]->setLeft(right() - trims[TRIMS_RV]->width());
  trims[TRIMS_RV]->setTop(vertTop);  
#endif
  
  // Place the flight-mode text box
  // -> between horiz trims (if existing)
  // else on top of horiz sliders
  //
  pos = trims[TRIMS_LH]->right();
  flightMode->setLeft(pos);

  pos = trims[TRIMS_RH]->left() - pos;
  flightMode->setWidth(pos);
  flightMode->setInnerWidth(pos);

  pos = sliders[SLIDERS_POT1]->top();
  pos -= flightMode->height();
  flightMode->setTop(pos);
}

rect_t ViewMainDecoration::getMainZone() const
{
  coord_t border = (getVisibilityMask() == VM_NONE ?
                    0 : MAIN_ZONE_BORDER);
  rect_t zone = {
    trims[TRIMS_LV]->right() + border, border,
    0, 0 // let's compute them!
  };

  zone.w = trims[TRIMS_RV]->left() - border - zone.x;

  // min(trims[TRIMS_LH]->top(), flightMode->top())
  if (trims[TRIMS_LH]->top() < flightMode->top())
    zone.h = trims[TRIMS_LH]->top();
  else
    zone.h = flightMode->top();

  zone.h -= border + zone.y;

  return zone;
}

void ViewMainDecoration::createSliders()
{
  // fixed size array, so that works
  memset(sliders, 0, sizeof(sliders));
    
  rect_t r = {
    // left
    HMARGIN, 0,
    HORIZONTAL_SLIDERS_WIDTH, 0
  };
    
  sliders[SLIDERS_POT1] = new MainViewHorizontalSlider(this, r, CALIBRATED_POT1);

  r = rect_t { 0, 0, 0, 0 };
  if (IS_POT_MULTIPOS(POT2)) {
    sliders[SLIDERS_POT2] = new MainView6POS(this, r, 1);
  }
  else if (IS_POT(POT2)) {
    sliders[SLIDERS_POT2] = new MainViewHorizontalSlider(this, r, CALIBRATED_POT2);
  }

#if defined(HARDWARE_POT3)
  r = rect_t {
    // right
    width() - HORIZONTAL_SLIDERS_WIDTH - HMARGIN, 0,
    HORIZONTAL_SLIDERS_WIDTH, 0
  };

  sliders[SLIDERS_POT3] = new MainViewHorizontalSlider(this, r, CALIBRATED_POT3);
#endif

#if NUM_SLIDERS > 0
  r = rect_t { 0, 0, 0, 0 };
  sliders[SLIDERS_REAR_LEFT] = new MainViewVerticalSlider(this, r, CALIBRATED_SLIDER_REAR_LEFT);
  sliders[SLIDERS_REAR_RIGHT] = new MainViewVerticalSlider(this, r, CALIBRATED_SLIDER_REAR_RIGHT);

#if defined(HARDWARE_EXT1) || defined(PCBX12S)
  sliders[SLIDERS_EXT1] = new MainViewVerticalSlider(this, r, CALIBRATED_POT_EXT1);
#endif

#if defined(HARDWARE_EXT2) || defined(PCBX12S)
  sliders[SLIDERS_EXT2] = new MainViewVerticalSlider(this, r, CALIBRATED_POT_EXT2);
#endif
#endif // NUM_SLIDERS > 0
}

void ViewMainDecoration::createTrims()
{
  // Trim order TRIM_LH, TRIM_LV, TRIM_RV, TRIM_RH

  rect_t r = {
    left() + HMARGIN, 0,
    HORIZONTAL_SLIDERS_WIDTH, 0
  };
  
  trims[TRIMS_LH] = new MainViewHorizontalTrim(this, r, TRIMS_LH);

  r = rect_t {
    right() - HORIZONTAL_SLIDERS_WIDTH - HMARGIN, 0,
    HORIZONTAL_SLIDERS_WIDTH, 0
  };

  trims[TRIMS_RH] = new MainViewHorizontalTrim(this, r, TRIMS_RH);

  r = rect_t {
    0, 0, 0,
    VERTICAL_SLIDERS_HEIGHT
  }; 

  trims[TRIMS_LV] = new MainViewVerticalTrim(this, r, TRIMS_LV);
  trims[TRIMS_RV] = new MainViewVerticalTrim(this, r, TRIMS_RV);
}

void ViewMainDecoration::createFlightMode()
{
  rect_t r = {
    // centered text box (50 pixels from either edge)
    // -> re-size once the other components are set
    50, 0, width() - 100, 0
  };

  std::function<std::string()> getFM = []() -> std::string {
      return stringFromNtString(g_model.flightModeData[mixerCurrentFlightMode].name);
};

  flightMode = new DynamicText(this, r, getFM, CENTERED | COLOR_THEME_SECONDARY1);
}

