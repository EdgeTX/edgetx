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

#include <algorithm>
#include <iostream>
#include <vector>

#include "libopenui.h"
#include "listbox.h"
#include "model_templates.h"
#include "opentx.h"
#include "standalone_lua.h"
#include "str_functions.h"

// bitmaps for toolbar
const uint8_t _mask_sort_alpha_up[] = {
#include "mask_sort_alpha_up.lbm"
};
STATIC_LZ4_BITMAP(mask_sort_alpha_up);

const uint8_t _mask_sort_alpha_down[] = {
#include "mask_sort_alpha_down.lbm"
};
STATIC_LZ4_BITMAP(mask_sort_alpha_down);

const uint8_t _mask_sort_date_up[] = {
#include "mask_sort_date_up.lbm"
};
STATIC_LZ4_BITMAP(mask_sort_date_up);

const uint8_t _mask_sort_date_down[] = {
#include "mask_sort_date_down.lbm"
};
STATIC_LZ4_BITMAP(mask_sort_date_down);

inline tmr10ms_t getTicks() { return g_tmr10ms; }

constexpr coord_t MODEL_CELL_PADDING = 6;
constexpr coord_t MODEL_SELECT_CELL_HEIGHT = 92;

#if LCD_W > LCD_H
constexpr int MODEL_CELLS_PER_LINE = 2;
constexpr LcdFlags textFont = FONT(STD);
constexpr int labelWidth = 150;
constexpr int LABELS_WIDTH = 132;
constexpr int LAY_MARGIN = 5;
constexpr coord_t MODEL_SELECT_CELL_WIDTH =
    (LCD_W - LABELS_WIDTH -
     (MODEL_CELLS_PER_LINE + 1) * MODEL_CELL_PADDING) /
    MODEL_CELLS_PER_LINE;

#else
constexpr int MODEL_CELLS_PER_LINE = 2;
constexpr LcdFlags textFont = FONT(XS);
constexpr int labelWidth = 150;
constexpr int LAY_MARGIN = 5;
constexpr int MODELS_TOP = 125;
constexpr int SORTBUTTONS_TOP = LCD_H - 70 - LAY_MARGIN;

constexpr coord_t MODEL_SELECT_CELL_WIDTH =
    (LCD_W - LAY_MARGIN - (MODEL_CELLS_PER_LINE + 1) * MODEL_CELL_PADDING) /
    MODEL_CELLS_PER_LINE;
#endif

class ToolbarButton : public Button
{
 public:
  ToolbarButton(FormGroup *parent, const rect_t &rect, const uint8_t *bitmap,
                std::function<uint8_t()> pressHandler = nullptr) :
      Button(parent, rect, pressHandler, 0), _bitmap(bitmap)
  {
    lv_obj_set_style_border_width(lvobj, lv_dpx(2), 0);
    lv_obj_set_style_border_opa(lvobj, LV_OPA_TRANSP, 0);

    lv_obj_set_style_border_color(lvobj, makeLvColor(COLOR_THEME_FOCUS), LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(lvobj, LV_OPA_100, LV_STATE_FOCUSED);
  }

  inline bool getSelected() { return _selected; }
  void setSelected(bool selected)
  {
    _selected = selected;
    invalidate();
  }

  void setBitmap(const uint8_t *bitmap)
  {
    _bitmap = bitmap;
    invalidate();
  }

  void paint(BitmapBuffer *dc) override
  {
    int width;
    uint32_t bgColor =
        !_selected ? COLOR_THEME_SECONDARY3 : COLOR_THEME_SECONDARY2;
    auto bm = getBitmap(_bitmap, bgColor, COLOR_THEME_PRIMARY1, &width);
    dc->drawScaledBitmap(bm, 2, 2, this->width() - 4, this->height() - 4);
    delete bm;
  }

 protected:
  const uint8_t *_bitmap;
  bool _selected = false;

  BitmapBuffer *getBitmap(const uint8_t *maskData, uint32_t bgColor,
                          uint32_t fgColor, int *width)
  {
    auto mask = BitmapBuffer::load8bitMask(maskData);
    BitmapBuffer *newBm =
        new BitmapBuffer(BMP_RGB565, mask->width(), mask->height());
    newBm->clear(bgColor);
    newBm->drawMask(0, 0, mask, fgColor);
    delete mask;
    return newBm;
  }
};

class ButtonHolder : public FormWindow
{
 public:
  struct ButtonInfo {
    ToolbarButton *button;
    const uint8_t *states[2];
    int sortState;
  };

