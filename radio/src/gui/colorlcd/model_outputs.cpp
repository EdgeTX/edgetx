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
#include "output_edit.h"
#include "list_line_button.h"
#include "channel_bar.h"

#include "opentx.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

static const uint8_t _mask_textline_curve[] = {
#include "mask_textline_curve.lbm"
};
STATIC_LZ4_BITMAP(mask_textline_curve);

#define CH_BAR_WIDTH  92
#define CH_BAR_HEIGHT 14

#if LCD_W > LCD_H // Landscape

#define CH_BAR_COL     7
#define CH_BAR_COLSPAN 1

static const lv_coord_t col_dsc[] = {
  80, 50, 54, 44, 60, 18, 18, LV_GRID_FR(1),
  LV_GRID_TEMPLATE_LAST
};

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};
#else // Portrait

#define CH_BAR_COL     3
#define CH_BAR_COLSPAN 3

static const lv_coord_t col_dsc[] = {
  80, 50, 60, 18, 18, LV_GRID_FR(1),
  LV_GRID_TEMPLATE_LAST
};

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT,
                                     LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};
#endif

class OutputLineButton : public ListLineButton
{
  bool init = false;

  lv_obj_t* source = nullptr;
  lv_obj_t* revert = nullptr;
  lv_obj_t* min = nullptr;
  lv_obj_t* max = nullptr;
  lv_obj_t* offset = nullptr;
  lv_obj_t* center = nullptr;
  lv_obj_t* curve = nullptr;
  OutputChannelBar* bar = nullptr;

  static lv_img_dsc_t curveIcon;
  static void loadCurveIcon();

  static void on_draw(lv_event_t * e)
  {
    lv_obj_t* target = lv_event_get_target(e);
    auto line = (OutputLineButton*)lv_obj_get_user_data(target);
    if (line) {
      if (!line->init)
        line->delayed_init(e);
      else
        line->refresh();
    }
  }
  
