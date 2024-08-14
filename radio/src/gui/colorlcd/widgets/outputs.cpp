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

#define ETX_STATE_BG_FILL LV_STATE_USER_1

#define ROW_HEIGHT 16

#define VIEW_CHANNELS_LIMIT_PCT \
  (g_model.extendedLimits ? LIMIT_EXT_PERCENT : 100)

class ChannelValue : public Window
{
 public:
  ChannelValue(Widget* parent, uint8_t col, uint8_t row, coord_t colWidth,
               uint8_t channel, LcdFlags txtColor, LcdFlags barColor) :
      Window(parent,
             {col * colWidth, row * ROW_HEIGHT, colWidth - 1 + (colWidth & 1), (ROW_HEIGHT + 1)}),
      channel(channel)
  {
    setWindowFlag(NO_FOCUS);

    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);

    etx_obj_add_style(lvobj, styles->border_thin, LV_PART_MAIN);
    etx_obj_add_style(lvobj, styles->border_color[COLOR_BLACK_INDEX], LV_PART_MAIN);

    padAll(PAD_ZERO);

    lv_style_init(&style);
    lv_style_set_width(&style, lv_pct(100));
    lv_style_set_height(&style, lv_pct(100));

    // lv_color_t color;
    // color.full = txtColor >> 16u;
    // lv_style_set_text_color(&style, color);
    // color.full = barColor >> 16u;
    // lv_style_set_bg_color(&style, color);

    bar = lv_obj_create(lvobj);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(bar, 0, ROW_HEIGHT - 1);
    etx_bg_color_from_flags(bar, barColor);

    valueLabel = lv_label_create(lvobj);
    etx_font(valueLabel, FONT_XS_INDEX);
    etx_obj_add_style(valueLabel, styles->text_align_right, LV_PART_MAIN);
    etx_txt_color_from_flags(valueLabel, txtColor);
    lv_obj_add_style(valueLabel, &style, LV_PART_MAIN);
    lv_label_set_text(valueLabel, "");

    chanLabel = lv_label_create(lvobj);
    etx_font(chanLabel, FONT_XS_INDEX);
    etx_obj_add_style(chanLabel, styles->text_align_left, LV_PART_MAIN);
    etx_txt_color_from_flags(chanLabel, txtColor);
    lv_label_set_text(chanLabel, "");

    chanHasName = g_model.limitData[channel].name[0] != 0;
    setChannel();

    divPoints[0] = {(lv_coord_t)(width() / 2 - 1), 0};
    divPoints[1] = {(lv_coord_t)(width() / 2 - 1), ROW_HEIGHT - 1};

    auto divLine = lv_line_create(lvobj);
    lv_line_set_points(divLine, divPoints, 2);
    etx_obj_add_style(divLine, styles->div_line, LV_PART_MAIN);

    checkEvents();
  }

  void setChannel()
  {
    char s[16];
    if (chanHasName) {
      formatNumberAsString(s, 16, channel + 1, LEADING0, 2, "", " ");
      strAppend(s + 3, g_model.limitData[channel].name, LEN_CHANNEL_NAME);
    } else {
      getSourceString(s, MIXSRC_FIRST_CH + channel);
    }
    lv_label_set_text(chanLabel, s);
  }

  void checkEvents() override
  {
    int16_t value = channelOutputs[channel];
    if (value != lastValue) {
      lastValue = value;

      std::string s;
      if (g_eeGeneral.ppmunit == PPM_US)
        s = formatNumberAsString(PPM_CH_CENTER(channel) + value / 2, 0, 0, "", STR_US);
      else if (g_eeGeneral.ppmunit == PPM_PERCENT_PREC1)
        s = formatNumberAsString(calcRESXto1000(value), PREC1, 0, "", "%");
      else
        s = formatNumberAsString(calcRESXto100(value), 0, 0, "", "%");

      lv_label_set_text(valueLabel, s.c_str());

      const int lim = (g_model.extendedLimits ? (1024 * LIMIT_EXT_PERCENT / 100) : 1024);
      uint16_t w = width() - 2;
      uint16_t fillW = divRoundClosest(
          w * limit<int16_t>(0, abs(lastValue), lim),
          lim * 2);
      uint16_t x = lastValue > 0 ? w / 2 : w / 2 - fillW + 1;

      lv_obj_set_pos(bar, x, 0);
      lv_obj_set_size(bar, fillW, ROW_HEIGHT - 1);
    }

    bool hasName = g_model.limitData[channel].name[0] != 0;
    if (hasName != chanHasName) {
      chanHasName = hasName;
      setChannel();
    }

    Window::checkEvents();
  }

 protected:
  uint8_t channel;
  int16_t lastValue = -32768;
  bool chanHasName = false;
  lv_style_t style;
  lv_obj_t* valueLabel = nullptr;
  lv_obj_t* chanLabel = nullptr;
  lv_point_t divPoints[2];
  lv_obj_t* bar = nullptr;
};

