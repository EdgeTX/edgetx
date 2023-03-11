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
#include "lvgl_widgets/input_mix_line.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

static std::string getFMTrimStr(uint8_t mode, bool spacer)
{
  mode &= 0x1F;
  if (mode == TRIM_MODE_NONE)
    return "-";
  std::string str((mode & 1) ? "+" : "=");
  if (spacer) str += " ";
  mode >>= 1;
  if (mode > MAX_FLIGHT_MODES - 1)
    mode = MAX_FLIGHT_MODES - 1;
  str += '0'+ mode;
  return str;
}

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
        lastTrim[t] = p_fm->trim[t].value;

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
        tr_mode[t]->setTextHandler([=](uint8_t mode) { return getFMTrimStr(mode, true); });
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

    void checkEvents() override
    {
      for (int i = 0; i < NUM_TRIMS; i += 1) {
        const auto& fm = g_model.flightModeData[index];
        if (lastTrim[i] != fm.trim[i].value) {
          lastTrim[i] = fm.trim[i].value;
          tr_value[i]->setValue(lastTrim[i]);
        }
      }
    }

  protected:
    uint8_t index;
    Choice* tr_mode[NUM_TRIMS] = {nullptr};
    NumberEdit* tr_value[NUM_TRIMS] = {nullptr};
    int lastTrim[NUM_TRIMS];

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

#if LCD_W > LCD_H  // Landscape

static const lv_coord_t b_col_dsc[] = {36, 95, 50, LV_GRID_FR(1), 40, 40,
                                       LV_GRID_TEMPLATE_LAST};

static const lv_coord_t b_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

#define NM_ROW_CNT 1
#define TRIM_ROW 0
#define TRIM_COL 3
#define TRIM_COL_CNT 1
#define TRIM_W 30
#define SWTCH_COL_CNT 1

#else  // Portrait

static const lv_coord_t b_col_dsc[] = {50, LV_GRID_FR(1), 40, 40,
                                       LV_GRID_TEMPLATE_LAST};

static const lv_coord_t b_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                       LV_GRID_TEMPLATE_LAST};

#define NM_ROW_CNT 2
#define TRIM_ROW 1
#define TRIM_COL 1
#define TRIM_COL_CNT 2
#define TRIM_W 40
#define SWTCH_COL_CNT 2

#endif

