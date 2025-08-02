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

#include "sliders.h"

#include "bitmaps.h"
#include "hal/adc_driver.h"
#include "edgetx.h"
#include "switches.h"

SliderIcon::SliderIcon(Window* parent) :
    Window(parent, rect_t{0, 0, MainViewSlider::SLIDER_BAR_SIZE, MainViewSlider::SLIDER_BAR_SIZE})
{
  setWindowFlag(NO_FOCUS);

  auto icon = getBuiltinIcon(ICON_TRIM_SHADOW);
  shadow = lv_canvas_create(lvobj);
  lv_canvas_set_buffer(shadow, (void*)icon->data, icon->width, icon->height, LV_IMG_CF_ALPHA_8BIT);
  etx_img_color(shadow, COLOR_THEME_PRIMARY1_INDEX);

  icon = getBuiltinIcon(ICON_TRIM);
  mask = lv_canvas_create(lvobj);
  lv_canvas_set_buffer(mask, (void*)icon->data, icon->width, icon->height, LV_IMG_CF_ALPHA_8BIT);
  etx_img_color(mask, COLOR_THEME_FOCUS_INDEX);
}

std::vector<MaskBitmap*> MainViewSlider::tickMasks;

MainViewSlider::MainViewSlider(Window* parent, const rect_t& rect, uint8_t idx,
                               bool isVertical) :
    Window(parent, rect), isVertical(isVertical)
{
  potIdx = adcGetInputOffset(ADC_INPUT_FLEX) + idx;

  auto mask = getTicksMask();

  if (mask) {
    maskCanvas = lv_canvas_create(lvobj);
    etx_img_color(maskCanvas, COLOR_THEME_SECONDARY1_INDEX);
    lv_canvas_set_buffer(maskCanvas, mask->data, mask->width, mask->height, LV_IMG_CF_ALPHA_8BIT);

    if (isVertical) {
      lv_obj_set_pos(maskCanvas, PAD_TINY, SLIDER_BAR_SIZE / 2);
    } else {
      lv_obj_set_pos(maskCanvas, SLIDER_BAR_SIZE / 2, PAD_TINY);
    }
  }

  sliderIcon = new SliderIcon(this);

  setPos();
}

MaskBitmap* MainViewSlider::getTicksMask()
{
  coord_t l, x, y, w, h;

  // Mask size
  if (isVertical) {
    h = height() - SLIDER_BAR_SIZE + 1;
    if ((h & 1) == 0) h += 1;
    w = MASK_SHORT_DIM;
  } else {
    w = width() - SLIDER_BAR_SIZE + 1;
    if ((w & 1) == 0) w += 1;
    h = MASK_SHORT_DIM;
  }

  for (auto mask : tickMasks) {
    // TRACE("Found Mask %d %d",w,h);
    if (mask->width == w && mask->height == h) return mask;
  }

  MaskBitmap* mask =(MaskBitmap*)malloc(w * h + sizeof(uint16_t) * 2);
  if (!mask) {
    TRACE("Could not create Slider mask");
    return nullptr;
  }

  mask->width = w;
  mask->height = h;
  memset(mask->data, 0, w * h);

  if (isVertical) {
    // Create vertical mask for tick lines
    int sliderTicksCount = h / SLIDER_TICK_SPACING;
    y = 0;
    for (uint8_t i = 0; i <= sliderTicksCount; i++) {
      if (i == 0 || i == sliderTicksCount / 2 || i == sliderTicksCount) {
        x = 0;
        l = w;
      } else {
        x = PAD_TINY;
        l = w - PAD_TINY * 2;
      }
      memset(&mask->data[y * w + x], 0xFF, l);
      y += SLIDER_TICK_SPACING;
    }
  } else {
    // Create horizontal mask for tick lines
    int sliderTicksCount = w / SLIDER_TICK_SPACING;
    x = 0;
    for (uint8_t i = 0; i <= sliderTicksCount; i++) {
      if (i == 0 || i == sliderTicksCount / 2 || i == sliderTicksCount) {
        y = 0;
        l = h;
      } else {
        y = PAD_TINY;
        l = h - PAD_TINY * 2;
      }
      for (uint8_t n = y; n < y + l; n += 1)
        mask->data[n * w + x] = 0xFF;
      x += SLIDER_TICK_SPACING;
    }
  }

  // TRACE("Created Mask %d %d",w,h);
  tickMasks.emplace_back(mask);

  return mask;
}

void MainViewSlider::setPos()
{
  coord_t x = 0, y = 0;
  if (isVertical) {
    y = divRoundClosest((height() - SLIDER_BAR_SIZE) * (-value + RESX),
                        2 * RESX);
  } else {
    x = divRoundClosest((width() - SLIDER_BAR_SIZE) * (value + RESX),
                        2 * RESX);
  }
  lv_obj_set_pos(sliderIcon->getLvObj(), x, y);
}

void MainViewSlider::checkEvents()
{
  Window::checkEvents();

  int16_t newValue = calibratedAnalogs[potIdx];
  if (value != newValue) {
    value = newValue;
    setPos();
  }
}

MainViewHorizontalSlider::MainViewHorizontalSlider(Window* parent,
                                                   uint8_t idx) :
    MainViewSlider(parent,
                   rect_t{0, 0, HORIZONTAL_SLIDERS_WIDTH, SLIDER_BAR_SIZE},
                   idx, false)
{
}

MainViewVerticalSlider::MainViewVerticalSlider(Window* parent,
                                               const rect_t& rect,
                                               uint8_t idx) :
    MainViewSlider(parent, rect, idx, true)
{
}

MainView6POS::MainView6POS(Window* parent, uint8_t idx) :
    Window(parent, rect_t{0, 0, MULTIPOS_W, MainViewSlider::SLIDER_BAR_SIZE}), idx(idx)
{
  char num[] = " ";
  coord_t x = MULTIPOS_W_SPACING / 4 + MainViewSlider::SLIDER_BAR_SIZE / 4;
  for (uint8_t value = 0; value < XPOTS_MULTIPOS_COUNT; value++) {
    num[0] = value + '1';
    auto p = lv_label_create(lvobj);
    lv_label_set_text(p, num);
    lv_obj_set_size(p, MULTIPOS_SZ, MULTIPOS_SZ);
    lv_obj_set_pos(p, x, 0);
    etx_txt_color(p, COLOR_THEME_SECONDARY1_INDEX, LV_PART_MAIN);
    etx_font(p, FONT_XS_INDEX, LV_PART_MAIN);
    x += MULTIPOS_W_SPACING;
  }

  posIcon = new SliderIcon(this);
  posVal = lv_label_create(posIcon->getLvObj());
  lv_obj_set_pos(posVal, PAD_THREE, -PAD_TINY);
  lv_obj_set_size(posVal, MULTIPOS_SZ, MULTIPOS_SZ);
  etx_txt_color(posVal, COLOR_THEME_PRIMARY2_INDEX, LV_PART_MAIN);
  etx_font(posVal, FONT_BOLD_INDEX, LV_PART_MAIN);

  checkEvents();
}

void MainView6POS::checkEvents()
{
  Window::checkEvents();
  int16_t newValue = getXPotPosition(idx);
  if (value != newValue) {
    value = newValue;

    coord_t x = MULTIPOS_W_SPACING / 4 + MULTIPOS_W_SPACING * value;
    lv_obj_set_pos(posIcon->getLvObj(), x, 0);

    char num[] = " ";
    num[0] = value + '1';
    lv_label_set_text(posVal, num);
  }
}
