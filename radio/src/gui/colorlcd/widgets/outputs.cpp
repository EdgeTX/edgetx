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

#include "opentx.h"
#include "widgets_container_impl.h"

#define RECT_BORDER 1
#define ROW_HEIGHT 17

#define VIEW_CHANNELS_LIMIT_PCT \
  (g_model.extendedLimits ? LIMIT_EXT_PERCENT : 100)

constexpr uint32_t OUTPUTS_REFRESH = 1000 / 5;  // 5 Hz

class OutputsWidget : public Widget
{
 public:
  OutputsWidget(const WidgetFactory* factory, FormGroup* parent,
                const rect_t& rect, Widget::PersistentData* persistentData) :
      Widget(factory, parent, rect, persistentData)
  {
  }

  void refresh(BitmapBuffer* dc) override
  {
    if (width() > 300 && height() > 20)
      twoColumns(dc);
    else if (width() > 100 && height() > 20)
      oneColumn(dc);
  }

  uint8_t drawChannels(BitmapBuffer* dc, const uint16_t& x, const uint16_t& y,
                       const uint16_t& w, const uint16_t& h,
                       const uint8_t& firstChan, const bool& bg_shown,
                       const uint16_t& bg_color, const uint16_t& txt_color,
                       const uint16_t& bar_color)
  {
    const uint8_t numChan = h / ROW_HEIGHT;
    const uint8_t lastChan = firstChan + numChan;
    const uint8_t rowH =
        (h - numChan * ROW_HEIGHT >= numChan ? ROW_HEIGHT + 1 : ROW_HEIGHT);
    const uint16_t barW = w - RECT_BORDER * 2;
    const uint8_t barH = rowH - RECT_BORDER;
    const uint16_t barLft = x + RECT_BORDER;
    const uint16_t barMid = barLft + barW / 2;

    LcdFlags barColor = COLOR2FLAGS(bar_color);
    LcdFlags txtColor = COLOR2FLAGS(txt_color);

    for (uint8_t curChan = firstChan;
         curChan < lastChan && curChan <= MAX_OUTPUT_CHANNELS; curChan++) {
      const int16_t chanVal = calcRESXto100(channelOutputs[curChan - 1]);
      const uint16_t rowTop = y + (curChan - firstChan) * rowH;
      const uint16_t barTop = rowTop + RECT_BORDER;
      const uint16_t fillW = divRoundClosest(
          barW * limit<int16_t>(0, abs(chanVal), VIEW_CHANNELS_LIMIT_PCT),
          VIEW_CHANNELS_LIMIT_PCT * 2);

      if (bg_shown) {
        lcdSetColor(bg_color);
        dc->drawSolidFilledRect(barLft, barTop, barW, barH, CUSTOM_COLOR);
      }
      if (fillW)
        dc->drawSolidFilledRect((chanVal > 0 ? barMid : barMid - fillW), barTop,
                                fillW, barH, barColor);
      lcd->drawSolidVerticalLine(barMid, barTop, barH, COLOR_THEME_SECONDARY1);
      dc->drawRect(x, rowTop, w, rowH + 1);
      dc->drawNumber(x + barW - 10, barTop, chanVal,
                     FONT(XS) | txtColor | RIGHT, 0, nullptr, "%");
      if (g_model.limitData[curChan - 1].name[0] != 0) {
        dc->drawNumber(barLft + 1, barTop, curChan,
                       FONT(XS) | txtColor | LEFT | LEADING0, 2);
        dc->drawSizedText(barLft + 23, barTop,
                          g_model.limitData[curChan - 1].name,
                          sizeof(g_model.limitData[curChan - 1].name),
                          FONT(XS) | txtColor | LEFT);
      } else {
        drawSource(dc, barLft + 1, barTop, MIXSRC_FIRST_CH + curChan - 1,
                   FONT(XS) | txtColor | LEFT);
      }
    }
    return lastChan - 1;
  }

  void twoColumns(BitmapBuffer* dc)
  {
    uint8_t endColumn =
        drawChannels(dc, 0, 0, (width() / 2) - 1, height(),
                     persistentData->options[0].value.unsignedValue,
                     persistentData->options[1].value.boolValue,
                     persistentData->options[2].value.unsignedValue,
                     persistentData->options[3].value.unsignedValue,
                     persistentData->options[4].value.unsignedValue);

    drawChannels(dc, width() / 2, 0, (width() / 2) - 1, height(), endColumn + 1,
                 persistentData->options[1].value.boolValue,
                 persistentData->options[2].value.unsignedValue,
                 persistentData->options[3].value.unsignedValue,
                 persistentData->options[4].value.unsignedValue);
  }

  void oneColumn(BitmapBuffer* dc)
  {
    drawChannels(dc, 0, 0, width(), height(),
                 persistentData->options[0].value.unsignedValue,
                 persistentData->options[1].value.boolValue,
                 persistentData->options[2].value.unsignedValue,
                 persistentData->options[3].value.unsignedValue,
                 persistentData->options[4].value.unsignedValue);
  }

  void checkEvents() override
  {
    Widget::checkEvents();

    // Last time we refreshed the window
    uint32_t now = RTOS_GET_MS();
    if (now - lastRefresh >= OUTPUTS_REFRESH) {
      lastRefresh = now;
      invalidate();
    }

    invalidate();
  }

  static const ZoneOption options[];

 protected:
  // Last time we refreshed the window
  uint32_t lastRefresh = 0;
};

const ZoneOption OutputsWidget::options[] = {
    {STR_FIRST_CHANNEL, ZoneOption::Integer, OPTION_VALUE_UNSIGNED(1),
     OPTION_VALUE_UNSIGNED(1), OPTION_VALUE_UNSIGNED(32)},
    {STR_FILL_BACKGROUND, ZoneOption::Bool, OPTION_VALUE_BOOL(false)},
    {STR_BG_COLOR, ZoneOption::Color, OPTION_VALUE_UNSIGNED(COLOR_THEME_SECONDARY3)},
    {STR_TEXT_COLOR, ZoneOption::Color,
     OPTION_VALUE_UNSIGNED(COLOR_THEME_PRIMARY1 >> 16)},
    {STR_COLOR, ZoneOption::Color,
     OPTION_VALUE_UNSIGNED(COLOR_THEME_SECONDARY1 >> 16)},
    {nullptr, ZoneOption::Bool}};

BaseWidgetFactory<OutputsWidget> outputsWidget("Outputs",
                                               OutputsWidget::options);
