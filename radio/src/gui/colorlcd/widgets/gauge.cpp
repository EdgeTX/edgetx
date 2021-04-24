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
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "opentx.h"
#include "widgets_container_impl.h"

class GaugeWidget: public Widget
{
  public:
    GaugeWidget(const WidgetFactory * factory, FormGroup * parent, const rect_t & rect, Widget::PersistentData * persistentData):
      Widget(factory, parent, rect, persistentData)
    {
    }

    void refresh(BitmapBuffer * dc) override
    {
      mixsrc_t index = persistentData->options[0].value.unsignedValue;
      int32_t min = persistentData->options[1].value.signedValue;
      int32_t max = persistentData->options[2].value.signedValue;
      uint16_t color = persistentData->options[3].value.unsignedValue;

      int32_t value = getValue(index);

      if (min > max) {
        SWAP(min, max);
        value = value - min - max;
      }

      value = limit(min, value, max);

      int w = divRoundClosest(width() * (value - min), (max - min));
      int percent = divRoundClosest(100 * (value - min), (max - min));

      // Gauge label
      drawSource(dc, 0, 0, index, FONT(XS) | FOCUS_COLOR);

      // Gauge
      lcdSetColor(color);
      dc->drawSolidFilledRect(0, 16, width(), 16, FOCUS_COLOR);
      dc->drawNumber(0+width()/2, 17, percent, FONT(XS) | CUSTOM_COLOR | CENTERED, 0, nullptr, "%");
      dc->invertRect(w, 16, width() - w, 16, CUSTOM_COLOR);
    }

    void checkEvents() override
    {
      Widget::checkEvents();

      auto newValue = getValue(persistentData->options[0].value.unsignedValue);
      if (lastValue != newValue) {
        lastValue = newValue;
        invalidate();
      }
    }

    static const ZoneOption options[];
    int32_t lastValue = 0;
};

const ZoneOption GaugeWidget::options[] = {
  { STR_SOURCE, ZoneOption::Source, OPTION_VALUE_UNSIGNED(1) },
  { STR_MIN, ZoneOption::Integer, OPTION_VALUE_SIGNED(-RESX), OPTION_VALUE_SIGNED(-RESX), OPTION_VALUE_SIGNED(RESX) },
  { STR_MAX, ZoneOption::Integer, OPTION_VALUE_SIGNED(RESX), OPTION_VALUE_SIGNED(-RESX), OPTION_VALUE_SIGNED(RESX) },
  { STR_COLOR, ZoneOption::Color, OPTION_VALUE_UNSIGNED(RED) },
  { nullptr, ZoneOption::Bool }
};

BaseWidgetFactory<GaugeWidget> gaugeWidget("Gauge", GaugeWidget::options);
