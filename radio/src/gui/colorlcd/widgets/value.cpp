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

#define ETX_STATE_TIMER_ELAPSED LV_STATE_USER_1
#define ETX_STATE_TELEM_STALE LV_STATE_USER_2
#define ETX_STATE_LARGE_FONT LV_STATE_USER_3

class ValueWidget : public Widget
{
 public:
  ValueWidget(const WidgetFactory* factory, Window* parent, const rect_t& rect,
              Widget::PersistentData* persistentData) :
      Widget(factory, parent, rect, persistentData)
  {
    lv_style_init(&labelStyle);
    lv_style_set_width(&labelStyle, lv_pct(100));
    lv_style_set_height(&labelStyle, lv_pct(100));

    lv_style_init(&valueStyle);
    lv_style_set_width(&valueStyle, lv_pct(100));
    lv_style_set_height(&valueStyle, lv_pct(100));

    labelShadow = lv_label_create(lvobj);
    lv_obj_add_style(labelShadow, &labelStyle, LV_PART_MAIN);
    lv_obj_set_style_text_color(labelShadow, lv_color_black(), LV_PART_MAIN);
    lv_label_set_text(labelShadow, "");

    label = lv_label_create(lvobj);
    lv_obj_add_style(label, &labelStyle, LV_PART_MAIN);
    etx_txt_color(label, COLOR_THEME_WARNING_INDEX, ETX_STATE_TIMER_ELAPSED);
    etx_txt_color(label, COLOR_THEME_DISABLED_INDEX, ETX_STATE_TELEM_STALE);
    lv_label_set_text(label, "");

    valueShadow = lv_label_create(lvobj);
    lv_obj_add_style(valueShadow, &valueStyle, LV_PART_MAIN);
    lv_obj_set_style_text_color(valueShadow, lv_color_black(), LV_PART_MAIN);
    etx_font(valueShadow, FONT_L_INDEX);
    etx_font(valueShadow, FONT_XL_INDEX, ETX_STATE_LARGE_FONT);
    lv_label_set_text(valueShadow, "");

    value = lv_label_create(lvobj);
    lv_obj_add_style(value, &valueStyle, LV_PART_MAIN);
    etx_txt_color(value, COLOR_THEME_WARNING_INDEX, ETX_STATE_TIMER_ELAPSED);
    etx_txt_color(value, COLOR_THEME_DISABLED_INDEX, ETX_STATE_TELEM_STALE);
    etx_font(value, FONT_L_INDEX);
    etx_font(value, FONT_XL_INDEX, ETX_STATE_LARGE_FONT);
    lv_label_set_text(value, "");

    update();
    checkEvents();
  }

  void checkEvents() override
  {
    Widget::checkEvents();

    bool changed = false;

    // get source from options[0]
    mixsrc_t field = persistentData->options[0].value.unsignedValue;

    // if value changed
    auto newValue = getValue(field);
    if (lastValue != newValue) {
      lastValue = newValue;
      changed = true;
    } else {
      // if telemetry value, and telemetry offline or old data
      if (field >= MIXSRC_FIRST_TELEM) {
        TelemetryItem& telemetryItem =
            telemetryItems[(field - MIXSRC_FIRST_TELEM) / 3];
        bool telemState = !telemetryItem.isAvailable() || telemetryItem.isOld();
        if (lastTelemState != telemState) {
          lastTelemState = telemState;
          changed = true;
        }
      }
    }

    if (changed) {
      // Set color to option value
      lv_obj_clear_state(label,
                         ETX_STATE_TIMER_ELAPSED | ETX_STATE_TELEM_STALE);
      lv_obj_clear_state(value,
                         ETX_STATE_TIMER_ELAPSED | ETX_STATE_TELEM_STALE);

      // Check for disabled or warning color states
      if (field >= MIXSRC_FIRST_TIMER && field <= MIXSRC_LAST_TIMER) {
        TimerState& timerState = timersStates[field - MIXSRC_FIRST_TIMER];
        if (timerState.val < 0) {
          // Set warning color
          lv_obj_add_state(label, ETX_STATE_TIMER_ELAPSED);
          lv_obj_add_state(value, ETX_STATE_TIMER_ELAPSED);
        }
      } else if (field >= MIXSRC_FIRST_TELEM) {
        TelemetryItem& telemetryItem =
            telemetryItems[(field - MIXSRC_FIRST_TELEM) / 3];
        if (!telemetryItem.isAvailable() || telemetryItem.isOld()) {
          // Set disabled color
          lv_obj_add_state(label, ETX_STATE_TELEM_STALE);
          lv_obj_add_state(value, ETX_STATE_TELEM_STALE);
        }
      }

      std::string valueTxt;

      // Set value text
      if (field >= MIXSRC_FIRST_TIMER && field <= MIXSRC_LAST_TIMER) {
        TimerState& timerState = timersStates[field - MIXSRC_FIRST_TIMER];
        TimerOptions timerOptions;
        timerOptions.options = SHOW_TIMER;
        valueTxt = getTimerString(abs(timerState.val), timerOptions);
      } else if (field == MIXSRC_TX_TIME) {
        int32_t tme = getValue(MIXSRC_TX_TIME);
        TimerOptions timerOptions;
        timerOptions.options = SHOW_TIME;
        valueTxt = getTimerString(tme, timerOptions);
      } else if (field >= MIXSRC_FIRST_TELEM) {
        std::string getSensorCustomValue(uint8_t sensor, int32_t value, LcdFlags flags);
        valueTxt = getSensorCustomValue((field - MIXSRC_FIRST_TELEM) / 3, getValue(field), valueFlags);
#if defined(LUA_INPUTS)
      }
      else if (field >= MIXSRC_FIRST_LUA && field <= MIXSRC_LAST_LUA) {
        valueTxt =
            getSourceCustomValueString(field, calcRESXto1000(getValue(field)), valueFlags | PREC1);
#endif
      } else {
        valueTxt =
            getSourceCustomValueString(field, getValue(field), valueFlags);
      }

      lv_label_set_text(value, valueTxt.c_str());
      lv_label_set_text(valueShadow, valueTxt.c_str());
    }
  }

