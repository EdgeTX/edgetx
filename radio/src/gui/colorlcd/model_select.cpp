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
#include "view_channels.h"

inline tmr10ms_t getTicks() { return g_tmr10ms; }

constexpr coord_t MODEL_CELL_PADDING = 6;
constexpr coord_t MODEL_SELECT_CELL_HEIGHT = 92;
constexpr int BUTTONS_HEIGHT = 32;
constexpr int MODEL_CELLS_PER_LINE = 2;

#if LCD_W > LCD_H  // Landscape
constexpr int LABELS_WIDTH = 132;
constexpr int LAY_MARGIN = 5;
constexpr coord_t MODEL_SELECT_CELL_WIDTH =
    (LCD_W - LABELS_WIDTH - (MODEL_CELLS_PER_LINE + 1) * MODEL_CELL_PADDING) /
    MODEL_CELLS_PER_LINE;
constexpr int LABELS_ROW = 0;
constexpr int MODELS_COL = 1;
constexpr int MODELS_ROW = 0;
constexpr int MODELS_ROW_CNT = 2;
constexpr int BUTTONS_ROW = 1;
#else  // Portrait
constexpr int LAY_MARGIN = 8;
constexpr int LABELS_HEIGHT = 140;
constexpr coord_t MODEL_SELECT_CELL_WIDTH =
    (LCD_W - LAY_MARGIN - (MODEL_CELLS_PER_LINE + 1) * MODEL_CELL_PADDING) /
    MODEL_CELLS_PER_LINE;
constexpr int LABELS_ROW = 1;
constexpr int MODELS_COL = 0;
constexpr int MODELS_ROW = 0;
constexpr int MODELS_ROW_CNT = 1;
constexpr int BUTTONS_ROW = 2;
#endif

class ModelButton : public Button
{
 public:
  ModelButton(FormWindow *parent, const rect_t &rect, ModelCell *modelCell,
              std::function<void()> setSelected) :
      Button(parent, rect, nullptr, 0, 0, etx_button_create),
      modelCell(modelCell),
      m_setSelected(std::move(setSelected))
  {
    padAll(0);

    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    setWidth(MODEL_SELECT_CELL_WIDTH);
    setHeight(MODEL_SELECT_CELL_HEIGHT);

    LcdFlags bg_color = modelCell == modelslist.getCurrentModel()
                            ? COLOR_THEME_ACTIVE
                            : COLOR_THEME_PRIMARY2;

    coord_t w = width() - 8;
    coord_t h = height() - 8;

    GET_FILENAME(filename, BITMAPS_PATH, modelCell->modelBitmap, "");
    const BitmapBuffer *bitmap = BitmapBuffer::loadBitmap(filename);

    if (bitmap) {
      buffer = new BitmapBuffer(BMP_RGB565, w, h);
      if (buffer) {
        buffer->clear(bg_color);
        buffer->drawScaledBitmap(bitmap, 0, 0, w, h);
        delete bitmap;

        lv_obj_t *bm = lv_canvas_create(lvobj);
        lv_obj_center(bm);
        lv_canvas_set_buffer(bm, buffer->getData(), buffer->width(),
                             buffer->height(), LV_IMG_CF_TRUE_COLOR);
      }
    }

    if (!buffer) {
      std::string errorMsg = "(";
      errorMsg += STR_NO_PICTURE;
      errorMsg += ")";
      new StaticText(this, {2, h / 2, w, 17}, errorMsg, 0,
                     CENTERED | COLOR_THEME_SECONDARY1 | FONT(XS));
    }

    auto name =
        new StaticText(this, {2, 2, width() - 8, 17}, modelCell->modelName, 0,
                       CENTERED | COLOR_THEME_SECONDARY1);
    name->setHeight(17);
    name->setBackgroudOpacity(LV_OPA_80);
    name->setBackgroundColor(bg_color);
    name->padTop(-3);

    check(modelCell == modelslist.getCurrentModel());
  }

