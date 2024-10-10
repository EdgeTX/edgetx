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
#include "hal/adc_driver.h"
#include "hal/switch_driver.h"
#include "edgetx.h"
#include "strhelpers.h"
#include "etx_lv_theme.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

class SwitchWarnMatrix : public ButtonMatrix
{
 public:
  SwitchWarnMatrix(Window* parent, const rect_t& rect) :
      ButtonMatrix(parent, rect)
  {
    // Setup button layout & texts
    uint8_t btn_cnt = 0;
    for (uint8_t i = 0; i < MAX_SWITCHES; i++) {
      if (SWITCH_WARNING_ALLOWED(i)) {
        sw_idx[btn_cnt] = i;
        btn_cnt++;
      }
    }

    initBtnMap(min((int)btn_cnt, SW_BTNS), btn_cnt);

    uint8_t btn_id = 0;
    for (uint8_t i = 0; i < MAX_SWITCHES; i++) {
      if (SWITCH_WARNING_ALLOWED(i)) {
        setTextAndState(btn_id);
        btn_id++;
      }
    }

    update();

    lv_obj_set_width(lvobj, min((int)btn_cnt, SW_BTNS) * SW_BTN_W + PAD_SMALL);

    uint8_t rows = ((btn_cnt - 1) / SW_BTNS) + 1;
    setHeight((rows * SW_BTN_H) + PAD_SMALL);

    padAll(PAD_SMALL);
  }

  void onPress(uint8_t btn_id)
  {
    if (btn_id >= MAX_SWITCHES) return;
    auto sw = sw_idx[btn_id];

    swarnstate_t newstate = bfGet(g_model.switchWarning, 3 * sw, 3);
    if (newstate == 1 && SWITCH_CONFIG(sw) != SWITCH_3POS)
      newstate = 3;
    else
      newstate = (newstate + 1) % 4;

    g_model.switchWarning =
        bfSet(g_model.switchWarning, newstate, 3 * sw, 3);
    SET_DIRTY();

    setTextAndState(btn_id);
  }

  bool isActive(uint8_t btn_id)
  {
    if (btn_id >= MAX_SWITCHES) return false;
    return bfGet(g_model.switchWarning, 3 * sw_idx[btn_id], 3) != 0;
  }

  void setTextAndState(uint8_t btn_id)
  {
    swsrc_t index = sw_idx[btn_id];
    auto warn_pos = g_model.switchWarning >> (3 * index) & 0x07;
    std::string s = std::string(switchGetName(index)) +
                    std::string(getSwitchWarnSymbol(warn_pos));
    setText(btn_id, s.c_str());
    setChecked(btn_id);
  }

  static LAYOUT_VAL(SW_BTNS, 8, 4)
  static LAYOUT_VAL(SW_BTN_W, 56, 72)
  static LAYOUT_VAL(SW_BTN_H, 36, 36)

 private:
  uint8_t sw_idx[MAX_SWITCHES];
};

class PotWarnMatrix : public ButtonMatrix
{
 public:
  PotWarnMatrix(Window* parent, const rect_t& rect) :
    ButtonMatrix(parent, rect)
  {
    // Setup button layout & texts
    uint8_t btn_cnt = 0;
    for (uint8_t i = 0; i < MAX_POTS; i++) {
      if (IS_POT_AVAILABLE(i)) {
        pot_idx[btn_cnt] = i;
        btn_cnt++;
      }
    }

    initBtnMap(min((int)btn_cnt, SwitchWarnMatrix::SW_BTNS), btn_cnt);

    uint8_t btn_id = 0;
    for (uint16_t i = 0; i < MAX_POTS; i++) {
      if (IS_POT_AVAILABLE(i)) {
        setTextAndState(btn_id);
        btn_id++;
      }
    }

    update();

    lv_obj_set_width(lvobj, min((int)btn_cnt, SwitchWarnMatrix::SW_BTNS) * SwitchWarnMatrix::SW_BTN_W + PAD_SMALL);

    uint8_t rows = ((btn_cnt - 1) / SwitchWarnMatrix::SW_BTNS) + 1;
    setHeight((rows * SwitchWarnMatrix::SW_BTN_H) + PAD_SMALL);

    padAll(PAD_SMALL);
  }

  void onPress(uint8_t btn_id)
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

  bool isActive(uint8_t btn_id)
  {
    if (btn_id >= MAX_POTS) return false;
    return (g_model.potsWarnEnabled & (1 << pot_idx[btn_id])) != 0;
  }

