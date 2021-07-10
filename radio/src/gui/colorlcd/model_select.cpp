/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include <algorithm>
#include "model_select.h"
#include "opentx.h"
#include "storage/modelslist.h"
#include "libopenui.h"

#if LCD_W > LCD_H
constexpr int MODEL_CELLS_PER_LINE = 3;
#else
constexpr int MODEL_CELLS_PER_LINE = 2;
#endif

constexpr coord_t MODEL_CELL_PADDING = 6;

constexpr coord_t MODEL_SELECT_CELL_WIDTH =
    (LCD_W - (MODEL_CELLS_PER_LINE + 1) * MODEL_CELL_PADDING) /
    MODEL_CELLS_PER_LINE;

constexpr coord_t MODEL_SELECT_CELL_HEIGHT = 92;

constexpr coord_t MODEL_IMAGE_WIDTH  = MODEL_SELECT_CELL_WIDTH;
constexpr coord_t MODEL_IMAGE_HEIGHT = 72;


enum ModelSelectMode {
  MODE_SELECT_MODEL,
  MODE_RENAME_CATEGORY,
  MODE_MOVE_MODEL,
};

enum ModelDeleteMode {
  MODE_DELETE_MODEL,
  MODE_DELETE_CATEGORY,
};

uint8_t selectMode, deleteMode;

#if 0
void setCurrentModel(unsigned int index)
{
  auto it = currentCategory->begin();
  std::advance(it, index);
  currentModel = *it;
}
#endif

#if 0
void setCurrentCategory(unsigned int index)
{
  currentCategoryIndex = index;
  auto it = modelslist.getCategories().begin();
  std::advance(it, index);
  currentCategory = *it;
  categoriesVerticalPosition = index;
  categoriesVerticalOffset = limit<int>(categoriesVerticalPosition-4, categoriesVerticalOffset, min<int>(categoriesVerticalPosition, max<int>(0, modelslist.getCategories().size()-5)));
  /*if (currentCategory->size() > 0)
    setCurrentModel(0);
  else
    currentModel = nullptr;*/
}
#endif

class ModelButton : public Button
{
 public:
  ModelButton(FormGroup *parent, const rect_t &rect, ModelCell *modelCell) :
      Button(parent, rect), modelCell(modelCell)
  {
    load();
  }

  ~ModelButton()
  {
    if (buffer) { delete buffer; }
  }
  
  void load()
  {
    uint8_t version;

    PACK(struct {
      ModelHeader header;
      TimerData timers[MAX_TIMERS];
    })
    partialModel;
    const char *error = nullptr;

    if (strncmp(modelCell->modelFilename, g_eeGeneral.currModelFilename,
                LEN_MODEL_FILENAME) == 0) {
      memcpy(&partialModel.header, &g_model.header, sizeof(partialModel));
      version = EEPROM_VER;
    } else {
      error =
          readModel(modelCell->modelFilename, (uint8_t *)&partialModel.header,
                    sizeof(partialModel), &version);
    }

    if (!error) {
      if (modelCell->modelName[0] == '\0' &&
          partialModel.header.name[0] != '\0') {
        
        if (version == 219) {
          int len = (int)sizeof(partialModel.header.name);
          char* str = partialModel.header.name;
          for (int i=0; i < len; i++) {
            str[i] = zchar2char(str[i]);
          }
          // Trim string
          while(len > 0 && str[len-1]) {
            if (str[len - 1] != ' ' && str[len - 1] != '\0') break;
            str[--len] = '\0';
          }
        }
        modelCell->setModelName(partialModel.header.name);
      }
    }

    delete buffer;
    buffer = new BitmapBuffer(BMP_RGB565, width(), height());
    if (buffer == nullptr) {
      return;
    }
    buffer->clear(FIELD_BGCOLOR);

    if (error) {
      buffer->drawText(width() / 2, 2, "(Invalid Model)",
                       DEFAULT_COLOR | CENTERED);
    } else {
      GET_FILENAME(filename, BITMAPS_PATH, partialModel.header.bitmap, "");
      const BitmapBuffer *bitmap = BitmapBuffer::loadBitmap(filename);
      if (bitmap) {
        buffer->drawScaledBitmap(bitmap, 0, 0, width(), height());
        delete bitmap;
      } else {
        buffer->drawText(width() / 2, 56, "(No Picture)",
                         FONT(XXS) | DEFAULT_COLOR | CENTERED);
      }
    }
  }