class FlightModeBtn : public Button
{
 public:
  FlightModeBtn(Window* parent, int index) :
      Button(parent, rect_t{}, nullptr, 0, 0, input_mix_line_create),
      index(index)
  {
    padTop(0);
    padBottom(0);
    padLeft(3);
    padRight(3);
    lv_obj_set_layout(lvobj, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(lvobj, b_col_dsc, b_row_dsc);
    lv_obj_set_style_pad_row(lvobj, 0, 0);
    lv_obj_set_style_pad_column(lvobj, 4, 0);

    check(isActive());
 
    lv_obj_update_layout(parent->getLvObj());
    if (lv_obj_is_visible(lvobj)) delayed_init(nullptr);

    lv_obj_add_event_cb(lvobj, FlightModeBtn::on_draw,
                        LV_EVENT_DRAW_MAIN_BEGIN, nullptr);
  }

  static void on_draw(lv_event_t* e)
  {
    lv_obj_t* target = lv_event_get_target(e);
    auto line = (FlightModeBtn*)lv_obj_get_user_data(target);
    if (line) {
      if (!line->init)
        line->delayed_init(e);
      else
        line->refresh();
    }
  }

  void delayed_init(lv_event_t* e)
  {
    fmID = lv_label_create(lvobj);
    lv_obj_set_style_text_align(fmID, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_grid_cell(fmID, LV_GRID_ALIGN_STRETCH, 0, 1,
                         LV_GRID_ALIGN_CENTER, 0, NM_ROW_CNT);

    fmName = lv_label_create(lvobj);
    lv_obj_set_style_text_align(fmName, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_grid_cell(fmName, LV_GRID_ALIGN_STRETCH, 1, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_style_text_font(fmName, getFont(FONT(XS)), 0);

    fmSwitch = lv_label_create(lvobj);
    lv_obj_set_style_text_align(fmSwitch, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_grid_cell(fmSwitch, LV_GRID_ALIGN_STRETCH, 2, SWTCH_COL_CNT,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t* container = lv_obj_create(lvobj);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_grow(container, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
    lv_obj_set_height(container, LV_SIZE_CONTENT);
    lv_obj_add_flag(container, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_grid_cell(container, LV_GRID_ALIGN_STRETCH, TRIM_COL, TRIM_COL_CNT,
                         LV_GRID_ALIGN_CENTER, TRIM_ROW, 1);

    for (int i = 0; i < NUM_TRIMS; i += 1) {
      lv_obj_t* tr_cont = lv_obj_create(container);
      lv_obj_set_flex_flow(tr_cont, LV_FLEX_FLOW_COLUMN);
      lv_obj_set_style_pad_all(tr_cont, 0, LV_PART_MAIN);
      lv_obj_set_height(tr_cont, LV_SIZE_CONTENT);
      lv_obj_set_width(tr_cont, TRIM_W);
      lv_obj_set_user_data(tr_cont, this);
      lv_obj_add_flag(tr_cont, LV_OBJ_FLAG_EVENT_BUBBLE);

      fmTrimMode[i] = lv_label_create(tr_cont);
      lv_obj_set_height(fmTrimMode[i], 16);
      lv_obj_set_style_text_align(fmTrimMode[i], LV_TEXT_ALIGN_CENTER, 0);

      fmTrimValue[i] = lv_label_create(tr_cont);
      lv_obj_set_style_text_align(fmTrimValue[i], LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_height(fmTrimValue[i], 16);
      lv_obj_set_style_text_font(fmTrimValue[i], getFont(FONT(XS)), 0);
    }

    fmFadeIn = lv_label_create(lvobj);
    lv_obj_set_style_text_align(fmFadeIn, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_grid_cell(fmFadeIn, LV_GRID_ALIGN_END, TRIM_COL+1, 1,
                         LV_GRID_ALIGN_CENTER, TRIM_ROW, 1);

    fmFadeOut = lv_label_create(lvobj);
    lv_obj_set_style_text_align(fmFadeOut, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_grid_cell(fmFadeOut, LV_GRID_ALIGN_END, TRIM_COL+2, 1,
                         LV_GRID_ALIGN_CENTER, TRIM_ROW, 1);

    init = true;
    refresh();
    lv_obj_update_layout(lvobj);

    if (e) {
      auto param = lv_event_get_param(e);
      lv_event_send(lvobj, LV_EVENT_DRAW_MAIN, param);
    }
  }

  bool isActive() const
  {
    return (getFlightMode() == index);
  }

  void checkEvents() override
  {
    Button::checkEvents();
    check(isActive());
    if (!refreshing && init) {
      refreshing = true;
      const auto& fm = g_model.flightModeData[index];
      for (int t = 0; t < NUM_TRIMS; t += 1) {
        if (lastTrim[t] != fm.trim[t].value) {
          lastTrim[t] = fm.trim[t].value;

          uint8_t mode = fm.trim[t].mode;
          bool checked = (mode != TRIM_MODE_NONE);
          bool showValue = (index == 0) || ((mode & 1) || (mode >> 1 == index));

          if (checked && showValue)
            lv_label_set_text(fmTrimValue[t], formatNumberAsString(fm.trim[t].value).c_str());
        }
      }
      refreshing = false;
    }
  }

  void refresh()
  {
    if (!init) return;

    const auto& fm = g_model.flightModeData[index];

    char label[8];
    getFlightModeString(label, index + 1);
    lv_label_set_text(fmID, label);

    if (fm.name[0] != '\0') {
      lv_label_set_text(fmName, fm.name);
    } else {
      lv_label_set_text(fmName, "");
    }

    if ((index > 0) && (fm.swtch != SWSRC_NONE)) {
      getSwitchPositionName(label, fm.swtch);
      lv_label_set_text(fmSwitch, label);
    } else {
      lv_label_set_text(fmSwitch, "");
    }

    for (int i = 0; i < NUM_TRIMS; i += 1) {
      uint8_t mode = fm.trim[i].mode;
      bool checked = (mode != TRIM_MODE_NONE);
      bool showValue = (index == 0) || ((mode & 1) || (mode >> 1 == index));

      lv_label_set_text(fmTrimMode[i], getFMTrimStr(mode, false).c_str());

      if (checked && showValue)
        lv_label_set_text(fmTrimValue[i], formatNumberAsString(fm.trim[i].value).c_str());
      else
        lv_label_set_text(fmTrimValue[i], "");
    }

    lv_label_set_text(fmFadeIn, formatNumberAsString(fm.fadeIn, PREC1, 0, nullptr, "s").c_str());
    lv_label_set_text(fmFadeOut, formatNumberAsString(fm.fadeOut, PREC1, 0, nullptr, "s").c_str());
  }

 protected:
  bool init = false;
  bool refreshing = false;
  uint8_t index;

  lv_obj_t* fmID = nullptr;
  lv_obj_t* fmName = nullptr;
  lv_obj_t* fmSwitch = nullptr;
  lv_obj_t* fmTrimMode[NUM_TRIMS] = {nullptr};
  lv_obj_t* fmTrimValue[NUM_TRIMS] = {nullptr};
  lv_obj_t* fmFadeIn = nullptr;
  lv_obj_t* fmFadeOut = nullptr;
  int lastTrim[NUM_TRIMS];
};

ModelFlightModesPage::ModelFlightModesPage():
  PageTab(STR_MENUFLIGHTMODES, ICON_MODEL_FLIGHT_MODES)
{
}

static const lv_coord_t fmt_col_dsc[] = {LV_GRID_FR(1),
                                         LV_GRID_TEMPLATE_LAST};

static const lv_coord_t fmt_row_dsc[] = {LV_GRID_CONTENT,
                                         LV_GRID_TEMPLATE_LAST};

void ModelFlightModesPage::build(FormWindow * window)
{
  window->padAll(4);
  lv_obj_set_scrollbar_mode(window->getLvObj(), LV_SCROLLBAR_MODE_AUTO);

  FormWindow* form = new FormWindow(window, rect_t{});
  form->setFlexLayout();
  form->padRow(lv_dpx(4));

  FlexGridLayout grid(fmt_col_dsc, fmt_row_dsc, 0);

  for (int i = 0; i < MAX_FLIGHT_MODES; i++) {
    auto fm_box = form->newLine(&grid);
    fm_box->padAll(0);

    auto btn = new FlightModeBtn(fm_box, i);
    btn->setPressHandler([=]() {
                           new FlightModeEdit(i);
                           return 0;
                         });
#if LCD_W > LCD_H
    // Initial scroll height is incorrect without this???
    btn->setHeight(36);
#endif
  }

  auto fm_box = form->newLine(&grid);
  trimCheck = new TextButton(fm_box, rect_t{0, 0, lv_pct(100), 40}, STR_CHECKTRIMS, [&]() -> uint8_t {
    if (trimsCheckTimer)
      trimsCheckTimer = 0;
    else
      trimsCheckTimer = 200;  // 2 seconds trims cancelled
    return trimsCheckTimer;
  });
}

void ModelFlightModesPage::checkEvents()
{
  trimCheck->check(trimsCheckTimer > 0);
}
