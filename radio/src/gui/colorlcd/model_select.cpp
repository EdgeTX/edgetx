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

#include "model_select.h"

#include "libopenui.h"
#include "menu_model.h"
#include "menu_radio.h"
#include "menu_screen.h"
#include "model_templates.h"
#include "opentx.h"
#include "standalone_lua.h"
#include "themes/etx_lv_theme.h"
#include "view_channels.h"

inline tmr10ms_t getTicks() { return g_tmr10ms; }

struct ModelButtonLayout {
  uint16_t width;
  uint16_t height;
  uint16_t padding;
  bool hasImage;
  uint16_t font;
  uint16_t columns;
};

static LAYOUT_VAL(L0_W, 165, 147)
static LAYOUT_VAL(L0_H, 92, 92)
static LAYOUT_VAL(L1_W, 108, 96)
static LAYOUT_VAL(L1_H, 61, 61)
static LAYOUT_VAL(L3_W, 336, 300)

ModelButtonLayout modelLayouts[] = {
    {L0_W, L0_H, PAD_SMALL, true, FONT(STD), 2},
    {L1_W, L1_H, PAD_SMALL, true, FONT(XS), 3},
    {L0_W, EdgeTxStyles::UI_ELEMENT_HEIGHT, PAD_SMALL, false, FONT(STD), 2},
    {L3_W, EdgeTxStyles::UI_ELEMENT_HEIGHT, PAD_SMALL, false, FONT(STD), 1},
};

class ModelButton : public Button
{
 public:
  ModelButton(Window *parent, const rect_t &rect, ModelCell *modelCell,
              std::function<void()> setSelected, uint8_t layout) :
      Button(parent, rect),
      layout(layout),
      modelCell(modelCell),
      m_setSelected(std::move(setSelected))
  {
    padAll(PAD_ZERO);

    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);

    check(modelCell == modelslist.getCurrentModel());

    lv_obj_add_event_cb(lvobj, ModelButton::on_draw, LV_EVENT_DRAW_MAIN_BEGIN,
                        nullptr);
  }

  void addDetails()
  {
    int bg_col_idx = modelCell == modelslist.getCurrentModel()
                         ? COLOR_THEME_ACTIVE_INDEX
                         : COLOR_THEME_PRIMARY2_INDEX;

    coord_t w = width() - 8;
    coord_t h = height() - 8;

    LcdFlags font = modelLayouts[layout].font;
    if ((getTextWidth(modelCell->modelName, 0, font) > w))
      font = (font == FONT(STD)) ? FONT(XS) : FONT(XXS);

    if (modelLayouts[layout].hasImage) {
      GET_FILENAME(filename, BITMAPS_PATH, modelCell->modelBitmap, "");
      auto bitmap = new StaticBitmap(this, {2, 2, w, h}, filename, COLOR(bg_col_idx));
      bitmap->show(bitmap->hasImage());

      if (!bitmap->hasImage()) {
        std::string errorMsg = "(";
        errorMsg += STR_NO_PICTURE;
        errorMsg += ")";
        new StaticText(this, {2, h / 2, w, 17}, errorMsg,
                       CENTERED | COLOR_THEME_SECONDARY1 | FONT(XS));
      }

      coord_t fh = (font == FONT(STD)) ? 17 : (font == FONT(XS)) ? 14 : 11;
      coord_t fo = (font == FONT(STD)) ? -3 : (font == FONT(XS)) ? -3 : -1;

      auto name = new StaticText(this, {2, 2, w, fh}, modelCell->modelName,
                                 CENTERED | COLOR_THEME_SECONDARY1 | font);
      lv_label_set_long_mode(name->getLvObj(), LV_LABEL_LONG_DOT);
      name->setWidth(w);
      name->setHeight(fh);
      etx_bg_color(name->getLvObj(), (LcdColorIndex)bg_col_idx);
      etx_obj_add_style(name->getLvObj(), styles->bg_opacity_75, LV_PART_MAIN);
      name->padTop(fo);
    } else {
      auto name = new StaticText(this, {2, 4, w, 24}, modelCell->modelName,
                                 COLOR_THEME_SECONDARY1 | font);
      lv_label_set_long_mode(name->getLvObj(), LV_LABEL_LONG_DOT);
      name->setWidth(w);
    }

    lv_obj_update_layout(lvobj);
  }

  static void on_draw(lv_event_t *e)
  {
    auto btn = (ModelButton *)lv_obj_get_user_data(lv_event_get_target(e));
    if (btn) {
      if (!btn->loaded) {
        btn->loaded = true;
        btn->addDetails();
      }
    }
  }

  const char *modelFilename() { return modelCell->modelFilename; }
  ModelCell *getModelCell() const { return modelCell; }

  void setFocused()
  {
    if (!lv_obj_has_state(lvobj, LV_STATE_FOCUSED)) {
      lv_group_focus_obj(lvobj);
    } else {
      if (m_setSelected) m_setSelected();
    }
  }

 protected:
  bool loaded = false;
  uint8_t layout;
  ModelCell *modelCell;
  std::function<void()> m_setSelected = nullptr;

  void onClicked() override
  {
    setFocused();
    ButtonBase::onClicked();
    if (m_setSelected) m_setSelected();
  }
};

