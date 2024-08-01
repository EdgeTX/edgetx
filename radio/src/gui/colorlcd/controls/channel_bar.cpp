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

#include "bitmaps.h"
#include "etx_lv_theme.h"

#define VIEW_CHANNELS_LIMIT_PCT \
  (g_model.extendedLimits ? LIMIT_EXT_PERCENT : LIMIT_STD_PERCENT)

#define CHANNELS_LIMIT (g_model.extendedLimits ? LIMIT_EXT_MAX : LIMIT_STD_MAX)

ChannelBar::ChannelBar(Window* parent, const rect_t& rect, uint8_t channel,
                       std::function<int16_t()> getValueFunc, LcdColorIndex barColorIndex,
                       LcdColorIndex txtColorIndex) :
    Window(parent, rect), channel(channel),
    getValue(std::move(getValueFunc))
{
  etx_solid_bg(lvobj, COLOR_THEME_PRIMARY2_INDEX);

  bar = lv_obj_create(lvobj);
  etx_solid_bg(bar, barColorIndex);
  lv_obj_set_pos(bar, width() / 2, 0);
  lv_obj_set_size(bar, 0, height());

  coord_t yo = (height() < 10) ? -1 : VAL_YO;

  valText = lv_label_create(lvobj);
  lv_obj_set_pos(valText, width() / 2 + VAL_XO, yo);
  lv_obj_set_size(valText, VAL_W, VAL_H);
  etx_obj_add_style(valText, styles->text_align_left, LV_PART_MAIN);
  lv_obj_set_style_translate_x(valText, VAL_XT, LV_STATE_USER_1);
  etx_obj_add_style(valText, styles->text_align_right, LV_STATE_USER_1);
  etx_font(valText, FONT_XS_INDEX);
  etx_txt_color(valText, txtColorIndex);
  lv_label_set_text(valText, "");

  divPoints[0] = {(lv_coord_t)(width() / 2), 0};
  divPoints[1] = {(lv_coord_t)(width() / 2), (lv_coord_t)height()};

  lv_obj_t* divLine = lv_line_create(lvobj);
  etx_obj_add_style(divLine, styles->div_line, LV_PART_MAIN);
  lv_line_set_points(divLine, divPoints, 2);

  checkEvents();
}

void ChannelBar::checkEvents()
{
  Window::checkEvents();

  int newValue = getValue();

  if (value != newValue) {
    value = newValue;

    std::string s;
    if (g_eeGeneral.ppmunit == PPM_US)
      s = formatNumberAsString(PPM_CH_CENTER(channel) + value / 2, 0, 0, "", STR_US);
    else if (g_eeGeneral.ppmunit == PPM_PERCENT_PREC1)
      s = formatNumberAsString(calcRESXto1000(value), PREC1, 0, "", "%");
    else
      s = formatNumberAsString(calcRESXto100(value), 0, 0, "", "%");

    if (s != valStr) {
      valStr = s;
      lv_label_set_text(valText, s.c_str());

      if (s[0] == '-')
        lv_obj_clear_state(valText, LV_STATE_USER_1);
      else
        lv_obj_add_state(valText, LV_STATE_USER_1);

      const int lim = (g_model.extendedLimits ? (1024 * LIMIT_EXT_PERCENT / 100) : 1024);
      int chanVal = limit<int>(-lim, value, lim);

      uint16_t size = divRoundClosest(abs(chanVal) * width(), lim * 2);

      int16_t x = width() / 2 - ((chanVal > 0) ? 0 : size);

      lv_obj_set_pos(bar, x, 0);
      lv_obj_set_size(bar, size, height());
    }
  }
}

//-----------------------------------------------------------------------------

MixerChannelBar::MixerChannelBar(Window* parent, const rect_t& rect,
                                 uint8_t channel) :
    ChannelBar(
        parent, rect, channel, [=] { return ex_chans[channel]; },
        COLOR_THEME_FOCUS_INDEX)
{
}

//-----------------------------------------------------------------------------

OutputChannelBar::OutputChannelBar(Window* parent, const rect_t& rect,
                                   uint8_t channel, bool editColor,
                                   bool drawLimits) :
    ChannelBar(
        parent, rect, channel,
        [=] { return channelOutputs[channel]; },
        COLOR_THEME_ACTIVE_INDEX),
    drawLimits(drawLimits)
{
  if (drawLimits) {
    leftLim = lv_line_create(lvobj);
    if (editColor)
      etx_obj_add_style(leftLim, styles->div_line_edit, LV_PART_MAIN);
    else
      etx_obj_add_style(leftLim, styles->div_line, LV_PART_MAIN);
    rightLim = lv_line_create(lvobj);
    if (editColor)
      etx_obj_add_style(rightLim, styles->div_line_edit, LV_PART_MAIN);
    else
      etx_obj_add_style(rightLim, styles->div_line, LV_PART_MAIN);
    drawLimitLines(true);
  }
}

static inline unsigned posOnBar(coord_t width, int value_to100)
{
  return divRoundClosest((value_to100 + VIEW_CHANNELS_LIMIT_PCT) * (width - 1),
                         VIEW_CHANNELS_LIMIT_PCT * 2);
}

