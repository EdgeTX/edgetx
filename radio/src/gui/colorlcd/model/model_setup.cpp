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

#include "model_setup.h"

#include "button_matrix.h"
#include "filechoice.h"
#include "libopenui.h"

#include "hal/adc_driver.h"
#include "storage/modelslist.h"
#include "trainer_setup.h"
#include "module_setup.h"
#include "timer_setup.h"
#include "trims_setup.h"
#include "throttle_params.h"
#include "preflight_checks.h"
#if defined(FUNCTION_SWITCHES)
#include "function_switches.h"
#endif
#include "throttle_params.h"
#include "timer_setup.h"
#include "trainer_setup.h"
#include "trims_setup.h"
#include "module_setup.h"
#include "edgetx.h"
#include "storage/modelslist.h"
#include "etx_lv_theme.h"

#if defined(USBJ_EX)
#include "model_usbjoystick.h"
#endif

#include <algorithm>

#define SET_DIRTY() storageDirty(EE_MODEL)

ModelSetupPage::ModelSetupPage() :
    PageTab(STR_MENU_MODEL_SETUP, ICON_MODEL_SETUP)
{
}

static void viewOption(Window* parent, coord_t x, coord_t y,
                std::function<uint8_t()> getValue,
                std::function<void(uint8_t)> setValue, bool globalState)
{
  auto lbl = new StaticText(parent, {x + ModelSetupPage::OPTS_W + PAD_MEDIUM, y + PAD_SMALL + 1, 0, 0},
                          STR_ADCFILTERVALUES[globalState ? 1 : 2], COLOR_THEME_SECONDARY1_INDEX);
  new Choice(parent, {x, y, ModelSetupPage::OPTS_W, 0}, STR_ADCFILTERVALUES, 0, 2, getValue,
              [=](int newValue) {
                setValue(newValue);
                lbl->show(newValue == 0);
              });
  lbl->show(getValue() == 0);
}

static SetupLineDef viewOptionsPageSetupLines[] = {
  {
    STR_RADIO_MENU_TABS, nullptr,
  },
  {
    STR_THEME_EDITOR,
    [](Window* parent, coord_t x, coord_t y) {
      viewOption(parent, x, y,
                GET_SET_DEFAULT(g_model.radioThemesDisabled),
                g_eeGeneral.radioThemesDisabled);
    }
  },
  {
    STR_MENUSPECIALFUNCS,
    [](Window* parent, coord_t x, coord_t y) {
      viewOption(parent, x, y,
                GET_SET_DEFAULT(g_model.radioGFDisabled),
                g_eeGeneral.radioGFDisabled);
    }
  },
  {
    STR_MENUTRAINER,
    [](Window* parent, coord_t x, coord_t y) {
      viewOption(parent, x, y,
                GET_SET_DEFAULT(g_model.radioTrainerDisabled),
                g_eeGeneral.radioTrainerDisabled);
    }
  },
  {
    STR_MODEL_MENU_TABS, nullptr,
  },
#if defined(HELI)
  {
    STR_MENUHELISETUP,
    [](Window* parent, coord_t x, coord_t y) {
      viewOption(parent, x, y,
                GET_SET_DEFAULT(g_model.modelHeliDisabled),
                g_eeGeneral.modelHeliDisabled);
    }
  },
#endif
#if defined(FLIGHT_MODES)
  {
    STR_MENUFLIGHTMODES,
    [](Window* parent, coord_t x, coord_t y) {
      viewOption(parent, x, y,
                GET_SET_DEFAULT(g_model.modelFMDisabled),
                g_eeGeneral.modelFMDisabled);
    }
  },
#endif
#if defined(GVARS)
  {
    STR_MENU_GLOBAL_VARS,
    [](Window* parent, coord_t x, coord_t y) {
      viewOption(parent, x, y,
                GET_SET_DEFAULT(g_model.modelGVDisabled),
                g_eeGeneral.modelGVDisabled);
    }
  },
#endif
  {
    STR_MENUCURVES,
    [](Window* parent, coord_t x, coord_t y) {
      viewOption(parent, x, y,
                GET_SET_DEFAULT(g_model.modelCurvesDisabled),
                g_eeGeneral.modelCurvesDisabled);
    }
  },
  {
    STR_MENULOGICALSWITCHES,
    [](Window* parent, coord_t x, coord_t y) {
      viewOption(parent, x, y,
                GET_SET_DEFAULT(g_model.modelLSDisabled),
                g_eeGeneral.modelLSDisabled);
    }
  },
  {
    STR_MENUCUSTOMFUNC,
    [](Window* parent, coord_t x, coord_t y) {
      viewOption(parent, x, y,
                GET_SET_DEFAULT(g_model.modelSFDisabled),
                g_eeGeneral.modelSFDisabled);
    }
  },
#if defined(LUA_MODEL_SCRIPTS)
  {
    STR_MENUCUSTOMSCRIPTS,
    [](Window* parent, coord_t x, coord_t y) {
      viewOption(parent, x, y,
                GET_SET_DEFAULT(g_model.modelCustomScriptsDisabled),
                g_eeGeneral.modelCustomScriptsDisabled);
    }
  },
#endif
  {
    STR_MENUTELEMETRY,
    [](Window* parent, coord_t x, coord_t y) {
      viewOption(parent, x, y,
                GET_SET_DEFAULT(g_model.modelTelemetryDisabled),
                g_eeGeneral.modelTelemetryDisabled);
    }
  },
};

