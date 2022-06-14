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

struct TimerBtnMatrix : public ButtonMatrix {
  TimerBtnMatrix(Window* parent, const rect_t& rect);
  void onPress(uint8_t btn_id) override;
  bool isActive(uint8_t btn_id) override;
};

class SubScreenButton : public Button
{
  std::string text;

 public:
  SubScreenButton(Window* parent, const char* text,
                  std::function<void(void)> pressHandler);

 protected:
  virtual bool isActive() { return false; }
  static void event_cb(lv_event_t* e);
};

SubScreenButton::SubScreenButton(Window* parent, const char* text,
                                 std::function<void(void)> pressHandler) :
  Button(parent, rect_t{}, [=]() -> uint8_t {
      pressHandler();
      return 0;
    }, 0, 0, lv_btn_create),
  text(text)
{
  lv_obj_add_event_cb(lvobj, SubScreenButton::event_cb, LV_EVENT_ALL, nullptr);
}

// TODO: move code to TextButton with BEGIN/END in main and using events
//      to grab lv_draw_label_dsc_t (see lv_btnmatrix.c)
//
void SubScreenButton::event_cb(lv_event_t* e)
{
  auto obj = lv_event_get_target(e);
  auto btn = (SubScreenButton*)lv_obj_get_user_data(obj);
  if (!btn) return;

  lv_event_code_t code = lv_event_get_code(e);  
  if (code == LV_EVENT_DRAW_PART_BEGIN) {

    lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
    if (dsc->part != LV_PART_MAIN) return;

    if (btn->isActive()) {
      dsc->rect_dsc->bg_color = makeLvColor(COLOR_THEME_ACTIVE);
    } else {
      dsc->rect_dsc->bg_color = makeLvColor(COLOR_THEME_PRIMARY2);
    }

  } else if (code == LV_EVENT_DRAW_PART_END) {

    if (btn->text.empty()) return;
    
    lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
    if (dsc->part != LV_PART_MAIN) return;

    lv_area_t txt_coords;
    lv_obj_get_content_coords(obj, &txt_coords);

    lv_draw_label_dsc_t label_draw_dsc;
    lv_draw_label_dsc_init(&label_draw_dsc);

    lv_obj_init_draw_label_dsc(obj, LV_PART_MAIN, &label_draw_dsc);
    label_draw_dsc.align = LV_TEXT_ALIGN_CENTER;

    if (btn->isActive()) {
      label_draw_dsc.color = makeLvColor(COLOR_THEME_PRIMARY2);
    } else {
      label_draw_dsc.color = makeLvColor(COLOR_THEME_SECONDARY1);
    }

    lv_area_t txt_clip;
    bool is_common = _lv_area_intersect(&txt_clip, &txt_coords, dsc->draw_ctx->clip_area);
    if (!is_common) return;

    lv_draw_label(dsc->draw_ctx, &label_draw_dsc, &txt_coords, btn->text.c_str(), nullptr);

  } else if (code == LV_EVENT_GET_SELF_SIZE) {

    // from lv_label_t with some simplifications
    lv_point_t size;
    const lv_font_t* font = lv_obj_get_style_text_font(obj, LV_PART_MAIN);
    lv_coord_t letter_space = lv_obj_get_style_text_letter_space(obj, LV_PART_MAIN);
    lv_coord_t line_space = lv_obj_get_style_text_line_space(obj, LV_PART_MAIN);
    lv_text_flag_t flag = LV_TEXT_FLAG_NONE;

    lv_coord_t w = lv_obj_get_content_width(obj);
    if (lv_obj_get_style_width(obj, LV_PART_MAIN) == LV_SIZE_CONTENT && !obj->w_layout)
      w = LV_COORD_MAX;
    else
      w = lv_obj_get_content_width(obj);

    lv_txt_get_size(&size, btn->text.c_str(), font, letter_space, line_space, w, flag);

    lv_point_t* self_size = (lv_point_t*)lv_event_get_param(e);
    self_size->x = LV_MAX(self_size->x, size.x);
    self_size->y = LV_MAX(self_size->y, size.y);
  }
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

  // Global functions
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_USE_GLOBAL_FUNCS, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_INVERTED(g_model.noGlobalFunctions));

  // Model ADC jitter filter
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_JITTER_FILTER, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_ADCFILTERVALUES, 0, 2,
             GET_SET_DEFAULT(g_model.jitterFilter));

  // Modules
  auto form = new FormGroup(window, rect_t{});
  form->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, lv_dpx(8));
  lv_obj_set_style_pad_all(form->getLvObj(), lv_dpx(8), 0);
  
  new IntmoduleButton(form);
  new ExtmoduleButton(form);
  new TrainerModuleButton(form);

  // Timer buttons
  form = new FormGroup(window, rect_t{});
  form->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, lv_dpx(8));
  lv_obj_set_style_flex_main_place(form->getLvObj(), LV_FLEX_ALIGN_SPACE_EVENLY, 0);
  form->padAll(lv_dpx(8));

  new SubScreenButton(form, TR_TIMER "1",
                      []() { new TimerWindow(0); });
  new SubScreenButton(form, TR_TIMER "2",
                      []() { new TimerWindow(1); });
  new SubScreenButton(form, TR_TIMER "3",
                      []() { new TimerWindow(2); });

  form = new FormGroup(window, rect_t{});
  form->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, lv_dpx(8));
  lv_obj_set_style_flex_main_place(form->getLvObj(), LV_FLEX_ALIGN_SPACE_EVENLY, 0);
  form->padAll(lv_dpx(8));

  new SubScreenButton(form, STR_PREFLIGHT,
                      []() { new PreflightChecks(); });
  new SubScreenButton(form, STR_TRIMS,
                      []() { new TrimsSetup(); });
  new SubScreenButton(form, STR_THROTTLE_LABEL,
                      []() { new ThrottleParams(); });
}

#define MAX_SUBSCREEN_BTNS 9

TimerBtnMatrix::TimerBtnMatrix(Window* parent, const rect_t& r) :
  ButtonMatrix(parent, r)
{
  initBtnMap(3, MAX_TIMERS);
  setText(0, TR_TIMER "1");
  setText(1, TR_TIMER "2");
  setText(2, TR_TIMER "3");
  update();
  
  lv_btnmatrix_set_btn_width(lvobj, 3, 2);
  lv_obj_set_width(lvobj, lv_pct(100));
  lv_obj_set_height(lvobj, LV_DPI_DEF / 2);
  
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_0, 0);
  lv_obj_set_style_pad_all(lvobj, lv_dpx(8), 0);

  lv_obj_set_style_pad_row(lvobj, lv_dpx(8), 0);
  lv_obj_set_style_pad_column(lvobj, lv_dpx(8), 0);
}

void TimerBtnMatrix::onPress(uint8_t btn_id)
{
  if (btn_id >= MAX_TIMERS) return;
  new TimerWindow((uint8_t)(btn_id));
}

bool TimerBtnMatrix::isActive(uint8_t btn_id)
{
  return false;
}