  ButtonHolder(Window *parent, const rect_t &rect) : FormWindow(parent, rect)
  {
    setHeight(25);
    addButton(mask_sort_alpha_up, mask_sort_alpha_down);
    addButton(mask_sort_date_up, mask_sort_date_down);

    _buttons[0].button->setSelected(true);
  }

  inline void setPressHandler(
      std::function<void(int index, ButtonInfo *button)> pressHandler)
  {
    _pressHandler = std::move(pressHandler);
  }

  void addButton(const uint8_t *state1Bm, const uint8_t *state2Bm)
  {
    int buttonNumber = _buttons.size();
    auto tb = new ToolbarButton(
        this, {buttonNumber * (height() + 4), 0, height(), height()}, state1Bm);
    tb->setPressHandler([=]() {
      bool isSelected = tb->getSelected();

      ButtonInfo *b = nullptr;
      int buttonIndex = -1;
      for (int i = 0; i < (int)_buttons.size(); i++) {
        _buttons[i].button->setSelected(false);
        if (tb == _buttons[i].button) {
          b = &_buttons[i];
          buttonIndex = i;
        }
      }
      tb->setSelected(true);

      if (isSelected && b != nullptr) {
        if (b->sortState == 0)
          b->sortState = 1;
        else
          b->sortState = 0;

        tb->setBitmap(b->states[b->sortState]);
      }

      if (_pressHandler != nullptr && b != nullptr)
        _pressHandler(buttonIndex, b);

      return 0;
    });

    ButtonInfo bi = {tb, state1Bm, state2Bm, 0};

    _buttons.push_back(bi);
  }

 protected:
  std::vector<ButtonInfo> _buttons;
  std::function<void(int index, ButtonInfo *button)> _pressHandler;
};

class ModelButton : public Button
{
 public:
  ModelButton(FormGroup *parent, const rect_t &rect, ModelCell *modelCell) :
      Button(parent, rect), modelCell(modelCell)
  {
    setWidth(MODEL_SELECT_CELL_WIDTH);
    setHeight(MODEL_SELECT_CELL_HEIGHT);
  }

  ~ModelButton()
  {
    if (buffer) {
      delete buffer;
    }
  }

  void load()
  {
    const char *error = nullptr;

    delete buffer;
    buffer = new BitmapBuffer(BMP_RGB565, width(), height());
    if (buffer == nullptr) {
      return;
    }
    buffer->clear(COLOR_THEME_PRIMARY2);

    if (error) {
      buffer->drawText(width() / 2, height() / 2, "(Invalid Model)",
                       COLOR_THEME_SECONDARY1 | CENTERED);
    } else {
      GET_FILENAME(filename, BITMAPS_PATH, modelCell->modelBitmap, "");
      const BitmapBuffer *bitmap = BitmapBuffer::loadBitmap(filename);
      if (bitmap) {
        buffer->drawScaledBitmap(bitmap, 0, 0, width(), height());
        delete bitmap;
      } else {
        buffer->drawText(width() / 2, 56, "(No Picture)",
                         FONT(XXS) | COLOR_THEME_SECONDARY1 | CENTERED);
      }
    }
  }

  void paint(BitmapBuffer *dc) override
  {
    if (!loaded) {  // Load them on the fly
      load();
      loaded = true;
    }
    FormField::paint(dc);

    if (buffer) dc->drawBitmap(0, 0, buffer);

    if (modelCell == modelslist.getCurrentModel()) {
      dc->drawSolidFilledRect(0, 0, width(), 20, COLOR_THEME_ACTIVE);
      dc->drawSizedText(width() / 2, 2, modelCell->modelName, LEN_MODEL_NAME,
                        COLOR_THEME_SECONDARY1 | CENTERED);
    } else {
      LcdFlags textColor;
      dc->drawFilledRect(0, 0, width(), 20, SOLID, COLOR_THEME_PRIMARY2);

      textColor = COLOR_THEME_SECONDARY1;

      dc->drawSizedText(width() / 2, 2, modelCell->modelName, LEN_MODEL_NAME,
                        textColor | CENTERED);
    }

    if (!hasFocus()) {
      dc->drawSolidRect(0, 0, width(), height(), 1, COLOR_THEME_SECONDARY2);
    } else {
      dc->drawSolidRect(0, 0, width(), height(), 2, COLOR_THEME_FOCUS);
    }
  }

