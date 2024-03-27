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

constexpr coord_t ROW_HEIGHT = 42;
constexpr coord_t BAR_HEIGHT = 13;
constexpr coord_t LMARGIN = 15;
constexpr coord_t TMARGIN = 2;

class ChannelBar : public Window
{
 public:
  ChannelBar(Window* parent, const rect_t& rect,
             std::function<int16_t()> getValue, LcdFlags barColor,
             LcdFlags textColor = COLOR_THEME_SECONDARY1);

 protected:
  int16_t value = -10000;
  std::function<int16_t()> getValue;
  lv_obj_t* valText = nullptr;
  lv_point_t divPoints[2];
  lv_obj_t* bar = nullptr;

  void checkEvents() override;
};

class MixerChannelBar : public ChannelBar
{
 public:
  MixerChannelBar(Window* parent, const rect_t& rect, uint8_t channel);
};

class OutputChannelBar : public ChannelBar
{
 public:
  OutputChannelBar(Window* parent, const rect_t& rect, uint8_t channel,
                   bool editColor = false, bool drawLimits = true);

 protected:
  uint8_t channel = 0;
  int limMax = 0;
  int limMin = 0;
  bool drawLimits = true;
  lv_point_t limPoints[9];
  lv_obj_t* leftLim = nullptr;
  lv_obj_t* rightLim = nullptr;

  void drawLimitLines(bool forced);

  void checkEvents() override;
};

class ComboChannelBar : public Window
{
 public:
  // using ChannelBar::ChannelBar;
  ComboChannelBar(Window* parent, const rect_t& rect, uint8_t channel,
                  bool isInHeader = false);

 protected:
  uint8_t channel;
  OutputChannelBar* outputChannelBar = nullptr;
#if defined(OVERRIDE_CHANNEL_FUNCTION)
  StaticIcon* overrideIcon = nullptr;

  void checkEvents() override;
#endif
};
