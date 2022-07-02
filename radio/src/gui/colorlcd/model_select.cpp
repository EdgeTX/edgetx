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

#include <vector>
#include <iostream>
#include <algorithm>

#include "model_select.h"
#include "opentx.h"
#include "listbox.h"
#include "libopenui.h"
#include "standalone_lua.h"
#include "str_functions.h"
#include "model_templates.h"

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

constexpr int LONG_PRESS_10MS = 40;
constexpr coord_t MODEL_CELL_PADDING = 6;
constexpr coord_t MODEL_SELECT_CELL_HEIGHT = 92;

#if LCD_W > LCD_H
constexpr int MODEL_CELLS_PER_LINE = 2;
constexpr int BUTTON_HEIGHT = 30;
constexpr int BUTTON_WIDTH  = 95;
constexpr LcdFlags textFont = FONT(STD);
constexpr rect_t detailsDialogRect = {50, 50, 400, 150};
constexpr int labelWidth = 150;
constexpr int LABELS_HEIGHT = MENUS_LINE_HEIGHT;
constexpr int LABELS_WIDTH = 120;
constexpr int LABELS_LEFT = 5;
constexpr int LABELS_TOP = 60;
constexpr coord_t MODEL_SELECT_CELL_WIDTH =
    (LCD_W - LABELS_WIDTH - LABELS_LEFT - (MODEL_CELLS_PER_LINE + 1) * MODEL_CELL_PADDING) /
    MODEL_CELLS_PER_LINE;

