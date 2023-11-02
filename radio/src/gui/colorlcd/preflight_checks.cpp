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

#include "hal/adc_driver.h"
#include "hal/switch_driver.h"
#include "strhelpers.h"

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

static void make_conditional(Window* w, ToggleSwitch* cb)
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
  void setTextAndState(uint8_t btn_id);
private:
  uint8_t sw_idx[MAX_SWITCHES];
};

struct PotWarnMatrix : public ButtonMatrix {
  PotWarnMatrix(Window* parent, const rect_t& rect);
  void onPress(uint8_t btn_id);
  bool isActive(uint8_t btn_id);
  void setTextAndState(uint8_t btn_id);
private:
  uint8_t pot_idx[MAX_POTS];
};

struct CenterBeepsMatrix : public ButtonMatrix {
  CenterBeepsMatrix(Window* parent, const rect_t& rect);
  void onPress(uint8_t btn_id);
  bool isActive(uint8_t btn_id);
  void setTextAndState(uint8_t btn_id);
private:
  uint8_t max_analogs;
  uint8_t ana_idx[MAX_ANALOG_INPUTS];
};

PreflightChecks::PreflightChecks() : Page(ICON_MODEL_SETUP)
{
  header.setTitle(STR_MENU_MODEL_SETUP);
  header.setTitle2(STR_PREFLIGHT);

  auto form = new FormWindow(&body, rect_t{});
  form->setFlexLayout();
  form->padAll(8);
  FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

  // Display checklist
  auto line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_CHECKLIST, 0, COLOR_THEME_PRIMARY1);
  auto chkList = new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(g_model.displayChecklist));

  // Interactive checklist
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_CHECKLIST_INTERACTIVE, 0, COLOR_THEME_PRIMARY1);
  auto interactiveChkList = new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(g_model.checklistInteractive));
  if(!chkList->getValue())
    interactiveChkList->disable();
  chkList->setSetValueHandler([=](int32_t newValue) {
    g_model.displayChecklist = newValue; SET_DIRTY();
    (g_model.displayChecklist)?interactiveChkList->enable():interactiveChkList->disable();
  });

  // Throttle warning
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_THROTTLE_WARNING, 0, COLOR_THEME_PRIMARY1);
  auto tw = new ToggleSwitch(line, rect_t{}, GET_SET_INVERTED(g_model.disableThrottleWarning));

  // Custom Throttle warning (conditional on previous field)
  line = form->newLine(&grid);
  make_conditional(line, tw);

  new StaticText(line, rect_t{}, STR_CUSTOM_THROTTLE_WARNING, 0, COLOR_THEME_PRIMARY1);
  auto box = new FormWindow::Line(line, window_create(line->getLvObj()));
  lv_obj_set_layout(box->getLvObj(), LV_LAYOUT_FLEX);
  box->setWidth(LCD_W /2 - 15);

  auto cst_tw = new ToggleSwitch(
      box, rect_t{}, GET_SET_DEFAULT(g_model.enableCustomThrottleWarning));

  // Custom Throttle warning value
  auto cst_val =
      new NumberEdit(box, rect_t{}, -100, 100,
                     GET_SET_DEFAULT(g_model.customThrottleWarningPosition));
  make_conditional(cst_val, cst_tw);

  // Switch warnings (TODO: add display switch?)
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_SWITCHES, 0, COLOR_THEME_PRIMARY1);
  line = form->newLine(&grid);
  line->padTop(0);
  line->padLeft(4);
  new SwitchWarnMatrix(line, rect_t{});

  // Pots and sliders warning
  if (adcGetMaxInputs(ADC_INPUT_FLEX) > 0) {
    line = form->newLine(&grid);
    new StaticText(line, rect_t{}, STR_POTWARNINGSTATE, 0, COLOR_THEME_PRIMARY1);
    auto pots_wm = new Choice(line, rect_t{}, STR_PREFLIGHT_POTSLIDER_CHECK, 0, 2,
                              GET_SET_DEFAULT(g_model.potsWarnMode));

    // Pot warnings
    line = form->newLine(&grid);
    line->padTop(0);
    line->padLeft(4);
    auto pwm = new PotWarnMatrix(line, rect_t{});
    make_conditional(pwm, pots_wm);
  }

  // Center beeps
  line = form->newLine(&grid);
  line->padTop(0);
  new StaticText(line, rect_t{}, STR_BEEPCTR, 0, COLOR_THEME_PRIMARY1);
  line = form->newLine(&grid);
  line->padLeft(4);
  new CenterBeepsMatrix(line, rect_t{});
}