struct CenterBeepsMatrix : public ButtonMatrix {
  CenterBeepsMatrix(Window* parent, const rect_t& rect) :
    ButtonMatrix(parent, rect)
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

    initBtnMap(min((int)btn_cnt, SW_BTNS), btn_cnt);

    uint8_t btn_id = 0;
    for (uint8_t i = 0; i < max_analogs; i++) {
      if (i < max_sticks || (IS_POT_SLIDER_AVAILABLE(i - max_sticks) &&
                            !IS_POT_MULTIPOS(i - max_sticks))) {
        setTextAndState(btn_id);
        btn_id++;
      }
    }

    update();

    setWidth(min((int)btn_cnt, SW_BTNS) * SW_BTN_W + 4);

    uint8_t rows = ((btn_cnt - 1) / SW_BTNS) + 1;
    setHeight((rows * 36) + 4);

    lv_obj_set_style_pad_all(lvobj, 4, LV_PART_MAIN);

    lv_obj_set_style_pad_row(lvobj, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_column(lvobj, 4, LV_PART_MAIN);
  }

  void onPress(uint8_t btn_id)
  {
    if (btn_id >= max_analogs) return;
    uint8_t i = ana_idx[btn_id];
    BFBIT_FLIP(g_model.beepANACenter, bfBit<BeepANACenter>(i));
    setTextAndState(btn_id);
    SET_DIRTY();
  }

  bool isActive(uint8_t btn_id)
  {
    if (btn_id >= max_analogs) return false;
    uint8_t i = ana_idx[btn_id];
    return bfSingleBitGet<BeepANACenter>(g_model.beepANACenter, i) != 0;
  }

  void setTextAndState(uint8_t btn_id)
  {
    auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
    if (ana_idx[btn_id] < max_sticks)
      setText(btn_id, getAnalogShortLabel(ana_idx[btn_id]));
    else
      setText(btn_id,
              getAnalogLabel(ADC_INPUT_FLEX, ana_idx[btn_id] - max_sticks));
    setChecked(btn_id);
  }

  static LAYOUT_VAL(SW_BTNS, 8, 4)
  static LAYOUT_VAL(SW_BTN_W, 56, 72)
  static LAYOUT_VAL(SW_BTN_H, 36, 36)

 private:
  uint8_t max_analogs;
  uint8_t ana_idx[MAX_ANALOG_INPUTS];
};

static SetupLineDef otherPageSetupLines[] = {
  {
    STR_JITTER_FILTER,
    [](Window* parent, coord_t x, coord_t y) {
      new Choice(parent, {x, y, 0, 0}, STR_ADCFILTERVALUES, 0, 2,
                GET_SET_DEFAULT(g_model.jitterFilter));
    }
  },
  {
    STR_BEEPCTR, [](Window* parent, coord_t x, coord_t y) {}
  },
  {
    nullptr,
    [](Window* parent, coord_t x, coord_t y) {
      auto bm = new CenterBeepsMatrix(parent, {PAD_MEDIUM, y, 0, 0});
      parent->setHeight(bm->height() + PAD_SMALL);
    }
  },
};

