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
#include "throttle_params.h"
#include "timer_setup.h"
#include "trainer_setup.h"
#include "trims_setup.h"
#include "module_setup.h"
#include "opentx.h"
#include "storage/modelslist.h"
#include "themes/etx_lv_theme.h"

#if defined(USBJ_EX)
#include "model_usbjoystick.h"
#endif

#include <algorithm>

#define SET_DIRTY() storageDirty(EE_MODEL)

ModelSetupPage::ModelSetupPage() :
    PageTab(STR_MENU_MODEL_SETUP, ICON_MODEL_SETUP)
{
}

static void onModelNameChanged()
{
  auto model = modelslist.getCurrentModel();
  if (model) {
    model->setModelName(g_model.header.name);
  }
  SET_DIRTY();
}

struct ModelNameEdit : public ModelTextEdit {
  ModelNameEdit(Window *parent, const rect_t &rect) :
      ModelTextEdit(parent, rect, g_model.header.name,
                    sizeof(g_model.header.name))
  {
    setChangeHandler(onModelNameChanged);
  }
};

static std::string getModelBitmap()
{
  return std::string(g_model.header.bitmap, LEN_BITMAP_NAME);
}

static void setModelBitmap(std::string newValue)
{
  strncpy(g_model.header.bitmap, newValue.c_str(), LEN_BITMAP_NAME);
  auto model = modelslist.getCurrentModel();
  if (model) {
    strncpy(model->modelBitmap, newValue.c_str(), LEN_BITMAP_NAME);
    model->modelBitmap[LEN_BITMAP_NAME] = '\0';
  }
  SET_DIRTY();
}

struct ModelBitmapEdit : public FileChoice {
  ModelBitmapEdit(Window *parent, const rect_t &rect) :
      FileChoice(parent, rect, BITMAPS_PATH, BITMAPS_EXT, LEN_BITMAP_NAME,
                 getModelBitmap, setModelBitmap)
  {
  }
};

class SubScreenButton : public TextButton
{
 public:
  SubScreenButton(Window *parent, const char *text,
                  std::function<void(void)> pressHandler,
                  std::function<bool(void)> checkActive = nullptr) :
      TextButton(parent, rect_t{}, text,
                 [=]() -> uint8_t {
                   pressHandler();
                   return 0;
                 }),
      m_isActive(std::move(checkActive))
  {
    // Room for two lines of text
    setHeight(62);
    setWidth((LCD_W - 30) / 3);

    lv_obj_set_width(label, lv_pct(100));
    etx_obj_add_style(label, styles->text_align_center, LV_PART_MAIN);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    setCheckHandler([=]() { check(isActive()); });
    check(isActive());
  }

 protected:
  std::function<bool(void)> m_isActive = nullptr;

  virtual bool isActive() { return m_isActive ? m_isActive() : false; }
};

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

class ModelViewOptions : public Page
{
 public:
  class OptChoice : Window
  {
   public:
    OptChoice(Window *parent, const char *const values[], int vmin, int vmax,
              std::function<int()> _getValue,
              std::function<void(int)> _setValue, bool globalState) :
        Window(parent, rect_t{}),
        m_getValue(std::move(_getValue)),
        m_setValue(std::move(_setValue))
    {
      padAll(PAD_TINY);
      setFlexLayout(LV_FLEX_FLOW_ROW, PAD_SMALL);
      lv_obj_set_flex_align(lvobj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                            LV_FLEX_ALIGN_SPACE_AROUND);

      new Choice(this, rect_t{}, values, vmin, vmax, m_getValue,
                 [=](int newValue) {
                   m_setValue(newValue);
                   setState();
                 });
      m_lbl = new StaticText(this, rect_t{},
                             STR_ADCFILTERVALUES[globalState ? 1 : 2], COLOR_THEME_SECONDARY1);
      setState();
    }

   protected:
    StaticText *m_lbl;
    std::function<int()> m_getValue;
    std::function<void(int)> m_setValue;

    void setState()
    {
      m_lbl->show(m_getValue() == 0);
    }
  };

  FormLine* optLine(FlexGridLayout& grid)
  {
    auto line = body->newLine(grid);
    line->padAll(PAD_ZERO);
    line->padLeft(10);
    return line;
  }

  ModelViewOptions() : Page(ICON_MODEL_SETUP)
  {
    header->setTitle(STR_MENU_MODEL_SETUP);
    header->setTitle2(STR_ENABLED_FEATURES);

    body->padAll(PAD_TINY);
    body->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

    FlexGridLayout grid(line_col_dsc, line_row_dsc, PAD_TINY);

    auto line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_RADIO_MENU_TABS);