//-----------------------------------------------------------------------------

class ModelsPageBody : public Window
{
 public:
  ModelsPageBody(Window *parent, const rect_t &rect) : Window(parent, rect)
  {
    padAll(PAD_TINY);
  }

  void update()
  {
    clear();

    ModelsVector models;
    if (selectedLabels.size()) {
      models = modelslabels.getModelsInLabels(selectedLabels);
    } else {
      models = modelslabels.getAllModels();
    }

    // Used to work out which button to set focus to.
    // Priority -
    //     current active model
    //     previously selected model
    //     first model in the list
    ModelButton *firstButton = nullptr;
    ModelButton *focusedButton = nullptr;

    int n = 0;
    int cols = modelLayouts[g_eeGeneral.modelSelectLayout].columns;
    coord_t w = modelLayouts[g_eeGeneral.modelSelectLayout].width;
    coord_t h = modelLayouts[g_eeGeneral.modelSelectLayout].height;

    for (auto &model : models) {
      coord_t x = (n % cols) * (w + PAD_TINY);
      coord_t y = (n / cols) * (h + PAD_TINY);
      n += 1;

      auto button = new ModelButton(
          this, {x, y, w, h}, model, [=]() { focusedModel = model; },
          g_eeGeneral.modelSelectLayout);

      if (!firstButton) firstButton = button;
      if (model == modelslist.getCurrentModel()) focusedButton = button;
      if (model == focusedModel && !focusedButton) focusedButton = button;

      // Press Handler for Models
      button->setPressHandler([=]() -> uint8_t {
        if (model == focusedModel) {
          if (g_eeGeneral.modelQuickSelect)
            selectModel(model);
          else
            openMenu();
        } else {
          focusedModel = model;
        }
        return 0;
      });

      // Long Press Handler for Models
      button->setLongPressHandler([=]() -> uint8_t {
        button->setFocused();
        focusedModel = model;
        openMenu();
        return 0;
      });
    }

    if (!focusedButton) focusedButton = firstButton;

    if (focusedButton) {
      focusedButton->setFocused();
      focusedModel = focusedButton->getModelCell();
    }
  }

  void setLabels(LabelsVector labels)
  {
    selectedLabels = labels;
    update();
  }

  inline void setSortOrder(ModelsSortBy sortOrder)
  {
    modelslabels.setSortOrder(sortOrder);
    update();
  }

  ModelsSortBy getSortOrder() const { return modelslabels.sortOrder(); }

  void setLblRefreshFunc(std::function<void()> fnc)
  {
    refreshLabels = std::move(fnc);
  }

 protected:
  ModelsSortBy _sortOrder;
  bool isDirty = false;
  bool refresh = false;
  std::string selectedLabel;
  LabelsVector selectedLabels;
  ModelCell *focusedModel = nullptr;
  std::function<void()> refreshLabels = nullptr;

  void openMenu()
  {
    Menu *menu = new Menu(this);
    menu->setTitle(focusedModel->modelName);
    if (g_eeGeneral.modelQuickSelect ||
        focusedModel != modelslist.getCurrentModel()) {
      menu->addLine(STR_SELECT_MODEL, [=]() { selectModel(focusedModel); });
    }
    menu->addLine(STR_DUPLICATE_MODEL, [=]() { duplicateModel(focusedModel); });
    menu->addLine(STR_LABEL_MODEL, [=]() { editLabels(focusedModel); });
    menu->addLine(STR_SAVE_TEMPLATE, [=]() { saveAsTemplate(focusedModel); });
    if (focusedModel != modelslist.getCurrentModel()) {
      menu->addLine(STR_DELETE_MODEL, [=]() { deleteModel(focusedModel); });
    }
  }

