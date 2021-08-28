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
#include "opentx.h"
#include "storage/modelslist.h"


#define CATEGORIES_WIDTH               62
#define MODELSEL_W                     62
#define PHASE_ONE_FIRST_LINE           (1 + 1 * FH)

enum ModelSelectMode {
  MODE_SELECT_MODEL,
  MODE_RENAME_CATEGORY,
  MODE_MOVE_MODEL,
};

enum ModelDeleteMode {
  MODE_DELETE_MODEL,
  MODE_DELETE_CATEGORY,
};

struct {
  uint8_t selectMode, deleteMode;
  ModelsCategory * currentCategory = nullptr;
  int currentCategoryIndex;
  int16_t subModelIndex = -1;
  bool modelSelected = false;
  ModelCell * currentModel = nullptr;
  ModelCell * selectedModel = nullptr;
  tmr10ms_t preEnterTime;
  bool preEnterValid = false;
  bool categorySelectMode = false;
  uint16_t categoriesVerticalOffset = 0;
  uint16_t categoriesVerticalPosition = 0;
} modelSelect;

bool eeModelExists(uint8_t id)
{
  int index = 0;

  for (ModelsCategory::iterator it = modelSelect.currentCategory->begin(); it != modelSelect.currentCategory->end(); ++it, ++index) {
    if (id == index){
      return true;
    }
  }

  return false;
}

void setCurrentModel(unsigned int index)
{
  std::list<ModelCell *>::iterator it = modelSelect.currentCategory->begin();
  std::advance(it, index);
  modelSelect.currentModel = *it;
}

void getSelectedModel(unsigned int index)
{
  std::list<ModelCell *>::iterator it = modelSelect.currentCategory->begin();
  std::advance(it, index);
  modelSelect.selectedModel = *it;
}

void setCurrentCategory(unsigned int index)
{
  modelSelect.currentCategoryIndex = index;
  const std::list<ModelsCategory *>& cats = modelslist.getCategories();
  std::list<ModelsCategory *>::const_iterator it = cats.begin();
  std::advance(it, index);
  modelSelect.currentCategory = *it;
  modelSelect.categoriesVerticalPosition = index;
  modelSelect.categoriesVerticalOffset = limit<int>(modelSelect.categoriesVerticalPosition - 4, modelSelect.categoriesVerticalOffset, min<int>(modelSelect.categoriesVerticalPosition, max<int>(0, cats.size() - 5)));
}

void initModelsList()
{
  modelslist.load();

  modelSelect.categoriesVerticalOffset = 0;
  bool found = false;
  int index = 0;
  const std::list<ModelsCategory *>& cats = modelslist.getCategories();
  for (std::list<ModelsCategory *>::const_iterator it = cats.begin(); it != cats.end(); ++it, ++index) {
    if (*it == modelslist.getCurrentCategory()) {
      setCurrentCategory(index);
      found = true;
      break;
    }
  }
  if (!found) {
    setCurrentCategory(0);
  }

  menuVerticalOffset = 0;
  found = false;
  index = 0;
  for (ModelsCategory::iterator it = modelSelect.currentCategory->begin(); it != modelSelect.currentCategory->end(); ++it, ++index) {
    if (*it == modelslist.getCurrentModel()) {
      setCurrentModel(index);
      menuVerticalPosition = index;
      found = true;
      break;
    }
  }
  if (!found) {
    modelslist.addModel(modelSelect.currentCategory, g_eeGeneral.currModelFilename);
    setCurrentModel(0);
    modelslist.setCurrentModel(modelSelect.currentModel);
    modelslist.save();
    storageDirty(EE_GENERAL);
    storageDirty(EE_MODEL);
    storageCheck(true);
  }
}