    line = optLine(grid);
    new StaticText(line, rect_t{}, STR_THEME_EDITOR);
    new OptChoice(line, STR_ADCFILTERVALUES, 0, 2,
                  GET_SET_DEFAULT(g_model.radioThemesDisabled),
                  g_eeGeneral.radioThemesDisabled);

    line = optLine(grid);
    new StaticText(line, rect_t{}, STR_MENUSPECIALFUNCS);
    new OptChoice(line, STR_ADCFILTERVALUES, 0, 2,
                  GET_SET_DEFAULT(g_model.radioGFDisabled),
                  g_eeGeneral.radioGFDisabled);

    line = optLine(grid);
    new StaticText(line, rect_t{}, STR_MENUTRAINER);
    new OptChoice(line, STR_ADCFILTERVALUES, 0, 2,
                  GET_SET_DEFAULT(g_model.radioTrainerDisabled),
                  g_eeGeneral.radioTrainerDisabled);

    line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_MODEL_MENU_TABS);

#if defined(HELI)
    line = optLine(grid);
    new StaticText(line, rect_t{}, STR_MENUHELISETUP);
    new OptChoice(line, STR_ADCFILTERVALUES, 0, 2,
                  GET_SET_DEFAULT(g_model.modelHeliDisabled),
                  g_eeGeneral.modelHeliDisabled);
#endif

#if defined(FLIGHT_MODES)
    line = optLine(grid);
    new StaticText(line, rect_t{}, STR_MENUFLIGHTMODES);
    new OptChoice(line, STR_ADCFILTERVALUES, 0, 2,
                  GET_SET_DEFAULT(g_model.modelFMDisabled),
                  g_eeGeneral.modelFMDisabled);
#endif

#if defined(GVARS)
    line = optLine(grid);
    new StaticText(line, rect_t{}, STR_MENU_GLOBAL_VARS);
    new OptChoice(line, STR_ADCFILTERVALUES, 0, 2,
                  GET_SET_DEFAULT(g_model.modelGVDisabled),
                  g_eeGeneral.modelGVDisabled);
#endif

    line = optLine(grid);
    new StaticText(line, rect_t{}, STR_MENUCURVES);
    new OptChoice(line, STR_ADCFILTERVALUES, 0, 2,
                  GET_SET_DEFAULT(g_model.modelCurvesDisabled),
                  g_eeGeneral.modelCurvesDisabled);

    line = optLine(grid);
    new StaticText(line, rect_t{}, STR_MENULOGICALSWITCHES);
    new OptChoice(line, STR_ADCFILTERVALUES, 0, 2,
                  GET_SET_DEFAULT(g_model.modelLSDisabled),
                  g_eeGeneral.modelLSDisabled);

    line = optLine(grid);
    new StaticText(line, rect_t{}, STR_MENUCUSTOMFUNC);
    new OptChoice(line, STR_ADCFILTERVALUES, 0, 2,
                  GET_SET_DEFAULT(g_model.modelSFDisabled),
                  g_eeGeneral.modelSFDisabled);

#if defined(LUA_MODEL_SCRIPTS)
    line = optLine(grid);
    new StaticText(line, rect_t{}, STR_MENUCUSTOMSCRIPTS);
    new OptChoice(line, STR_ADCFILTERVALUES, 0, 2,
                  GET_SET_DEFAULT(g_model.modelCustomScriptsDisabled),
                  g_eeGeneral.modelCustomScriptsDisabled);
#endif

    line = optLine(grid);
    new StaticText(line, rect_t{}, STR_MENUTELEMETRY);
    new OptChoice(line, STR_ADCFILTERVALUES, 0, 2,
                  GET_SET_DEFAULT(g_model.modelTelemetryDisabled),
                  g_eeGeneral.modelTelemetryDisabled);
  }
};

#if LCD_W > LCD_H
#define SW_BTNS 8
#define SW_BTN_W 56
#else
#define SW_BTNS 4
#define SW_BTN_W 72
#endif

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

    lv_obj_set_width(lvobj, min((int)btn_cnt, SW_BTNS) * SW_BTN_W + 4);

    uint8_t rows = ((btn_cnt - 1) / SW_BTNS) + 1;
    lv_obj_set_height(lvobj, (rows * 36) + 4);

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

 private:
  uint8_t max_analogs;
  uint8_t ana_idx[MAX_ANALOG_INPUTS];
};