  void selectModel(ModelCell *model)
  {
    // Don't need to check connection to receiver if re-selecting the active
    // model
    if (model != modelslist.getCurrentModel()) {
      bool modelConnected =
          TELEMETRY_STREAMING() && !g_eeGeneral.disableRssiPoweroffAlarm;
      if (modelConnected) {
        AUDIO_ERROR_MESSAGE(AU_MODEL_STILL_POWERED);
        if (!confirmationDialog(STR_MODEL_STILL_POWERED, nullptr, false, []() {
              tmr10ms_t startTime = getTicks();
              while (!TELEMETRY_STREAMING()) {
                if (getTicks() - startTime > TELEMETRY_CHECK_DELAY10ms) break;
              }
              return !TELEMETRY_STREAMING() ||
                     g_eeGeneral.disableRssiPoweroffAlarm;
            })) {
          return;  // stop if connected but not confirmed
        }
      }
    }

    // Exit to main view
    auto w = Layer::back();
    if (w) w->onCancel();

    // Skip reloading model if re-selecting the active model
    if (model != modelslist.getCurrentModel()) {
      // store changes (if any) and load selected model
      storageFlushCurrentModel();
      storageCheck(true);
      memcpy(g_eeGeneral.currModelFilename, model->modelFilename,
             LEN_MODEL_FILENAME);

      loadModel(g_eeGeneral.currModelFilename, true);
      modelslist.setCurrentModel(model);

      storageDirty(EE_GENERAL);
      storageCheck(true);
    }
  }

  void duplicateModel(ModelCell *model)
  {
    new ConfirmDialog(
        parent, STR_DUPLICATE_MODEL,
        std::string(model->modelName, sizeof(model->modelName)).c_str(), [=] {
          storageFlushCurrentModel();
          storageCheck(true);

          char duplicatedFilename[LEN_MODEL_FILENAME + 1];
          memcpy(duplicatedFilename, model->modelFilename,
                 sizeof(duplicatedFilename));
          if (findNextFileIndex(duplicatedFilename, LEN_MODEL_FILENAME,
                                MODELS_PATH)) {
            sdCopyFile(model->modelFilename, MODELS_PATH, duplicatedFilename,
                       MODELS_PATH);
            // Make a new model which is a copy of the selected one, set the
            // same labels
            auto new_model =
                modelslist.addModel(duplicatedFilename, true, model);
            for (const auto &lbl : modelslabels.getLabelsByModel(model)) {
              modelslabels.addLabelToModel(lbl, new_model);
            }
            update();
          } else {
            TRACE("ModelsListError: Invalid File");
          }
        });
  }

  void deleteModel(ModelCell *model)
  {
    new ConfirmDialog(
        parent, STR_DELETE_MODEL,
        std::string(model->modelName, sizeof(model->modelName)).c_str(), [=] {
          modelslist.removeModel(model);
          if (refreshLabels != nullptr) refreshLabels();

          update();
        });
  }

  void editLabels(ModelCell *model)
  {
    auto labels = modelslabels.getLabels();

    // dont display menu if there will be no labels
    if (labels.size()) {
      auto menu = new Menu(getParent(), true);
      menu->setTitle(model->modelName);
      menu->setCloseHandler([=]() {
        if (isDirty) {
          isDirty = false;
          update();
        }
      });

      for (auto &label : modelslabels.getLabels()) {
        menu->addLineBuffered(
            label,
            [=]() {
              if (!modelslabels.isLabelSelected(label, model))
                modelslabels.addLabelToModel(label, model, true);
              else
                modelslabels.removeLabelFromModel(label, model, true);
              isDirty = true;
              if (refreshLabels != nullptr) refreshLabels();
            },
            [=]() { return modelslabels.isLabelSelected(label, model); });
      }
      menu->updateLines();
    }
  }

