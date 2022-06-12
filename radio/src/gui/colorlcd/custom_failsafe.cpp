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

#include "custom_failsafe.h"
#include "opentx.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

class ChannelFailsafeBargraph : public Window
{
 public:
  ChannelFailsafeBargraph(Window* parent, const rect_t& rect, uint8_t moduleIdx,
                          uint8_t channel) :
      Window(parent, rect), moduleIdx(moduleIdx), channel(channel)
  {
  }

  void checkEvents() override
  {
    invalidate();
    Window::checkEvents();
  }

  void paint(BitmapBuffer* dc) override
  {
    int32_t failsafeValue = g_model.failsafeChannels[channel];
    int32_t channelValue = channelOutputs[channel];

    const int lim =
        (g_model.extendedLimits ? (512 * LIMIT_EXT_PERCENT / 100) : 512) * 2;

    coord_t x = 0;
    dc->drawRect(x, 0, width(), height());
    const coord_t lenChannel = limit(
        (uint8_t)1, uint8_t((abs(channelValue) * width() / 2 + lim / 2) / lim),
        uint8_t(width() / 2));
    const coord_t lenFailsafe = limit(
        (uint8_t)1, uint8_t((abs(failsafeValue) * width() / 2 + lim / 2) / lim),
        uint8_t(width() / 2));
    x += width() / 2;
    const coord_t xChannel = (channelValue > 0) ? x : x + 1 - lenChannel;
    const coord_t xFailsafe = (failsafeValue > 0) ? x : x + 1 - lenFailsafe;
    dc->drawSolidFilledRect(xChannel, +2, lenChannel, (height() / 2) - 3,
                            COLOR_THEME_SECONDARY1);
    dc->drawSolidFilledRect(xFailsafe, (height() / 2) + 1, lenFailsafe,
                            (height() / 2) - 3, COLOR_THEME_WARNING);
  }

 protected:
  uint8_t moduleIdx;
  uint8_t channel;
};

class FailSafeBody : public FormGroup
{
 public:
  FailSafeBody(FormGroup* parent, const rect_t& rect, uint8_t moduleIdx) :
      FormGroup(parent, rect, FORM_FORWARD_FOCUS), moduleIdx(moduleIdx)
  {
    build();
  }

  void checkEvents() override
  {
    invalidate();
    FormGroup::checkEvents();
  }

  void build()
  {
    FormGridLayout grid;
    grid.setLabelWidth(60);
    grid.spacer(8);

    const int lim = calcRESXto1000(
        (g_model.extendedLimits ? (512 * LIMIT_EXT_PERCENT / 100) : 512) * 2);

    for (int ch = 0; ch < maxModuleChannels(moduleIdx); ch++) {
      // Channel name
      // TODO if (g_model.limitData[ch].name[0] != '\0') { <= add channel name
      new StaticText(this, grid.getLabelSlot(),
                     getSourceString(MIXSRC_CH1 + ch), 0, COLOR_THEME_PRIMARY1);

      // Channel numeric value
      new NumberEdit(
          this, grid.getFieldSlot(8, 0), -lim, lim,
          [=]() { return calcRESXto1000(g_model.failsafeChannels[ch]); },
          [=](int32_t newValue) {
            g_model.failsafeChannels[ch] = calc1000toRESX(newValue);
            SET_DIRTY();
          },
          0, PREC1 | RIGHT);

      // Channel bargraph
      new ChannelFailsafeBargraph(
          this, {180, grid.getWindowHeight(), 150, PAGE_LINE_HEIGHT}, moduleIdx,
          ch);
      grid.nextLine();
    }

    grid.spacer();
    auto out2fail =
        new TextButton(this, grid.getLineSlot(), STR_CHANNELS2FAILSAFE);
    out2fail->setPressHandler([=]() {
      setCustomFailsafe(moduleIdx);
      AUDIO_WARNING1();
      SET_DIRTY();
      return 0;
    });

    grid.nextLine();
  }

 protected:
  uint8_t moduleIdx;
};

FailSafePage::FailSafePage(uint8_t moduleIdx) :
    Page(ICON_STATS_ANALOGS), moduleIdx(moduleIdx)
{
  new FailSafeBody(&body, {0, 0, LCD_W, body.height()}, moduleIdx);
  new StaticText(&header,
                 {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                  PAGE_LINE_HEIGHT},
                 STR_FAILSAFESET, 0, COLOR_THEME_PRIMARY2);
}