#else
constexpr int MODEL_CELLS_PER_LINE = 2;
constexpr int BUTTON_HEIGHT = 30;
constexpr int BUTTON_WIDTH  = 75;
constexpr LcdFlags textFont = FONT(XS);
constexpr rect_t detailsDialogRect = {5, 50, LCD_W - 10, 150};
constexpr int labelWidth = 150;
constexpr int LABELS_HEIGHT = 30;
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
    ToolbarButton(FormGroup *parent, const rect_t &rect, const uint8_t *bitmap, std::function<uint8_t ()> pressHandler = nullptr) :
      Button(parent, rect, pressHandler, 0),
      _bitmap(bitmap)
    {
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

    void paint (BitmapBuffer *dc) override
    {
      int width;
      uint32_t bgColor = !_selected ? COLOR_THEME_SECONDARY3 : COLOR_THEME_SECONDARY2;
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
    struct ButtonInfo
    {
      ToolbarButton *button;
      const uint8_t *states[2];
      int sortState;
    };

    ButtonHolder(Window *parent, const rect_t &rect) :
      FormWindow(parent, rect)
    {
      addButton(mask_sort_alpha_up, mask_sort_alpha_down);
      addButton(mask_sort_date_up, mask_sort_date_down);

      _buttons[0].button->setSelected(true);
    }

    inline void setPressHandler(std::function<void (int index, ButtonInfo *button)> pressHandler)
    {
      _pressHandler = std::move(pressHandler);
    }

    void addButton(const uint8_t *state1Bm, const uint8_t *state2Bm)
    {
      int buttonNumber = _buttons.size();
      auto tb = new ToolbarButton(this, {buttonNumber * (height() + 4), 0, height(), height()}, state1Bm);
      tb->setPressHandler([=] () {
        bool isSelected = tb->getSelected();

        ButtonInfo *b = nullptr;
        int buttonIndex = -1;
        for (int i = 0; i < (int) _buttons.size(); i++) {
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

      ButtonInfo bi = {
        tb,
        state1Bm,
        state2Bm,
        0
      };

      _buttons.push_back(bi);
    }

  protected:
    std::vector<ButtonInfo> _buttons;
    std::function<void (int index, ButtonInfo *button)> _pressHandler;
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
    if (buffer) { delete buffer; }
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
      const BitmapBuffer *bitmap = BitmapBuffer::loadBitmap(filename);   // TODO: Delay read until visible for first time on screen?
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
    if(!loaded) { // Load them on the fly
      load();
      loaded = true;
    }
    FormField::paint(dc);

    if (buffer)
      dc->drawBitmap(0, 0, buffer);

    if (modelCell == modelslist.getCurrentModel()) {
      dc->drawSolidFilledRect(0, 0, width(), 20, COLOR_THEME_ACTIVE);
      dc->drawSizedText(width() / 2, 2, modelCell->modelName,
                        LEN_MODEL_NAME,
                        COLOR_THEME_SECONDARY1 | CENTERED);
    } else {
      LcdFlags textColor;
      dc->drawFilledRect(0, 0, width(), 20, SOLID, COLOR_THEME_PRIMARY2);

      textColor = COLOR_THEME_SECONDARY1;

      dc->drawSizedText(width() / 2, 2, modelCell->modelName,
                        LEN_MODEL_NAME,
                        textColor | CENTERED);
    }

    if (!hasFocus()) {
      dc->drawSolidRect(0, 0, width(), height(), 1, COLOR_THEME_SECONDARY2);
    } else {
      dc->drawSolidRect(0, 0, width(), height(), 2, COLOR_THEME_FOCUS);
    }
  }

  const char* modelFilename() { return modelCell->modelFilename; }
  ModelCell* getModelCell() const { return modelCell; }

 protected:
  bool loaded=false;
  ModelCell *modelCell;
  BitmapBuffer *buffer = nullptr;
};

//-----------------------------------------------------------------------------
class MyMenu : public Menu
{
  public:
    using Menu::Menu;
    void setFinishHandler(std::function<void ()> finishHandler)
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
    std::function<void ()> _finishHandler = nullptr;
};

//-----------------------------------------------------------------------------

ModelsPageBody::ModelsPageBody(Window *parent, const rect_t &rect) :
    FormWindow(parent, rect),
  innerWindow(this, { 4, 4, rect.w - 8, rect.h - 8 })
{
  update();
  setFocusHandler([=](bool focus) {
    if (focus) {
      if (innerWindow.getChildren().size() > 0) {
        //auto firstModel = *innerWindow.getChildren().begin();
        //firstModel->setFocus();
      }
    }
  });

}

void ModelsPageBody::paint(BitmapBuffer *dc)
{
  //dc->drawSolidRect(0, getScrollPositionY(), rect.w, rect.h, 2, COLOR_THEME_FOCUS);
}

void ModelsPageBody::initPressHandlers(ModelButton *button, ModelCell *model, int index)
{
  button->setPressHandler([=]() -> uint8_t {return 1;});
  // Long Press Handler for Models
  button->setLongPressHandler([=]() -> uint8_t {
    Menu *menu = new Menu(this);
    menu->setTitle(model->modelName);
    if (model != modelslist.getCurrentModel()) {
      menu->addLine(STR_SELECT_MODEL, [=]() {
        bool modelConnected = TELEMETRY_STREAMING() &&
                                    !g_eeGeneral.disableRssiPoweroffAlarm;
        if (modelConnected) {
          AUDIO_ERROR_MESSAGE(AU_MODEL_STILL_POWERED);
          if (!confirmationDialog(STR_MODEL_STILL_POWERED, nullptr, false, []() {
                    tmr10ms_t startTime = getTicks();
                    while (!TELEMETRY_STREAMING()) {
                      if (getTicks() - startTime > TELEMETRY_CHECK_DELAY10ms)
                        break;
                    }
                    return !TELEMETRY_STREAMING() ||
                            g_eeGeneral.disableRssiPoweroffAlarm;
            })) {
          return;  // stop if connected but not confirmed
          }
        }

        // we store the latest changes if any
        storageFlushCurrentModel();
        storageCheck(true);
        memcpy(g_eeGeneral.currModelFilename, model->modelFilename,
                LEN_MODEL_FILENAME);
        loadModel(g_eeGeneral.currModelFilename, false);
        storageDirty(EE_GENERAL);
        storageCheck(true);
        modelslist.setCurrentModel(model);
        checkAll();
        //this->getParent()->getParent()->deleteLater();
      });
    }
    menu->addLine(STR_DUPLICATE_MODEL, [=]() {
      char duplicatedFilename[LEN_MODEL_FILENAME + 1];
      memcpy(duplicatedFilename, model->modelFilename,
              sizeof(duplicatedFilename));
      if (findNextFileIndex(duplicatedFilename, LEN_MODEL_FILENAME,
                            MODELS_PATH)) {
        sdCopyFile(model->modelFilename, MODELS_PATH, duplicatedFilename,
                    MODELS_PATH);
        ModelCell *newmodel = modelslist.addModel(duplicatedFilename);
        // Duplicated model should have same labels as orig. Add them
        for(const auto &lbl : modelsLabels.getLabelsByModel(model)) {
          modelsLabels.addLabelToModel(lbl, newmodel);
        }
        // Copy name & new filename
        strcpy(newmodel->modelName, model->modelName);
        strcpy(newmodel->modelFilename,duplicatedFilename);
        update();
      } else {
        POPUP_WARNING("Invalid File");
      }
    });
    if (model != modelslist.getCurrentModel()) {
      menu->addLine(STR_DELETE_MODEL, [=]() {
        new ConfirmDialog(
            parent, STR_DELETE_MODEL,
            std::string(model->modelName, sizeof(model->modelName)).c_str(), [=]() {
              modelslist.removeModel(model);
              if(refreshLabels != nullptr)
                refreshLabels();
              update();
            });
      });
    }
    menu->addLine(STR_EDIT_LABELS, [=]() {
      auto labels = modelsLabels.getLabels();
      //button->setFocus();

      // dont display menu if there will be no labels
      if (labels.size()) {
        MyMenu *menu = new MyMenu(this->parent, true);
        menu->setTitle(model->modelName);
        menu->setFinishHandler([=] () {
          if (isDirty) {
            isDirty = false;
            refresh = true;
          }
        });

        for (auto &label: modelsLabels.getLabels()) {
          menu->addLine(label,
            [=] () {
              if (!modelsLabels.isLabelSelected(label, model))
                modelsLabels.addLabelToModel(label, model, true);
              else
                modelsLabels.removeLabelFromModel(label, model, true);
              isDirty = true;
              if(refreshLabels != nullptr)
                refreshLabels();
            }, [=] () {
              return modelsLabels.isLabelSelected(label, model);
            });
        }
      }
    });
    return 0;
  });
}

void ModelsPageBody::update(int selected)
{
  innerWindow.clear();

  int index = 0;
  coord_t y = 0;
  coord_t x = 0;

  ModelButton* selectButton = nullptr;
  ModelsVector models = modelsLabels.getModelsInLabels(selectedLabels, _sortOrder);

  for (auto &model : models) {
    auto button = new ModelButton(
        &innerWindow, {x, y, MODEL_SELECT_CELL_WIDTH, MODEL_SELECT_CELL_HEIGHT},
        model);
    initPressHandlers(button, model, index);
    if (selected == index) {
      selectButton = button;
    }

    index++;

    if (index % MODEL_CELLS_PER_LINE == 0) {
      x = 0;
      y += MODEL_SELECT_CELL_HEIGHT + MODEL_CELL_PADDING;
    } else {
      x += MODEL_CELL_PADDING + MODEL_SELECT_CELL_WIDTH;
    }
  }

  if (index % MODEL_CELLS_PER_LINE != 0) {
    y += MODEL_SELECT_CELL_HEIGHT + MODEL_CELL_PADDING;
  }
  //innerWindow.setInnerHeight(y);

  if (selectButton != nullptr) {
    //selectButton->setFocus();
  }
}

//-----------------------------------------------------------------------------
class LabelDialog : public Dialog
{
  public:
    LabelDialog(Window *parent, char *label, std::function<void (std::string label)> saveHandler = nullptr) :
      Dialog(parent, STR_ENTER_LABEL, detailsDialogRect),
      saveHandler(saveHandler)
    {
      strncpy(this->label, label, MAX_LABEL_SIZE);
      this->label[MAX_LABEL_SIZE] = '\0';

      FormGridLayout grid(detailsDialogRect.w);
      grid.setLabelWidth(labelWidth);
      grid.spacer(8);

      new StaticText(&content->form, grid.getLabelSlot(), STR_LABEL, 0, COLOR_THEME_PRIMARY1);
      new TextEdit(&content->form, grid.getFieldSlot(), label, MAX_LABEL_SIZE);
      grid.nextLine();

      rect_t r = {detailsDialogRect.w - (BUTTON_WIDTH + 5), grid.getWindowHeight() + 5, BUTTON_WIDTH, BUTTON_HEIGHT };
      new TextButton(&content->form, r, STR_SAVE, [=] () {
        if (saveHandler != nullptr)
          saveHandler(label);
        deleteLater();
        return 0;
      }, BUTTON_BACKGROUND | OPAQUE, textFont);
      r.x -= (BUTTON_WIDTH + 5);
      new TextButton(&content->form, r, STR_CANCEL, [=] () {
        deleteLater();
        return 0;
      }, BUTTON_BACKGROUND | OPAQUE, textFont);
    }

  protected:
    std::function<void (std::string label)> saveHandler;
    char label[MAX_LABEL_SIZE+1];
};

//-----------------------------------------------------------------------------

ModelLabelsWindow::ModelLabelsWindow() :
  Page(ICON_MODEL)
{
  buildBody(&body);
  buildHead(&header);

  // // make PG_UP and PG_DN work
  // lblselector->setNextField(mdlselector);
  // lblselector->setPreviousField(newButton);
  // mdlselector->setNextField(newButton);
  // mdlselector->setPreviousField(lblselector);
  // newButton->setNextField(lblselector);
  // newButton->setPreviousField(mdlselector);

  // lblselector->setFocus();

  // find the first label of the current model and make that label active
  auto currentModel = modelslist.getCurrentModel();
  if (currentModel != nullptr) {
    auto modelLabels = modelsLabels.getLabelsByModel(currentModel);
    if (modelLabels.size() > 0) {
      auto allLabels = getLabels();
      auto found = std::find(allLabels.begin(), allLabels.end(), modelLabels[0]);
      if (found != allLabels.end()) {
        lblselector->setSelected(found - allLabels.begin());
      }
    } else {
      // the current model has no labels so set the active label to "Unlabled"
      lblselector->setSelected(getLabels().size() - 1);
    }
  }
}

bool isChildOfMdlSelector(Window *window)
{
  while (window != nullptr)
  {
    if (dynamic_cast<ModelsPageBody *>(window) != nullptr)
      return true;

    window = window->getParent();
  }

  return false;
}

#if defined(HARDWARE_KEYS)
void ModelLabelsWindow::onEvent(event_t event)
{
  // if (event == EVT_KEY_BREAK(KEY_PGUP)) {
  //   onKeyPress();
  //   FormField *focus = dynamic_cast<FormField *>(getFocus());
  //   if (isChildOfMdlSelector(focus))
  //     lblselector->setFocus();
  //   else if (focus != nullptr && focus->getPreviousField()) {
  //     focus->getPreviousField()->setFocus(SET_FOCUS_BACKWARD, focus);
  //   }
  // } else if (event == EVT_KEY_BREAK(KEY_PGDN)) {
  //   onKeyPress();
  //   FormField *focus = dynamic_cast<FormField *>(getFocus());
  //   if (isChildOfMdlSelector(focus))
  //     newButton->setFocus();
  //   else
  //    if (focus != nullptr && focus->getNextField()) {
  //     focus->getNextField()->setFocus(SET_FOCUS_FORWARD, focus);
  //   }
  // } else if (event == EVT_KEY_BREAK(KEY_EXIT)) {
  //   killEvents(event);
  //   deleteLater();
  // } else {
  //   Page::onEvent(event);
  // }
}
#endif

void ModelLabelsWindow::buildHead(PageHeader *window)
{
  LcdFlags flags = 0;

  bool verticalDisplay = LCD_W < LCD_H;

  if (verticalDisplay) {
    flags = FONT(XS);
  }

  // page title
  new StaticText(window,
                  {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                  PAGE_LINE_HEIGHT},
                  STR_SELECT_MODEL, 0, COLOR_THEME_PRIMARY2 | flags);

  auto curModel = modelslist.getCurrentModel();
  auto modelName = curModel != nullptr ? curModel->modelName : STR_NONE;

  std::string titleName = ! verticalDisplay ?
                  STR_CURRENT_MODEL + std::string(": ") + std::string(modelName) :
                  std::string(modelName);
  new StaticText(window,
                  {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
                  LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                  titleName, 0, COLOR_THEME_PRIMARY2 | flags);

  // new model button
  rect_t r = {LCD_W - (BUTTON_WIDTH + 5), 6, BUTTON_WIDTH, BUTTON_HEIGHT };
  newButton = new TextButton(window, r, STR_NEW_MODEL, [=] () {
      storageCheck(true); // Save current
      ModelCell *newCell = modelslist.addModel("", false);
      modelslist.setCurrentModel(newCell);
      const char *nfname = createModel();
      strncpy(newCell->modelFilename, nfname, LEN_MODEL_FILENAME);
      newCell->modelFilename[LEN_MODEL_FILENAME] = '\0';
      newCell->setModelName(g_model.header.name);
      auto labels = getLabels();
      lblselector->setNames(labels);
      //lblselector->clearSelection();
      lblselector->setSelected(modelsLabels.getLabels().size());
      mdlselector->update(0);
      new SelectTemplateFolder([=]() {
      });

    return 0;
  }, BUTTON_BACKGROUND | OPAQUE, textFont);
}

void ModelLabelsWindow::buildBody(FormWindow *window)
{
  #if LCD_W > LCD_H
  // Models List and Filters - Right
  mdlselector = new ModelsPageBody(window, {LABELS_WIDTH + LABELS_LEFT + 3, 5, window->width() - LABELS_WIDTH - 3 - LABELS_LEFT, window->height() - 40});
  auto buttonHolder = new ButtonHolder(window, {LABELS_WIDTH + LABELS_LEFT + 3, window->height() - 33, window->width() - LABELS_WIDTH - 3 - LABELS_LEFT, 25 });
  #else
  mdlselector = new ModelsPageBody(window, {LAY_MARGIN, MODELS_TOP , window->width() - LAY_MARGIN*2, SORTBUTTONS_TOP - MODELS_TOP - LAY_MARGIN});
  auto buttonHolder = new ButtonHolder(window, {LAY_MARGIN, SORTBUTTONS_TOP, window->width() - LAY_MARGIN*2, window->height() - SORTBUTTONS_TOP - LAY_MARGIN});
  #endif

  mdlselector->setLblRefreshFunc(std::bind(&ModelLabelsWindow::labelRefreshRequest, this));
  buttonHolder->setPressHandler([=](int index, ButtonHolder::ButtonInfo *button) {
    if (index == 0) {  // alpha
      sort = button->sortState == 0 ? NAME_ASC : NAME_DES;
    } else {
      sort = button->sortState == 0 ? DATE_ASC : DATE_DES;
    }
    mdlselector->setSortOrder(sort); // Update the list asynchronously
  });
#if LCD_W > LCD_H
  lblselector = new ListBox(window, {LABELS_LEFT, 5, LABELS_WIDTH, window->height() - 10 },
#else
  lblselector = new ListBox(window, {LAY_MARGIN, LAY_MARGIN, window->width() - LAY_MARGIN*2, MODELS_TOP - LAY_MARGIN*2},
#endif
    getLabels());

  lblselector->setMultiSelectMode(true);
  lblselector->setSelected(modelsLabels.filteredLabels());
  updateFilteredLabels(modelsLabels.filteredLabels(), false);

  lblselector->setMultiSelectHandler([=](std::set<uint32_t> selected){
    updateFilteredLabels(selected);
  });

  lblselector->setLongPressHandler([=] () {
    int selected = lblselector->getSelected();
    auto labels = getLabels();

    if (selected < (int)labels.size()) {
      Menu * menu = new Menu(window);
      std::string selecetedlabel = labels.at(selected);
      menu->setTitle(selecetedlabel);
      menu->addLine(STR_NEW_LABEL, [=] () {
        new LabelDialog(window, tmpLabel,
          [=] (std::string label) {
            if(modelsLabels.addLabel(label) >= 0) {
              auto labels = getLabels();
              lblselector->setNames(labels);
            }
          });
        return 0;
      });
      if(selecetedlabel != STR_UNLABELEDMODEL) {
        menu->addLine(STR_RENAME_LABEL, [=] () {
          auto oldLabel = labels[selected];
          strncpy(tmpLabel, oldLabel.c_str(), MAX_LABEL_SIZE);
          tmpLabel[MAX_LABEL_SIZE] = '\0';

          new LabelDialog(window, tmpLabel,
            [=] (std::string newLabel) {
              modelsLabels.renameLabel(oldLabel, newLabel);
              auto labels = getLabels();
              lblselector->setNames(labels);
            });
          return 0;
        });
        menu->addLine(STR_DELETE_LABEL, [=] () {
          auto labelToDelete = labels[selected];
          // if (confirmationDialog(STR_DELETE_LABEL, labelToDelete.c_str())) {
          //   modelsLabels.removeLabel(labelToDelete);
          //   auto labels = getLabels();
          //   lblselector->setNames(labels);
          // }
          return 0;
        });
        if(modelsLabels.getLabels().size() > 1) {
          if(selected != 0) {
            menu->addLine("Move Up", [=] () {
              modelsLabels.moveLabelTo(selected, selected - 1);
              auto labels = getLabels();
              lblselector->setNames(labels);
              return 0;
            });
          }
          if(selected != (int)modelsLabels.getLabels().size() - 1) {
            menu->addLine("Move Down", [=] () {
              modelsLabels.moveLabelTo(selected, selected + 1);
              auto labels = getLabels();
              lblselector->setNames(labels);
              return 0;
            });
          }
        }
      }

    }
  });
}

void ModelLabelsWindow::updateFilteredLabels(std::set<uint32_t> selected, bool setdirty)
{
    LabelsVector sellabels;
    LabelsVector labels = getLabels();
    for(auto sel : selected) {
      if(sel < labels.size())
        sellabels.push_back(labels[sel]);
    }
    if(setdirty) {
      modelsLabels.setFilteredLabels(selected); // Update the model map, will be saved to file
      modelsLabels.setDirty();
    }
    mdlselector->setLabels(sellabels); // Update the list
}

void ModelLabelsWindow::labelRefreshRequest()
{
  auto labels = getLabels();
  lblselector->setNames(labels);
}