  void saveAsTemplate(ModelCell *model)
  {
    new ConfirmDialog(
        parent, STR_SAVE_TEMPLATE,
        std::string(model->modelName, sizeof(model->modelName)).c_str(), [=] {
          storageDirty(EE_MODEL);
          storageCheck(true);
          constexpr size_t size = sizeof(model->modelName) + sizeof(YAML_EXT);
          char modelName[size];
          snprintf(modelName, size, "%s%s", model->modelName, YAML_EXT);
          char templatePath[FF_MAX_LFN];
          sdCheckAndCreateDirectory(TEMPLATES_PATH);
          const char* persFolder = nullptr;
          if (isFileAvailable(PERS_TEMPL_PATH)) {
            persFolder = PERS_TEMPL_PATH;
          } else if (isFileAvailable(PERS_TEMPL_PATH_OLD)) {
            persFolder = PERS_TEMPL_PATH_OLD;
          } else {
            persFolder = PERS_TEMPL_PATH;
            sdCheckAndCreateDirectory(PERS_TEMPL_PATH);
          }
          snprintf(templatePath, FF_MAX_LFN, "%s%c%s", persFolder, '/',
                   modelName);
          if (isFileAvailable(templatePath)) {
            new ConfirmDialog(parent, STR_FILE_EXISTS, STR_ASK_OVERWRITE, [=] {
              sdCopyFile(model->modelFilename, MODELS_PATH, modelName,
                         persFolder);
            });
          } else {
            sdCopyFile(model->modelFilename, MODELS_PATH, modelName,
                       persFolder);
          }
        });
  }
};

//-----------------------------------------------------------------------------

class ModelLayoutButton : public IconButton
{
 public:
  ModelLayoutButton(Window *parent, coord_t x, coord_t y, uint8_t layout,
                    std::function<uint8_t(void)> pressHandler) :
      IconButton(parent, (EdgeTxIcon)(ICON_MODEL_GRID_LARGE + layout), x, y,
                 pressHandler),
      layout(layout)
  {
  }

  uint8_t getLayout() const { return layout; }

  void setLayout(uint8_t newLayout)
  {
    layout = newLayout;
    setIcon((EdgeTxIcon)(ICON_MODEL_GRID_LARGE + layout));
  }

 protected:
  uint8_t layout = 0;
};

//-----------------------------------------------------------------------------

ModelLabelsWindow::ModelLabelsWindow() : Page(ICON_MODEL, PAD_ZERO)
{
  buildHead(header);
  buildBody(body);

  // find the first label of the current model and make that label active
  auto currentModel = modelslist.getCurrentModel();
  if (currentModel != nullptr) {
    auto modelLabels = modelslabels.getLabelsByModel(currentModel);
    if (modelLabels.size() > 0) {
      auto allLabels = getLabels();
      auto found =
          std::find(allLabels.begin(), allLabels.end(), modelLabels[0]);
      if (found != allLabels.end()) {
        lblselector->setSelected(found - allLabels.begin());
      }
    } else {
      // the current model has no labels so set the active label to "Unlabeled"
      lblselector->setSelected(getLabels().size() - 1);
    }
  }
}

#if defined(HARDWARE_KEYS)
void ModelLabelsWindow::onPressSYS()
{
  onCancel();
  new RadioMenu();
}
void ModelLabelsWindow::onLongPressSYS()
{
  onCancel();
  // Radio setup
  (new RadioMenu())->setCurrentTab(2);
}
void ModelLabelsWindow::onPressMDL()
{
  onCancel();
  new ModelMenu();
}
void ModelLabelsWindow::onPressTELE()
{
  onCancel();
  new ScreenMenu();
}
void ModelLabelsWindow::onLongPressTELE()
{
  onCancel();
  new ChannelsViewMenu();
}
void ModelLabelsWindow::onPressPG(bool isNext)
{
  int rowcount = lblselector->getRowCount();
  std::set<uint32_t> sellist;
  int select = -1;

  if (g_eeGeneral.labelSingleSelect) {
    select = lblselector->getActiveItem();
  } else {
    std::set<uint32_t> sel = lblselector->getSelection();
    if (sel.size()) {
      if (isNext)
        select = *sel.rbegin();
      else
        select = *sel.begin();
    }
  }

  if (isNext) {
    select = (select + 1) % rowcount;
  } else {
    select = select - 1;
    if (select < 0)
      select = rowcount - 1;
  }

  if (g_eeGeneral.labelSingleSelect)
    lblselector->setActiveItem(select);

  if (select >= 0)
    sellist.insert(select);
  lblselector->setSelected(sellist);  // Check the items
  lblselector->setSelected(select, true); // Causes the list to scroll

  updateFilteredLabels(sellist);      // Update the models
}
void ModelLabelsWindow::onPressPGUP() { onPressPG(false); }
void ModelLabelsWindow::onPressPGDN() { onPressPG(true); }
#endif

