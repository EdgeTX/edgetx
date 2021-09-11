/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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
 * This program is distributed in the hope that it width()ill be useful,
 * but WITHOUT ANY WARRANTY; width()ithout even the implied width()arranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#pragma once

#include "opentx.h"
#include "libopenui.h"
#include "static.h"

constexpr coord_t ROW_HEIGHT = 42;
constexpr coord_t BAR_HEIGHT = 13;
constexpr coord_t COLUMN_SIZE = 200;
constexpr coord_t X_OFFSET = 25;
constexpr coord_t LEG_COLORBOX = 15;

#define VIEW_CHANNELS_LIMIT_PCT        (g_model.extendedLimits ? LIMIT_EXT_PERCENT : 100)

class ChannelBar : public Window
{
  public:
    ChannelBar(Window * parent, const rect_t & rect, uint8_t channel):
      Window(parent, rect),
      channel(channel)
    {
    }
    
  protected:
    uint8_t channel = 0;
};

class MixerChannelBar : public ChannelBar
{
  public:
    using ChannelBar::ChannelBar;

    void paint(BitmapBuffer * dc) override
    {
      int chanVal = calcRESXto100(ex_chans[channel]);
      const int displayVal = chanVal;

      // this could be handled nicer, but slower, by checking actual range for this mixer
      chanVal = limit<int>(-VIEW_CHANNELS_LIMIT_PCT, chanVal, VIEW_CHANNELS_LIMIT_PCT);

      //  Draw Background
      dc->drawSolidFilledRect(0, 0, width(), height(), COLOR_THEME_PRIMARY2);

      // Draw mixer bar
      if (chanVal > 0) {

        dc->drawSolidFilledRect(
            0 + width() / 2, 0,
            divRoundClosest(chanVal * width(), VIEW_CHANNELS_LIMIT_PCT * 2),
            height(), COLOR_THEME_FOCUS);

        dc->drawNumber(width() / 2 - 10, 0, displayVal,
                       FONT(XS) | COLOR_THEME_SECONDARY1 | RIGHT, 0, nullptr, "%");

      } else if (chanVal < 0) {

        const unsigned endpoint = width() / 2;
        const unsigned size =
            divRoundClosest(-chanVal * width(), VIEW_CHANNELS_LIMIT_PCT * 2);

        dc->drawSolidFilledRect(endpoint - size, 0, size, height(),
                                COLOR_THEME_FOCUS);

        dc->drawNumber(10 + width() / 2, 0, displayVal,
                       FONT(XS) | COLOR_THEME_SECONDARY1, 0, nullptr, "%");
      }

      // Draw middle bar
      dc->drawSolidVerticalLine(width() / 2, 0, height(), COLOR_THEME_SECONDARY1);
    }
    
    void checkEvents() override
    {
      Window::checkEvents();
      int newValue = ex_chans[channel];
      if (value != newValue) {
        value = newValue;
        invalidate();
      }
    }

  protected:
    int value = 0;
};

class OutputChannelBar : public ChannelBar
{
  public:
    using ChannelBar::ChannelBar;

    void paint(BitmapBuffer * dc) override
    {
      int chanVal = calcRESXto100(channelOutputs[channel]);
      int displayVal = chanVal;

      chanVal = limit<int>(-VIEW_CHANNELS_LIMIT_PCT, chanVal,
                           VIEW_CHANNELS_LIMIT_PCT);

      //  Draw Background
      dc->drawSolidFilledRect(0, 0, width(), height(), COLOR_THEME_PRIMARY2);

      // Draw output bar
      if (chanVal > 0) {

        dc->drawSolidFilledRect(
            width() / 2, 0,
            divRoundClosest(chanVal * width(), VIEW_CHANNELS_LIMIT_PCT * 2),
            height(), COLOR_THEME_ACTIVE);

        dc->drawNumber(width() / 2 - 10, 0, displayVal,
                       FONT(XS) | COLOR_THEME_SECONDARY1 | RIGHT, 0, nullptr, "%");

      } else if (chanVal < 0) {
        unsigned endpoint = width() / 2;
        unsigned size =
            divRoundClosest(-chanVal * width(), VIEW_CHANNELS_LIMIT_PCT * 2);

        dc->drawSolidFilledRect(endpoint - size, 0, size, height(),
                                COLOR_THEME_ACTIVE);

        dc->drawNumber(width() / 2 + 10, 0, displayVal,
                       FONT(XS) | COLOR_THEME_SECONDARY1, 0, nullptr, "%");
      }

      // Draw middle bar
      dc->drawSolidVerticalLine(width() / 2, 0, height(), COLOR_THEME_SECONDARY1);

      // Draw output limits bars
      int limit = (g_model.extendedLimits ? LIMIT_EXT_MAX : 1000);
      LimitData* ld = limitAddress(channel);
      int32_t ldMax;
      int32_t ldMin;

      if (GV_IS_GV_VALUE(ld->min - 1000, -limit, 0)) {
        ldMin = limMin;
      } else {
        ldMin = ld->min;
      }

      if (GV_IS_GV_VALUE(ld->max + 1000, 0, limit)) {
        ldMax = limMax;
      } else {
        ldMax = ld->max;
      }

      if (ld && ld->revert) {
        drawOutputBarLimits(dc, posOnBar(-100 - ldMax / 10),
                            posOnBar(100 - ldMin / 10));
      } else if (ld) {
        drawOutputBarLimits(dc, posOnBar(-100 + ldMin / 10),
                            posOnBar(100 + ldMax / 10));
      }
    }

