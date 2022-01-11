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

#include "opentx.h"
#include "libopenui.h"
#include "button_matrix.h"

#include "storage/modelslist.h"
#include "trainer_setup.h"
#include "module_setup.h"
#include "timer_setup.h"
#include "trims_setup.h"
#include "throttle_params.h"
#include "preflight_checks.h"

#if defined(USBJ_EX)
#include "model_usbjoystick.h"
#endif

#include <algorithm>

#define SET_DIRTY()     storageDirty(EE_MODEL)

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
                    sizeof(g_model.header.name), 0)
  {
    setChangeHandler(onModelNameChanged);
  }
};

static std::string getModelBitmap()
{
  return std::string(g_model.header.bitmap, sizeof(g_model.header.bitmap));
}

static void setModelBitmap(std::string newValue)
{
  strncpy(g_model.header.bitmap, newValue.c_str(),
          sizeof(g_model.header.bitmap));
  g_model.header.bitmap[sizeof(g_model.header.bitmap)-1] = '\0';
  auto model = modelslist.getCurrentModel();
  if (model) {
    strncpy(model->modelBitmap, newValue.c_str(),
            sizeof(ModelCell::modelBitmap));
    model->modelBitmap[sizeof(ModelCell::modelBitmap)-1] = '\0';
  }
  SET_DIRTY();
}

struct ModelBitmapEdit : public FileChoice {
  ModelBitmapEdit(Window *parent, const rect_t &rect) :
      FileChoice(parent, rect, BITMAPS_PATH, BITMAPS_EXT,
                 sizeof(g_model.header.bitmap), getModelBitmap, setModelBitmap)
  {
  }
};

class SubScreenButton : public TextButton
{
 public:
  SubScreenButton(Window* parent, const char* text,
                  std::function<void(void)> pressHandler);

 protected:
  virtual bool isActive() { return false; }
};

SubScreenButton::SubScreenButton(Window* parent, const char* text,
                                 std::function<void(void)> pressHandler) :
  TextButton(parent, rect_t{}, text, [=]() -> uint8_t {
      pressHandler();
      return 0;
    })
{
  // Room for two lines of text
  setHeight(62);
  setWidth((LCD_W - 30) / 3);

  lv_obj_set_width(label, lv_pct(100));
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
}

struct IntmoduleButton : public SubScreenButton {
  IntmoduleButton(Window* parent) :
      SubScreenButton(parent, STR_INTERNALRF,
                      []() { new ModulePage(INTERNAL_MODULE); })
  {
  }
  bool isActive() override
  {
    return g_model.moduleData[INTERNAL_MODULE].type > 0;
  }
};

struct ExtmoduleButton : public SubScreenButton {
  ExtmoduleButton(Window* parent) :
      SubScreenButton(parent, STR_EXTERNALRF,
                      []() { new ModulePage(EXTERNAL_MODULE); })
  {
  }
  bool isActive() override
  {
    return g_model.moduleData[EXTERNAL_MODULE].type > 0;
  }
};

struct TrainerModuleButton : public SubScreenButton {
  TrainerModuleButton(Window* parent) :
      SubScreenButton(parent, STR_TRAINER, []() { new TrainerPage(); })
  {
  }
  bool isActive() override { return g_model.trainerData.mode > 0; }
};

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

class ModelViewOptions : public Page
{
   public:
    class OptChoice : FormWindow
    {
      public:
        OptChoice(Window* parent, const char *const values[], int vmin, int vmax,
                  std::function<int()> _getValue,
                  std::function<void(int)> _setValue,
                  bool globalState) :
          FormWindow(parent, rect_t{}),
          m_getValue(std::move(_getValue)),
          m_setValue(std::move(_setValue))
        {
          setFlexLayout(LV_FLEX_FLOW_ROW, 4);
          lv_obj_set_flex_align(lvobj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

          new Choice(this, rect_t{}, values, vmin, vmax,
                     m_getValue,
                     [=](int newValue) {
                       m_setValue(newValue);
                       setState();
                     });
          m_lbl = new StaticText(this, rect_t{}, STR_ADCFILTERVALUES[globalState ? 1: 2], 0, COLOR_THEME_SECONDARY1);
          setState();
        }

      protected:
        StaticText* m_lbl;
        std::function<int()> m_getValue;
        std::function<void(int)> m_setValue;

        void setState()
        {
          if (m_getValue() == 0) {
            lv_obj_clear_flag(m_lbl->getLvObj(), LV_OBJ_FLAG_HIDDEN);
          } else {
            lv_obj_add_flag(m_lbl->getLvObj(), LV_OBJ_FLAG_HIDDEN);
          }
        }
    };

