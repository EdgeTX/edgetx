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

#define LABEL_RIGHT_MARGIN 5

bool isTrimModeAvailable(int mode)
{
  return (mode < 0 || (mode%2) == 0 || (mode/2) != 0); //ToDo menuVerticalPosition
}

struct FlightModeEdit : public Page {
  FlightModeEdit(uint8_t index);
};

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};

static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

// static const lv_coord_t trims_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
//                                            LV_GRID_TEMPLATE_LAST};

static const lv_coord_t trims_row_dsc[] = {LV_GRID_CONTENT,
                                           LV_GRID_CONTENT,
                                           LV_GRID_CONTENT,
                                           LV_GRID_TEMPLATE_LAST};

static void btn_changed(lv_event_t* e)
{
  auto target = lv_event_get_target(e);
  auto btn = (Button*)lv_obj_get_user_data(target);
  auto obj = (lv_obj_t*)lv_event_get_user_data(e);

  if (!btn->checked()) {
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    lv_event_send(obj, LV_EVENT_VALUE_CHANGED, nullptr);
  }
}

static void make_conditional(Window* w, Button* btn)
{
  lv_obj_t* w_obj = w->getLvObj();
  if (!btn->checked()) { lv_obj_add_flag(w_obj, LV_OBJ_FLAG_HIDDEN); }

  lv_obj_t* btn_obj = btn->getLvObj();
  lv_obj_add_event_cb(btn_obj, btn_changed, LV_EVENT_CLICKED, w_obj);
}

static std::string getFMTrimStr(uint8_t mode)
{
  char str[4];

  mode &= 0x1F;

  if (mode & 1) str[0] = '+';
  else str[0] = '=';
  str[1] = ' ';

  mode >>= 1;
  if (mode > MAX_FLIGHT_MODES - 1) mode = MAX_FLIGHT_MODES - 1;
  str[2] = '0' + mode;
  str[3] = '\0';

  return str;
}

struct FMTrimSettings : public Dialog {
  FMTrimSettings(Window* parent, FlightModeData* p_fm) :
    Dialog(parent->getFullScreenWindow(), STR_TRIMS, rect_t{})
  {
    setCloseWhenClickOutside(true);
    auto form = &content->form;

    FlexGridLayout trim_grid(line_col_dsc, trims_row_dsc);
    auto line = form->newLine(&trim_grid);

    for (int t = 0; t < NUM_TRIMS; t++) {

      auto trim = new FormGroup(line, rect_t{});
      trim->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(8));

      auto trim_obj = trim->getLvObj();
      lv_obj_set_style_pad_column(trim_obj, lv_dpx(8), 0);
      lv_obj_set_style_flex_cross_place(trim_obj, LV_FLEX_ALIGN_CENTER, 0);
      lv_obj_set_style_grid_cell_x_align(trim_obj, LV_GRID_ALIGN_STRETCH, 0);

      trim_t* tr = &p_fm->trim[t];
      auto btn = new TextButton(
          trim, rect_t{}, getSourceString(MIXSRC_FIRST_TRIM + t),
          [=]() {
            if (tr->mode == TRIM_MODE_NONE) {
              tr->mode = 0;
              SET_DIRTY();
              return 1;
            } else {
              tr->mode = TRIM_MODE_NONE;
              SET_DIRTY();
              return 0;
            }
          },
          0, COLOR_THEME_PRIMARY1);

      if (tr->mode != TRIM_MODE_NONE) btn->check();
      btn->setWidth(LV_DPI_DEF / 2);
      btn->setHeight(LV_DPI_DEF / 4);

      auto tr_mode = new Choice(trim, rect_t{}, 0, 2 * MAX_FLIGHT_MODES - 1,
                                GET_SET_DEFAULT(tr->mode));
      tr_mode->setTextHandler(getFMTrimStr);

      // show trim value choice iff btn->checked()
      make_conditional(tr_mode, btn);
    }

    content->setWidth(LCD_W * 0.8);
    content->updateSize();
  }
};

FlightModeEdit::FlightModeEdit(uint8_t index) :
    Page(ICON_MODEL_FLIGHT_MODES)
{
  std::string title = std::string(TR_MENUFLIGHTMODE)
    + " " + std::to_string(index);
  header.setTitle(title);

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
  new TextButton(line, rect_t{}, STR_SETUP, [=]() {
    new FMTrimSettings(this, p_fm);
    return 0;
  });
}

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
    txt_flags = FONT(BOLD) | COLOR_THEME_PRIMARY2;
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

  new TextButton(window, rect_t{}, "Check FM Trims", [&]() -> uint8_t {
    if (trimsCheckTimer)
      trimsCheckTimer = 0;
    else
      trimsCheckTimer = 200;  // 2 seconds trims cancelled
    return trimsCheckTimer;
  });
}