    inline unsigned posOnBar(int value_to100)
    {
      return divRoundClosest((value_to100 + VIEW_CHANNELS_LIMIT_PCT) * (width() - 1), VIEW_CHANNELS_LIMIT_PCT * 2);
    }

    void drawOutputBarLimits(BitmapBuffer * dc, coord_t left, coord_t right)
    {
      dc->drawSolidVerticalLine(left, 0, BAR_HEIGHT, COLOR_THEME_SECONDARY1);
      dc->drawSolidHorizontalLine(left, 0, 3, COLOR_THEME_SECONDARY1);
      dc->drawSolidHorizontalLine(left, BAR_HEIGHT - 1, 3, COLOR_THEME_SECONDARY1);

      dc->drawSolidVerticalLine(right, 0, BAR_HEIGHT, COLOR_THEME_SECONDARY1);
      dc->drawSolidHorizontalLine(right - 3, 0, 3, COLOR_THEME_SECONDARY1);
      dc->drawSolidHorizontalLine(right - 3, BAR_HEIGHT - 1, 3, COLOR_THEME_SECONDARY1);
    }

    void checkEvents() override
    {
      Window::checkEvents();
      int newValue = channelOutputs[channel];
      if (value != newValue) {
        value = newValue;
        invalidate();
      }
      int limit = (g_model.extendedLimits ? LIMIT_EXT_MAX : 1000);
      LimitData* lim = limitAddress(channel);
      
      if (GV_IS_GV_VALUE(lim->min - 1000, -limit, 0)) {
        int ldMin =
            GET_GVAR_PREC1(lim->min - 1000, -limit, 0, mixerCurrentFlightMode) +
            1000;
        if (limMin != ldMin) invalidate();
        limMin = ldMin;
      }
      if (GV_IS_GV_VALUE(lim->max + 1000, 0, limit)) {
        int ldMax =
            GET_GVAR_PREC1(lim->max + 1000, 0, limit, mixerCurrentFlightMode) -
            1000;
        if (limMax != ldMax)  invalidate();
        limMax = ldMax;                  
      }
    }

  protected:
    int value = 0;
    int limMax = 0;
    int limMin = 0;
};

constexpr coord_t lmargin = 25;

class ComboChannelBar : public ChannelBar
{
  public:
    using ChannelBar::ChannelBar;
    ComboChannelBar(Window * parent, const rect_t & rect, uint8_t channel):
      ChannelBar(parent, rect, channel)
    {
      new OutputChannelBar(this, {lmargin, BAR_HEIGHT, width() - lmargin, BAR_HEIGHT}, channel);
      new MixerChannelBar(this, {lmargin, 2 * BAR_HEIGHT + 1, width() - lmargin, BAR_HEIGHT}, channel);
    }

    void paint(BitmapBuffer * dc) override
    {
      char chanString[] = "CH32 ";
      int usValue = PPM_CH_CENTER(channel) + channelOutputs[channel] / 2;

      // Channel number
      strAppendSigned(&chanString[2], channel + 1, 2);
      dc->drawText(lmargin, 0, chanString, FONT(XS) | COLOR_THEME_SECONDARY1 | LEFT);

      // Channel name
      dc->drawSizedText(lmargin + 45, 0, g_model.limitData[channel].name, sizeof(g_model.limitData[channel].name), FONT(XS) | COLOR_THEME_SECONDARY1 | LEFT);

      // Channel value in µS
      dc->drawNumber(width(), 0, usValue, FONT(XS) | COLOR_THEME_SECONDARY1 | RIGHT, 0, nullptr, STR_US);

      // Override icon
#if defined(OVERRIDE_CHANNEL_FUNCTION)
      if (safetyCh[channel] != OVERRIDE_CHANNEL_UNDEFINED)
        dc->drawMask(0, 1, chanMonLockedBitmap, COLOR_THEME_SECONDARY1);
#endif

      // Channel reverted icon
      LimitData * ld = limitAddress(channel);
      if (ld && ld->revert) {
        lcd->drawMask(0, 20, chanMonInvertedBitmap, COLOR_THEME_SECONDARY1);
      }
    }

    void checkEvents() override
    {
      Window::checkEvents();
      int newValue = channelOutputs[channel];
      if (value != newValue) {
        value = newValue;
        invalidate();
      }
    }

  protected:
    int value = 0;
};
