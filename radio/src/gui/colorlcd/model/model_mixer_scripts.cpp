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

#include "model_mixer_scripts.h"

#include "dataconstants.h"
#include "filechoice.h"
#include "libopenui.h"
#include "list_line_button.h"
#include "lua/lua_api.h"
#include "menus.h"
#include "edgetx.h"
#include "page.h"
#include "sourcechoice.h"
#include "etx_lv_theme.h"
#include "translations.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

// Edit grid
#if !PORTRAIT_LCD
static const lv_coord_t e_col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
                                       LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t e_col_dsc[] = {LV_GRID_FR(5), LV_GRID_FR(4),
                                       LV_GRID_TEMPLATE_LAST};
#endif

// Line button grid
static const lv_coord_t b_col_dsc[] = {40, 84, 84, LV_GRID_FR(1),
                                       LV_GRID_TEMPLATE_LAST};

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class ScriptEditWindow : public Page
{
 public:
  explicit ScriptEditWindow(uint8_t idx) :
      Page(ICON_MODEL_LUA_SCRIPTS), idx(idx)
  {
    buildBody(body);
    buildHeader(header);
  }

 protected:
  const uint8_t idx;
  bool update = false;

  void checkEvents() override
  {
    if ((update) && (luaState == INTERPRETER_RUNNING)) {
      TRACE("rebuilding ScriptEditWindow...");
      rebuildBody(body);
      update = false;
    }
    // note: 'update' is set from Page::checkEvents()
    Page::checkEvents();
  }

  void buildHeader(Window* window)
  {
    header->setTitle(STR_MENUCUSTOMSCRIPTS);
    header->setTitle2(std::string("LUA") + std::to_string(idx + 1));
  }

  void buildBody(Window* window, bool focusScript = false)
  {
    window->setFlexLayout();

    FlexGridLayout grid(e_col_dsc, row_dsc, PAD_TINY);

    // the general pattern seems to be using capture-by-value for the closures:
    // so need to copy the pointers, not the objects
    ScriptData* const sd = &(g_model.scriptsData[idx]);
    ScriptInputsOutputs* const sio = &(scriptInputsOutputs[idx]);

    // File
    auto line = window->newLine(grid);
    new StaticText(line, rect_t{}, STR_SCRIPT);
    new FileChoice(
        line, rect_t{}, SCRIPTS_MIXES_PATH, SCRIPTS_EXT, LEN_SCRIPT_FILENAME,
        [=]() { return stringFromNtString(sd->file); },
        [=](std::string newValue) {
          clearStruct(*sd);
          clearStruct(*sio);
          if (!newValue.empty()) {
            copyToUnTerminated(sd->file, newValue);
          }
          storageDirty(EE_MODEL);
          LUA_LOAD_MODEL_SCRIPT(idx);  // async reload ...
          update = true;
        },
        true, STR_SCRIPT);

    // Custom name
    line = window->newLine(grid);
    new StaticText(line, rect_t{}, STR_NAME);
    new ModelTextEdit(line, rect_t{}, sd->name, sizeof(sd->name));

    if (sio->inputsCount > 0) {
      line = window->newLine(grid);
      new Subtitle(line, STR_INPUTS);

      for (int i = 0; i < sio->inputsCount; i++) {
        line = window->newLine(grid);
        ScriptInput& si = sio->inputs[i];
        auto lbl =
            new StaticText(line, rect_t{}, si.name);
        lbl->padLeft(PAD_LARGE);
        if (si.type == INPUT_TYPE_VALUE) {
          (new NumberEdit(line, rect_t{}, si.min, si.max,
                          GET_SET_WITH_OFFSET(sd->inputs[i].value, si.def)))
              ->setDefault(si.def);
        } else {
          new SourceChoice(line, rect_t{}, 0, MIXSRC_LAST_TELEM,
                           GET_SET_DEFAULT(sd->inputs[i].source));
        }
      }
    }

    if (sio->outputsCount > 0) {
      line = window->newLine(grid);
      new Subtitle(line, STR_OUTPUTS);

      for (int i = 0; i < sio->outputsCount; i++) {
        line = window->newLine(grid);
        ScriptOutput* so = &(sio->outputs[i]);
        auto lbl = new DynamicText(
            line, rect_t{},
            [=]() {
              char s[16];
              getSourceString(
                  s, MIXSRC_FIRST_LUA + (idx * MAX_SCRIPT_OUTPUTS) + i);
              return std::string(s, sizeof(s) - 1);
            });
        lbl->padLeft(PAD_LARGE);
        new DynamicNumber<int16_t>(
            line, rect_t{}, [=]() { return calcRESXto1000(so->value); },
            COLOR_THEME_PRIMARY1_INDEX, PREC1);
      }
    }
  }

  void rebuildBody(Window* window)
  {
    auto scroll_y = lv_obj_get_scroll_y(window->getLvObj());
    window->clear();
    buildBody(window);
    lv_obj_scroll_to_y(window->getLvObj(), scroll_y, LV_ANIM_OFF);
  }
};