class OutputsWidget : public Widget
{
 public:
  OutputsWidget(const WidgetFactory* factory, Window* parent,
                const rect_t& rect, Widget::PersistentData* persistentData) :
      Widget(factory, parent, rect, persistentData)
  {
    padAll(PAD_ZERO);

    lv_style_init(&style);
    lv_obj_add_style(lvobj, &style, LV_PART_MAIN);

    etx_obj_add_style(lvobj, styles->bg_opacity_transparent, LV_PART_MAIN);
    etx_obj_add_style(lvobj, styles->bg_opacity_cover,
                      LV_PART_MAIN | ETX_STATE_BG_FILL);

    update();
  }

  void update() override
  {
    // get background color from options[2]
    etx_bg_color_from_flags(lvobj, persistentData->options[2].value.unsignedValue);

    // Set background opacity from options[1]
    if (persistentData->options[1].value.boolValue)
      lv_obj_add_state(lvobj, ETX_STATE_BG_FILL);
    else
      lv_obj_clear_state(lvobj, ETX_STATE_BG_FILL);

    // Colors
    txtColor = persistentData->options[3].value.unsignedValue;
    barColor = persistentData->options[4].value.unsignedValue;

    // Setup channels
    firstChan = persistentData->options[0].value.unsignedValue;

    clear();

    cols = 0;
    rows = 0;

    if (height() > 20 && width() > 100) {
      rows = height() / ROW_HEIGHT;
      cols = (width() > 300) ? 2 : 1;
      coord_t colWidth = width() / cols;
      uint8_t chan = firstChan;
      for (uint8_t c = 0; c < cols && chan <= MAX_OUTPUT_CHANNELS; c += 1) {
        for (uint8_t r = 0; r < rows && chan <= MAX_OUTPUT_CHANNELS;
             r += 1, chan += 1) {
          new ChannelValue(this, c, r, colWidth, chan - 1, txtColor, barColor);
        }
      }
    }
  }

  static const ZoneOption options[];

 protected:
  // Last time we refreshed the window
  uint32_t lastRefresh = 0;
  uint8_t firstChan = 0;
  uint8_t cols = 0;
  uint8_t rows = 0;
  LcdFlags txtColor = 0;
  LcdFlags barColor = 0;
  lv_style_t style;
};

const ZoneOption OutputsWidget::options[] = {
    {STR_FIRST_CHANNEL, ZoneOption::Integer, OPTION_VALUE_UNSIGNED(1),
     OPTION_VALUE_UNSIGNED(1), OPTION_VALUE_UNSIGNED(32)},
    {STR_FILL_BACKGROUND, ZoneOption::Bool, OPTION_VALUE_BOOL(false)},
    {STR_BG_COLOR, ZoneOption::Color, COLOR2FLAGS(COLOR_THEME_SECONDARY3_INDEX)},
    {STR_TEXT_COLOR, ZoneOption::Color, COLOR2FLAGS(COLOR_THEME_PRIMARY1_INDEX)},
    {STR_COLOR, ZoneOption::Color, COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX)},
    {nullptr, ZoneOption::Bool}};

BaseWidgetFactory<OutputsWidget> outputsWidget("Outputs",
                                               OutputsWidget::options,
                                               STR_WIDGET_OUTPUTS);