    ModelViewOptions() : Page(ICON_MODEL_SETUP)
    {
      header.setTitle(STR_MENU_MODEL_SETUP);
      header.setTitle2(STR_ENABLED_FEATURES);

      body.padAll(8);

      auto form = new FormWindow(&body, rect_t{});
      form->setFlexLayout();
      form->padAll(4);

      FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

      auto line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_RADIO_MENU_TABS, 0, COLOR_THEME_PRIMARY1);

      line = form->newLine(&grid);
      line->padLeft(10);
      new StaticText(line, rect_t{}, STR_THEME_EDITOR, 0, COLOR_THEME_PRIMARY1);
      new OptChoice(line, STR_ADCFILTERVALUES, 0, 2, GET_SET_DEFAULT(g_model.radioThemesDisabled), g_eeGeneral.radioThemesDisabled);

      line = form->newLine(&grid);
      line->padLeft(10);
      new StaticText(line, rect_t{}, STR_MENUSPECIALFUNCS, 0, COLOR_THEME_PRIMARY1);
      new OptChoice(line, STR_ADCFILTERVALUES, 0, 2, GET_SET_DEFAULT(g_model.radioGFDisabled), g_eeGeneral.radioGFDisabled);

      line = form->newLine(&grid);
      line->padLeft(10);
      new StaticText(line, rect_t{}, STR_MENUTRAINER, 0, COLOR_THEME_PRIMARY1);
      new OptChoice(line, STR_ADCFILTERVALUES, 0, 2, GET_SET_DEFAULT(g_model.radioTrainerDisabled), g_eeGeneral.radioTrainerDisabled);

      line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_MODEL_MENU_TABS, 0, COLOR_THEME_PRIMARY1);

#if defined(HELI)
      line = form->newLine(&grid);
      line->padLeft(10);
      new StaticText(line, rect_t{}, STR_MENUHELISETUP, 0, COLOR_THEME_PRIMARY1);
      new OptChoice(line, STR_ADCFILTERVALUES, 0, 2, GET_SET_DEFAULT(g_model.modelHeliDisabled), g_eeGeneral.modelHeliDisabled);
#endif

#if defined(FLIGHT_MODES)
      line = form->newLine(&grid);
      line->padLeft(10);
      new StaticText(line, rect_t{}, STR_MENUFLIGHTMODES, 0, COLOR_THEME_PRIMARY1);
      new OptChoice(line, STR_ADCFILTERVALUES, 0, 2, GET_SET_DEFAULT(g_model.modelFMDisabled), g_eeGeneral.modelFMDisabled);
#endif

#if defined(GVARS)
      line = form->newLine(&grid);
      line->padLeft(10);
      new StaticText(line, rect_t{}, STR_MENU_GLOBAL_VARS, 0, COLOR_THEME_PRIMARY1);
      new OptChoice(line, STR_ADCFILTERVALUES, 0, 2, GET_SET_DEFAULT(g_model.modelGVDisabled), g_eeGeneral.modelGVDisabled);
#endif

      line = form->newLine(&grid);
      line->padLeft(10);
      new StaticText(line, rect_t{}, STR_MENUCURVES, 0, COLOR_THEME_PRIMARY1);
      new OptChoice(line, STR_ADCFILTERVALUES, 0, 2, GET_SET_DEFAULT(g_model.modelCurvesDisabled), g_eeGeneral.modelCurvesDisabled);

      line = form->newLine(&grid);
      line->padLeft(10);
      new StaticText(line, rect_t{}, STR_MENULOGICALSWITCHES, 0, COLOR_THEME_PRIMARY1);
      new OptChoice(line, STR_ADCFILTERVALUES, 0, 2, GET_SET_DEFAULT(g_model.modelLSDisabled), g_eeGeneral.modelLSDisabled);

