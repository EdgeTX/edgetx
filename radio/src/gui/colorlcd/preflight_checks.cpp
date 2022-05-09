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

#include "preflight_checks.h"
#include "button_matrix.h"
#include "opentx.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

static void cb_changed(lv_event_t* e)
{
  auto target = lv_event_get_target(e);
  auto obj = (lv_obj_t*)lv_event_get_user_data(e);

  if (lv_obj_has_state(target, LV_STATE_CHECKED)) {
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  }
}

static void make_conditional(Window* w, CheckBox* cb)
{
  lv_obj_t* w_obj = w->getLvObj();
  if (!cb->getValue()) { lv_obj_add_flag(w_obj, LV_OBJ_FLAG_HIDDEN); }

  lv_obj_t* cb_obj = cb->getLvObj();
  lv_obj_add_event_cb(cb_obj, cb_changed, LV_EVENT_VALUE_CHANGED, w_obj);
}

static void choice_changed(lv_event_t* e)
{
  auto target = lv_event_get_target(e);
  auto choice = (Choice*)lv_obj_get_user_data(target);
  auto obj = (lv_obj_t*)lv_event_get_user_data(e);

  if (choice->getIntValue() != 0) {
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  }
}

static void make_conditional(Window* w, Choice* choice)
{
  lv_obj_t* w_obj = w->getLvObj();
  if (choice->getIntValue() == 0) { lv_obj_add_flag(w_obj, LV_OBJ_FLAG_HIDDEN); }

  lv_obj_t* choice_obj = choice->getLvObj();
  lv_obj_add_event_cb(choice_obj, choice_changed, LV_EVENT_VALUE_CHANGED, w_obj);
}

struct SwitchWarnMatrix : public ButtonMatrix {
  SwitchWarnMatrix(Window* parent, const rect_t& rect);
  void onPress(uint8_t btn_id);
  bool isActive(uint8_t btn_id);
private:
  uint8_t sw_idx[NUM_SWITCHES];
};

struct PotWarnMatrix : public ButtonMatrix {
  PotWarnMatrix(Window* parent, const rect_t& rect);
  void onPress(uint8_t btn_id);
  bool isActive(uint8_t btn_id);
private:
  uint8_t pot_idx[NUM_POTS + NUM_SLIDERS];
};

PreflightChecks::PreflightChecks() : Page(ICON_MODEL_SETUP)
{
  new StaticText(&header,
                 {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                  PAGE_LINE_HEIGHT},
                 STR_PREFLIGHT, 0, COLOR_THEME_PRIMARY2);

  auto form = new FormWindow(&body, rect_t{0, 0, body.width(), body.height()},
                             NO_FOCUS | FORM_FORWARD_FOCUS);
  form->setFlexLayout();
  FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

  // Display checklist
  auto line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_CHECKLIST, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_DEFAULT(g_model.displayChecklist));

  // Throttle warning
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_THROTTLE_WARNING, 0, COLOR_THEME_PRIMARY1);
  auto tw = new CheckBox(line, rect_t{}, GET_SET_INVERTED(g_model.disableThrottleWarning));

  // Custom Throttle warning (conditional on previous field)
  line = form->newLine(&grid);
  make_conditional(line, tw);

  new StaticText(line, rect_t{}, STR_CUSTOM_THROTTLE_WARNING, 0, COLOR_THEME_PRIMARY1);
  auto box = new FormGroup::Line(line, lv_obj_create(line->getLvObj()), nullptr, form);
  lv_obj_set_layout(box->getLvObj(), LV_LAYOUT_FLEX);

  auto cst_tw = new CheckBox(
      box, rect_t{}, GET_SET_DEFAULT(g_model.enableCustomThrottleWarning));

  // Custom Throttle warning value
  auto cst_val =
      new NumberEdit(box, rect_t{}, -100, 100,
                     GET_SET_DEFAULT(g_model.customThrottleWarningPosition));
  make_conditional(cst_val, cst_tw);

  // Switch warnings (TODO: add display switch?)
  new SwitchWarnMatrix(form, rect_t{});

  // Pots and sliders warning