void ModelLabelsWindow::newModel()
{
  // Save current
  storageFlushCurrentModel();
  storageCheck(true);

  new SelectTemplateFolder([=](std::string folder, std::string name) {
    // Create a new blank ModelCell and activate it first, createmodel() will
    // modify the model in memory.
    auto newCell = modelslist.addModel("", false);
    modelslist.setCurrentModel(newCell);

    // Make the new model
    createModel();

    // Close Window
    auto w = Layer::back();
    if (w) w->onCancel();

    // Check for not 'Blank Model'
    if (name.size() > 0) {
      static constexpr size_t LEN_BUFFER =
          sizeof(TEMPLATES_PATH) + 2 * TEXT_FILENAME_MAXLEN + 1;

      char path[LEN_BUFFER + 1];
      snprintf(path, LEN_BUFFER, "%s/%s", TEMPLATES_PATH, folder.c_str());

      // Read model template
      loadModelTemplate((name + YAML_EXT).c_str(), path);
      storageFlushCurrentModel();
      storageCheck(true);

      // Update the current cell's data
      modelslist.updateCurrentModelCell();

#if defined(LUA)
      // If there is a wizard Lua script, fire it up
      int len = strlen(path);
      snprintf(path + len, LEN_BUFFER - len, "/%s%s", name.c_str(), SCRIPT_EXT);
      if (f_stat(path, 0) == FR_OK) {
        luaExec(path);
        StandaloneLuaWindow::instance()->attach();
      }
#endif
    }
  });
}

void ModelLabelsWindow::newLabel()
{
  tmpLabel[0] = '\0';
  new LabelDialog(this, tmpLabel, LABEL_LENGTH, STR_ENTER_LABEL, [=](std::string label) {
    int newlabindex = modelslabels.addLabel(label);
    if (newlabindex >= 0) {
      std::set<uint32_t> newset;
      newset.insert(newlabindex);
      auto labels = getLabels();
      lblselector->setNames(labels);
      lblselector->setSelected(newset);
      if (g_eeGeneral.labelSingleSelect)
        lblselector->setActiveItem(newlabindex);
      updateFilteredLabels(newset);
    }
  });
}

void ModelLabelsWindow::buildHead(Window *hdr)
{
  // page title
  setTitle();

  // new model button
  new TextButton(hdr, {LCD_W - NEW_BTN_W - PAD_LARGE, LAYOUT_BTN_YO, NEW_BTN_W, EdgeTxStyles::UI_ELEMENT_HEIGHT}, STR_NEW, [=]() {
    auto menu = new Menu(this);
    menu->setTitle(STR_CREATE_NEW);
    menu->addLine(STR_NEW_MODEL, [=]() { newModel(); });
    menu->addLine(STR_NEW_LABEL, [=]() { newLabel(); });
    return 0;
  });

  mdlLayout = new ModelLayoutButton(this, LCD_W - LAYOUT_BTN_XO, LAYOUT_BTN_YO, g_eeGeneral.modelSelectLayout, [=]() {
    uint8_t l = mdlLayout->getLayout();
    l = (l + 1) & 3;
    mdlLayout->setLayout(l);
    g_eeGeneral.modelSelectLayout = l;
    storageDirty(EE_GENERAL);
    mdlselector->update();
    return 0;
  });
}

