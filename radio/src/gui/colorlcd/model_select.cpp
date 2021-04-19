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

#define CATEGORIES_WIDTH               120
#define MODELS_LEFT                    123
#define MODELS_COLUMN_WIDTH            174

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
//ModelsList modelslist;

//ModelsCategory * currentCategory;
//int currentCategoryIndex;
//ModelCell * currentModel;

/*
void drawCategory(BitmapBuffer * dc, coord_t y, const char * name, bool selected)
{
  if (selected) {
    lcdDrawSolidFilledRect(1, y-INVERT_VERT_MARGIN, CATEGORIES_WIDTH-10, INVERT_LINE_HEIGHT+2, FOCUS_BGCOLOR);
    lcdDrawText(6, y, name, DEFAULT_COLOR | INVERS);
  }
  else {
    lcdDrawText(6, y, name, DEFAULT_COLOR);
  }
}

void drawModel(coord_t x, coord_t y, ModelCell * model, bool current, bool selected)
{
  lcd->drawBitmap(x+1, y+1, model->getBuffer());
  if (current) {
    lcd->drawBitmapPattern(x+66, y+43, LBM_ACTIVE_MODEL, TITLE_BGCOLOR);
  }
  if (selected) {
    lcdDrawSolidRect(x, y, MODELCELL_WIDTH+2, MODELCELL_HEIGHT+2, 1, TITLE_BGCOLOR);
    drawShadow(x, y, MODELCELL_WIDTH+2, MODELCELL_HEIGHT+2);
    if (selectMode == MODE_MOVE_MODEL) {
      lcd->drawMask(x+MODELCELL_WIDTH+2-modelselModelMoveBackground->width(), y, modelselModelMoveBackground, TITLE_BGCOLOR);
      lcd->drawMask(x+MODELCELL_WIDTH+2-modelselModelMoveBackground->width()+12, y+5, modelselModelMoveIcon, TEXT_BGCOLOR);
    }
  }
}
*/

//uint16_t categoriesVerticalOffset = 0;
//uint16_t categoriesVerticalPosition = 0;
#define MODEL_INDEX()       (menuVerticalPosition*2+menuHorizontalPosition)

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

class ModelButton: public Button {
  public:
    ModelButton(FormGroup * parent, const rect_t & rect, ModelCell * modelCell, Window * footer) :
      Button(parent, rect),
      modelCell(modelCell)
    {
      load();
    }

    void load()
    {
      uint8_t version;

      PartialModel partialModel;
      const char * error = nullptr;

      if (strncmp(modelCell->modelFilename, g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME) == 0) {
        memcpy(&partialModel.header, &g_model.header, sizeof(partialModel));
      }
      else {
        error = readModel(modelCell->modelFilename, (uint8_t *)&partialModel.header, sizeof(partialModel), &version);
        // LEN_BITMAP_NAME has now 4 bytes more
        if (version <= 218) {
          memmove(partialModel.timers, &(partialModel.header.bitmap[10]), sizeof(TimerData)*MAX_TIMERS);
          memclear(&(partialModel.header.bitmap[10]), 4);
        }
      }

//      if (modelCell->modelName[0] == '\0' && !error)
//        setModelName(partialModel.header.name); // resets buffer!!!

      buffer = new BitmapBuffer(BMP_RGB565, MODELCELL_WIDTH, MODELCELL_HEIGHT);
      if (buffer == nullptr) {
        return;
      }
      buffer->clear(DEFAULT_BGCOLOR);

      if (error) {
        buffer->drawText(5, 2, "(Invalid Model)", DEFAULT_COLOR);
        buffer->drawBitmapPattern(5, 23, LBM_LIBRARY_SLOT, DEFAULT_COLOR);
      }
      else {
        char timerName[LEN_TIMER_STRING];
        buffer->drawSizedText(5, 2, modelCell->modelName, LEN_MODEL_NAME, FONT(XS) | DEFAULT_COLOR);
        getTimerString(timerName, 0);
        for (auto & timer : partialModel.timers) {
          if (timer.mode > 0 && timer.persistent) {
            getTimerString(timerName, timer.value);
            break;
          }
        }
        buffer->drawText(101, 40, timerName, DEFAULT_COLOR);
        for (int i = 0; i < 4; i++) {
          buffer->drawBitmapPattern(104+i*11, 25, LBM_SCORE0, TITLE_BGCOLOR);
        }
        GET_FILENAME(filename, BITMAPS_PATH, partialModel.header.bitmap, "");
        const BitmapBuffer * bitmap = BitmapBuffer::loadBitmap(filename);
        if (bitmap) {
          buffer->drawScaledBitmap(bitmap, 5, 24, 56, 32);
          delete bitmap;
        }
        else {
          buffer->drawBitmapPattern(5, 23, LBM_LIBRARY_SLOT, DEFAULT_COLOR);
        }
      }
      buffer->drawSolidHorizontalLine(5, 19, 143, LINE_COLOR);
    }

