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

#include "model_flightmodes.h"
#include "opentx.h"
#include "libopenui.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};

static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

#if LCD_W > LCD_H
#define TRIMS_PER_LINE 2
static const lv_coord_t trims_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
#else
#define TRIMS_PER_LINE 1
static const lv_coord_t trims_col_dsc[] = {LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
#endif

class FlightModeEdit : public Page
{
 public:
    FlightModeEdit(uint8_t index) :
      Page(ICON_MODEL_FLIGHT_MODES),
      index(index)
    {
      std::string title2 = std::string(STR_FM) + std::to_string(index);
      header.setTitle(STR_MENUFLIGHTMODES);
      header.setTitle2(title2);

      body.padAll(lv_dpx(8));
      lv_obj_set_scrollbar_mode(body.getLvObj(), LV_SCROLLBAR_MODE_AUTO);

      FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);
      auto form = new FormWindow(&body, rect_t{});
      form->setFlexLayout();

      FlightModeData* p_fm = &g_model.flightModeData[index];
  
      // Flight mode name
      auto line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_NAME, 0, COLOR_THEME_PRIMARY1);
      new ModelTextEdit(line, rect_t{}, p_fm->name, LEN_FLIGHT_MODE_NAME);

      if (index > 0) {
        // Switch
        line = form->newLine(&grid);
        new StaticText(line, rect_t{}, STR_SWITCH, 0, COLOR_THEME_PRIMARY1);
        new SwitchChoice(line, rect_t{}, SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES,
                         GET_SET_DEFAULT(p_fm->swtch));
      }

      // Fade in
      line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_FADEIN, 0, COLOR_THEME_PRIMARY1);
      new NumberEdit(line, rect_t{}, 0, DELAY_MAX, GET_DEFAULT(p_fm->fadeIn),
                     SET_VALUE(p_fm->fadeIn, newValue), 0, PREC1);

      // Fade out
      line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_FADEOUT, 0, COLOR_THEME_PRIMARY1);
      new NumberEdit(line, rect_t{}, 0, DELAY_MAX, GET_DEFAULT(p_fm->fadeOut),
                     SET_VALUE(p_fm->fadeOut, newValue), 0, PREC1);

      // Trims
      line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_TRIMS, 0, COLOR_THEME_PRIMARY1);

      FlexGridLayout trim_grid(trims_col_dsc, line_row_dsc);

      for (int t = 0; t < NUM_TRIMS; t++) {
        if ((t % TRIMS_PER_LINE) == 0) {
          line = form->newLine(&trim_grid);
          line->padLeft(10);
        }

        auto trim = new FormGroup(line, rect_t{});
        trim->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(8));

        auto trim_obj = trim->getLvObj();
        lv_obj_set_style_pad_column(trim_obj, lv_dpx(8), 0);
        lv_obj_set_style_flex_cross_place(trim_obj, LV_FLEX_ALIGN_CENTER, 0);
        lv_obj_set_style_grid_cell_x_align(trim_obj, LV_GRID_ALIGN_STRETCH, 0);

        trim_t* tr = &p_fm->trim[t];
        auto tr_btn = new TextButton(
            trim, rect_t{}, getSourceString(MIXSRC_FIRST_TRIM + t),
            [=]() {
              tr->mode = (tr->mode == TRIM_MODE_NONE) ? 0 : TRIM_MODE_NONE;
              tr_mode[t]->setValue(tr->mode);
              SET_DIRTY();
              showControls(t, tr->mode);
              return tr->mode == 0;
            });

        if (tr->mode != TRIM_MODE_NONE) tr_btn->check();
        tr_btn->setWidth(LV_DPI_DEF / 2);
        tr_btn->setHeight(33);

        tr_mode[t] = new Choice(trim, rect_t{}, 0, 2 * MAX_FLIGHT_MODES - 1,
                                GET_DEFAULT(tr->mode),
                                [=](int val) {
                                  tr->mode = val;
                                  showControls(t, tr->mode);
                                });
        tr_mode[t]->setTextHandler([=](uint8_t mode) {
                                     mode &= 0x1F;
                                     std::string str((mode & 1) ? "+ " : "= ");
                                     mode >>= 1;
                                     if (mode > MAX_FLIGHT_MODES - 1)
                                       mode = MAX_FLIGHT_MODES - 1;
                                     str += '0'+ mode;
                                     return str;
                                   });
        tr_mode[t]->setAvailableHandler([=](int mode) {
          return ((mode & 1) == 0) || ((mode >> 1) != index);
        });

        tr_value[t] = new NumberEdit(trim, rect_t{}, 
                                     g_model.extendedTrims ? -512 : -128, g_model.extendedTrims ? 512 : 128,
                                     GET_SET_DEFAULT(tr->value));

        // show trim value choice iff btn->checked()
        showControls(t, tr->mode);
      }
    }

  protected:
    uint8_t index;
    Choice* tr_mode[NUM_TRIMS] = {nullptr};
    NumberEdit* tr_value[NUM_TRIMS] = {nullptr};
    
    void showControls(int trim, uint8_t mode)
    {
      bool checked = (mode != TRIM_MODE_NONE);
      bool showValue = (index == 0) || ((mode & 1) || (mode >> 1 == index));

      if (checked && (index > 0)) {
        lv_obj_clear_flag(tr_mode[trim]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      } else {
        lv_obj_add_flag(tr_mode[trim]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      }
      if (checked && showValue) {
        lv_obj_clear_flag(tr_value[trim]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      } else {
        lv_obj_add_flag(tr_value[trim]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      }
    }
};

class FlightModeBtn: public Button
{
  uint8_t index;
  bool active = false;

public:
  FlightModeBtn(Window * parent, uint8_t index);
  void checkEvents() override;
  void paint(BitmapBuffer* dc) override;

  bool isActive() { return active; }
  void onPress() override;
};

static void fm_draw_begin(lv_event_t *e)
{
  auto fmg = (FlightModeBtn*)lv_event_get_user_data(e);
  if (!fmg) return;

  lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
  if (fmg->isActive()) {
    dsc->rect_dsc->bg_color = makeLvColor(COLOR_THEME_ACTIVE);
  } else {
    dsc->rect_dsc->bg_color = makeLvColor(COLOR_THEME_PRIMARY2);
  }
}

FlightModeBtn::FlightModeBtn(Window * parent, uint8_t index) :
  Button(parent, rect_t{}, nullptr, 0, 0, lv_btn_create),
  index(index)
{
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, 0);
  
  lv_obj_set_style_grid_cell_x_align(lvobj, LV_GRID_ALIGN_STRETCH, 0);
  lv_obj_set_width(lvobj, lv_pct(100));
  lv_obj_set_height(lvobj, LV_DPI_DEF / 3);

  lv_obj_add_event_cb(lvobj, fm_draw_begin, LV_EVENT_DRAW_PART_BEGIN, this);
}

void FlightModeBtn::checkEvents()
{
  bool newActive = (getFlightMode() == index);
  if (newActive != active) {
    active = newActive;
    invalidate();
  }
}

void FlightModeBtn::paint(BitmapBuffer* dc)
{
  coord_t x=8, y=0;

  LcdFlags txt_flags;
  if (active) {
    txt_flags = FONT(BOLD) | COLOR_THEME_PRIMARY1;
  } else {
    txt_flags = FONT(STD) | COLOR_THEME_SECONDARY1;
  }
  
  auto h = lv_obj_get_height(lvobj);
  y = (h - getFontHeight(txt_flags)) / 2;

  const auto& fm = g_model.flightModeData[index];
  if (fm.name[0] != '\0') {
    dc->drawSizedText(x, y, fm.name, sizeof(fm.name), txt_flags);
  } else {
    char label[8];
    getFlightModeString(label, index + 1);
    dc->drawSizedText(x, y, label, sizeof(label), txt_flags);
  }

  if ((index > 0) && (fm.swtch != SWSRC_NONE)) {
    char sw[8];
    getSwitchPositionName(sw, fm.swtch);

    x = lv_obj_get_width(lvobj) - 8;
    dc->drawSizedText(x, y, sw, sizeof(sw), txt_flags | RIGHT);
  }
}

void FlightModeBtn::onPress()
{
  new FlightModeEdit(index);
}

ModelFlightModesPage::ModelFlightModesPage():
  PageTab(STR_MENUFLIGHTMODES, ICON_MODEL_FLIGHT_MODES)
{
}

#if LCD_W > LCD_H
// FM table: 3 x 3
static const lv_coord_t fmt_col_dsc[] = {LV_GRID_FR(1),
                                         LV_GRID_FR(1),
                                         LV_GRID_FR(1),
                                         LV_GRID_TEMPLATE_LAST};

static const lv_coord_t fmt_row_dsc[] = {LV_GRID_CONTENT,
                                         LV_GRID_CONTENT,
                                         LV_GRID_CONTENT,
                                         LV_GRID_TEMPLATE_LAST};
#else
// FM table: 2 x 5
static const lv_coord_t fmt_col_dsc[] = {LV_GRID_FR(1),
                                         LV_GRID_FR(1),
                                         LV_GRID_TEMPLATE_LAST};

static const lv_coord_t fmt_row_dsc[] = {LV_GRID_CONTENT,
                                         LV_GRID_CONTENT,
                                         LV_GRID_CONTENT,
                                         LV_GRID_CONTENT,
                                         LV_GRID_CONTENT,
                                         LV_GRID_TEMPLATE_LAST};
#endif

void ModelFlightModesPage::build(FormWindow * window)
{
  window->setFlexLayout();
  window->padRow(lv_dpx(8));
  
  lv_obj_t* obj = window->getLvObj();
  lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, 0);

  FlexGridLayout grid(fmt_col_dsc, fmt_row_dsc, 0);
  auto fm_box = window->newLine(&grid);
  fm_box->padRow(lv_dpx(2));
  fm_box->padColumn(lv_dpx(2));
  
  for (int i = 0; i < MAX_FLIGHT_MODES; i++) {
    new FlightModeBtn(fm_box, i);
  }

  new TextButton(window, rect_t{}, STR_CHECKTRIMS, [&]() -> uint8_t {
    if (trimsCheckTimer)
      trimsCheckTimer = 0;
    else
      trimsCheckTimer = 200;  // 2 seconds trims cancelled
    return trimsCheckTimer;
  });
}