class ScriptLineButton : public ListLineButton
{
 public:
  ScriptLineButton(Window* parent,
                   const ScriptData& scriptData,
                   const ScriptInternalData* runtimeData, uint8_t index) :
      ListLineButton(parent, index),
      scriptData(scriptData),
      runtimeData(runtimeData)
  {
    setHeight(EdgeTxStyles::UI_ELEMENT_HEIGHT);
    padTop(PAD_SMALL);
    padLeft(PAD_TINY);
    padRight(PAD_TINY);
    lv_obj_set_layout(lvobj, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(lvobj, b_col_dsc, row_dsc);
    lv_obj_set_style_pad_row(lvobj, 0, 0);
    lv_obj_set_style_pad_column(lvobj, 4, 0);

    lv_obj_update_layout(parent->getLvObj());
    if (lv_obj_is_visible(lvobj)) delayed_init();

    lv_obj_add_event_cb(lvobj, ScriptLineButton::on_draw,
                        LV_EVENT_DRAW_MAIN_BEGIN, nullptr);
  }

  static void on_draw(lv_event_t* e)
  {
    lv_obj_t* target = lv_event_get_target(e);
    auto line = (ScriptLineButton*)lv_obj_get_user_data(target);
    if (line) {
      if (!line->init)
        line->delayed_init();
      line->refresh();
    }
  }

  void delayed_init()
  {
    init = true;

    auto lbl = lv_label_create(lvobj);
    etx_obj_add_style(lbl, styles->text_align_left, LV_PART_MAIN);
    lv_obj_set_grid_cell(lbl, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER,
                         0, 1);

    lv_label_set_text(lbl,
                      (std::string("LUA") + std::to_string(index + 1)).c_str());

    if (runtimeData) {
      char s[20];

      lbl = lv_label_create(lvobj);
      etx_obj_add_style(lbl, styles->text_align_left, LV_PART_MAIN);
      lv_obj_set_grid_cell(lbl, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER,
                           0, 1);

      strAppend(s, scriptData.name, LEN_SCRIPT_NAME);
      lv_label_set_text(lbl, s);

      lbl = lv_label_create(lvobj);
      etx_obj_add_style(lbl, styles->text_align_left, LV_PART_MAIN);
      lv_obj_set_grid_cell(lbl, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER,
                           0, 1);

      strAppend(s, scriptData.file, LEN_SCRIPT_FILENAME);
      lv_label_set_text(lbl, s);

      lbl = lv_label_create(lvobj);
      etx_obj_add_style(lbl, styles->text_align_left, LV_PART_MAIN);
      lv_obj_set_grid_cell(lbl, LV_GRID_ALIGN_START, 3, 1, LV_GRID_ALIGN_CENTER,
                           0, 1);

      // TODO: runtimeData->instructions has no value
      switch (runtimeData->state) {
        case SCRIPT_SYNTAX_ERROR:
          lv_label_set_text(lbl, STR_SCRIPT_ERROR);
          break;
        case SCRIPT_NOFILE:
          lv_label_set_text(lbl, STR_NEEDS_FILE);
          break;
        case SCRIPT_OK:
          lv_label_set_text(lbl, "-");
          break;
        default:
          lv_label_set_text(lbl, "");
          break;
      }
    }

    lv_obj_update_layout(lvobj);
  }

  bool isActive() const override { return false; }
  void refresh() override {}

 protected:
  bool init = false;
  const ScriptData& scriptData;
  const ScriptInternalData* runtimeData;
};

ModelMixerScriptsPage::ModelMixerScriptsPage() :
    PageTab(STR_MENUCUSTOMSCRIPTS, ICON_MODEL_LUA_SCRIPTS)
{
}

void ModelMixerScriptsPage::rebuild(Window* window, int8_t focusIdx)
{
  auto scroll_y = lv_obj_get_scroll_y(window->getLvObj());
  window->clear();
  build(window, focusIdx);
  lv_obj_scroll_to_y(window->getLvObj(), scroll_y, LV_ANIM_OFF);
}

void ModelMixerScriptsPage::build(Window* window, int8_t focusIdx)
{
  window->padBottom(PAD_LARGE);
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);

  int8_t scriptIdx = 0;
  for (int8_t idx = 0; idx < MAX_SCRIPTS; idx++) {
    ScriptInternalData* runtimeData = nullptr;
    ScriptData* const sd = &(g_model.scriptsData[idx]);
    ScriptInputsOutputs* const sio = &(scriptInputsOutputs[idx]);

    if (sd->file[0] != '\0') {
      runtimeData = &(scriptInternalData[scriptIdx++]);
    }

    auto button = new ScriptLineButton(window, *sd, runtimeData, idx);

    button->setPressHandler([=]() -> uint8_t {
      Menu* const menu = new Menu();
      menu->addLine(STR_EDIT, [=]() { editLine(window, idx); });

      if (runtimeData != nullptr) {
        menu->addLine(STR_DELETE, [=]() {
          clearStruct(*sd);
          clearStruct(*sio);
          LUA_LOAD_MODEL_SCRIPTS();
          storageDirty(EE_MODEL);
          rebuild(window, idx);
        });
        return 0;
      }
      return 0;
    });
  }
}

void ModelMixerScriptsPage::editLine(Window* window, uint8_t idx)
{
  Window* editWindow = new ScriptEditWindow(idx);
  editWindow->setCloseHandler([=]() { rebuild(window, idx); });
}