void ModelLabelsWindow::buildBody(Window *window)
{
  // Models List
  mdlselector = new ModelsPageBody(window, {MDLS_X, MDLS_Y, MDLS_W, MDLS_H});
  mdlselector->setLblRefreshFunc([=]() { labelRefreshRequest(); });
  auto mdl_obj = mdlselector->getLvObj();
  lv_obj_set_style_max_width(mdl_obj, MDLS_W, LV_PART_MAIN);
  lv_obj_set_style_max_height(mdl_obj, MDLS_H, LV_PART_MAIN);
  etx_scrollbar(mdl_obj);

  if (mdlselector->getSortOrder() == NO_SORT)
    mdlselector->setSortOrder(NAME_ASC);

  // Labels
  lblselector =
      new ListBox(window, rect_t{PAD_SMALL, LABELS_Y, LABELS_WIDTH, LABELS_HEIGHT}, getLabels());
  lblselector->setSmallSelectMarker();
  auto lbl_obj = lblselector->getLvObj();
  etx_scrollbar(lbl_obj);

  lblselector->setColumnWidth(0, LABELS_WIDTH);

  // Sort Button
  new Choice(
      window, {PAD_SMALL, LABELS_Y + LABELS_HEIGHT + PAD_SMALL, SORT_BUTTON_W, 0}, STR_SORT_ORDERS, NAME_ASC, DATE_DES,
      [=]() { return mdlselector->getSortOrder(); },
      [=](int newValue) { mdlselector->setSortOrder((ModelsSortBy)newValue); },
      STR_SORT_MODELS_BY);

  std::set<uint32_t> filteredLabels = modelslabels.filteredLabels();

  if (g_eeGeneral.labelSingleSelect == 0) {
    lblselector->setMultiSelect(true);
    lblselector->setSelected(filteredLabels);
    lblselector->setMultiSelectHandler([=](std::set<uint32_t> selected,
                                           std::set<uint32_t> oldselection) {
      if (modelslabels.getUnlabeledModels().size() != 0) {
        // Special case for mutually exclusive Unsorted
        bool unsrt_is_selected =
            selected.find(lblselector->getRowCount() - 1) != selected.end();
        bool unsrt_was_selected = oldselection.find(lblselector->getRowCount() -
                                                    1) != oldselection.end();

        // Unsorted was just picked
        if (unsrt_is_selected && !unsrt_was_selected) {
          selected.clear();
          selected.insert(lblselector->getRowCount() - 1);
        } else if (unsrt_is_selected && unsrt_was_selected) {
          selected.erase(selected.find(lblselector->getRowCount() - 1));
        }
      }

      lblselector->setSelected(selected);
      updateFilteredLabels(selected);
    });
  } else {
    if (filteredLabels.size() > 0)
      lblselector->setActiveItem(*filteredLabels.begin());
    lblselector->setPressHandler([=]() {
      int item = lblselector->getActiveItem();
      int selected = lblselector->getSelected();
      std::set<uint32_t> newset;
      // Clicking active label unselects it and selects all models
      if (selected == item) {
        lblselector->setActiveItem(-1);
      } else {
        lblselector->setActiveItem(selected);
        newset.insert(selected);
      }
      updateFilteredLabels(newset);
    });
  }

  updateFilteredLabels(filteredLabels, false);

  lblselector->setGetSelectedSymbol([=](uint16_t row) {
    if (g_eeGeneral.labelSingleSelect)
      return LV_SYMBOL_OK;
    if (lblselector->getSelection().size() == 1)
      return LV_SYMBOL_OK;
    bool hasMoreSelections = false;
    for (uint16_t i = row + 1; i < lblselector->getRowCount(); i += 1)
      if (lblselector->isRowSelected(i)) {
        hasMoreSelections = true;
        break;
      }
    if (!hasMoreSelections)
      return LV_SYMBOL_OK;
    if (row == 0 && (g_eeGeneral.labelMultiMode == 0 || g_eeGeneral.favMultiMode == 0))
      return STR_VCSWFUNC[7]; // AND
    if (g_eeGeneral.labelMultiMode == 0)
      return STR_VCSWFUNC[7]; // AND
    return STR_VCSWFUNC[8]; // OR
  });

  lblselector->setLongPressHandler([=]() {
    int selected = lblselector->getSelected();
    auto labels = getLabels();

    if (selected < (int)labels.size()) {
      std::string selectedLabel = labels.at(selected);

      if (selectedLabel != STR_UNLABELEDMODEL) {
        Menu *menu = new Menu(window);
        menu->setTitle(selectedLabel);
        menu->addLine(STR_RENAME_LABEL, [=]() {
          auto oldLabel = labels[selected];
          strncpy(tmpLabel, oldLabel.c_str(), LABEL_LENGTH);
          tmpLabel[LABEL_LENGTH] = '\0';
          new LabelDialog(this, tmpLabel, LABEL_LENGTH, STR_ENTER_LABEL, [=](std::string newLabel) {
            if (newLabel.size() > 0) {
              auto rndialog =
                  new ProgressDialog(this, STR_RENAME_LABEL, [=]() {});
              modelslabels.renameLabel(
                  oldLabel, newLabel, [=](const char *name, int percentage) {
                    rndialog->setTitle(std::string(STR_RENAME_LABEL) + " " +
                                       name);
                    rndialog->updateProgress(percentage);
                    if (percentage >= 100) rndialog->closeDialog();
                  });
              auto labels = getLabels();
              lblselector->setNames(labels);
              updateFilteredLabels(modelslabels.filteredLabels(), false);
            }
          });
          return 0;
        });
        menu->addLine(STR_DELETE_LABEL, [=]() {
          auto labelToDelete = labels[selected];
          new ConfirmDialog(
              parent, STR_DELETE_LABEL, labelToDelete.c_str(), [=]() {
                auto deldialog =
                    new ProgressDialog(this, STR_DELETE_LABEL, [=]() {});
                modelslabels.removeLabel(
                    labelToDelete, [=](const char *name, int percentage) {
                      deldialog->setTitle(std::string(STR_DELETE_LABEL) + " " +
                                          name);
                      deldialog->updateProgress(percentage);
                      if (percentage >= 100) deldialog->closeDialog();
                    });
                auto labels = getLabels();
                std::set<uint32_t> newset;
                lblselector->setNames(labels);
                lblselector->setSelected(newset);
                if (g_eeGeneral.labelSingleSelect && selected == lblselector->getActiveItem())
                  lblselector->setActiveItem(-1);
                updateFilteredLabels(newset);
              });
          return 0;
        });
        if (modelslabels.getLabels().size() > 1) {
          if (selected != 0) {
            menu->addLine(STR_MOVE_UP, [=]() {
              modelslabels.moveLabelTo(selected, selected - 1);
              std::set<uint32_t> newset;
              newset.insert(selected - 1);
              auto labels = getLabels();
              lblselector->setNames(labels);
              lblselector->setSelected(newset);
              if (g_eeGeneral.labelSingleSelect)
                lblselector->setActiveItem(selected - 1);
              updateFilteredLabels(newset);
              return 0;
            });
          }
          if (selected != (int)modelslabels.getLabels().size() - 1) {
            menu->addLine(STR_MOVE_DOWN, [=]() {
              modelslabels.moveLabelTo(selected, selected + 1);
              std::set<uint32_t> newset;
              newset.insert(selected + 1);
              auto labels = getLabels();
              lblselector->setNames(labels);
              lblselector->setSelected(newset);
              if (g_eeGeneral.labelSingleSelect)
                lblselector->setActiveItem(selected + 1);
              updateFilteredLabels(newset);
              return 0;
            });
          }
        }
      }
    }
  });
}