  const char *modelFilename() { return modelCell->modelFilename; }
  ModelCell *getModelCell() const { return modelCell; }

 protected:
  bool loaded = false;
  ModelCell *modelCell;
  BitmapBuffer *buffer = nullptr;
};

//-----------------------------------------------------------------------------

class MyMenu : public Menu
{
 public:
  using Menu::Menu;
  void setFinishHandler(std::function<void()> finishHandler)
  {
    _finishHandler = std::move(finishHandler);
  }

  void deleteLater(bool detach = true, bool trash = true) override
  {
    if (_finishHandler != nullptr) {
      _finishHandler();
    }
    Menu::deleteLater(detach, trash);
  }

 protected:
  std::function<void()> _finishHandler = nullptr;
};

//-----------------------------------------------------------------------------

ModelsPageBody::ModelsPageBody(Window *parent, const rect_t &rect) :
    FormWindow(parent, rect)
{
  setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, MODEL_CELL_PADDING);
  padRow(MODEL_CELL_PADDING);
  padAll(MODEL_CELL_PADDING);
  
  update();
}

void ModelsPageBody::selectModel(ModelButton *btn)
{
  bool modelConnected =
      TELEMETRY_STREAMING() && !g_eeGeneral.disableRssiPoweroffAlarm;
  if (modelConnected) {
    AUDIO_ERROR_MESSAGE(AU_MODEL_STILL_POWERED);
    if (!confirmationDialog(STR_MODEL_STILL_POWERED, nullptr, false, []() {
          tmr10ms_t startTime = getTicks();
          while (!TELEMETRY_STREAMING()) {
            if (getTicks() - startTime > TELEMETRY_CHECK_DELAY10ms) break;
          }
          return !TELEMETRY_STREAMING() || g_eeGeneral.disableRssiPoweroffAlarm;
        })) {
      return;  // stop if connected but not confirmed
    }
  }

  // store changes (if any) and load selected model
  storageFlushCurrentModel();
  storageCheck(true);
  memcpy(g_eeGeneral.currModelFilename, btn->modelFilename(),
         LEN_MODEL_FILENAME);

  loadModel(g_eeGeneral.currModelFilename, true);
  storageDirty(EE_GENERAL);
  storageCheck(true);

  modelslist.setCurrentModel(btn->getModelCell());
  checkAll();

  // Exit to main view
  auto w = Layer::back();
  if (w) w->onCancel();
}

void ModelsPageBody::duplicateModel(ModelCell* model)
{
  char duplicatedFilename[LEN_MODEL_FILENAME + 1];
  memcpy(duplicatedFilename, model->modelFilename,
         sizeof(duplicatedFilename));
  if (findNextFileIndex(duplicatedFilename, LEN_MODEL_FILENAME,
                        MODELS_PATH)) {
    sdCopyFile(model->modelFilename, MODELS_PATH, duplicatedFilename,
               MODELS_PATH);
    auto new_model = modelslist.addModel(duplicatedFilename, true, model->modelName);
    // Duplicated model should have same labels as orig. Add them
    for (const auto &lbl : modelslabels.getLabelsByModel(model)) {
      modelslabels.addLabelToModel(lbl, new_model);
    }
    update();
  } else {
    POPUP_WARNING("Invalid File"); // TODO: translation
  }
}

void ModelsPageBody::deleteModel(ModelButton *btn)
{
  ModelCell *model = btn->getModelCell();
  new ConfirmDialog(
      parent, STR_DELETE_MODEL,
      std::string(model->modelName, sizeof(model->modelName)).c_str(), [=] {
        modelslist.removeModel(model);
        if (refreshLabels != nullptr) refreshLabels();
        btn->deleteLater(); // TODO: rather "update()" ???
      });
}