  ~ModelButton()
  {
    if (buffer) {
      delete buffer;
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
  ModelCell *modelCell;
  BitmapBuffer *buffer = nullptr;
  std::function<void()> m_setSelected = nullptr;

  void onClicked() override
  {
    setFocused();
    Button::onClicked();
    if (m_setSelected) m_setSelected();
  }
};

//-----------------------------------------------------------------------------

class ModelsPageBody : public FormWindow
{
 public:
  ModelsPageBody(Window *parent, const rect_t &rect) : FormWindow(parent, rect)
  {
    padAll(0);
#if LCD_W > LCD_H
    padRight(LAY_MARGIN);
#else
    padLeft(LAY_MARGIN);
#endif
    setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, MODEL_CELL_PADDING);
    padRow(MODEL_CELL_PADDING);
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

    for (auto &model : models) {
      auto button = new ModelButton(this, rect_t{}, model,
                                    [=]() { focusedModel = model; });

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
          snprintf(templatePath, FF_MAX_LFN, "%s%c%s", PERS_TEMPL_PATH, '/',
                   modelName);
          sdCheckAndCreateDirectory(TEMPLATES_PATH);
          sdCheckAndCreateDirectory(PERS_TEMPL_PATH);
          if (isFileAvailable(templatePath)) {
            new ConfirmDialog(parent, STR_FILE_EXISTS, STR_ASK_OVERWRITE, [=] {
              sdCopyFile(model->modelFilename, MODELS_PATH, modelName,
                         PERS_TEMPL_PATH);
            });
          } else {
            sdCopyFile(model->modelFilename, MODELS_PATH, modelName,
                       PERS_TEMPL_PATH);
          }
        });
  }
};

//-----------------------------------------------------------------------------

class LabelDialog : public Dialog
{
 public:
  LabelDialog(Window *parent, char *label,
              std::function<void(std::string label)> _saveHandler = nullptr) :
      Dialog(parent, STR_ENTER_LABEL, rect_t{}),
      saveHandler(std::move(_saveHandler))
  {
    strncpy(this->label, label, LABEL_LENGTH);
    this->label[LABEL_LENGTH] = '\0';

    auto form = &content->form;
    form->padRow(lv_dpx(8));

    auto form_obj = form->getLvObj();
    lv_obj_set_style_flex_cross_place(form_obj, LV_FLEX_ALIGN_CENTER, 0);

    new TextEdit(form, rect_t{}, label, LABEL_LENGTH);

    auto box = new FormWindow(form, rect_t{});
    box->setFlexLayout(LV_FLEX_FLOW_ROW);

    auto box_obj = box->getLvObj();
    lv_obj_set_style_flex_main_place(box_obj, LV_FLEX_ALIGN_SPACE_EVENLY, 0);

    auto btn = new TextButton(box, rect_t{}, STR_SAVE, [=]() {
      if (saveHandler != nullptr) saveHandler(label);
      deleteLater();
      return 0;
    });
    btn->setWidth(100);

    btn = new TextButton(box, rect_t{}, STR_CANCEL, [=]() {
      deleteLater();
      return 0;
    });
    btn->setWidth(100);

    content->setWidth(LCD_W * 0.8);
    content->updateSize();
  }

 protected:
  std::function<void(std::string label)> saveHandler;
  char label[LABEL_LENGTH + 1];
};

//-----------------------------------------------------------------------------

ModelLabelsWindow::ModelLabelsWindow() : Page(ICON_MODEL)
{
  buildHead(&header);
  buildBody(&body);

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
  std::set<uint32_t> curSel = lblselector->getSelection();
  std::set<uint32_t> sellist;
  int select = 0;
  int rowcount = lblselector->getRowCount();

  if (isNext) {
    if (curSel.size()) select = (*curSel.rbegin() + 1) % rowcount;
  } else {
    if (curSel.size()) {
      select = (int)*curSel.begin() - 1;
      if (select < 0) select += rowcount;
    } else {
      select = rowcount - 1;
    }
  }

  sellist.insert(select);
  lblselector->setSelected(sellist);  // Check the items
  lblselector->setSelected(-1);       // Force update
  lblselector->setSelected(select);   // Causes the list to scroll
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
  new LabelDialog(parent, tmpLabel, [=](std::string label) {
    int newlabindex = modelslabels.addLabel(label);
    if (newlabindex >= 0) {
      std::set<uint32_t> newset;
      newset.insert(newlabindex);
      auto labels = getLabels();
      lblselector->setNames(labels);
      lblselector->setSelected(newset);
      updateFilteredLabels(newset);
    }
  });
}