static SetupLineDef setupLines[] = {
  {
    // Model name
    STR_MODELNAME,
    [](Window* parent, coord_t x, coord_t y) {
      new ModelTextEdit(parent, {x, y, ModelSetupPage::NAM_W, 0},
                        g_model.header.name, sizeof(g_model.header.name),
                        [=]() {
                          auto model = modelslist.getCurrentModel();
                          if (model) {
                            model->setModelName(g_model.header.name);
                          }
                          SET_DIRTY();
                        });
    }
  },
  {
    // Model labels
    STR_LABELS,
    [](Window* parent, coord_t x, coord_t y) {
      auto curmod = modelslist.getCurrentModel();
      TextButton* btn = new TextButton(parent, {x, y, 0, 0}, modelslabels.getBulletLabelString(curmod, STR_UNLABELEDMODEL));
      btn->setPressHandler([=]() {
            Menu *menu = new Menu(true);
            menu->setTitle(STR_LABELS);
            for (auto &label : modelslabels.getLabels()) {
              menu->addLineBuffered(
                  label,
                  [=]() {
                    if (!modelslabels.isLabelSelected(label, curmod))
                      modelslabels.addLabelToModel(label, curmod);
                    else
                      modelslabels.removeLabelFromModel(label, curmod);
                    btn->setText(modelslabels.getBulletLabelString(
                        curmod, STR_UNLABELEDMODEL));
                    strncpy(g_model.header.labels,
                            ModelMap::toCSV(modelslabels.getLabelsByModel(curmod))
                                .c_str(),
                            sizeof(g_model.header.labels));
                    g_model.header.labels[sizeof(g_model.header.labels) - 1] = '\0';
                    SET_DIRTY();
                  },
                  [=]() { return modelslabels.isLabelSelected(label, curmod); });
            }
            menu->updateLines();
            return 0;
          });
    }
  },
  {
    // Model bitmap
    STR_BITMAP,
    [](Window* parent, coord_t x, coord_t y) {
      new FileChoice(parent, {x, y, 0, 0}, BITMAPS_PATH, BITMAPS_EXT, LEN_BITMAP_NAME,
                     [=]() {
                       return std::string(g_model.header.bitmap, LEN_BITMAP_NAME);
                     },
                     [=](std::string newValue) {
                       strncpy(g_model.header.bitmap, newValue.c_str(), LEN_BITMAP_NAME);
                       auto model = modelslist.getCurrentModel();
                       if (model) {
                         strncpy(model->modelBitmap, newValue.c_str(), LEN_BITMAP_NAME);
                         model->modelBitmap[LEN_BITMAP_NAME] = '\0';
                       }
                       SET_DIRTY();
                     }, false, STR_BITMAP);
    }
  },
};

void ModelSetupPage::build(Window * window)
{
  coord_t y = SetupLine::showLines(window, 0, SubPage::EDT_X, padding, setupLines, DIM(setupLines));

  new SetupButtonGroup(window, {0, y, LCD_W - padding * 2, 0}, nullptr, BTN_COLS, PAD_TINY, {
    // Modules
    {STR_INTERNALRF, []() { new ModulePage(INTERNAL_MODULE); }, []() { return g_model.moduleData[INTERNAL_MODULE].type > 0; }},
    {STR_EXTERNALRF, []() { new ModulePage(EXTERNAL_MODULE); }, []() { return g_model.moduleData[EXTERNAL_MODULE].type > 0; }},
    {STR_TRAINER, []() { new TrainerPage(); }, []() { return g_model.trainerData.mode > 0; }},
    // Timer buttons
    {TR_TIMER "1", []() { new TimerWindow(0); }, []() { return g_model.timers[0].mode > 0; }},
    {TR_TIMER "2", []() { new TimerWindow(1); }, []() { return g_model.timers[1].mode > 0; }},
    {TR_TIMER "3", []() { new TimerWindow(2); }, []() { return g_model.timers[2].mode > 0; }},

    {STR_PREFLIGHT, []() { new PreflightChecks(); }},
    {STR_TRIMS, []() { new TrimsSetup(); }},
    {STR_THROTTLE_LABEL, []() { new ThrottleParams(); }},
    {STR_ENABLED_FEATURES, []() { new SubPage(ICON_MODEL_SETUP, STR_MENU_MODEL_SETUP, STR_ENABLED_FEATURES, viewOptionsPageSetupLines, DIM(viewOptionsPageSetupLines)); }},
#if defined(USBJ_EX)
    {STR_USBJOYSTICK_LABEL, []() { new ModelUSBJoystickPage(); }},
#endif
#if defined(FUNCTION_SWITCHES)
    {STR_FUNCTION_SWITCHES, []() { new ModelFunctionSwitches(); }},
#endif
    {STR_MENU_OTHER, []() { new SubPage(ICON_MODEL_SETUP, STR_MENU_MODEL_SETUP, STR_MENU_OTHER, otherPageSetupLines, DIM(otherPageSetupLines)); }},
  }, BTN_H);
}