  void paint(BitmapBuffer *dc) override
  {
    FormField::paint(dc);

    if (buffer)
      dc->drawBitmap(0, 0, buffer);

    if (modelCell == modelslist.getCurrentModel()) {
      dc->drawSolidFilledRect(0, 0, width(), 20, HIGHLIGHT_COLOR);
      dc->drawSizedText(width() / 2, 2, modelCell->modelName,
                        LEN_MODEL_NAME,
                        DEFAULT_COLOR | CENTERED);
    } else {
      LcdFlags textColor;
      // if (hasFocus()) {
      //   dc->drawFilledRect(0, 0, width(), 20, SOLID, FOCUS_BGCOLOR, 5);
      //   textColor = FOCUS_COLOR;
      // }
      // else {
        dc->drawFilledRect(0, 0, width(), 20, SOLID, FIELD_BGCOLOR, 5);
        textColor = DEFAULT_COLOR;
      // }

      dc->drawSizedText(width() / 2, 2, modelCell->modelName,
                        LEN_MODEL_NAME,
                        textColor | CENTERED);
    }

    if (!hasFocus()) {
      dc->drawSolidRect(0, 0, width(), height(), 1, FIELD_FRAME_COLOR);
    } else {
      dc->drawSolidRect(0, 0, width(), height(), 2, FOCUS_BGCOLOR);
    }
  }

  const char *modelFilename() { return modelCell->modelFilename; }

 protected:
  ModelCell *modelCell;
  BitmapBuffer *buffer = nullptr;
};

class ModelCategoryPageBody : public FormWindow
{
 public:
  ModelCategoryPageBody(FormWindow *parent, const rect_t &rect,
                        ModelsCategory *category) :
      FormWindow(parent, rect, FORM_FORWARD_FOCUS), category(category)
  {
    update();
  }