#if NUM_POTS + NUM_SLIDERS
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_POTWARNINGSTATE, 0, COLOR_THEME_PRIMARY1);
  auto pots_wm = new Choice(line, rect_t{}, {"OFF", "ON", "AUTO"}, 0, 2,
                            GET_SET_DEFAULT(g_model.potsWarnMode));
#if (NUM_POTS)
  // Pot warnings
  auto pwm = new PotWarnMatrix(form, rect_t{});
  make_conditional(pwm, pots_wm);
#endif
#endif

  // // Center beeps
  // {
  //   new StaticText(window, grid.getLabelSlot(false), STR_BEEPCTR, 0,
  //                  COLOR_THEME_PRIMARY1);
  //   auto group = new FormGroup(window, grid.getFieldSlot(),
  //                              FORM_BORDER_FOCUS_ONLY | PAINT_CHILDREN_FIRST);
  //   GridLayout centerGrid(group);
  //   for (int i = 0, j = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
  //     char s[2];
  //     if (i < NUM_STICKS ||
  //         (IS_POT_SLIDER_AVAILABLE(i) &&
  //          !IS_POT_MULTIPOS(i))) {  // multipos cannot be centered
  //       if (j > 0 && (j % 6) == 0) centerGrid.nextLine();

  //       new TextButton(
  //           group, centerGrid.getSlot(6, j % 6),
  //           getStringAtIndex(s, STR_RETA123, i),
  //           [=]() -> uint8_t {
  //             BFBIT_FLIP(g_model.beepANACenter, bfBit<BeepANACenter>(i));
  //             SET_DIRTY();
  //             return (bfSingleBitGet<BeepANACenter>(g_model.beepANACenter, i)
  //                         ? 1
  //                         : 0);
  //           },
  //           OPAQUE | (bfSingleBitGet<BeepANACenter>(g_model.beepANACenter, i)
  //                         ? BUTTON_CHECKED
  //                         : 0));
  //       j++;
  //     }
  //   }
  //   grid.addWindow(group);
  // }
}

static std::string switchWarninglabel(swsrc_t index)
{
  auto warn_pos = g_model.switchWarningState >> (3 * index) & 0x07;
  return TEXT_AT_INDEX(STR_VSRCRAW,
                       (index + MIXSRC_FIRST_SWITCH - MIXSRC_Rud + 1)) +
         std::string(getSwitchWarnSymbol(warn_pos));
}

