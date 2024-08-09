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

#include "edgetx.h"
#include "widget.h"

#define TEXT_WIDGET_DEFAULT_LABEL \
  'M', 'y', ' ', 'L', 'a', 'b', 'e', 'l'  // "My Label"

class TextWidget : public Widget
{
 public:
  TextWidget(const WidgetFactory* factory, Window* parent, const rect_t& rect,
             Widget::PersistentData* persistentData) :
      Widget(factory, parent, rect, persistentData)
  {
    lv_style_init(&style);
    lv_style_set_width(&style, lv_pct(100));
    lv_style_set_height(&style, lv_pct(100));

    shadow = lv_label_create(lvobj);
    lv_obj_add_style(shadow, &style, LV_PART_MAIN);
    lv_obj_set_style_text_color(shadow, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_pos(shadow, 1, 1);

    label = lv_label_create(lvobj);
    lv_obj_add_style(label, &style, LV_PART_MAIN);

    update();
  }

  static const ZoneOption options[];

 protected:
  lv_style_t style;
  lv_obj_t* shadow;
  lv_obj_t* label;

  void update() override
  {
    // Set text value from options
    lv_label_set_text(shadow, persistentData->options[0].value.stringValue);
    lv_label_set_text(label, persistentData->options[0].value.stringValue);

    // get font color from options[1]
    etx_txt_color_from_flags(label, persistentData->options[1].value.unsignedValue);
    // get font size from options[2]
    lv_style_set_text_font(
        &style, getFont(persistentData->options[2].value.unsignedValue << 8));
    // get alignment from options[4]
    LcdFlags alignment = persistentData->options[4].value.unsignedValue;
    lv_style_set_text_align(&style,
                            (alignment == ALIGN_RIGHT)    ? LV_TEXT_ALIGN_RIGHT
                            : (alignment == ALIGN_CENTER) ? LV_TEXT_ALIGN_CENTER
                                                          : LV_TEXT_ALIGN_LEFT);

    // Show or hide shadow
    if (persistentData->options[3].value.boolValue)
      lv_obj_clear_flag(shadow, LV_OBJ_FLAG_HIDDEN);
    else
      lv_obj_add_flag(shadow, LV_OBJ_FLAG_HIDDEN);
  }
};

const ZoneOption TextWidget::options[] = {
    {STR_TEXT, ZoneOption::String,
     OPTION_VALUE_STRING(TEXT_WIDGET_DEFAULT_LABEL)},
    {STR_COLOR, ZoneOption::Color, COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX)},
    {STR_SIZE, ZoneOption::TextSize, OPTION_VALUE_UNSIGNED(0)},
    {STR_SHADOW, ZoneOption::Bool, OPTION_VALUE_BOOL(false)},
    {STR_ALIGNMENT, ZoneOption::Align, OPTION_VALUE_UNSIGNED(ALIGN_LEFT)},
    {nullptr, ZoneOption::Bool}};

BaseWidgetFactory<TextWidget> textWidget("Text", TextWidget::options,
                                         STR_WIDGET_TEXT);
