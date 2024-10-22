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

#include "model_outputs.h"

#include "channel_bar.h"
#include "list_line_button.h"
#include "edgetx.h"
#include "output_edit.h"
#include "etx_lv_theme.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

#define ETX_STATE_MINMAX_BOLD LV_STATE_USER_1
#define ETX_STATE_NAME_FONT_SMALL LV_STATE_USER_1

class OutputLineButton : public ListLineButton
{
  bool init = false;

  lv_obj_t* source = nullptr;
  lv_obj_t* revert = nullptr;
  lv_obj_t* min = nullptr;
  lv_obj_t* max = nullptr;
  lv_obj_t* offset = nullptr;
  lv_obj_t* center = nullptr;
  StaticIcon* curve = nullptr;

  static void on_draw(lv_event_t* e)
  {
    lv_obj_t* target = lv_event_get_target(e);
    auto line = (OutputLineButton*)lv_obj_get_user_data(target);
    if (line) {
      if (!line->init)
        line->delayed_init();
      line->refresh();
    }
  }

  void delayed_init()
  {
    init = true;

    lv_obj_enable_style_refresh(false);

    source = lv_label_create(lvobj);
    lv_obj_set_pos(source, SRC_X, SRC_Y);
    lv_obj_set_size(source, SRC_W, SRC_H);

#if !PORTRAIT_LCD
    etx_font(source, FONT_XS_INDEX, ETX_STATE_NAME_FONT_SMALL);
    lv_obj_set_style_pad_top(source, -2, ETX_STATE_NAME_FONT_SMALL);
    lv_obj_set_style_text_line_space(source, -3, ETX_STATE_NAME_FONT_SMALL);
#endif

    min = lv_label_create(lvobj);
    etx_obj_add_style(min, styles->text_align_right, LV_PART_MAIN);
    etx_font(min, FONT_BOLD_INDEX, ETX_STATE_MINMAX_BOLD);
    lv_obj_set_pos(min, MIN_X, MIN_Y);
    lv_obj_set_size(min, MIN_W, MIN_H);

    max = lv_label_create(lvobj);
    etx_obj_add_style(max, styles->text_align_right, LV_PART_MAIN);
    etx_font(max, FONT_BOLD_INDEX, ETX_STATE_MINMAX_BOLD);
    lv_obj_set_pos(max, MAX_X, MAX_Y);
    lv_obj_set_size(max, MAX_W, MAX_H);

    offset = lv_label_create(lvobj);
    etx_obj_add_style(offset, styles->text_align_right, LV_PART_MAIN);
    lv_obj_set_pos(offset, OFF_X, OFF_Y);
    lv_obj_set_size(offset, OFF_W, OFF_H);

    center = lv_label_create(lvobj);
    etx_obj_add_style(center, styles->text_align_right, LV_PART_MAIN);
    lv_obj_set_pos(center, CTR_X, CTR_Y);
    lv_obj_set_size(center, CTR_W, CTR_H);

    revert = lv_img_create(lvobj);
    lv_img_set_src(revert, LV_SYMBOL_SHUFFLE);
    lv_obj_set_pos(revert, REV_X, REV_Y);

    curve =
        new StaticIcon(this, CRV_X, CRV_Y, ICON_TEXTLINE_CURVE, COLOR_THEME_SECONDARY1_INDEX);

    new OutputChannelBar(this, rect_t{BAR_X, PAD_MEDIUM, CH_BAR_WIDTH, CH_BAR_HEIGHT},
                               index, false, false);

    checkEvents();

    lv_obj_update_layout(lvobj);
  
    lv_obj_enable_style_refresh(true);
    lv_obj_refresh_style(lvobj, LV_PART_ANY, LV_STYLE_PROP_ANY);
  }

 public:
  OutputLineButton(Window* parent, uint8_t channel) :
      ListLineButton(parent, channel)
  {
    setHeight(CH_LINE_H);
    padAll(PAD_ZERO);

    lv_obj_add_event_cb(lvobj, OutputLineButton::on_draw,
                        LV_EVENT_DRAW_MAIN_BEGIN, nullptr);
  }