SwitchWarnMatrix::SwitchWarnMatrix(Window* parent, const rect_t& r) :
  ButtonMatrix(parent, r)
{
  // Setup button layout & texts
  uint8_t btn_cnt = 0;
  for (uint8_t i = 0; i < NUM_SWITCHES; i++) {
    if (SWITCH_EXISTS(i)) {
      sw_idx[btn_cnt] = i;
      btn_cnt++;
    }    
  }

  initBtnMap(4, btn_cnt);

  uint8_t btn_id = 0;
  for (uint8_t i = 0; i < NUM_SWITCHES; i++) {
    if (SWITCH_EXISTS(i)) {
      std::string txt = switchWarninglabel(i);
      setText(btn_id, txt.c_str());
      btn_id++;
    }
  }
  update();
  
  lv_obj_set_width(lvobj, LV_DPI_DEF * 2);

  uint8_t rows = ((btn_cnt - 1) / 4) + 1;
  lv_obj_set_height(lvobj, (rows * LV_DPI_DEF) / 3);
  
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_0, LV_PART_MAIN);

  lv_obj_set_style_pad_all(lvobj, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_left(lvobj, LV_DPI_DEF / 10, LV_PART_MAIN);

  lv_obj_set_style_pad_row(lvobj, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_column(lvobj, 4, LV_PART_MAIN);

  lv_obj_remove_style(lvobj, nullptr, LV_PART_MAIN | LV_STATE_FOCUSED);
  lv_obj_remove_style(lvobj, nullptr, LV_PART_MAIN | LV_STATE_EDITED);
}

void SwitchWarnMatrix::onPress(uint8_t btn_id)
{
  if (btn_id >= NUM_SWITCHES) return;
  auto sw = sw_idx[btn_id];

  swarnstate_t newstate = bfGet(g_model.switchWarningState, 3 * sw, 3);
  if (newstate == 1 && SWITCH_CONFIG(sw) != SWITCH_3POS)
    newstate = 3;
  else
    newstate = (newstate + 1) % 4;

  g_model.switchWarningState =
      bfSet(g_model.switchWarningState, newstate, 3 * sw, 3);
  SET_DIRTY();

  // TODO: save state in object
  std::string txt = switchWarninglabel(sw);
  setText(btn_id, txt.c_str());
}

bool SwitchWarnMatrix::isActive(uint8_t btn_id)
{
  if (btn_id >= NUM_SWITCHES) return false;
  return bfGet(g_model.switchWarningState, 3 * sw_idx[btn_id], 3) != 0;
}

PotWarnMatrix::PotWarnMatrix(Window* parent, const rect_t& r) :
  ButtonMatrix(parent, r)
{
  // Setup button layout & texts
  uint8_t btn_cnt = 0;
  for (uint8_t i = POT_FIRST; i <= POT_LAST; i++) {
    if ((IS_POT(i) || IS_POT_MULTIPOS(i)) && IS_POT_AVAILABLE(i)) {
      pot_idx[btn_cnt] = i;
      btn_cnt++;
    }
  }

  for (int8_t i = SLIDER_FIRST; i <= SLIDER_LAST; i++) {
    if (IS_SLIDER(i)) {
      pot_idx[btn_cnt] = i;
      btn_cnt++;
    }
  }

  initBtnMap(3, btn_cnt);

  uint8_t btn_id = 0;
  for (uint16_t i = POT_FIRST; i <= POT_LAST; i++) {
    if ((IS_POT(i) || IS_POT_MULTIPOS(i)) && IS_POT_AVAILABLE(i)) {
      setText(btn_id, STR_VSRCRAW[i + 1]);
      btn_id++;
    }
  }
  for (int8_t i = SLIDER_FIRST; i <= SLIDER_LAST; i++) {
    if (IS_SLIDER(i)) {
      setText(btn_id, STR_VSRCRAW[i + 1]);
      btn_id++;
    }
  }
  update();

  lv_obj_set_width(lvobj, (LV_DPI_DEF * 3) / 2);
  
  uint8_t rows = ((btn_cnt - 1) / 4) + 1;
  lv_obj_set_height(lvobj, (rows * LV_DPI_DEF) / 3);

  lv_obj_set_style_bg_opa(lvobj, LV_OPA_0, LV_PART_MAIN);

  lv_obj_set_style_pad_all(lvobj, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_left(lvobj, LV_DPI_DEF / 10, LV_PART_MAIN);

  lv_obj_set_style_pad_row(lvobj, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_column(lvobj, 4, LV_PART_MAIN);

  lv_obj_remove_style(lvobj, nullptr, LV_PART_MAIN | LV_STATE_FOCUSED);
  lv_obj_remove_style(lvobj, nullptr, LV_PART_MAIN | LV_STATE_EDITED);
}

void PotWarnMatrix::onPress(uint8_t btn_id)
{
  if (btn_id >= NUM_POTS + NUM_SLIDERS) return;
  auto pot = pot_idx[btn_id];
  
  g_model.potsWarnEnabled ^= (1 << pot);
  if ((g_model.potsWarnMode == POTS_WARN_MANUAL) &&
      (g_model.potsWarnEnabled & (1 << pot))) {
    SAVE_POT_POSITION(pot);
  }
  SET_DIRTY();
}

bool PotWarnMatrix::isActive(uint8_t btn_id)
{
  if (btn_id >= NUM_POTS + NUM_SLIDERS) return false;
  return (g_model.potsWarnEnabled & (1 << pot_idx[btn_id])) != 0;
}