    void paint(BitmapBuffer * dc) override
    {
      FormField::paint(dc);
      dc->drawBitmap(10, 2, buffer);
      if (modelCell == modelslist.getCurrentModel()) {
        dc->drawBitmapPattern(112, 71, LBM_ACTIVE_MODEL, TITLE_BGCOLOR);
      }
    }

    const char * modelFilename()
    {
      return modelCell->modelFilename;
    }

  protected:
    ModelCell * modelCell;
    BitmapBuffer * buffer = nullptr;
};

class ModelSelectFooter: public Window {
  public:
    explicit ModelSelectFooter(Window * parent):
      Window(parent,  {0, parent->height() - MODEL_SELECT_FOOTER_HEIGHT, LCD_W, MODEL_SELECT_FOOTER_HEIGHT}, OPAQUE)
    {
    }

    void setCurrentModel(ModelCell * model)
    {
      currentModel = model;
      invalidate();
    }

    void paint(BitmapBuffer * dc) override
    {
      dc->drawSolidFilledRect(0, 0, width(), height(), DISABLE_COLOR);
      uint32_t size = sdGetSize() / 100;
      coord_t x = 7;
      if (modelselSdFreeBitmap) {
        dc->drawMask(7, 4, modelselSdFreeBitmap, DEFAULT_COLOR);
        x += modelselSdFreeBitmap->width() + 3;
      }
      x = dc->drawNumber(x, 3, size, PREC1|FONT(XS), 0, nullptr, "GB");
      x += 20;
      if (modelselModelQtyBitmap) {
        dc->drawMask(x, 4, modelselModelQtyBitmap, DEFAULT_COLOR);
        x += modelselModelQtyBitmap->width() + 3;
      }
      x = dc->drawNumber(x, 3, modelslist.getModelsCount(), FONT(XS));
      if (currentModel) {
        x += 20;
        if (modelselModelNameBitmap) {
          dc->drawMask(x, 4, modelselModelNameBitmap, DEFAULT_COLOR);
          x += modelselModelNameBitmap->width() + 3;
        }
        dc->drawText(x, 3, currentModel->modelFilename, FONT(XS) | DEFAULT_COLOR);
      }
    }

  protected:
    ModelCell * currentModel = nullptr;
};

#if defined(PCBX10)
constexpr coord_t MODEL_SELECT_CELL_WIDTH = (LCD_W - 3 * PAGE_PADDING) / 2;
#else
constexpr coord_t MODEL_SELECT_CELL_WIDTH = LCD_W - 2 * PAGE_PADDING;
#endif
constexpr coord_t MODEL_SELECT_CELL_HEIGHT = 94;

class ModelCategoryPageBody: public FormWindow {
  public:
    ModelCategoryPageBody(FormWindow * parent, const rect_t & rect, ModelsCategory * category, ModelSelectFooter * footer):
      FormWindow(parent, rect, FORM_FORWARD_FOCUS),
      category(category),
      footer(footer)
    {
      update();
    }