void ModelLabelsWindow::buildHead(PageHeader *hdr)
{
  // page title
  setTitle();

  // new model button
  auto btn = new TextButton(hdr, rect_t{}, STR_NEW, [=]() {
    auto menu = new Menu(this);
    menu->setTitle(STR_CREATE_NEW);
    menu->addLine(STR_NEW_MODEL, [=]() { newModel(); });
    menu->addLine(STR_NEW_LABEL, [=]() { newLabel(); });
    return 0;
  });

  btn->padAll(lv_dpx(4));

  // button placement
  hdr->padRight(lv_dpx(8));
  lv_obj_align(btn->getLvObj(), LV_ALIGN_RIGHT_MID, 0, 0);
}

#if LCD_W > LCD_H
static const lv_coord_t col_dsc[] = {LABELS_WIDTH, LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_FR(1), BUTTONS_HEIGHT,
                                     LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_FR(1), LABELS_HEIGHT,
                                     BUTTONS_HEIGHT, LV_GRID_TEMPLATE_LAST};
#endif

void ModelLabelsWindow::buildBody(FormWindow *window)
{
  window->padAll(LAY_MARGIN);
  window->padRow(LAY_MARGIN);
  window->padColumn(LAY_MARGIN);
  window->padLeft(0);
  window->padRight(0);

  lv_obj_set_grid_dsc_array(window->getLvObj(), col_dsc, row_dsc);

  // Models List
  mdlselector = new ModelsPageBody(window, rect_t{});
  mdlselector->setLblRefreshFunc([=]() { labelRefreshRequest(); });
  auto mdl_obj = mdlselector->getLvObj();
  lv_obj_set_grid_cell(mdl_obj, LV_GRID_ALIGN_STRETCH, MODELS_COL, 1,
                       LV_GRID_ALIGN_STRETCH, MODELS_ROW, MODELS_ROW_CNT);

  // Labels
  auto box = new Window(window, rect_t{});
  box->padAll(0);
  box->padLeft(LAY_MARGIN);
#if LCD_H > LCD_W
  box->padRight(LAY_MARGIN);
#endif
  lv_obj_set_grid_cell(box->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 1,
                       LV_GRID_ALIGN_STRETCH, LABELS_ROW, 1);
  lblselector =
      new ListBox(box, rect_t{0, 0, LV_PCT(100), LV_PCT(100)}, getLabels());
  auto lbl_obj = lblselector->getLvObj();

  // Sort Buttons
  box = new Window(window, rect_t{});
  box->padAll(0);
  box->padLeft(LAY_MARGIN);
  box->padRight(LAY_MARGIN);
  lv_obj_set_grid_cell(box->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 1,
                       LV_GRID_ALIGN_STRETCH, BUTTONS_ROW, 1);
  auto sortOrder = new Choice(
      box, {}, STR_SORT_ORDERS, NAME_ASC, DATE_DES,
      [=]() { return mdlselector->getSortOrder(); },
      [=](int newValue) { mdlselector->setSortOrder((ModelsSortBy)newValue); });
  sortOrder->setMenuTitle(STR_SORT_MODELS_BY);

  lv_obj_update_layout(mdl_obj);
  lblselector->setColumnWidth(0, lv_obj_get_content_width(lbl_obj));

  lblselector->setMultiSelect(true);
  lblselector->setSelected(modelslabels.filteredLabels());
  updateFilteredLabels(modelslabels.filteredLabels(), false);
  lv_obj_set_scrollbar_mode(lbl_obj, LV_SCROLLBAR_MODE_AUTO);
  lv_obj_set_scrollbar_mode(mdl_obj, LV_SCROLLBAR_MODE_AUTO);

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
          new LabelDialog(this, tmpLabel, [=](std::string newLabel) {
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
                      deldialog->setTitle(std::string(STR_RENAME_LABEL) + " " +
                                          name);
                      deldialog->updateProgress(percentage);
                      if (percentage >= 100) deldialog->closeDialog();
                    });
                auto labels = getLabels();
                std::set<uint32_t> newset;
                lblselector->setNames(labels);
                lblselector->setSelected(newset);
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
              updateFilteredLabels(newset);
              return 0;
            });
          }
        }
      }
    }
  });
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

  header.setTitle(STR_MANAGE_MODELS);
  header.setTitle2(title2);
}
