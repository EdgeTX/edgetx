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

#define TEXT_WIDGET_DEFAULT_LABEL  'M', 'y', ' ', 'L', 'a', 'b', 'e', 'l' // "My Label"

class TextWidget: public Widget
{
  public:
    TextWidget(const WidgetFactory * factory, FormGroup * parent, const rect_t & rect, Widget::PersistentData * persistentData):
      Widget(factory, parent, rect, persistentData)
    {
    }

    void refresh(BitmapBuffer * dc) override
    {
      // get font color from options[1]
      lcdSetColor(persistentData->options[1].value.unsignedValue);

      // get font size from options[2]
      LcdFlags fontsize = persistentData->options[2].value.unsignedValue << 8u;

      // draw shadow
      if (persistentData->options[3].value.boolValue) {
        dc->drawText(1, 1, persistentData->options[0].value.stringValue, fontsize | BLACK);
      }

      // draw text
      dc->drawText(0, 0, persistentData->options[0].value.stringValue, fontsize | CUSTOM_COLOR);
    }

    static const ZoneOption options[];
};

const ZoneOption TextWidget::options[] = {
  { STR_TEXT, ZoneOption::String, OPTION_VALUE_STRING(TEXT_WIDGET_DEFAULT_LABEL) },
  { STR_COLOR, ZoneOption::Color, OPTION_VALUE_UNSIGNED(RED) },
  { STR_SIZE, ZoneOption::TextSize, OPTION_VALUE_UNSIGNED(0) },
  { STR_SHADOW, ZoneOption::Bool, OPTION_VALUE_BOOL(false)  },
  { nullptr, ZoneOption::Bool }
};

BaseWidgetFactory<TextWidget> textWidget("Text", TextWidget::options);