  void refresh() override
  {
    if (!init) return;

    const LimitData* output = limitAddress(index);
    if (g_model.limitData[index].name[0] != '\0') {
#if !PORTRAIT_LCD
      lv_obj_add_state(source, ETX_STATE_NAME_FONT_SMALL);
#endif
      lv_label_set_text_fmt(source, "%s\n" TR_CH "%u",
                            getSourceString(MIXSRC_FIRST_CH + index),
                            index + 1);
    } else {
#if !PORTRAIT_LCD
      lv_obj_clear_state(source, ETX_STATE_NAME_FONT_SMALL);
#endif
      lv_label_set_text(source, getSourceString(MIXSRC_FIRST_CH + index));
    }
    if (output->revert) {
      lv_obj_clear_flag(revert, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(revert, LV_OBJ_FLAG_HIDDEN);
    }

    char s[32];
    getValueOrGVarString(s, sizeof(s), output->min, -GV_RANGELARGE, 0, PREC1,
                         nullptr, -LIMITS_MIN_MAX_OFFSET, true);
    lv_label_set_text(min, s);

    getValueOrGVarString(s, sizeof(s), output->max, 0, GV_RANGELARGE, PREC1,
                         nullptr, +LIMITS_MIN_MAX_OFFSET, true);
    lv_label_set_text(max, s);

    getValueOrGVarString(s, sizeof(s), output->offset, -LIMIT_STD_MAX,
                         +LIMIT_STD_MAX, PREC1);
    lv_label_set_text(offset, s);

    lv_label_set_text_fmt(center, "%d%s", PPM_CENTER + output->ppmCenter,
                          output->symetrical ? " =" : STR_CHAR_DELTA);

    curve->show(output->curve);
  }

  static LAYOUT_VAL(CH_LINE_H, 32, 50)
  static LAYOUT_VAL(CH_BAR_WIDTH, 100, 100)
  static LAYOUT_VAL(CH_BAR_HEIGHT, 16, 16)
  static LAYOUT_VAL(BAR_XO, 17, 17)
  static constexpr coord_t BAR_X = LCD_W - CH_BAR_WIDTH - BAR_XO;

  static constexpr coord_t SRC_X = PAD_TINY;
  static constexpr coord_t SRC_Y = 1;
  static LAYOUT_VAL(SRC_W, 80, 80)
  static constexpr coord_t SRC_H = CH_LINE_H - PAD_MEDIUM;
  static constexpr coord_t MIN_X = SRC_X + SRC_W + PAD_TINY;
  static LAYOUT_VAL(MIN_Y, 4, 2)
  static LAYOUT_VAL(MIN_W, 52, 52)
  static LAYOUT_VAL(MIN_H, 20, 20)
  static constexpr coord_t MAX_X = MIN_X + MIN_W + PAD_TINY;
  static constexpr coord_t MAX_Y = MIN_Y;
  static LAYOUT_VAL(MAX_W, 52, 60)
  static constexpr coord_t MAX_H = MIN_H;
  static LAYOUT_VAL(OFF_X, MAX_X + MAX_W + PAD_TINY, SRC_X + SRC_W + PAD_TINY)
  static LAYOUT_VAL(OFF_Y, MIN_Y, 24)
  static LAYOUT_VAL(OFF_W, 44, 52)
  static constexpr coord_t OFF_H = MIN_H;
  static constexpr coord_t CTR_X = OFF_X + OFF_W + PAD_TINY;
  static constexpr coord_t CTR_Y = OFF_Y;
  static LAYOUT_VAL(CTR_W, 60, 60)
  static constexpr coord_t CTR_H = MIN_H;
  static constexpr coord_t REV_X = CTR_X + CTR_W + PAD_TINY;
  static constexpr coord_t REV_Y = CTR_Y;
  static LAYOUT_VAL(REV_W, 16, 16)
  static constexpr coord_t CRV_X = REV_X + REV_W + PAD_TINY;
  static constexpr coord_t CRV_Y = REV_Y + 1;

 protected:
  int value = -10000;

  bool isActive() const override { return false; }

  void checkEvents() override
  {
    ListLineButton::checkEvents();
    if (!init) return;

    int newValue = channelOutputs[index];
    if (value != newValue) {
      value = newValue;

      int chanVal = calcRESXto100(ex_chans[index]);

      if (chanVal < -DEADBAND) {
        lv_obj_add_state(min, ETX_STATE_MINMAX_BOLD);
      } else {
        lv_obj_clear_state(min, ETX_STATE_MINMAX_BOLD);
      }

      if (chanVal > DEADBAND) {
        lv_obj_add_state(max, ETX_STATE_MINMAX_BOLD);
      } else {
        lv_obj_clear_state(max, ETX_STATE_MINMAX_BOLD);
      }
    }
  }
};

ModelOutputsPage::ModelOutputsPage() :
    PageTab(STR_MENULIMITS, ICON_MODEL_OUTPUTS)
{
}

void ModelOutputsPage::build(Window* window)
{
  window->padAll(PAD_ZERO);
  window->padBottom(PAD_LARGE);

  new TextButton(window, {ADD_TRIMS_X, ADD_TRIMS_Y, ADD_TRIMS_W, ADD_TRIMS_H}, STR_ADD_ALL_TRIMS_TO_SUBTRIMS, [=]() {
    new ConfirmDialog(
        STR_TRIMS2OFFSETS, STR_ADD_ALL_TRIMS_TO_SUBTRIMS,
        [=] {
          moveTrimsToOffsets();
        });
    return 0;
  });

  new StaticText(window, {EXLIM_X, EXLIM_Y, EXLIM_W, EXLIM_H}, STR_ELIMITS, COLOR_THEME_PRIMARY1_INDEX, RIGHT);
  new ToggleSwitch(window, {EXLIMCB_X, EXLIMCB_Y, EXLIMCB_W, EXLIMCB_H}, GET_SET_DEFAULT(g_model.extendedLimits));

  for (uint8_t ch = 0; ch < MAX_OUTPUT_CHANNELS; ch++) {
    // Channel settings
    auto btn = new OutputLineButton(window, ch);
    lv_obj_set_pos(btn->getLvObj(), TRIMB_X, TRIMB_Y + (ch * (OutputLineButton::CH_LINE_H + 2)));
    btn->setWidth(TRIMB_W);

    LimitData* output = limitAddress(ch);
    btn->setPressHandler([=]() -> uint8_t {
      Menu* menu = new Menu();
      menu->addLine(STR_EDIT, [=]() { editOutput(ch, btn); });
      menu->addLine(STR_RESET, [=]() {
        output->min = 0;
        output->max = 0;
        output->offset = 0;
        output->ppmCenter = 0;
        output->revert = false;
        output->curve = 0;
        output->symetrical = 0;
        storageDirty(EE_MODEL);
        btn->refresh();
      });
      menu->addLine(STR_COPY_STICKS_TO_OFS, [=]() {
        copySticksToOffset(ch);
        storageDirty(EE_MODEL);
        btn->refresh();
      });
      menu->addLine(STR_COPY_TRIMS_TO_OFS, [=]() {
        copyTrimsToOffset(ch);
        storageDirty(EE_MODEL);
        btn->refresh();
      });
      menu->addLine(STR_COPY_MIN_MAX_TO_OUTPUTS, [=]() {
        copyMinMaxToOutputs(ch);
        storageDirty(EE_MODEL);
        btn->refresh();
      });
      return 0;
    });
  }
}

void ModelOutputsPage::editOutput(uint8_t channel, OutputLineButton* btn)
{
  (new OutputEditWindow(channel))->setCloseHandler([=]() { btn->refresh(); });
}