void onModelSelectMenu(const char * result)
{
  if (result == STR_SELECT_MODEL) {
    setCurrentModel(menuVerticalPosition);
    storageFlushCurrentModel();
    storageCheck(true);
    memcpy(g_eeGeneral.currModelFilename, modelSelect.currentModel->modelFilename, LEN_MODEL_FILENAME);
    modelslist.setCurrentModel(modelSelect.currentModel);
    loadModel(g_eeGeneral.currModelFilename, true);
    modelslist.setCurrentCategory(modelSelect.currentCategory);
    modelslist.save();
#if defined(INTERNAL_MODULE_CRSF)
    if (g_model.moduleData[INTERNAL_MODULE].type == MODULE_TYPE_CROSSFIRE) {
      crossfireTurnOnRf();
      bkregSetStatusFlag(CRSF_SET_MODEL_ID_PENDING);
    }
    else {
      crossfireTurnOffRf(false);
    }
#endif
    storageDirty(EE_GENERAL);
    storageCheck(true);
  }
  else if (result == STR_DELETE_MODEL) {
    getSelectedModel(menuVerticalPosition);
    POPUP_CONFIRMATION(STR_DELETEMODEL, nullptr);
    SET_WARNING_INFO(modelSelect.selectedModel->modelName, LEN_MODEL_NAME, 0);
    modelSelect.deleteMode = MODE_DELETE_MODEL;
  }
  else if (result == STR_CREATE_MODEL) {
    storageCheck(true);
    modelslist.addModel(modelSelect.currentCategory, createModel());
    modelSelect.selectMode = MODE_SELECT_MODEL;
    setCurrentModel(modelSelect.currentCategory->size() - 1 );
    modelslist.setCurrentModel(modelSelect.currentModel);
    modelslist.onNewModelCreated(modelSelect.currentModel, &g_model);
    modelslist.setCurrentCategory(modelSelect.currentCategory);
    modelslist.save();
#if defined(INTERNAL_MODULE_CRSF)
    if (g_model.moduleData[INTERNAL_MODULE].type == MODULE_TYPE_CROSSFIRE) {
      crossfireTurnOnRf();
      bkregSetStatusFlag(CRSF_SET_MODEL_ID_PENDING);
    }
    else {
      crossfireTurnOffRf(false);
    }
#endif
    storageDirty(EE_GENERAL);
    storageCheck(true);
    menuVerticalPosition = modelSelect.currentCategory->size() - 1;
  }
  else if (result == STR_DUPLICATE_MODEL) {
    char duplicatedFilename[LEN_MODEL_FILENAME + 1];
    setCurrentModel(menuVerticalPosition);
    memcpy(duplicatedFilename, modelSelect.currentModel->modelFilename, sizeof(duplicatedFilename));
    if (findNextFileIndex(duplicatedFilename, LEN_MODEL_FILENAME, MODELS_PATH)) {
      sdCopyFile(modelSelect.currentModel->modelFilename, MODELS_PATH, duplicatedFilename, MODELS_PATH);
      ModelCell* dup_model = modelslist.addModel(modelSelect.currentCategory, duplicatedFilename);
      dup_model->fetchRfData();
      menuVerticalPosition = modelSelect.currentCategory->size() - 1;
    }
    else {
      POPUP_WARNING("Invalid File");
    }
  }
  else if (result == STR_MOVE_MODEL) {
    setCurrentModel(menuVerticalPosition);
    modelSelect.selectMode = MODE_MOVE_MODEL;
    modelSelect.categorySelectMode = true;
  }
  else if (result == STR_CREATE_CATEGORY) {
    modelSelect.currentCategory = modelslist.createCategory();
    setCurrentCategory(modelslist.getCategories().size() - 1);
    modelslist.save();
    storageDirty(EE_GENERAL);
    storageCheck(true);
  }
  else if (result == STR_RENAME_CATEGORY) {
    modelSelect.selectMode = MODE_RENAME_CATEGORY;
    s_editMode = EDIT_MODIFY_STRING;
    editNameCursorPos = 0;
  }
  else if (result == STR_DELETE_CATEGORY) {
    if (modelSelect.currentCategory->size() > 0){
      POPUP_WARNING(STR_DELETE_ERROR);
      SET_WARNING_INFO(STR_CAT_NOT_EMPTY, sizeof(TR_CAT_NOT_EMPTY), 0);
    }
    else {
      POPUP_CONFIRMATION(STR_DELETEMODEL, nullptr);
      SET_WARNING_INFO(modelSelect.currentCategory->name, LEN_MODEL_FILENAME, 0);
      modelSelect.deleteMode = MODE_DELETE_CATEGORY;
    }
  }
}

