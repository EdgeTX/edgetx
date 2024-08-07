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

#include "bitmaps.h"
#include "edgetx.h"
#include "widget.h"

#define ETX_STATE_BG_WARNING LV_STATE_USER_1
#define EXT_NAME_ALIGN_RIGHT LV_STATE_USER_1
#define ETX_NAME_TXT_WARNING LV_STATE_USER_2
#define ETX_NAME_COLOR_WHITE LV_STATE_USER_3
#define ETX_VALUE_SMALL_FONT LV_STATE_USER_1

class TimerWidget : public Widget
{
 public:
  TimerWidget(const WidgetFactory* factory, Window* parent, const rect_t& rect,
              Widget::PersistentData* persistentData) :
      Widget(factory, parent, rect, persistentData)
  {
    etx_solid_bg(lvobj, COLOR_THEME_WARNING_INDEX,
                 LV_PART_MAIN | ETX_STATE_BG_WARNING);

    lv_style_init(&style);
    lv_style_set_width(&style, lv_pct(100));
    lv_style_set_height(&style, LV_SIZE_CONTENT);

    timerBg = new StaticIcon(this, 0, 0, ICON_TIMER_BG, COLOR_THEME_PRIMARY2_INDEX);
    timerIcon = new StaticIcon(this, 3, 4, ICON_TIMER, COLOR_THEME_SECONDARY1_INDEX);

    // Timer name
    nameLabel = lv_label_create(lvobj);
    lv_label_set_text(nameLabel, "");
    lv_obj_add_style(nameLabel, &style, LV_PART_MAIN);
    etx_font(nameLabel, FONT_XS_INDEX);
    etx_obj_add_style(nameLabel, styles->text_align_left, LV_PART_MAIN);
    etx_obj_add_style(nameLabel, styles->text_align_right,
                      LV_PART_MAIN | EXT_NAME_ALIGN_RIGHT);
    etx_txt_color(nameLabel, COLOR_THEME_SECONDARY1_INDEX);
    etx_txt_color(nameLabel, COLOR_THEME_SECONDARY2_INDEX,
                  LV_PART_MAIN | ETX_NAME_TXT_WARNING);
    etx_txt_color(nameLabel, COLOR_THEME_PRIMARY2_INDEX,
                  LV_PART_MAIN | ETX_NAME_COLOR_WHITE);

    // Timer value - on small size widgets
    valLabel = lv_label_create(lvobj);
    lv_label_set_text(valLabel, "");
    lv_obj_add_style(valLabel, &style, LV_PART_MAIN);
    etx_txt_color(valLabel, COLOR_THEME_PRIMARY2_INDEX);
    etx_font(valLabel, FONT_XS_INDEX, LV_PART_MAIN | ETX_VALUE_SMALL_FONT);
    lv_obj_set_pos(valLabel, 3, 20);

    // Timer value - on large widgets
    unit0 = createUnitLabel();
    lv_obj_set_pos(unit0, U0_X, U0_Y);
    unit1 = createUnitLabel();
    lv_obj_set_pos(unit1, U1_X, U1_Y);
    digits0 = createDigitsLabel();
    lv_obj_set_pos(digits0, D0_X, D0_Y);
    digits1 = createDigitsLabel();
    lv_obj_set_pos(digits1, D1_X, D1_Y);

    timerArc = lv_arc_create(lvobj);
    lv_arc_set_rotation(timerArc, 270);
    lv_arc_set_bg_angles(timerArc, 0, 360);
    lv_arc_set_range(timerArc, 0, 360);
    lv_arc_set_angles(timerArc, 0, 360);
    lv_arc_set_start_angle(timerArc, 0);
    lv_obj_remove_style(timerArc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(timerArc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_pos(timerArc, 2, 3);
    lv_obj_set_size(timerArc, TMR_ARC_SZ, TMR_ARC_SZ);
    lv_obj_set_style_arc_opa(timerArc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(timerArc, TMR_ARC_W, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(timerArc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(timerArc, TMR_ARC_W, LV_PART_INDICATOR);
    etx_obj_add_style(timerArc, styles->arc_color[COLOR_THEME_SECONDARY1_INDEX], LV_PART_INDICATOR);
    lv_obj_add_flag(timerArc, LV_OBJ_FLAG_HIDDEN);

    update();
    checkEvents();
  }

  void checkEvents() override
  {
    Widget::checkEvents();

    uint32_t index = persistentData->options[0].value.unsignedValue;
    TimerData& timerData = g_model.timers[index];
    TimerState& timerState = timersStates[index];

    if (lastValue != timerState.val || lastStartValue != timerData.start) {
      lastValue = timerState.val;
      lastStartValue = timerData.start;

      if (lastStartValue && lastValue > 0) {
        int pieEnd = 360 * (lastStartValue - lastValue) / lastStartValue;
        if (!timerData.showElapsed) {
          pieEnd = 360 - pieEnd;
        }
        lv_arc_set_end_angle(timerArc, pieEnd);
      }

      int val = lastValue;
      if (lastStartValue && timerData.showElapsed &&
          (int)lastStartValue != lastValue)
        val = (int)lastStartValue - lastValue;

      if (isLarge) {
        char sDigitGroup0[LEN_TIMER_STRING];
        char sDigitGroup1[LEN_TIMER_STRING];
        char sUnit0[] = "M";
        char sUnit1[] = "S";

        splitTimer(sDigitGroup0, sDigitGroup1, sUnit0, sUnit1, abs(val), false);

        lv_label_set_text(digits0, sDigitGroup0);
        lv_label_set_text(digits1, sDigitGroup1);
        lv_label_set_text(unit0, sUnit0);
        lv_label_set_text(unit1, sUnit1);

        if (lastValue > 0) {
          lv_obj_clear_flag(timerArc, LV_OBJ_FLAG_HIDDEN);
          timerIcon->hide();
        } else {
          lv_obj_add_flag(timerArc, LV_OBJ_FLAG_HIDDEN);
          timerIcon->show();
        }
      } else {
        char str[LEN_TIMER_STRING];

        TimerOptions timerOptions;
        timerOptions.options = (abs(val) >= 3600) ? SHOW_TIME : SHOW_TIMER;
        getTimerString(str, abs(val), timerOptions);
        lv_label_set_text(valLabel, str);

        if (width() <= 100 && height() <= 40 && abs(val) >= 3600)
          lv_obj_add_state(valLabel, ETX_VALUE_SMALL_FONT);
        else
          lv_obj_clear_state(valLabel, ETX_VALUE_SMALL_FONT);

        lv_obj_add_flag(timerArc, LV_OBJ_FLAG_HIDDEN);
        timerIcon->hide();
      }

      // Set colors if timer has elapsed.
      if (lastValue < 0 && lastValue % 2) {
        if (isLarge) {
          lv_obj_add_state(nameLabel, ETX_NAME_TXT_WARNING);
          lv_obj_add_state(digits0, ETX_NAME_TXT_WARNING);
          lv_obj_add_state(digits1, ETX_NAME_TXT_WARNING);
          lv_obj_add_state(unit0, ETX_NAME_TXT_WARNING);
          lv_obj_add_state(unit1, ETX_NAME_TXT_WARNING);
          lv_obj_clear_state(lvobj, ETX_STATE_BG_WARNING);
          timerBg->setColor(COLOR_THEME_WARNING_INDEX);
          timerIcon->setColor(COLOR_THEME_SECONDARY2_INDEX);
        } else {
          lv_obj_add_state(lvobj, ETX_STATE_BG_WARNING);
        }
      } else {
        if (isLarge) {
          lv_obj_clear_state(nameLabel, ETX_NAME_TXT_WARNING);
          lv_obj_clear_state(digits0, ETX_NAME_TXT_WARNING);
          lv_obj_clear_state(digits1, ETX_NAME_TXT_WARNING);
          lv_obj_clear_state(unit0, ETX_NAME_TXT_WARNING);
          lv_obj_clear_state(unit1, ETX_NAME_TXT_WARNING);
          timerBg->setColor(COLOR_THEME_PRIMARY2_INDEX);
          timerIcon->setColor(COLOR_THEME_SECONDARY1_INDEX);
        }
        lv_obj_clear_state(lvobj, ETX_STATE_BG_WARNING);
      }
    }
  }

  static const ZoneOption options[];

  static LAYOUT_VAL(TMR_LRG_W, 180, 180)
  static LAYOUT_VAL(TMR_ARC_SZ, 64, 64)
  static LAYOUT_VAL(TMR_ARC_W, 10, 10)
  static LAYOUT_VAL(NM_LRG_X, 78, 78)
  static LAYOUT_VAL(NM_LRG_Y, 19, 19)
  static LAYOUT_VAL(NM_LRG_W, 93, 93)
  static LAYOUT_VAL(U0_X, 111, 111)
  static LAYOUT_VAL(U0_Y, 33, 33)
  static LAYOUT_VAL(U1_X, 161, 161)
  static LAYOUT_VAL(U1_Y, 33, 33)
  static LAYOUT_VAL(D0_X, 76, 76)
  static LAYOUT_VAL(D0_Y, 31, 31)
  static LAYOUT_VAL(D1_X, 126, 126)
  static LAYOUT_VAL(D1_Y, 31, 31)

 protected:
  tmrval_t lastValue = 0;
  uint32_t lastStartValue = 0;
  bool isLarge = false;
  lv_style_t style;
  lv_obj_t* nameLabel = nullptr;
  lv_obj_t* valLabel = nullptr;
  lv_obj_t* digits0 = nullptr;
  lv_obj_t* digits1 = nullptr;
  lv_obj_t* unit0 = nullptr;
  lv_obj_t* unit1 = nullptr;
  lv_obj_t* timerArc = nullptr;
  StaticIcon* timerBg = nullptr;
  StaticIcon* timerIcon = nullptr;

  void update() override
  {
    // Set up widget from options.
    char s[16];

    uint32_t index = persistentData->options[0].value.unsignedValue;
    TimerData& timerData = g_model.timers[index];

    bool hasName = ZLEN(timerData.name) > 0;

    if (width() >= TMR_LRG_W && height() >= 70) {
      isLarge = true;
      if (hasName)
        lv_obj_clear_state(nameLabel, EXT_NAME_ALIGN_RIGHT);
      else
        lv_obj_add_state(nameLabel, EXT_NAME_ALIGN_RIGHT);
      lv_obj_set_pos(nameLabel, NM_LRG_X, NM_LRG_Y);
      lv_obj_set_width(nameLabel, NM_LRG_W);
      lv_obj_clear_state(nameLabel, ETX_NAME_COLOR_WHITE);

      lv_obj_add_flag(valLabel, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(digits0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(digits1, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(unit0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(unit1, LV_OBJ_FLAG_HIDDEN);
      timerBg->show();
    } else {
      isLarge = false;
      lv_obj_set_pos(nameLabel, 2, 0);
      lv_obj_set_width(nameLabel, lv_pct(100));
      lv_obj_add_state(nameLabel, ETX_NAME_COLOR_WHITE);

      lv_obj_clear_flag(valLabel, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(digits0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(digits1, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(unit0, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(unit1, LV_OBJ_FLAG_HIDDEN);
      timerBg->hide();
    }

    // name
    if (hasName) {
      strAppend(s, timerData.name, LEN_TIMER_NAME);
    } else {  // user name not exist "TMRn"
      formatNumberAsString(s, 16, index + 1, 1, 0, "TMR");
    }
    lv_label_set_text(nameLabel, s);
  }

  lv_obj_t* createUnitLabel()
  {
    auto lbl = lv_label_create(lvobj);
    lv_label_set_text(lbl, "");
    lv_obj_add_style(lbl, &style, LV_PART_MAIN);
    etx_txt_color(lbl, COLOR_THEME_SECONDARY1_INDEX);
    etx_txt_color(lbl, COLOR_THEME_SECONDARY2_INDEX,
                  LV_PART_MAIN | ETX_NAME_TXT_WARNING);

    return lbl;
  }

  lv_obj_t* createDigitsLabel()
  {
    auto lbl = createUnitLabel();
    etx_font(lbl, FONT_XL_INDEX);

    return lbl;
  }
};

const ZoneOption TimerWidget::options[] = {
    {STR_TIMER_SOURCE, ZoneOption::Timer, OPTION_VALUE_UNSIGNED(0)},
    {nullptr, ZoneOption::Bool}};

BaseWidgetFactory<TimerWidget> timerWidget("Timer", TimerWidget::options,
                                           STR_WIDGET_TIMER);