  void delayed_init(lv_event_t* e)
  {
    uint8_t col = 1, row = 0;

    min = lv_label_create(lvobj);
    lv_obj_set_style_text_align(min, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_font(min, getFont(FONT(BOLD)), LV_STATE_USER_1);
    lv_obj_set_grid_cell(min, LV_GRID_ALIGN_END, col++, 1, LV_GRID_ALIGN_START,
                         row, 1);

    max = lv_label_create(lvobj);
    lv_obj_set_style_text_align(max, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_font(max, getFont(FONT(BOLD)), LV_STATE_USER_1);
    lv_obj_set_grid_cell(max, LV_GRID_ALIGN_END, col++, 1, LV_GRID_ALIGN_START,
                         row, 1);

#if LCD_H > LCD_W
    col = 1;
    row++;
#endif

    offset = lv_label_create(lvobj);
    lv_obj_set_grid_cell(offset, LV_GRID_ALIGN_END, col++, 1,
                         LV_GRID_ALIGN_START, row, 1);

    center = lv_label_create(lvobj);
    lv_obj_set_style_pad_left(center, 8, 0);
    lv_obj_set_grid_cell(center, LV_GRID_ALIGN_START, col++, 1,
                         LV_GRID_ALIGN_START, row, 1);

    revert = lv_img_create(lvobj);
    lv_img_set_src(revert, LV_SYMBOL_SHUFFLE);
    lv_obj_set_grid_cell(revert, LV_GRID_ALIGN_START, col++, 1,
                         LV_GRID_ALIGN_START, row, 1);

    curve = lv_img_create(lvobj);
    loadCurveIcon();
    lv_img_set_src(curve, &curveIcon);
    lv_obj_set_style_img_recolor(curve, makeLvColor(COLOR_THEME_SECONDARY1), 0);
    lv_obj_set_style_img_recolor_opa(curve, LV_OPA_COVER, 0);
    lv_obj_set_grid_cell(curve, LV_GRID_ALIGN_START, col++, 1,
                         LV_GRID_ALIGN_START, row, 1);

    bar = new OutputChannelBar(this, rect_t{}, index);
    bar->setWidth(CH_BAR_WIDTH);
    bar->setHeight(CH_BAR_HEIGHT);
    bar->setDrawLimits(false);

    lv_obj_set_grid_cell(bar->getLvObj(), LV_GRID_ALIGN_END, CH_BAR_COL,
                         CH_BAR_COLSPAN, LV_GRID_ALIGN_CENTER, 0, 1);

    init = true;
    refresh();
    lv_obj_update_layout(lvobj);

    if(e) {
      auto param = lv_event_get_param(e);
      lv_event_send(lvobj, LV_EVENT_DRAW_MAIN, param);
    }
  }
  
 public:
  OutputLineButton(Window* parent, uint8_t channel) :
      ListLineButton(parent, channel)
  {
    lv_obj_set_layout(lvobj, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(lvobj, col_dsc, row_dsc);

    source = lv_label_create(lvobj);

#if LCD_H > LCD_W
    lv_obj_set_grid_cell(source, LV_GRID_ALIGN_START, 0, 1,
                         LV_GRID_ALIGN_CENTER, 0, 2);

#else
    lv_obj_set_style_text_font(source, getFont(FONT(XS)), 0);
    lv_obj_set_grid_cell(source, LV_GRID_ALIGN_START, 0, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);
#endif

    lv_obj_update_layout(parent->getLvObj());
    if(lv_obj_is_visible(lvobj)) delayed_init(nullptr);

    lv_obj_add_event_cb(lvobj, OutputLineButton::on_draw, LV_EVENT_DRAW_MAIN_BEGIN, nullptr);
  }

  void refresh() override
  {
    if (!init) return;
    
    const LimitData* output = limitAddress(index);
    if(g_model.limitData[index].name[0] != '\0')
    {
#if LCD_W > LCD_H
      lv_obj_set_style_text_line_space(source, -3, LV_PART_MAIN);
      lv_obj_set_style_pad_top(source, -7, 0);
      lv_obj_set_style_pad_bottom(source, -7, 0);
#endif
      lv_label_set_text_fmt(source, "%s\n" TR_CH "%u", getSourceString(MIXSRC_FIRST_CH + index), index + 1);
    } else {
      lv_obj_set_style_text_font(source, getFont(FONT(STD)), 0);
      lv_label_set_text(source, getSourceString(MIXSRC_FIRST_CH + index));
    }
    if (output->revert) {
      lv_obj_clear_flag(revert, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(revert, LV_OBJ_FLAG_HIDDEN);
    }

    char s[32];
    getValueOrGVarString(s, sizeof(s), output->min, -GV_RANGELARGE, 0, PREC1,
                         nullptr, -LIMITS_MIN_MAX_OFFSET);
    lv_label_set_text(min, s);

    getValueOrGVarString(s, sizeof(s), output->max, 0, GV_RANGELARGE, PREC1,
                         nullptr, +LIMITS_MIN_MAX_OFFSET);
    lv_label_set_text(max, s);

    getValueOrGVarString(s, sizeof(s), output->offset, -LIMIT_STD_MAX,
                         +LIMIT_STD_MAX, PREC1);
    lv_label_set_text(offset, s);

    lv_label_set_text_fmt(center, "%d%s", PPM_CENTER + output->ppmCenter,
                          output->symetrical ? " =" : STR_CHAR_DELTA);

    if (output->curve) {
      lv_obj_clear_flag(curve, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(curve, LV_OBJ_FLAG_HIDDEN);
    }
  }

 protected:
  int value = 0;

  bool isActive() const override { return false; }

  void checkEvents() override
  {
    Window::checkEvents();
    if (!init) return;

    int newValue = channelOutputs[index];
    if (value != newValue) {
      value = newValue;

      const LimitData* output = limitAddress(index);
      int chanZero = output->ppmCenter;

      if (value < chanZero - 5) {
        lv_obj_add_state(min, LV_STATE_USER_1);
      } else {
        lv_obj_clear_state(min, LV_STATE_USER_1);
      }

      if (value > chanZero + 5) {
        lv_obj_add_state(max, LV_STATE_USER_1);
      } else {
        lv_obj_clear_state(max, LV_STATE_USER_1);
      }
    }
  }
};

lv_img_dsc_t OutputLineButton::curveIcon = {
    .header =
        {
            .cf = LV_IMG_CF_ALPHA_8BIT,
            .always_zero = 0,
            .reserved = 0,
            .w = 0,
            .h = 0,
        },
    .data_size = 0,
    .data = nullptr,
};

void OutputLineButton::loadCurveIcon()
{
  if (curveIcon.data) return;
  
  auto mask = (const uint8_t*)mask_textline_curve;
  auto mask_hdr = (const uint16_t*)mask;
  mask += 4;

  auto w = mask_hdr[0];
  auto h = mask_hdr[1];
  curveIcon.header.w = w;
  curveIcon.header.h = h;
  curveIcon.data_size = w * h;
  curveIcon.data = mask;
}

ModelOutputsPage::ModelOutputsPage() :
  PageTab(STR_MENULIMITS, ICON_MODEL_OUTPUTS)
{
}

void ModelOutputsPage::build(FormWindow *window)
{
  window->setFlexLayout();
  window->padRow(lv_dpx(4));

  auto form = new FormGroup(window, rect_t{});
  form->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, lv_dpx(16));
  form->padRow(lv_dpx(8));
  form->padBottom(lv_dpx(4));

  auto form_obj = form->getLvObj();
  lv_obj_set_style_flex_cross_place(form_obj, LV_FLEX_ALIGN_CENTER, 0);

  new TextButton(form, rect_t{}, STR_ADD_ALL_TRIMS_TO_SUBTRIMS, [=]() {
    moveTrimsToOffsets();
    window->invalidate();
    return 0;
  });

  auto box = new FormGroup(form, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(8));

  auto box_obj = box->getLvObj();
  lv_obj_set_width(box_obj, LV_SIZE_CONTENT);
  lv_obj_set_style_flex_cross_place(box_obj, LV_FLEX_ALIGN_CENTER, 0);

  new StaticText(box, rect_t{}, STR_ELIMITS, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(box, rect_t{}, GET_SET_DEFAULT(g_model.extendedLimits));  

  for (uint8_t ch = 0; ch < MAX_OUTPUT_CHANNELS; ch++) {

    // Channel settings
    auto btn = new OutputLineButton(window, ch);
    // btn->refresh();

    LimitData* output = limitAddress(ch);
    btn->setPressHandler([=]() -> uint8_t {
      Menu *menu = new Menu(window);
      menu->addLine(STR_EDIT, [=]() {
          editOutput(ch, btn);
      });
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
      return 0;
    });
  }
}

void ModelOutputsPage::editOutput(uint8_t channel, OutputLineButton* btn)
{
  auto btn_obj = btn->getLvObj();
  auto edit = new OutputEditWindow(channel);
  edit->setCloseHandler(
      [=]() { lv_event_send(btn_obj, LV_EVENT_VALUE_CHANGED, nullptr); });
}
