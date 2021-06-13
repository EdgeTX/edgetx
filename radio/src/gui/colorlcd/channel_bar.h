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
      dc->drawSolidFilledRect(0, 0, width(), height(), FIELD_BGCOLOR);

      // Draw mixer bar
      if (chanVal > 0) {

        dc->drawSolidFilledRect(
            0 + width() / 2, 0,
            divRoundClosest(chanVal * width(), VIEW_CHANNELS_LIMIT_PCT * 2),
            height(), FOCUS_BGCOLOR);

        dc->drawNumber(width() / 2 - 10, 0, displayVal,
                       FONT(XS) | DEFAULT_COLOR | RIGHT, 0, nullptr, "%");

      } else if (chanVal < 0) {

        const unsigned endpoint = width() / 2;
        const unsigned size =
            divRoundClosest(-chanVal * width(), VIEW_CHANNELS_LIMIT_PCT * 2);

        dc->drawSolidFilledRect(endpoint - size, 0, size, height(),
                                FOCUS_BGCOLOR);

        dc->drawNumber(10 + width() / 2, 0, displayVal,
                       FONT(XS) | DEFAULT_COLOR, 0, nullptr, "%");
      }

      // Draw middle bar
      dc->drawSolidVerticalLine(width() / 2, 0, height(), DEFAULT_COLOR);
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
      dc->drawSolidFilledRect(0, 0, width(), height(), FIELD_BGCOLOR);

      // Draw output bar
      if (chanVal > 0) {

        dc->drawSolidFilledRect(
            width() / 2, 0,
            divRoundClosest(chanVal * width(), VIEW_CHANNELS_LIMIT_PCT * 2),
            height(), HIGHLIGHT_COLOR);

        dc->drawNumber(width() / 2 - 10, 0, displayVal,
                       FONT(XS) | DEFAULT_COLOR | RIGHT, 0, nullptr, "%");

      } else if (chanVal < 0) {
        unsigned endpoint = width() / 2;
        unsigned size =
            divRoundClosest(-chanVal * width(), VIEW_CHANNELS_LIMIT_PCT * 2);

        dc->drawSolidFilledRect(endpoint - size, 0, size, height(),
                                HIGHLIGHT_COLOR);

        dc->drawNumber(width() / 2 + 10, 0, displayVal,
                       FONT(XS) | DEFAULT_COLOR, 0, nullptr, "%");
      }

      // Draw middle bar
      dc->drawSolidVerticalLine(width() / 2, 0, height(), DEFAULT_COLOR);

      // Draw output limits bars
      LimitData* ld = limitAddress(channel);
      if (ld && ld->revert) {
        drawOutputBarLimits(dc, posOnBar(-100 - ld->max / 10),
                            posOnBar(100 - ld->min / 10));
      } else if (ld) {
        drawOutputBarLimits(dc, posOnBar(-100 + ld->min / 10),
                            posOnBar(100 + ld->max / 10));
      }
    }

    inline unsigned posOnBar(int value_to100)
    {
      return divRoundClosest((value_to100 + VIEW_CHANNELS_LIMIT_PCT) * (width() - 1), VIEW_CHANNELS_LIMIT_PCT * 2);
    }

    void drawOutputBarLimits(BitmapBuffer * dc, coord_t left, coord_t right)
    {
      dc->drawSolidVerticalLine(left, 0, BAR_HEIGHT, DEFAULT_COLOR);
      dc->drawSolidHorizontalLine(left, 0, 3, DEFAULT_COLOR);
      dc->drawSolidHorizontalLine(left, BAR_HEIGHT - 1, 3, DEFAULT_COLOR);

      dc->drawSolidVerticalLine(right, 0, BAR_HEIGHT, DEFAULT_COLOR);
      dc->drawSolidHorizontalLine(right - 3, 0, 3, DEFAULT_COLOR);
      dc->drawSolidHorizontalLine(right - 3, BAR_HEIGHT - 1, 3, DEFAULT_COLOR);
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
      dc->drawText(lmargin, 0, chanString, FONT(XS) | DEFAULT_COLOR | LEFT);

      // Channel name
      dc->drawSizedText(lmargin + 45, 0, g_model.limitData[channel].name, sizeof(g_model.limitData[channel].name), FONT(XS) | DEFAULT_COLOR | LEFT);

      // Channel value in µS
      dc->drawNumber(width(), 0, usValue, FONT(XS) | DEFAULT_COLOR | RIGHT, 0, nullptr, STR_US);

      // Override icon
#if defined(OVERRIDE_CHANNEL_FUNCTION)
      if (safetyCh[channel] != OVERRIDE_CHANNEL_UNDEFINED)
        dc->drawMask(0, 1, chanMonLockedBitmap, DEFAULT_COLOR);
#endif

      // Channel reverted icon
      LimitData * ld = limitAddress(channel);
      if (ld && ld->revert) {
        lcd->drawMask(0, 20, chanMonInvertedBitmap, DEFAULT_COLOR);
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