  void setTextAndState(uint8_t btn_id)
  {
    setText(btn_id, getPotLabel(pot_idx[btn_id]));
    setChecked(btn_id);
  }

 private:
  uint8_t pot_idx[MAX_POTS];
};

PreflightChecks::PreflightChecks() : SubPage(ICON_MODEL_SETUP, STR_MENU_MODEL_SETUP, STR_PREFLIGHT)
{
  body->setFlexLayout();

  // Display checklist
  setupLine(STR_CHECKLIST,
    [=](Window* parent, coord_t x, coord_t y) {
      new ToggleSwitch(parent, {x, y, 0, 0},
                        GET_DEFAULT(g_model.displayChecklist),
                        [=](uint8_t newValue) {
                          g_model.displayChecklist = newValue;
                          SET_DIRTY();
                          interactive->enable(g_model.displayChecklist);
                        });
    });

  // Interactive checklist
  setupLine(STR_CHECKLIST_INTERACTIVE,
    [=](Window* parent, coord_t x, coord_t y) {
      interactive = new ToggleSwitch(parent, {x, y, 0, 0}, GET_SET_DEFAULT(g_model.checklistInteractive));
      interactive->enable(g_model.displayChecklist);
    });

  // Throttle warning
  setupLine(STR_THROTTLE_WARNING,
    [=](Window* parent, coord_t x, coord_t y) {
      new ToggleSwitch(parent, {x, y, 0, 0},
                      GET_INVERTED(g_model.disableThrottleWarning),
                      [=](uint8_t newValue) {
                        g_model.disableThrottleWarning = !newValue;
                        SET_DIRTY();
                        customThrottle->show(!g_model.disableThrottleWarning);
                      });
    });

  // Custom Throttle warning (conditional on previous field)
  customThrottle = setupLine(STR_CUSTOM_THROTTLE_WARNING,
    [=](Window* parent, coord_t x, coord_t y) {
      new ToggleSwitch(parent, {x, y, 0, 0}, GET_DEFAULT(g_model.enableCustomThrottleWarning),
                      [=](uint8_t newValue) {
                        g_model.enableCustomThrottleWarning = newValue;
                        SET_DIRTY();
                        customThrottleValue->show(g_model.enableCustomThrottleWarning);
                      });

      // Custom Throttle warning value
      customThrottleValue = new NumberEdit(parent, {x + ToggleSwitch::TOGGLE_W + PAD_SMALL, 0, 0}, -100, 100,
                                          GET_SET_DEFAULT(g_model.customThrottleWarningPosition));
      customThrottleValue->show(g_model.enableCustomThrottleWarning);
    });

  // Switch warnings (TODO: add display switch?)
  setupLine(STR_SWITCHES, [](Window*, coord_t, coord_t){});
  setupLine(nullptr,
    [=](Window* parent, coord_t x, coord_t y) {
      auto w = new SwitchWarnMatrix(parent, rect_t{PAD_SMALL, y, 0, 0});
      parent->setHeight(w->height() + PAD_TINY * 2);
    });

  // Pots and sliders warning
  if (adcGetMaxInputs(ADC_INPUT_FLEX) > 0) {
    uint8_t pot_cnt = 0;
    for (uint8_t i = 0; i < MAX_POTS; i++) {
      if (IS_POT_AVAILABLE(i)) {
        pot_cnt++;
      }
    }
    if (pot_cnt > 0) {
      setupLine(STR_POTWARNINGSTATE,
        [=](Window* parent, coord_t x, coord_t y) {
          new Choice(parent, {x, y, 0, 0}, STR_PREFLIGHT_POTSLIDER_CHECK,
                      0, 2, GET_DEFAULT(g_model.potsWarnMode),
                      [=](int newValue) {
                        g_model.potsWarnMode = newValue;
                        SET_DIRTY();
                        potsWarnMatrix->show(g_model.potsWarnMode > 0);
                      });
        });

      // Pot warnings
      potsWarnMatrix = setupLine(nullptr,
        [=](Window* parent, coord_t x, coord_t y) {
          auto w = new PotWarnMatrix(parent, {PAD_SMALL, y, 0, 0});
          parent->setHeight(w->height() + PAD_TINY * 2);
        });
      potsWarnMatrix->show(g_model.potsWarnMode > 0);
    }
  }
}
