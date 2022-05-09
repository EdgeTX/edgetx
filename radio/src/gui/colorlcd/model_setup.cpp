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

#include <algorithm>

#define SET_DIRTY()     storageDirty(EE_MODEL)

ModelSetupPage::ModelSetupPage() :
  PageTab(STR_MENU_MODEL_SETUP, ICON_MODEL_SETUP)
{
}

const char MODEL_NAME_EXTRA_CHARS[] = "_-.,:;<=>";

static void onModelNameChanged()
{
  modelslist.load();
  auto model = modelslist.getCurrentModel();
  if (model) {
    model->setModelName(g_model.header.name);
    modelslist.save();
  }
  SET_DIRTY();
}

struct ModelNameEdit : public ModelTextEdit {
  ModelNameEdit(Window *parent, const rect_t &rect) :
      ModelTextEdit(parent, rect, g_model.header.name,
                    sizeof(g_model.header.name), 0, MODEL_NAME_EXTRA_CHARS)
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
  SET_DIRTY();
}

struct ModelBitmapEdit : public FileChoice {
  ModelBitmapEdit(Window *parent, const rect_t &rect) :
      FileChoice(parent, rect, BITMAPS_PATH, BITMAPS_EXT,
                 sizeof(g_model.header.bitmap), getModelBitmap, setModelBitmap)
  {
  }
};

struct SubScreenButtonMatrix : public ButtonMatrix {
  SubScreenButtonMatrix(Window* parent, const rect_t& rect);
  void onPress(uint8_t btn_id) override;
  bool isActive(uint8_t btn_id) override;
};

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

void ModelSetupPage::build(FormWindow * window)
{
  window->setFlexLayout();
  FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

  // Model name
  auto line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_MODELNAME, 0, COLOR_THEME_PRIMARY1);
  new ModelNameEdit(line, rect_t{});

  // Bitmap
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_BITMAP, 0, COLOR_THEME_PRIMARY1);
  // TODO: show bitmap thumbnail instead?
  new ModelBitmapEdit(line, rect_t{});

  // Extended limits
  // TODO: move to "Outputs" screen ?
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_ELIMITS, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_DEFAULT(g_model.extendedLimits));

  // Global functions
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_USE_GLOBAL_FUNCS, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_INVERTED(g_model.noGlobalFunctions));

  // Model ADC jitter filter
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_JITTER_FILTER, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_ADCFILTERVALUES, 0, 2,
             GET_SET_DEFAULT(g_model.jitterFilter));

  // Sub-screens
  new SubScreenButtonMatrix(window, rect_t{});
}

#define MAX_SUBSCREEN_BTNS 9

SubScreenButtonMatrix::SubScreenButtonMatrix(Window* parent, const rect_t& r) :
  ButtonMatrix(parent, r)
{
  initBtnMap(3, MAX_SUBSCREEN_BTNS);

  setText(0, STR_INTERNALRF);
  setText(1, STR_EXTERNALRF);
  setText(2, STR_TRAINER);

  setText(3, STR_PREFLIGHT);
  setText(4, STR_TRIMS);
  setText(5, STR_THROTTLE_LABEL);
  setText(6, TR_TIMER "1");
  setText(7, TR_TIMER "2");
  setText(8, TR_TIMER "3");
  update();
  
  lv_btnmatrix_set_btn_width(lvobj, 3, 2);
  lv_obj_set_width(lvobj, lv_pct(100));
  lv_obj_set_height(lvobj, (4 * LV_DPI_DEF) / 3);
  
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(lvobj, LV_DPI_DEF / 10, LV_PART_MAIN);

  lv_obj_remove_style(lvobj, nullptr, LV_PART_MAIN | LV_STATE_FOCUSED);
  lv_obj_remove_style(lvobj, nullptr, LV_PART_MAIN | LV_STATE_EDITED);
}

void SubScreenButtonMatrix::onPress(uint8_t btn_id)
{
  if (btn_id >= MAX_SUBSCREEN_BTNS) return;

  Window* w = nullptr;
  switch (btn_id) {
    case 0:
      w = new ModulePage(INTERNAL_MODULE);
      break;
    case 1:
      w = new ModulePage(EXTERNAL_MODULE);
      break;
    case 2:
      w = new TrainerPage();
      break;
    case 3:
      w = new PreflightChecks();
      break;
    case 4:
      w = new TrimsSetup();
      break;
    case 5:
      w = new ThrottleParams();
      break;
    default:
      w = new TimerWindow((uint8_t)(btn_id - 6));
      break;
  }

  if (w) w->setFocus();
}

bool SubScreenButtonMatrix::isActive(uint8_t btn_id)
{
  if (btn_id == 0)
    return g_model.moduleData[INTERNAL_MODULE].type > 0;

  if (btn_id == 1)
    return g_model.moduleData[EXTERNAL_MODULE].type > 0;

  if (btn_id == 2)
    return g_model.trainerData.mode > 0;
  
  return false;
}