  void update(int selected = -1)
  {
    clear();

    if (selected < 0) {
      auto model = modelslist.getCurrentModel();
      selected = category->getModelIndex(model);
      if (selected < 0) selected = 0;
    }

    int index = 0;
    coord_t y = MODEL_CELL_PADDING;
    coord_t x = MODEL_CELL_PADDING;

    ModelButton* selectButton = nullptr;
    for (auto &model : *category) {
      auto button = new ModelButton(
          this, {x, y, MODEL_SELECT_CELL_WIDTH, MODEL_SELECT_CELL_HEIGHT},
          model);
      button->setPressHandler([=]() -> uint8_t {
        if (button->hasFocus()) {
          Menu *menu = new Menu(parent);
          if (model != modelslist.getCurrentModel()) {
            menu->addLine(STR_SELECT_MODEL, [=]() {
              // we store the latest changes if any
              storageFlushCurrentModel();
              storageCheck(true);
              memcpy(g_eeGeneral.currModelFilename, model->modelFilename,
                     LEN_MODEL_FILENAME);
              loadModel(g_eeGeneral.currModelFilename, false);
              storageDirty(EE_GENERAL);
              storageCheck(true);

              modelslist.setCurrentModel(model);
              modelslist.setCurrentCategory(category);
              checkAll();
              this->onEvent(EVT_KEY_FIRST(KEY_EXIT));
            });
          }
          menu->addLine(STR_CREATE_MODEL, getCreateModelAction());
          menu->addLine(STR_DUPLICATE_MODEL, [=]() {
            char duplicatedFilename[LEN_MODEL_FILENAME + 1];
            memcpy(duplicatedFilename, model->modelFilename,
                   sizeof(duplicatedFilename));
            if (findNextFileIndex(duplicatedFilename, LEN_MODEL_FILENAME,
                                  MODELS_PATH)) {
              sdCopyFile(model->modelFilename, MODELS_PATH, duplicatedFilename,
                         MODELS_PATH);
              modelslist.addModel(category, duplicatedFilename);
              update(index);
            } else {
              POPUP_WARNING("Invalid File");
            }
          });
          // menu->addLine(STR_MOVE_MODEL);
          if (model != modelslist.getCurrentModel()) {
            menu->addLine(STR_DELETE_MODEL, [=]() {
              new ConfirmDialog(
                  parent, STR_DELETE_MODEL,
                  std::string(model->modelName, sizeof(model->modelName))
                      .c_str(),
                  [=] {
                    modelslist.removeModel(category, model);
                    update(index < (int)category->size() - 1 ? index : index - 1);
                  });
            });
          }
        } else {
          button->setFocus(SET_FOCUS_DEFAULT);
        }
        return 1;
      });

      if (selected == index) {
        selectButton = button;
      }

      index++;

#if LCD_W > LCD_H
      if (index % MODEL_CELLS_PER_LINE == 0) {
        x = MODEL_CELL_PADDING;
        y += MODEL_SELECT_CELL_HEIGHT + MODEL_CELL_PADDING;
      } else {
        x += MODEL_CELL_PADDING + MODEL_SELECT_CELL_WIDTH;
      }
#else
      y += MODEL_SELECT_CELL_HEIGHT + MODEL_CELL_PADDING;
#endif
    }

    if (index % MODEL_CELLS_PER_LINE != 0) {
      y += MODEL_SELECT_CELL_HEIGHT + MODEL_CELL_PADDING;
    }
    setInnerHeight(y);

    if (category->empty()) {
      setFocus();
    } else if (selectButton) {
      selectButton->setFocus();
    }
  }

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override
  {
    if (event == EVT_KEY_BREAK(KEY_ENTER)) {
      Menu *menu = new Menu(this);
      menu->addLine(STR_CREATE_MODEL, getCreateModelAction());
      // TODO: create category?
    } else {
      FormWindow::onEvent(event);
    }
  }
#endif

  void setFocus(uint8_t flag = SET_FOCUS_DEFAULT,
                Window *from = nullptr) override
  {
    if (category->empty()) {
      // use Window::setFocus() to avoid forwarding focus to nowhere
      // this crashes currently in libopenui
      Window::setFocus(flag, from);
    } else {
      FormWindow::setFocus(flag, from);
    }
  }

 protected:
  ModelsCategory *category;

  std::function<void(void)> getCreateModelAction()
  {
    return [=]() {
      storageCheck(true);
      modelslist.setCurrentModel(modelslist.addModel(category, createModel()));
      update(category->size() - 1);
    };
  }
};

class ModelCategoryPage : public PageTab
{
 public:
  explicit ModelCategoryPage(ModelsCategory *category) :
      PageTab(category->name, ICON_MODEL_SELECT_CATEGORY), category(category)
  {
  }

 protected:
  ModelsCategory *category;

  void build(FormWindow *window) override
  {
    new ModelCategoryPageBody(
        window, {0, 0, LCD_W, window->height()},
        category);
  }
};

ModelSelectMenu::ModelSelectMenu():
  TabsGroup(ICON_MODEL_SELECT)
{
  modelslist.load();

  TRACE("TabsGroup: %p", this);
  for (auto category: modelslist.getCategories()) {
    addTab(new ModelCategoryPage(category));
  }

  int idx = modelslist.getCurrentCategoryIdx();
  if (idx >= 0) {
    setCurrentTab(idx);
  }
}