void ModelsPageBody::editLabels(ModelCell* model)
{
  auto labels = modelslabels.getLabels();

  // dont display menu if there will be no labels
  if (labels.size()) {
    MyMenu *menu = new MyMenu(getParent(), true);
    menu->setTitle(model->modelName);
    menu->setFinishHandler([=]() {
      if (isDirty) {
        isDirty = false;
        update();
      }
    });

    for (auto &label : modelslabels.getLabels()) {
      menu->addLine(
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
  }
}

void ModelsPageBody::update(int selected)
{
  clear();

  ModelsVector models;
  if (selectedLabels.size()) {
    models = modelslabels.getModelsInLabels(selectedLabels, _sortOrder);
  } else {
    models = modelslabels.getAllModels(_sortOrder);
  }

  for (auto &model : models) {
    auto button = new ModelButton(this, rect_t{}, model);
    button->setPressHandler([=]() -> uint8_t { return 1; });

    // Long Press Handler for Models
    button->setLongPressHandler([=]() -> uint8_t {
      Menu *menu = new Menu(this);
      menu->setTitle(model->modelName);
      if (model != modelslist.getCurrentModel()) {
        menu->addLine(STR_SELECT_MODEL, [=]() { selectModel(button); });
      }
      menu->addLine(STR_DUPLICATE_MODEL, [=]() { duplicateModel(model); });
      if (model != modelslist.getCurrentModel()) {
        menu->addLine(STR_DELETE_MODEL, [=]() { selectModel(button); });
      }
      menu->addLine(STR_EDIT_LABELS, [=]() { editLabels(model); });
      return 0;
    });
  }
}

//-----------------------------------------------------------------------------

class LabelDialog : public Dialog
{
 public:
  LabelDialog(Window *parent, char *label,
              std::function<void(std::string label)> _saveHandler = nullptr) :
      Dialog(parent, STR_ENTER_LABEL, rect_t{}),
      saveHandler(std::move(_saveHandler))
  {
    strncpy(this->label, label, MAX_LABEL_SIZE);
    this->label[MAX_LABEL_SIZE] = '\0';

    auto form = &content->form;
    form->padRow(lv_dpx(8));

    auto form_obj = form->getLvObj();
    lv_obj_set_style_flex_cross_place(form_obj, LV_FLEX_ALIGN_CENTER, 0);

    new TextEdit(form, rect_t{}, label, MAX_LABEL_SIZE);

    auto box = new FormGroup(form, rect_t{});
    box->setFlexLayout(LV_FLEX_FLOW_ROW);

    auto box_obj = box->getLvObj();
    lv_obj_set_style_flex_main_place(box_obj, LV_FLEX_ALIGN_SPACE_EVENLY, 0);

    auto btn = new TextButton(
        box, rect_t{}, STR_SAVE,
        [=]() {
          if (saveHandler != nullptr) saveHandler(label);
          deleteLater();
          return 0;
        },
        BUTTON_BACKGROUND | OPAQUE, textFont);
    btn->setWidth(LV_DPI_DEF);

    btn = new TextButton(
        box, rect_t{}, STR_CANCEL,
        [=]() {
          deleteLater();
          return 0;
        },
        BUTTON_BACKGROUND | OPAQUE, textFont);
    btn->setWidth(LV_DPI_DEF);

    content->setWidth(LCD_W * 0.8);
    content->updateSize();
  }

 protected:
  std::function<void(std::string label)> saveHandler;
  char label[MAX_LABEL_SIZE + 1];
};

//-----------------------------------------------------------------------------

ModelLabelsWindow::ModelLabelsWindow() : Page(ICON_MODEL)
{
  buildBody(&body);
  buildHead(&header);

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
      // the current model has no labels so set the active label to "Unlabled"
      lblselector->setSelected(getLabels().size() - 1);
    }
  }
}

void ModelLabelsWindow::newModel()
{
  // Save current
  storageCheck(true);

  const char* model_name = g_model.header.name;
  auto newCell = modelslist.addModel(::createModel(), false, model_name);
  modelslist.setCurrentModel(newCell);

  auto labels = getLabels();
  lblselector->setNames(labels);
  lblselector->setSelected(modelslabels.getLabels().size());
  mdlselector->update(0);

  new SelectTemplateFolder([=]() {});
}

void ModelLabelsWindow::buildHead(PageHeader *hdr)
{
  // page title
  auto curModel = modelslist.getCurrentModel();
  auto modelName = curModel != nullptr ? curModel->modelName : STR_NONE;

  std::string titleName = STR_SELECT_MODEL;
  titleName += "\n";
  titleName += STR_CURRENT_MODEL;
  titleName += ": ";
  titleName += modelName;

  hdr->setTitle(titleName);
  
  // new model button
  auto btn = new TextButton(
      hdr, rect_t{}, STR_NEW_MODEL,
      [=]() {
        newModel();
        return 0;
      },
      BUTTON_BACKGROUND | OPAQUE, textFont);

  btn->padAll(lv_dpx(4));

  // button placement
  hdr->padRight(lv_dpx(8));
  lv_obj_align(btn->getLvObj(), LV_ALIGN_RIGHT_MID, 0, 0);
}