void menuModelSelect(event_t event)
{
  int8_t old_editMode = s_editMode;

  if (warningResult) {
    warningResult = 0;

    if (modelSelect.deleteMode == MODE_DELETE_CATEGORY) {
      TRACE("DELETE CATEGORY");
      modelslist.removeCategory(modelSelect.currentCategory);
      modelslist.save();
      setCurrentCategory(modelSelect.currentCategoryIndex > 0 ? modelSelect.currentCategoryIndex-1 : modelSelect.currentCategoryIndex);
    }
    else if (modelSelect.deleteMode == MODE_DELETE_MODEL) {
      int modelIndex = menuVerticalPosition;
      setCurrentModel(modelIndex);
      modelslist.removeModel(modelSelect.currentCategory, modelSelect.currentModel);
      modelSelect.selectMode = 0;
      menuVerticalPosition = modelSelect.currentCategory->size()-1;
    }
  }

  const std::list<ModelsCategory*>& cats = modelslist.getCategories();

  title(STR_MENUMODELSEL);
  drawScreenIndex(MENU_MODEL_SELECT, DIM(menuTabModel), 0);

  event_t _event_ = ((event==EVT_KEY_BREAK(KEY_ENTER) || event==EVT_KEY_LONG(KEY_ENTER)) ? 0 : event);
  check_submenu_simple(_event_, modelSelect.currentCategory ? modelSelect.currentCategory->size() : 30);

  switch (event) {
    case EVT_ENTRY:
      modelSelect.selectMode = MODE_SELECT_MODEL;
      initModelsList();
      modelSelect.preEnterValid = false;
      modelSelect.categorySelectMode = false;
      break;
    case EVT_KEY_BREAK(KEY_ENTER):
      if (modelSelect.selectMode == MODE_MOVE_MODEL) {
        modelSelect.selectMode = MODE_SELECT_MODEL;
        modelSelect.categorySelectMode = false;
      }
      killEvents(event);
      break;
    case EVT_KEY_FIRST(KEY_EXIT):
      switch (modelSelect.selectMode) {
        case MODE_MOVE_MODEL:
          modelSelect.selectMode = MODE_SELECT_MODEL;
          break;
      }
      break;
    case EVT_KEY_FIRST(KEY_ENTER):
      if (!modelSelect.preEnterValid) {
        modelSelect.preEnterValid = true;
        modelSelect.preEnterTime = get_tmr10ms();
      }
      else {
        modelSelect.preEnterValid = false;
        modelSelect.categorySelectMode = !modelSelect.categorySelectMode;
        // if (modelSelect.categorySelectMode)
        //   AUDIO_CATEGORY_ENABLE();
        // else
        //   AUDIO_CATEGORY_DISABLE();
      }
      break;

#if defined(KEYS_GPIO_REG_PAGE)
    case EVT_KEY_BREAK(KEY_PAGE):
    {
      if (modelSelect.categorySelectMode) {
        if (modelSelect.categoriesVerticalPosition >= cats.size() - 1)
          modelSelect.categoriesVerticalPosition = 0;
        else
          modelSelect.categoriesVerticalPosition += 1;

        if (modelSelect.selectMode == MODE_SELECT_MODEL) {
          setCurrentCategory(modelSelect.categoriesVerticalPosition);
          menuVerticalPosition = 0;
        }
        if (modelSelect.selectMode == MODE_MOVE_MODEL) {
          ModelsCategory * previous_category = modelSelect.currentCategory;
          ModelCell * model = modelSelect.currentModel;
          setCurrentCategory(modelSelect.categoriesVerticalPosition);
          modelslist.moveModel(model, previous_category, modelSelect.currentCategory);
          menuVerticalPosition = modelSelect.currentCategory->size()-1;
        }

        modelSelect.subModelIndex = -1;
        modelSelect.modelSelected = false;
        killEvents(event);
      }
      else {
        chainMenu(menuModelSetup);
      }
      break;
    }

    case EVT_KEY_LONG(KEY_PAGE):
    {
      if (modelSelect.categorySelectMode) {
        if (modelSelect.categoriesVerticalPosition == 0)
          modelSelect.categoriesVerticalPosition = cats.size() - 1;
        else
          modelSelect.categoriesVerticalPosition -= 1;

        if (modelSelect.selectMode == MODE_SELECT_MODEL) {
          setCurrentCategory(modelSelect.categoriesVerticalPosition);
          menuVerticalPosition = 0;
        }
        if (modelSelect.selectMode == MODE_MOVE_MODEL) {
          ModelsCategory * previous_category = modelSelect.currentCategory;
          ModelCell * model = modelSelect.currentModel;
          setCurrentCategory(modelSelect.categoriesVerticalPosition);
          modelslist.moveModel(model, previous_category, modelSelect.currentCategory);
        }

        modelSelect.subModelIndex = -1;
        modelSelect.modelSelected = false;
        killEvents(event);
      }
      else {
        chainMenu(menuTabModel[DIM(menuTabModel)-1]);
        killEvents(event);
      }
      break;
    }
#endif
    case EVT_KEY_LONG(KEY_ENTER):
      if (modelSelect.selectMode == MODE_SELECT_MODEL) {
        killEvents(event);
        if (!modelSelect.modelSelected && !modelSelect.currentCategory->empty()) {
          POPUP_MENU_ADD_ITEM(STR_SELECT_MODEL);
        }
        POPUP_MENU_ADD_ITEM(STR_CREATE_MODEL);
        if (!modelSelect.currentCategory->empty()) {
          if (modelSelect.currentModel) {
            POPUP_MENU_ADD_ITEM(STR_DUPLICATE_MODEL);
            POPUP_MENU_ADD_ITEM(STR_MOVE_MODEL);
          }
          if (!modelSelect.modelSelected) {
            POPUP_MENU_ADD_ITEM(STR_DELETE_MODEL);
          }
        }
        POPUP_MENU_ADD_ITEM(STR_CREATE_CATEGORY);
        POPUP_MENU_ADD_ITEM(STR_RENAME_CATEGORY);
        if (cats.size() > 1) {
          POPUP_MENU_ADD_ITEM(STR_DELETE_CATEGORY);
        }
        POPUP_MENU_START(onModelSelectMenu);
      }
      killEvents(event);
      break;
    default:
      break;
  }

  if (modelSelect.preEnterValid && (get_tmr10ms() - modelSelect.preEnterTime) > 50) {
    modelSelect.preEnterValid = false;
  }

  int index = 0;
  coord_t y = 18;

  drawVerticalScrollbar(CATEGORIES_WIDTH - 1, y - 1, 4 * (FH + 7) - 5, modelSelect.categoriesVerticalOffset, cats.size(), 5);

  // Categories
  for (std::list<ModelsCategory *>::const_iterator it = cats.begin(); it != cats.end(); ++it, ++index) {
    if (index >= modelSelect.categoriesVerticalOffset && index < modelSelect.categoriesVerticalOffset + 5) {
      coord_t y = MENU_HEADER_HEIGHT * 2 + 1 + (index - modelSelect.categoriesVerticalOffset) * FH * 4 / 3;
      uint8_t k = index ;

      if (modelSelect.selectMode == MODE_RENAME_CATEGORY && modelSelect.currentCategory == *it) {
        lcdDrawSolidFilledRect(9, y, MODELSEL_W - 1 - 9, 7);
        lcdDrawRect(8, y - 1, MODELSEL_W - 1 - 7, 9,  DOTTED);
        editName(4, y, modelSelect.currentCategory->name, sizeof(modelSelect.currentCategory->name), event, 1, 0, old_editMode);
        if (s_editMode == 0 || event == EVT_KEY_BREAK(KEY_EXIT)) {
          modelslist.save();
          modelSelect.selectMode = MODE_SELECT_MODEL;
        }
      }
      else {
        lcdDrawSizedText(2, y, (*it)->name, strlen((*it)->name),  ((modelSelect.categoriesVerticalPosition == k) ? INVERS : 0));
      }
      if (modelSelect.selectMode == MODE_MOVE_MODEL && modelSelect.categoriesVerticalPosition == k) {
        lcdDrawSolidFilledRect(9, y, MODELSEL_W - 1 - 9, 7);
        lcdDrawRect(8, y - 1, MODELSEL_W - 1 - 7, 9, modelSelect.selectMode == COPY_MODE ? SOLID : DOTTED);
      }
    }
  }

  // Models
  index = 0;
  bool selected = false;
  bool current = false;

  for (uint8_t i = 0; i < modelSelect.currentCategory->size(); i++) {
    coord_t y = MENU_HEADER_HEIGHT*2 + 1 + i*FH;
    uint8_t k = i+menuVerticalOffset;

    if (k >= modelSelect.currentCategory->size())
      break;

    std::list<ModelCell *>::iterator it = modelSelect.currentCategory->begin();
    std::advance(it, k);

    selected = ((modelSelect.selectMode == MODE_SELECT_MODEL || modelSelect.selectMode == MODE_MOVE_MODEL) && k == menuVerticalPosition);
    current = !strncmp((*it)->modelFilename, g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME);

    if (current) {
      lcdDrawChar(9 * FW + 11, y, '*');
      lcdDrawText(9 * FW + 18, y, g_model.header.name, LEADING0 | ((selected) ? INVERS : 0));
      modelSelect.subModelIndex = k;
    }
    else {
      lcdDrawText(9 * FW + 18, y, (*it)->modelName, LEADING0 | ((selected) ? INVERS : 0));
    }

    y += 16;

    if (selected) {
      lcdDrawText(5, LCD_H - FH - 1, (*it)->modelFilename, SMLSIZE);
    }
  }

  if (modelSelect.subModelIndex == menuVerticalPosition) {
    modelSelect.modelSelected = true;
  }
  else {
    modelSelect.modelSelected = false;
  }
}