static std::string switchWarninglabel(swsrc_t index)
{
  auto warn_pos = g_model.switchWarningState >> (3 * index) & 0x07;
  return std::string(switchGetName(index)) +
         std::string(getSwitchWarnSymbol(warn_pos));
}

#if LCD_W > LCD_H
#define SW_BTNS 8
#else
#define SW_BTNS 4
#endif
#define SW_BTN_W ((LCD_W-24)/SW_BTNS)

SwitchWarnMatrix::SwitchWarnMatrix(Window* parent, const rect_t& r) :
  ButtonMatrix(parent, r)
{
  // Setup button layout & texts
  uint8_t btn_cnt = 0;
  for (uint8_t i = 0; i < MAX_SWITCHES; i++) {
    if (SWITCH_EXISTS(i)) {
      sw_idx[btn_cnt] = i;
      btn_cnt++;
    }    
  }

  initBtnMap(SW_BTNS, btn_cnt);
  update();

  uint8_t btn_id = 0;
  for (uint8_t i = 0; i < MAX_SWITCHES; i++) {
    if (SWITCH_EXISTS(i)) {
      setTextAndState(btn_id);
      btn_id++;
    }
  }

  lv_obj_set_width(lvobj, min((int)btn_cnt, SW_BTNS) * SW_BTN_W);

  uint8_t rows = ((btn_cnt - 1) / SW_BTNS) + 1;
  lv_obj_set_height(lvobj, (rows * LV_DPI_DEF) / 3);

  lv_obj_set_style_pad_all(lvobj, 4, LV_PART_MAIN);

  lv_obj_set_style_pad_row(lvobj, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_column(lvobj, 4, LV_PART_MAIN);
}

void SwitchWarnMatrix::setTextAndState(uint8_t btn_id)
{
  setText(btn_id, switchWarninglabel(sw_idx[btn_id]).c_str());
  setChecked(btn_id);
}

void SwitchWarnMatrix::onPress(uint8_t btn_id)
{
  if (btn_id >= MAX_SWITCHES) return;
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
  setTextAndState(sw);
}

bool SwitchWarnMatrix::isActive(uint8_t btn_id)
{
  if (btn_id >= MAX_SWITCHES) return false;
  return bfGet(g_model.switchWarningState, 3 * sw_idx[btn_id], 3) != 0;
}

PotWarnMatrix::PotWarnMatrix(Window* parent, const rect_t& r) :
  ButtonMatrix(parent, r)
{
  // Setup button layout & texts
  uint8_t btn_cnt = 0;
  for (uint8_t i = 0; i < MAX_POTS; i++) {
    if (IS_POT_AVAILABLE(i)) {
      pot_idx[btn_cnt] = i;
      btn_cnt++;
    }
  }

  initBtnMap(SW_BTNS, btn_cnt);
  update();

  uint8_t btn_id = 0;
  for (uint16_t i = 0; i < MAX_POTS; i++) {
    if (IS_POT_AVAILABLE(i)) {
      setTextAndState(btn_id);
      btn_id++;
    }
  }

  lv_obj_set_width(lvobj, min((int)btn_cnt, SW_BTNS) * SW_BTN_W);
  
  uint8_t rows = ((btn_cnt - 1) / SW_BTNS) + 1;
  lv_obj_set_height(lvobj, (rows * LV_DPI_DEF) / 3);

  lv_obj_set_style_pad_all(lvobj, 4, LV_PART_MAIN);

  lv_obj_set_style_pad_row(lvobj, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_column(lvobj, 4, LV_PART_MAIN);
}

void PotWarnMatrix::setTextAndState(uint8_t btn_id)
{
  setText(btn_id, getPotLabel(pot_idx[btn_id]));
  setChecked(btn_id);
}

void PotWarnMatrix::onPress(uint8_t btn_id)
{
  if (btn_id >= MAX_POTS) return;
  auto pot = pot_idx[btn_id];
  
  g_model.potsWarnEnabled ^= (1 << pot);
  if ((g_model.potsWarnMode == POTS_WARN_MANUAL) &&
      (g_model.potsWarnEnabled & (1 << pot))) {
    SAVE_POT_POSITION(pot);
  }
  setTextAndState(btn_id);
  SET_DIRTY();
}

bool PotWarnMatrix::isActive(uint8_t btn_id)
{
  if (btn_id >= MAX_POTS) return false;
  return (g_model.potsWarnEnabled & (1 << pot_idx[btn_id])) != 0;
}

CenterBeepsMatrix::CenterBeepsMatrix(Window* parent, const rect_t& r) :
  ButtonMatrix(parent, r)
{
  // Setup button layout & texts
  uint8_t btn_cnt = 0;

  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  auto max_pots = adcGetMaxInputs(ADC_INPUT_FLEX);
  max_analogs = max_sticks + max_pots;

  for (uint8_t i = 0; i < max_analogs; i++) {
    // multipos cannot be centered
    if (i < max_sticks || (IS_POT_SLIDER_AVAILABLE(i - max_sticks) &&
                           !IS_POT_MULTIPOS(i - max_sticks))) {
      ana_idx[btn_cnt] = i;
      btn_cnt++;
    }
  }

  initBtnMap(SW_BTNS, btn_cnt);
  update();

  uint8_t btn_id = 0;
  for (uint8_t i = 0; i < max_analogs; i++) {
    if (i < max_sticks || (IS_POT_SLIDER_AVAILABLE(i - max_sticks) &&
                           !IS_POT_MULTIPOS(i - max_sticks))) {
      setTextAndState(btn_id);
      btn_id++;
    }
  }

  lv_obj_set_width(lvobj, min((int)btn_cnt, SW_BTNS) * SW_BTN_W);
  
  uint8_t rows = ((btn_cnt - 1) / SW_BTNS) + 1;
  lv_obj_set_height(lvobj, (rows * LV_DPI_DEF) / 3);

  lv_obj_set_style_pad_all(lvobj, 4, LV_PART_MAIN);

  lv_obj_set_style_pad_row(lvobj, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_column(lvobj, 4, LV_PART_MAIN);
}

void CenterBeepsMatrix::setTextAndState(uint8_t btn_id)
{
  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  if (ana_idx[btn_id] < max_sticks)
    setText(btn_id, getAnalogShortLabel(ana_idx[btn_id]));
  else
    setText(btn_id, getAnalogLabel(ADC_INPUT_FLEX, ana_idx[btn_id] - max_sticks));
  setChecked(btn_id);
}

void CenterBeepsMatrix::onPress(uint8_t btn_id)
{
  if (btn_id >= max_analogs) return;
  uint8_t i = ana_idx[btn_id];
  BFBIT_FLIP(g_model.beepANACenter, bfBit<BeepANACenter>(i));
  setTextAndState(btn_id);
  SET_DIRTY();  
}

bool CenterBeepsMatrix::isActive(uint8_t btn_id)
{
  if (btn_id >= max_analogs) return false;
  uint8_t i = ana_idx[btn_id];
  return bfSingleBitGet<BeepANACenter>(g_model.beepANACenter, i) != 0;
}