static const lv_coord_t col_dsc[] = {LABELS_WIDTH, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

void ModelLabelsWindow::buildBody(FormWindow *window)
{
  auto win_obj = window->getLvObj();
  lv_obj_set_grid_dsc_array(win_obj, col_dsc, row_dsc);

  // Labels + sorting buttons - Left
  auto box = new Window(window, rect_t{});
  box->padAll(lv_dpx(8));
  box->padRight(0);

  auto box_obj = box->getLvObj();
  lv_obj_set_width(box_obj, lv_pct(100));
  lv_obj_set_height(box_obj, lv_pct(100));
  lv_obj_set_grid_cell(box_obj, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

  lv_obj_update_layout(box_obj);
  auto box_cw = lv_obj_get_content_width(box_obj);
  
  // Models List - Right
  mdlselector = new ModelsPageBody(window, rect_t{});
  mdlselector->setLblRefreshFunc([=]() { labelRefreshRequest(); });

  auto obj = mdlselector->getLvObj();
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

  auto btnh = new ButtonHolder(box, rect_t{});
  btnh->setPressHandler([=](int index, ButtonHolder::ButtonInfo *button) {
        if (index == 0) {  // alpha
          sort = button->sortState == 0 ? NAME_ASC : NAME_DES;
        } else {
          sort = button->sortState == 0 ? DATE_ASC : DATE_DES;
        }
        // Update the list asynchronously
        mdlselector->setSortOrder(sort);
      });

  lv_obj_align(btnh->getLvObj(), LV_ALIGN_BOTTOM_LEFT, 0, 0);

  lblselector = new ListBox(box, rect_t{}, getLabels());
  lblselector->setColumnWidth(0, box_cw);
  lblselector->setMultiSelect(true);
  lblselector->setSelected(modelslabels.filteredLabels());
  updateFilteredLabels(modelslabels.filteredLabels(), false);

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
      Menu *menu = new Menu(window);
      std::string selectedLabel = labels.at(selected);
      menu->setTitle(selectedLabel);
      menu->addLine(STR_NEW_LABEL, [=]() {
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
        return 0;
      });
      if (selectedLabel != STR_UNLABELEDMODEL) {
        menu->addLine(STR_RENAME_LABEL, [=]() {
          auto oldLabel = labels[selected];
          strncpy(tmpLabel, oldLabel.c_str(), MAX_LABEL_SIZE);
          tmpLabel[MAX_LABEL_SIZE] = '\0';

          new LabelDialog(this, tmpLabel, [=](std::string newLabel) {
            auto rndialog =
                new ProgressDialog(this, STR_RENAME_LABEL, [=]() {});
            modelslabels.renameLabel(
                oldLabel, newLabel, [=](const char *name, int percentage) {
                  rndialog->updateProgress(name, percentage);
                });
            auto labels = getLabels();
            lblselector->setNames(labels);
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
                      deldialog->updateProgress(name, percentage);
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
            menu->addLine("Move Up", [=]() {
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
            menu->addLine("Move Down", [=]() {
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

//-----------------------------------------------------------------------------

ProgressDialog::ProgressDialog(Window *parent, std::string title,
                               std::function<void()> onClose) :
    Dialog(parent, title, rect_t{}),
    progress(new Progress(&content->form, rect_t{})),
    onClose(std::move(onClose)),
    _title(title)
{
  progress->setHeight(LV_DPI_DEF / 4);

  content->setWidth(LCD_W * 0.8);
  content->updateSize();

  auto content_w = lv_obj_get_content_width(content->form.getLvObj());
  progress->setWidth(content_w);

  // disable canceling dialog
  setCloseWhenClickOutside(false);
}

void ProgressDialog::updateProgress(const char *filename, int percentage)
{
  content->setTitle(_title + " " + filename);
  progress->setValue(percentage);
  if (percentage >= 100) {
    deleteLater();
    onClose();
  } else {
    lv_refr_now(nullptr);
  }
}

// disable keys
void ProgressDialog::onEvent(event_t) { return; }