void OutputChannelBar::drawLimitLines(bool forced)
{
  if (drawLimits) {
    // Draw output limits bars
    bool changed = forced;
    LimitData* ld = limitAddress(channel);
    int32_t ldMin;
    int32_t ldMax;

    if (GV_IS_GV_VALUE(ld->min, -CHANNELS_LIMIT, 0)) {
      ldMin =
          GET_GVAR_PREC1(ld->min, -CHANNELS_LIMIT, 0, mixerCurrentFlightMode) +
          LIMIT_STD_MAX;
    } else {
      ldMin = ld->min;
    }
    if (limMin != ldMin) {
      changed = true;
      limMin = ldMin;
    }

    if (GV_IS_GV_VALUE(ld->max, 0, CHANNELS_LIMIT)) {
      ldMax =
          GET_GVAR_PREC1(ld->max, 0, CHANNELS_LIMIT, mixerCurrentFlightMode) -
          LIMIT_STD_MAX;
    } else {
      ldMax = ld->max;
    }
    if (limMax != ldMax) {
      changed = true;
      limMax = ldMax;
    }

    if (changed) {
      lv_coord_t left, right;
      lv_coord_t h = height() - 1;

      if (ld->revert) {
        left = posOnBar(width(), -100 - ldMax / 10);
        right = posOnBar(width(), 100 - ldMin / 10);
      } else {
        left = posOnBar(width(), -100 + ldMin / 10);
        right = posOnBar(width(), 100 + ldMax / 10);
      }

      limPoints[0] = {(lv_coord_t)(left + 3), 0};
      limPoints[1] = {(lv_coord_t)(left), 0};
      limPoints[2] = {(lv_coord_t)(left), h};
      limPoints[3] = {(lv_coord_t)(left + 3), h};
      limPoints[4] = {(lv_coord_t)(right - 2), 0};
      limPoints[5] = {(lv_coord_t)(right), 0};
      limPoints[6] = {(lv_coord_t)(right), h};
      limPoints[7] = {(lv_coord_t)(right - 2), h};
      // Workaround for bugs in LVGL line drawing
      limPoints[8] = {(lv_coord_t)(right + 1), h};

      lv_line_set_points(leftLim, &limPoints[0], 4);
      lv_line_set_points(rightLim, &limPoints[4], 5);
    }
  }
}

void OutputChannelBar::checkEvents()
{
  ChannelBar::checkEvents();
  drawLimitLines(false);
}

//-----------------------------------------------------------------------------

ComboChannelBar::ComboChannelBar(Window* parent, const rect_t& rect,
                                 uint8_t _channel, bool isInHeader) :
    Window(parent, rect), channel(_channel)
{
  LcdColorIndex txtColIdx = isInHeader ? COLOR_THEME_PRIMARY2_INDEX : COLOR_THEME_SECONDARY1_INDEX;

  outputChannelBar = new OutputChannelBar(
      this, {PAD_TINY, ChannelBar::BAR_HEIGHT + PAD_TINY, width() - PAD_TINY, ChannelBar::BAR_HEIGHT},
      channel, isInHeader);

  new MixerChannelBar(
      this,
      {PAD_TINY, (2 * ChannelBar::BAR_HEIGHT) + PAD_TINY + 1, width() - PAD_TINY, ChannelBar::BAR_HEIGHT},
      channel);

  // Channel number
  char chanString[] = TR_CH "32 ";
  strAppendSigned(&chanString[2], channel + 1, 2);
  new StaticText(this, {PAD_TINY, 0, LV_SIZE_CONTENT, ChannelBar::VAL_H}, chanString, 
                 txtColIdx, FONT(XS) | LEFT);

  // Channel name
  if (g_model.limitData[channel].name[0]) {
    char nm[LEN_CHANNEL_NAME + 1];
    strAppend(nm, g_model.limitData[channel].name, LEN_CHANNEL_NAME);
    new StaticText(this, {PAD_TINY + ChannelBar::VAL_W, 0, LV_SIZE_CONTENT, ChannelBar::VAL_H}, nm, 
                   txtColIdx, FONT(XS) | LEFT);
  }

  // Channel value in µS
  const char* suffix = (g_eeGeneral.ppmunit == PPM_US) ? "%" : STR_US;
  new DynamicNumber<int16_t>(
      this, {width() - ChannelBar::VAL_W, 0, ChannelBar::VAL_W, ChannelBar::VAL_H},
      [=] {
        if (g_eeGeneral.ppmunit == PPM_US)
          return calcRESXto100(channelOutputs[channel]);
        return PPM_CH_CENTER(channel) + channelOutputs[channel] / 2;
      },
      txtColIdx, FONT(XS) | RIGHT, "", suffix);

  // Override icon
#if defined(OVERRIDE_CHANNEL_FUNCTION)
  overrideIcon = new StaticIcon(
      this, 0, 5, ICON_CHAN_MONITOR_LOCKED, txtColIdx);
  overrideIcon->show(safetyCh[channel] != OVERRIDE_CHANNEL_UNDEFINED);
#endif

  // Channel reverted icon
  LimitData* ld = limitAddress(channel);
  if (ld && ld->revert) {
    new StaticIcon(this, 0, ICON_SZ, ICON_CHAN_MONITOR_INVERTED,
                   txtColIdx);
  }
}

#if defined(OVERRIDE_CHANNEL_FUNCTION)
void ComboChannelBar::checkEvents()
{
  Window::checkEvents();

  overrideIcon->show(safetyCh[channel] != OVERRIDE_CHANNEL_UNDEFINED);
}
#endif
