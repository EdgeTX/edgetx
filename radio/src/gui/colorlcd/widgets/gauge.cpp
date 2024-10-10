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

class GaugeWidget : public Widget
{
 public:
  GaugeWidget(const WidgetFactory* factory, Window* parent, const rect_t& rect,
              Widget::PersistentData* persistentData) :
      Widget(factory, parent, rect, persistentData)
  {
    // Gauge label
    sourceText = new StaticText(this, {0, 0, LV_SIZE_CONTENT, 16}, "", 
                                COLOR_THEME_PRIMARY2_INDEX, FONT(XS));

    valueText = new DynamicNumber<int16_t>(
        this, {0, 0, lv_pct(100), 16}, [=]() { return getGuageValue(); },
        COLOR_THEME_PRIMARY2_INDEX, FONT(XS) | CENTERED, "", "%");
    etx_obj_add_style(valueText->getLvObj(), styles->text_align_right,
                      LV_STATE_USER_1);

    auto box = lv_obj_create(lvobj);
    lv_obj_set_pos(box, 0, 16);
    lv_obj_set_size(box, lv_pct(100), 16);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_CLICKABLE);
    etx_solid_bg(box, COLOR_THEME_PRIMARY2_INDEX);

    bar = lv_obj_create(box);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_CLICKABLE);
    etx_obj_add_style(bar, styles->bg_opacity_cover, LV_PART_MAIN);

    update();
  }

  int16_t getGuageValue()
  {
    mixsrc_t index = persistentData->options[0].value.unsignedValue;
    int32_t min = persistentData->options[1].value.signedValue;
    int32_t max = persistentData->options[2].value.signedValue;

    int32_t value = getValue(index);

    if (min > max) {
      SWAP(min, max);
      value = value - min - max;
    }

    value = limit(min, value, max);

    return divRoundClosest(100 * (value - min), (max - min));
  }

  void update() override
  {
    mixsrc_t index = persistentData->options[0].value.unsignedValue;
    sourceText->setText(getSourceString(index));

    if (width() < 90)
      lv_obj_add_state(valueText->getLvObj(), LV_STATE_USER_1);
    else
      lv_obj_clear_state(valueText->getLvObj(), LV_STATE_USER_1);

    etx_bg_color_from_flags(bar, persistentData->options[3].value.unsignedValue);
  }

  static const ZoneOption options[];

 protected:
  int16_t lastValue = -10000;
  StaticText* sourceText = nullptr;
  DynamicNumber<int16_t>* valueText = nullptr;
  lv_obj_t* bar = nullptr;

  void checkEvents() override
  {
    Widget::checkEvents();

    auto newValue = getGuageValue();
    if (lastValue != newValue) {
      lastValue = newValue;

      lv_coord_t w = (width() * lastValue) / 100;
      lv_obj_set_size(bar, w, 16);
    }
  }
};

const ZoneOption GaugeWidget::options[] = {
    {STR_SOURCE, ZoneOption::Source, OPTION_VALUE_UNSIGNED(1)},
    {STR_MIN, ZoneOption::Integer, OPTION_VALUE_SIGNED(-RESX),
     OPTION_VALUE_SIGNED(-RESX), OPTION_VALUE_SIGNED(RESX)},
    {STR_MAX, ZoneOption::Integer, OPTION_VALUE_SIGNED(RESX),
     OPTION_VALUE_SIGNED(-RESX), OPTION_VALUE_SIGNED(RESX)},
    {STR_COLOR, ZoneOption::Color, COLOR2FLAGS(COLOR_THEME_WARNING_INDEX)},
    {nullptr, ZoneOption::Bool}};

BaseWidgetFactory<GaugeWidget> gaugeWidget("Gauge", GaugeWidget::options,
                                           STR_WIDGET_GAUGE);
