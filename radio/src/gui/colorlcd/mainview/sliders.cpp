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

bool SliderIcon::maskCreated = false;
uint8_t SliderIcon::maskBuffer[];

SliderIcon::SliderIcon(Window* parent) :
    Window(parent, rect_t{0, 0, SLIDER_ICON_SIZE, SLIDER_ICON_SIZE})
{
  setWindowFlag(NO_FOCUS);

  shadow = lv_canvas_create(lvobj);
  lv_canvas_set_buffer(shadow, maskBuffer, SLIDER_MASK_SIZE, SLIDER_MASK_SIZE, LV_IMG_CF_ALPHA_8BIT);
  etx_img_color(shadow, COLOR_THEME_PRIMARY1_INDEX);
  lv_obj_set_style_img_opa(shadow, LV_OPA_40, LV_PART_MAIN);
  lv_obj_set_pos(shadow, 2, 2);

  mask = lv_canvas_create(lvobj);
  lv_canvas_set_buffer(mask, maskBuffer, SLIDER_MASK_SIZE, SLIDER_MASK_SIZE, LV_IMG_CF_ALPHA_8BIT);
  etx_img_color(mask, COLOR_THEME_FOCUS_INDEX);
  lv_obj_set_pos(mask, 1, 1);

  if (!maskCreated) {
    maskCreated = true;
    lv_canvas_fill_bg(mask, lv_color_black(), LV_OPA_TRANSP);
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_opa = LV_OPA_COVER;
    rect_dsc.radius = PAD_THREE;
    lv_canvas_draw_rect(mask, 0, 0, SLIDER_MASK_SIZE, SLIDER_MASK_SIZE, &rect_dsc);
  }
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
      lv_obj_set_pos(maskCanvas, 2, SliderIcon::SLIDER_ICON_SIZE / 2);
    } else {
      lv_obj_set_pos(maskCanvas, SliderIcon::SLIDER_ICON_SIZE / 2, 2);
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
    h = height() - SliderIcon::SLIDER_ICON_SIZE + 1;
    if ((h & 1) == 0) h += 1;
    w = MASK_SHORT_DIM;
  } else {
    w = width() - SliderIcon::SLIDER_ICON_SIZE + 1;
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
    y = divRoundClosest((height() - SliderIcon::SLIDER_ICON_SIZE) * (-value + RESX),
                        2 * RESX);
  } else {
    x = divRoundClosest((width() - SliderIcon::SLIDER_ICON_SIZE) * (value + RESX),
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
                   rect_t{0, 0, HORIZONTAL_SLIDERS_WIDTH, SliderIcon::SLIDER_ICON_SIZE},
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
    Window(parent, rect_t{0, 0, MULTIPOS_W, SliderIcon::SLIDER_ICON_SIZE}), idx(idx)
{
  char num[] = " ";
  coord_t x = MULTIPOS_W_SPACING / 4 + SliderIcon::SLIDER_ICON_SIZE / 4;
  for (uint8_t value = 0; value < XPOTS_MULTIPOS_COUNT; value++) {
    num[0] = value + '1';
    auto p = etx_label_create(lvobj, FONT_XS_INDEX);
    lv_label_set_text(p, num);
    lv_obj_set_size(p, MULTIPOS_SZ, MULTIPOS_SZ);
    lv_obj_set_pos(p, x, 0);
    etx_txt_color(p, COLOR_THEME_SECONDARY1_INDEX, LV_PART_MAIN);
    x += MULTIPOS_W_SPACING;
  }

  posIcon = new SliderIcon(this);
  posVal = etx_label_create(posIcon->getLvObj(), FONT_BOLD_INDEX);
  lv_obj_set_pos(posVal, PAD_SMALL, -PAD_TINY);
  lv_obj_set_size(posVal, MULTIPOS_SZ, MULTIPOS_SZ);
  etx_txt_color(posVal, COLOR_THEME_PRIMARY2_INDEX, LV_PART_MAIN);

  checkEvents();
}

void MainView6POS::checkEvents()
{
  Window::checkEvents();
  int16_t newValue = getXPotPosition(idx);
  if (value != newValue) {
    value = newValue;

    coord_t x = MULTIPOS_W_SPACING / 4 + MULTIPOS_W_SPACING * value - 1;
    lv_obj_set_pos(posIcon->getLvObj(), x, 0);

    char num[] = " ";
    num[0] = value + '1';
    lv_label_set_text(posVal, num);
  }
}