void ModelLabelsWindow::moveLabel(int selected, int direction)
{
  int swapSelected = selected + direction;

  modelslabels.moveLabelTo(selected, swapSelected);

  std::set<uint32_t> newset = lblselector->getSelection();
  bool isSelected = newset.find(selected) != newset.end();
  bool isSwapSelected = newset.find(swapSelected) != newset.end();
  if (isSelected && !isSwapSelected) {
    newset.erase(newset.find(selected));
    newset.insert(swapSelected);
  } else if (isSwapSelected && !isSelected) {
    newset.erase(newset.find(swapSelected));
    newset.insert(selected);
  }

  lblselector->setNames(getLabels());

  if (g_eeGeneral.labelSingleSelect) {
    int active = lblselector->getActiveItem();
    if (active == selected) {
      lblselector->setActiveItem(swapSelected);
      newset.insert(swapSelected);
    } else if (active == swapSelected) {
      lblselector->setActiveItem(selected);
      newset.insert(selected);
    } else if (active >= 0) {
      newset.insert(active);
    }
  }

  lblselector->setSelected(newset);
  updateFilteredLabels(newset);
}

void ModelLabelsWindow::updateFilteredLabels(std::set<uint32_t> selected,
                                             bool setdirty)
{
  LabelsVector sellabels;
  LabelsVector labels = getLabels();
  for (auto sel : selected) {
    if (sel < labels.size()) sellabels.push_back(labels[sel]);
  }
  if (setdirty) {  // Save to file?
    modelslabels.setFilteredLabels(selected);
    modelslabels.setDirty();
  }
  mdlselector->setLabels(sellabels);  // Update the list
}

void ModelLabelsWindow::labelRefreshRequest()
{
  auto labels = getLabels();
  lblselector->setNames(labels);
}

void ModelLabelsWindow::setTitle()
{
  auto curModel = modelslist.getCurrentModel();
  auto modelName = curModel != nullptr ? curModel->modelName : STR_NONE;

  std::string title2 = STR_ACTIVE;
  title2 += ": ";
  title2 += modelName;

  header->setTitle(STR_MANAGE_MODELS);
  header->setTitle2(title2);
}