class ModelOtherOptions : public Page
{
   public:
    ModelOtherOptions() : Page(ICON_MODEL_SETUP)
    {
      header->setTitle(STR_MENU_MODEL_SETUP);
      header->setTitle2(STR_MENU_OTHER);

      body->padAll(PAD_SMALL);

      body->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

      FlexGridLayout grid(line_col_dsc, line_row_dsc, PAD_SMALL);

      // Model ADC jitter filter
      auto line = body->newLine(grid);
      new StaticText(line, rect_t{}, STR_JITTER_FILTER);
      new Choice(line, rect_t{}, STR_ADCFILTERVALUES, 0, 2,
                GET_SET_DEFAULT(g_model.jitterFilter));

      // Center beeps
      line = body->newLine(grid);
      new StaticText(line, rect_t{}, STR_BEEPCTR);
      line = body->newLine(grid);
      line->padLeft(4);
      new CenterBeepsMatrix(line, rect_t{});
    }
};

void ModelSetupPage::build(Window * window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, 0);

  FlexGridLayout grid(line_col_dsc, line_row_dsc, PAD_TINY);

  // Model name
  auto line = window->newLine(grid);
  new StaticText(line, rect_t{}, STR_MODELNAME);
  new ModelNameEdit(line, {0, 0, LCD_W / 2 - 20, 0});

  // Model labels
  line = window->newLine(grid);
  new StaticText(line, rect_t{}, STR_LABELS);
  auto curmod = modelslist.getCurrentModel();
  labelTextButton = new TextButton(
      line, rect_t{},
      modelslabels.getBulletLabelString(curmod, STR_UNLABELEDMODEL), [=]() {
        Menu *menu = new Menu(window, true);
        menu->setTitle(STR_LABELS);
        for (auto &label : modelslabels.getLabels()) {
          menu->addLineBuffered(
              label,
              [=]() {
                if (!modelslabels.isLabelSelected(label, curmod))
                  modelslabels.addLabelToModel(label, curmod);
                else
                  modelslabels.removeLabelFromModel(label, curmod);
                labelTextButton->setText(modelslabels.getBulletLabelString(
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

  // Bitmap
  line = window->newLine(grid);
  new StaticText(line, rect_t{}, STR_BITMAP);
  // TODO: show bitmap thumbnail instead?
  new ModelBitmapEdit(line, rect_t{});

  static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

  FlexGridLayout grid2(col_dsc, row_dsc);

  line = window->newLine(grid2);
  line->padTop(8);

  // Modules
  new SubScreenButton(
      line, STR_INTERNALRF, []() { new ModulePage(INTERNAL_MODULE); },
      []() { return g_model.moduleData[INTERNAL_MODULE].type > 0; });
  new SubScreenButton(
      line, STR_EXTERNALRF, []() { new ModulePage(EXTERNAL_MODULE); },
      []() { return g_model.moduleData[EXTERNAL_MODULE].type > 0; });
  new SubScreenButton(
      line, STR_TRAINER, []() { new TrainerPage(); },
      []() { return g_model.trainerData.mode > 0; });

  line = window->newLine(grid2);
  line->padTop(2);

  // Timer buttons
  new SubScreenButton(
      line, TR_TIMER "1", []() { new TimerWindow(0); },
      []() { return g_model.timers[0].mode > 0; });
  new SubScreenButton(
      line, TR_TIMER "2", []() { new TimerWindow(1); },
      []() { return g_model.timers[1].mode > 0; });
  new SubScreenButton(
      line, TR_TIMER "3", []() { new TimerWindow(2); },
      []() { return g_model.timers[2].mode > 0; });

  line = window->newLine(grid2);
  line->padTop(2);

  new SubScreenButton(line, STR_PREFLIGHT, []() { new PreflightChecks(); });
  new SubScreenButton(line, STR_TRIMS, []() { new TrimsSetup(); });
  new SubScreenButton(line, STR_THROTTLE_LABEL, []() { new ThrottleParams(); });

  line = window->newLine(grid2);
  line->padTop(2);

  new SubScreenButton(line, STR_ENABLED_FEATURES,
                      []() { new ModelViewOptions(); });

#if defined(USBJ_EX)
  new SubScreenButton(line, STR_USBJOYSTICK_LABEL,
                      []() { new ModelUSBJoystickPage(); });
#endif

  new SubScreenButton(line, STR_MENU_OTHER, []() { new ModelOtherOptions(); });
}
