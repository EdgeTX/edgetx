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

#pragma once

#include "opentx.h"
#include "libopenui.h"
#include "static.h"

constexpr coord_t ROW_HEIGHT = 42;
constexpr coord_t BAR_HEIGHT = 13;
constexpr coord_t LEG_COLORBOX = 15;

class ChannelBar : public Window
{
 public:
  ChannelBar(Window* parent, const rect_t& rect, uint8_t channel);

  void setChannel(uint8_t ch)
  {
    channel = ch;
    invalidate();
  }

 protected:
  uint8_t channel = 0;
};

class MixerChannelBar : public ChannelBar
{
 public:
  using ChannelBar::ChannelBar;

  void setDrawMiddleBar(bool enable) { drawMiddleBar = enable; }

  void paint(BitmapBuffer* dc) override;
  void checkEvents() override;

 protected:
  bool drawMiddleBar = true;
  int value = 0;
};

class OutputChannelBar : public ChannelBar
{
 public:
  using ChannelBar::ChannelBar;

  void setDrawLimits(bool enable) { drawLimits = enable; }
  void setOutputBarLimitColor(uint32_t color) { outputBarLimitsColor = color; }

  void paint(BitmapBuffer* dc) override;
  void checkEvents() override;

 protected:
  int value = 0;
  int limMax = 0;
  int limMin = 0;
  bool drawLimits = true;
  LcdFlags outputBarLimitsColor = COLOR_THEME_SECONDARY1;
};

constexpr coord_t lmargin = 25;

class ComboChannelBar : public Window
{
  public:
    // using ChannelBar::ChannelBar;
    ComboChannelBar(Window * parent, const rect_t & rect, uint8_t channel):
      Window(parent, rect), channel(channel)
    {
      outputChannelBar = new OutputChannelBar(
          this, {leftMargin, BAR_HEIGHT, width() - leftMargin, BAR_HEIGHT},
          channel);
      new MixerChannelBar(
          this,
          {leftMargin, 2 * BAR_HEIGHT + 1, width() - leftMargin, BAR_HEIGHT},
          channel);
    }

    void setOutputChannelBarLimitColor(uint32_t color)
    {
      if (outputChannelBar != nullptr) {
        outputChannelBar->setOutputBarLimitColor(color);
      }
    }

    void setTextColor(uint32_t color)
    {
      textColor = color;
      invalidate();
    }

    void setLeftMargin(int margin)
    {
      leftMargin = margin; 
    }

    void paint(BitmapBuffer * dc) override
    {
      char chanString[] = "CH32 ";
      int usValue = PPM_CH_CENTER(channel) + channelOutputs[channel] / 2;

      // Channel number
      strAppendSigned(&chanString[2], channel + 1, 2);
      dc->drawText(leftMargin, 0, chanString, FONT(XS) | textColor | LEFT);

      // Channel name
      dc->drawSizedText(leftMargin + 45, 0, g_model.limitData[channel].name, sizeof(g_model.limitData[channel].name), FONT(XS) | textColor | LEFT);

      // Channel value in µS
      dc->drawNumber(width(), 0, usValue, FONT(XS) | textColor | RIGHT, 0, nullptr, STR_US);

      // Override icon
#if defined(OVERRIDE_CHANNEL_FUNCTION)
      if (safetyCh[channel] != OVERRIDE_CHANNEL_UNDEFINED)
        dc->drawMask(0, 1, chanMonLockedBitmap, textColor);
#endif

      // Channel reverted icon
      LimitData * ld = limitAddress(channel);
      if (ld && ld->revert) {
        dc->drawMask(0, 20, chanMonInvertedBitmap, textColor);
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
#if defined(OVERRIDE_CHANNEL_FUNCTION)
      int newSafetyChValue = safetyCh[channel];
      if (safetyChValue != newSafetyChValue)
      {
        safetyChValue = newSafetyChValue;
        invalidate();
      }
#endif
    }

  protected:
    uint8_t channel;
    OutputChannelBar *outputChannelBar = nullptr;
    int value = 0;
    int leftMargin = lmargin;
    uint32_t textColor = COLOR_THEME_SECONDARY1;
#if defined(OVERRIDE_CHANNEL_FUNCTION)
    int safetyChValue = OVERRIDE_CHANNEL_UNDEFINED;
#endif
};