  static const ZoneOption options[];

 protected:
  int32_t lastValue = -10000;
  bool lastTelemState = false;
  lv_style_t labelStyle;
  lv_style_t valueStyle;
  lv_obj_t* label;
  lv_obj_t* labelShadow;
  lv_obj_t* value;
  lv_obj_t* valueShadow;
  LcdFlags valueFlags = 0;

  static LAYOUT_VAL(VAL_Y1, 14, 14)
  static LAYOUT_VAL(VAL_Y2, 18, 18)

  void update() override
  {
    // get source from options[0]
    mixsrc_t field = persistentData->options[0].value.unsignedValue;

    // get color from options[1]
    etx_txt_color_from_flags(label, persistentData->options[1].value.unsignedValue);
    etx_txt_color_from_flags(value, persistentData->options[1].value.unsignedValue);

    // get label alignment from options[3]
    LcdFlags lblAlign = persistentData->options[3].value.unsignedValue;

    // get value alignment from options[4]
    LcdFlags valAlign = persistentData->options[4].value.unsignedValue;

    lv_coord_t labelX = 0;
    lv_coord_t labelY = 0;
    lv_coord_t valueX = 0;
    lv_coord_t valueY = VAL_Y1;

    // Set font to L
    lv_obj_clear_state(value, ETX_STATE_LARGE_FONT);
    lv_obj_clear_state(valueShadow, ETX_STATE_LARGE_FONT);

    // Get positions, alignment and value font size.
    if (height() < 50) {
      valueFlags = NO_UNIT;
      if (width() >= 120) {
        lblAlign = ALIGN_LEFT;
        valAlign = ALIGN_RIGHT;
        labelX = 4;
        labelY = 2;
        valueX = -4;
        valueY = -2;
      }
    } else {
      labelX = (lblAlign == ALIGN_LEFT)     ? 4
               : (lblAlign == ALIGN_CENTER) ? -3
                                            : -4;
      labelY = 2;
      valueX = (valAlign == ALIGN_LEFT)     ? 4
               : (valAlign == ALIGN_CENTER) ? 1
                                            : -4;
      valueY = VAL_Y2;
      if (field >= MIXSRC_FIRST_TELEM) {
        int8_t sensor = 1 + (field - MIXSRC_FIRST_TELEM) / 3;
        if (!isGPSSensor(sensor) && !isSensorUnit(sensor, UNIT_DATETIME) && !isSensorUnit(sensor, UNIT_TEXT)) {
          // Set font to XL
          lv_obj_add_state(value, ETX_STATE_LARGE_FONT);
          lv_obj_add_state(valueShadow, ETX_STATE_LARGE_FONT);
        }
      }
#if defined(INTERNAL_GPS)
      else if (field == MIXSRC_TX_GPS) {
      }
#endif
      else {
        // Set font to XL
        lv_obj_add_state(value, ETX_STATE_LARGE_FONT);
        lv_obj_add_state(valueShadow, ETX_STATE_LARGE_FONT);
      }
    }

    // Set text alignment
    lv_style_set_text_align(&labelStyle,
                            (lblAlign == ALIGN_RIGHT)    ? LV_TEXT_ALIGN_RIGHT
                            : (lblAlign == ALIGN_CENTER) ? LV_TEXT_ALIGN_CENTER
                                                         : LV_TEXT_ALIGN_LEFT);
    lv_style_set_text_align(&valueStyle,
                            (valAlign == ALIGN_RIGHT)    ? LV_TEXT_ALIGN_RIGHT
                            : (valAlign == ALIGN_CENTER) ? LV_TEXT_ALIGN_CENTER
                                                         : LV_TEXT_ALIGN_LEFT);

    // Set label text
    char* labelTxt = getSourceString(field);
    lv_label_set_text(label, labelTxt);
    lv_label_set_text(labelShadow, labelTxt);

    // Set label and value positions.
    lv_obj_set_pos(labelShadow, labelX + 1, labelY + 1);
    lv_obj_set_pos(label, labelX, labelY);
    lv_obj_set_pos(valueShadow, valueX + 1, valueY + 1);
    lv_obj_set_pos(value, valueX, valueY);

    // Show / hide shadow
    if (persistentData->options[2].value.boolValue) {
      lv_obj_clear_flag(labelShadow, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(valueShadow, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(labelShadow, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(valueShadow, LV_OBJ_FLAG_HIDDEN);
    }
  }
};

const ZoneOption ValueWidget::options[] = {
    {STR_SOURCE, ZoneOption::Source, OPTION_VALUE_UNSIGNED(MIXSRC_FIRST_STICK)},
    {STR_COLOR, ZoneOption::Color, COLOR2FLAGS(COLOR_THEME_PRIMARY2_INDEX)},
    {STR_SHADOW, ZoneOption::Bool, OPTION_VALUE_BOOL(false)},
    {STR_ALIGN_LABEL, ZoneOption::Align, OPTION_VALUE_UNSIGNED(ALIGN_LEFT)},
    {STR_ALIGN_VALUE, ZoneOption::Align, OPTION_VALUE_UNSIGNED(ALIGN_LEFT)},
    {nullptr, ZoneOption::Bool}};

BaseWidgetFactory<ValueWidget> ValueWidget("Value", ValueWidget::options,
                                           STR_WIDGET_VALUE);