      line = form->newLine(&grid);
      line->padLeft(10);
      new StaticText(line, rect_t{}, STR_MENUCUSTOMFUNC, 0, COLOR_THEME_PRIMARY1);
      new OptChoice(line, STR_ADCFILTERVALUES, 0, 2, GET_SET_DEFAULT(g_model.modelSFDisabled), g_eeGeneral.modelSFDisabled);

#if defined(LUA_MODEL_SCRIPTS)
      line = form->newLine(&grid);
      line->padLeft(10);
      new StaticText(line, rect_t{}, STR_MENUCUSTOMSCRIPTS, 0, COLOR_THEME_PRIMARY1);
      new OptChoice(line, STR_ADCFILTERVALUES, 0, 2, GET_SET_DEFAULT(g_model.modelCustomScriptsDisabled), g_eeGeneral.modelCustomScriptsDisabled);
#endif

      line = form->newLine(&grid);
      line->padLeft(10);
      new StaticText(line, rect_t{}, STR_MENUTELEMETRY, 0, COLOR_THEME_PRIMARY1);
      new OptChoice(line, STR_ADCFILTERVALUES, 0, 2, GET_SET_DEFAULT(g_model.modelTelemetryDisabled), g_eeGeneral.modelTelemetryDisabled);
    }
};

void ModelSetupPage::build(FormWindow * window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, 0);

  FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

  // Model name
  auto line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_MODELNAME, 0, COLOR_THEME_PRIMARY1);
  new ModelNameEdit(line, rect_t{});

  // Model labels
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_LABELS, 0, COLOR_THEME_PRIMARY1);
  auto curmod = modelslist.getCurrentModel();
  labelTextButton =
    new TextButton(line, rect_t{}, modelslabels.getBulletLabelString(curmod ,STR_UNLABELEDMODEL), [=] () {
       Menu *menu = new Menu(window, true);
       menu->setTitle(STR_LABELS);
       for (auto &label: modelslabels.getLabels()) {
         menu->addLineBuffered(label,
           [=] () {
             if (!modelslabels.isLabelSelected(label, curmod))
               modelslabels.addLabelToModel(label, curmod);
             else
               modelslabels.removeLabelFromModel(label, curmod);
             labelTextButton->setText(modelslabels.getBulletLabelString(curmod,STR_UNLABELEDMODEL));
             strncpy(g_model.header.labels, ModelMap::toCSV(modelslabels.getLabelsByModel(curmod)).c_str(),sizeof(g_model.header.labels));
             g_model.header.labels[sizeof(g_model.header.labels)-1] = '\0';
             SET_DIRTY();
           }, [=] () {
             return modelslabels.isLabelSelected(label, curmod);
           });
       }
       menu->updateLines();
       return 0;
     });

  // Bitmap
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_BITMAP, 0, COLOR_THEME_PRIMARY1);
  // TODO: show bitmap thumbnail instead?
  new ModelBitmapEdit(line, rect_t{});

  // Model ADC jitter filter
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_JITTER_FILTER, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_ADCFILTERVALUES, 0, 2,
             GET_SET_DEFAULT(g_model.jitterFilter));

  static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

  FlexGridLayout grid2(col_dsc, row_dsc, 4);

  line = window->newLine(&grid2);
  line->padTop(8);

  // Modules
  new IntmoduleButton(line);
  new ExtmoduleButton(line);
  new TrainerModuleButton(line);

  line = window->newLine(&grid2);
  line->padTop(2);
#endif
  // Timer buttons
  new SubScreenButton(line, TR_TIMER "1", []() { new TimerWindow(0); });
  new SubScreenButton(line, TR_TIMER "2", []() { new TimerWindow(1); });
  new SubScreenButton(line, TR_TIMER "3", []() { new TimerWindow(2); });

  line = window->newLine(&grid2);
  line->padTop(2);

  new SubScreenButton(line, STR_PREFLIGHT, []() { new PreflightChecks(); });
  new SubScreenButton(line, STR_TRIMS, []() { new TrimsSetup(); });
  new SubScreenButton(line, STR_THROTTLE_LABEL, []() { new ThrottleParams(); });

  line = window->newLine(&grid2);
  line->padTop(2);

  new SubScreenButton(line, STR_ENABLED_FEATURES, []() { new ModelViewOptions(); });

#if defined(USBJ_EX)
  new SubScreenButton(line, STR_USBJOYSTICK_LABEL, []() { new ModelUSBJoystickPage(); });
#endif
}
