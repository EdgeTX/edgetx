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

#include "channel_bar.h"

#define VIEW_CHANNELS_LIMIT_PCT \
  (g_model.extendedLimits ? LIMIT_EXT_PERCENT : LIMIT_STD_PERCENT)
#define CHANNELS_LIMIT (g_model.extendedLimits ? LIMIT_EXT_MAX : LIMIT_STD_MAX)

ChannelBar::ChannelBar(Window* parent, const rect_t& rect, uint8_t channel) :
    Window(parent, rect), channel(channel)
{
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_PRIMARY2), 0);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_COVER, 0);
}

void MixerChannelBar::paint(BitmapBuffer * dc)
{
  int chanVal = calcRESXto100(ex_chans[channel]);
  const int displayVal = chanVal;

  // this could be handled nicer, but slower, by checking actual range for this
  // mixer
  chanVal =
      limit<int>(-VIEW_CHANNELS_LIMIT_PCT, chanVal, VIEW_CHANNELS_LIMIT_PCT);

  // Draw mixer bar
  if (chanVal > 0) {
    dc->drawSolidFilledRect(
        0 + width() / 2, 0,
        divRoundClosest(chanVal * width(), VIEW_CHANNELS_LIMIT_PCT * 2),
        height(), COLOR_THEME_FOCUS);

    dc->drawNumber(width() / 2 - 10, -2, displayVal,
                   FONT(XS) | COLOR_THEME_SECONDARY1 | RIGHT, 0, nullptr, "%");

  } else if (chanVal < 0) {
    const unsigned endpoint = width() / 2;
    const unsigned size =
        divRoundClosest(-chanVal * width(), VIEW_CHANNELS_LIMIT_PCT * 2);

    dc->drawSolidFilledRect(endpoint - size, 0, size, height(),
                            COLOR_THEME_FOCUS);

    dc->drawNumber(10 + width() / 2, -2, displayVal,
                   FONT(XS) | COLOR_THEME_SECONDARY1, 0, nullptr, "%");
  }

  if (drawMiddleBar) {
    // Draw middle bar
    dc->drawSolidVerticalLine(width() / 2, 0, height(), COLOR_THEME_SECONDARY1);
  }
}

void MixerChannelBar::checkEvents()
{
  Window::checkEvents();
  int newValue = ex_chans[channel];
  if (value != newValue) {
    value = newValue;
    invalidate();
  }
}

static inline unsigned posOnBar(coord_t width, int value_to100)
{
  return divRoundClosest(
      (value_to100 + VIEW_CHANNELS_LIMIT_PCT) * (width - 1),
      VIEW_CHANNELS_LIMIT_PCT * 2);
}

static void drawOutputBarLimits(BitmapBuffer* dc, coord_t left, coord_t right,
                                LcdFlags color)
{
  dc->drawSolidVerticalLine(left, 0, BAR_HEIGHT, color);
  dc->drawSolidHorizontalLine(left, 0, 3, color);
  dc->drawSolidHorizontalLine(left, BAR_HEIGHT - 1, 3, color);

  dc->drawSolidVerticalLine(right, 0, BAR_HEIGHT, color);
  dc->drawSolidHorizontalLine(right - 3, 0, 3, color);
  dc->drawSolidHorizontalLine(right - 3, BAR_HEIGHT - 1, 3, color);
}

void OutputChannelBar::paint(BitmapBuffer* dc)
{
  int chanVal = calcRESXto100(channelOutputs[channel]);
  int displayVal = chanVal;

  chanVal =
      limit<int>(-VIEW_CHANNELS_LIMIT_PCT, chanVal, VIEW_CHANNELS_LIMIT_PCT);

  // Draw output bar
  if (chanVal > 0) {
    dc->drawSolidFilledRect(
        width() / 2, 0,
        divRoundClosest(chanVal * width(), VIEW_CHANNELS_LIMIT_PCT * 2),
        height(), COLOR_THEME_ACTIVE);

    dc->drawNumber(width() / 2 - 10, -2, displayVal,
                   FONT(XS) | COLOR_THEME_SECONDARY1 | RIGHT, 0, nullptr, "%");

  } else if (chanVal < 0) {
    unsigned endpoint = width() / 2;
    unsigned size =
        divRoundClosest(-chanVal * width(), VIEW_CHANNELS_LIMIT_PCT * 2);

    dc->drawSolidFilledRect(endpoint - size, 0, size, height(),
                            COLOR_THEME_ACTIVE);

    dc->drawNumber(width() / 2 + 10, -2, displayVal,
                   FONT(XS) | COLOR_THEME_SECONDARY1, 0, nullptr, "%");
  }

  // Draw middle bar
  dc->drawSolidVerticalLine(width() / 2, 0, height(), COLOR_THEME_SECONDARY1);

  if (drawLimits) {
    // Draw output limits bars
    int limit = CHANNELS_LIMIT;
    LimitData* ld = limitAddress(channel);
    int32_t ldMax;
    int32_t ldMin;

    if (GV_IS_GV_VALUE(ld->min, -limit, 0)) {
      ldMin = limMin;
    } else {
      ldMin = ld->min;
    }

    if (GV_IS_GV_VALUE(ld->max, 0, limit)) {
      ldMax = limMax;
    } else {
      ldMax = ld->max;
    }

    if (ld && ld->revert) {
      drawOutputBarLimits(dc, posOnBar(width(), -100 - ldMax / 10),
                          posOnBar(width(), 100 - ldMin / 10),
                          outputBarLimitsColor);
    } else if (ld) {
      drawOutputBarLimits(dc, posOnBar(width(), -100 + ldMin / 10),
                          posOnBar(width(), 100 + ldMax / 10),
                          outputBarLimitsColor);
    }
  }
}

void OutputChannelBar::checkEvents()
{
  Window::checkEvents();
  int newValue = channelOutputs[channel];
  if (value != newValue) {
    value = newValue;
    invalidate();
  }
  int limit = CHANNELS_LIMIT;
  LimitData* lim = limitAddress(channel);

  if (GV_IS_GV_VALUE(lim->min, -limit, 0)) {
    int ldMin = GET_GVAR_PREC1(lim->min, -limit, 0, mixerCurrentFlightMode) +
                LIMIT_STD_MAX;
    if (limMin != ldMin) invalidate();
    limMin = ldMin;
  }
  if (GV_IS_GV_VALUE(lim->max, 0, limit)) {
    int ldMax = GET_GVAR_PREC1(lim->max, 0, limit, mixerCurrentFlightMode) -
                LIMIT_STD_MAX;
    if (limMax != ldMax) invalidate();
    limMax = ldMax;
  }
}