    void update(int selected = 0)
    {
      clear();

      int index = 0;
      coord_t y = PAGE_PADDING;
      coord_t x = PAGE_PADDING;
      for (auto & model: * category) {
        auto button = new ModelButton(this, {x, y, MODEL_SELECT_CELL_WIDTH, MODEL_SELECT_CELL_HEIGHT}, model, nullptr);

        button->setFocusHandler([=](bool active) {
          if (active) {
            footer->setCurrentModel(model);
          }
        });

        button->setPressHandler([=]() -> uint8_t {
            if (button->hasFocus()) {
              Menu * menu = new Menu(parent);
              if (model != modelslist.getCurrentModel()) {
                menu->addLine(STR_SELECT_MODEL, [=]() {
                    // we store the latest changes if any
                    storageFlushCurrentModel();
                    storageCheck(true);
                    memcpy(g_eeGeneral.currModelFilename, model->modelFilename, LEN_MODEL_FILENAME);
                    loadModel(g_eeGeneral.currModelFilename, false);
                    storageDirty(EE_GENERAL);
                    storageCheck(true);

                    modelslist.setCurrentModel(model);
                    update(); // modelslist.getModelIndex(modelCell));
                });
              }
              menu->addLine(STR_CREATE_MODEL, getCreateModelAction());
              menu->addLine(STR_DUPLICATE_MODEL, [=]() {
                  char duplicatedFilename[LEN_MODEL_FILENAME + 1];
                  memcpy(duplicatedFilename, model->modelFilename, sizeof(duplicatedFilename));
                  if (findNextFileIndex(duplicatedFilename, LEN_MODEL_FILENAME, MODELS_PATH)) {
                    sdCopyFile(model->modelFilename, MODELS_PATH, duplicatedFilename, MODELS_PATH);
                    modelslist.addModel(category, duplicatedFilename);
                    update(index);
                  }
                  else {
                    POPUP_WARNING("Invalid File");
                  }
              });
              // menu->addLine(STR_MOVE_MODEL);
              if (model != modelslist.getCurrentModel()) {
                menu->addLine(STR_DELETE_MODEL, [=]() {
                  new ConfirmDialog(parent, STR_DELETE_MODEL, std::string(model->modelName, sizeof(model->modelName)).c_str(), [=] {
                      modelslist.removeModel(category, model);
                      update(index > 0 ? index - 1 : 0);
                  });
                });
              }
            }
            else {
              button->setFocus(SET_FOCUS_DEFAULT);
            }
            return 1;
        });

        if (selected == index) {
          button->setFocus(SET_FOCUS_DEFAULT);
          footer->setCurrentModel(model);
        }

        index++;
#if LCD_W >= 480
        if (x == PAGE_PADDING) {
          x = PAGE_PADDING + MODEL_SELECT_CELL_WIDTH + PAGE_PADDING;
        }
        else {
          x = PAGE_PADDING;
          y += MODEL_SELECT_CELL_HEIGHT + PAGE_PADDING;
        }
#else
        y += MODEL_SELECT_CELL_HEIGHT + PAGE_PADDING;
#endif
      }

      setInnerHeight(y);

      if (category->empty()) {
        setFocus();
      }
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override
    {
      if (event == EVT_KEY_BREAK(KEY_ENTER)) {
        Menu * menu = new Menu(this);
        menu->addLine(STR_CREATE_MODEL, getCreateModelAction());
        //TODO: create category?
      }
      else {
        FormWindow::onEvent(event);
      }
    }
#endif

    void setFocus(uint8_t flag = SET_FOCUS_DEFAULT, Window * from = nullptr) override
    {
      if (category->empty()) {
        // use Window::setFocus() to avoid forwarding focus to nowhere
        // this crashes currently in libopenui
        Window::setFocus(flag, from);
      }
      else {
        FormWindow::setFocus(flag, from);
      }
    }

  
  protected:
    ModelsCategory * category;
    ModelSelectFooter * footer;

    std::function<void(void)> getCreateModelAction()
    {
      return [=]() {
        storageCheck(true);
        modelslist.setCurrentModel(modelslist.addModel(category, createModel()));
#if defined(LUA)
        // chainMenu(menuModelWizard);
#endif
        update(category->size() - 1);
      };
    }

};

class ModelCategoryPage: public PageTab {
  public:
    explicit ModelCategoryPage(ModelsCategory * category) :
      PageTab(category->name, ICON_MODEL),
      category(category)
    {
    }

  protected:
    ModelsCategory * category;

    void build(FormWindow * window) override
    {
      auto footer = new ModelSelectFooter(window);
      new ModelCategoryPageBody(window, {0, 0, LCD_W, window->height() - MODEL_SELECT_FOOTER_HEIGHT}, category, footer);
    }
};


ModelSelectMenu::ModelSelectMenu():
  TabsGroup(ICON_MODEL)
{
  modelslist.load();

  TRACE("TabsGroup: %p", this);
  for (auto category: modelslist.getCategories()) {
    addTab(new ModelCategoryPage(category));
  }
}
