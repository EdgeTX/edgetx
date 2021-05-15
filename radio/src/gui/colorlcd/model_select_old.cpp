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

#include "edgetx.h"
#include "modelslist.h"

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

ModelsCategory * currentCategory;
int currentCategoryIndex;
ModelCell * currentModel;

void drawCategory(coord_t y, const char * name, bool selected)
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
    if (selectMode == MODE_MOVE_MODEL && modelselModelMoveBackground) {
      lcd->drawMask(x+MODELCELL_WIDTH+2-modelselModelMoveBackground->width(), y, modelselModelMoveBackground, TITLE_BGCOLOR);
      lcd->drawMask(x+MODELCELL_WIDTH+2-modelselModelMoveBackground->width()+12, y+5, modelselModelMoveIcon, DEFAULT_BGCOLOR);
    }
  }
}

uint16_t categoriesVerticalOffset = 0;
uint16_t categoriesVerticalPosition = 0;
#define MODEL_INDEX()       (menuVerticalPosition*2+menuHorizontalPosition)

void setCurrentModel(unsigned int index)
{
  std::list<ModelCell *>::iterator it = currentCategory->begin();
  std::advance(it, index);
  currentModel = *it;
  menuVerticalPosition = index / 2;
  menuHorizontalPosition = index & 1;
  menuVerticalOffset = limit<int>(menuVerticalPosition-2, menuVerticalOffset, min<int>(menuVerticalPosition, max<int>(0, (currentCategory->size()-7)/2)));
}

void setCurrentCategory(unsigned int index)
{
  currentCategoryIndex = index;
  const std::list<ModelsCategory *>& cats = modelslist.getCategories();
  std::list<ModelsCategory *>::const_iterator it = cats.begin();
  std::advance(it, index);
  currentCategory = *it;
  categoriesVerticalPosition = index;
  categoriesVerticalOffset = limit<int>(categoriesVerticalPosition-4, categoriesVerticalOffset, min<int>(categoriesVerticalPosition, max<int>(0, cats.size()-5)));
  if (currentCategory->size() > 0)
    setCurrentModel(0);
  else
    currentModel = nullptr;
}

#if defined(LUA)
#define MAX_WIZARD_NAME_LEN            (sizeof(WIZARD_PATH)+20)
#define WIZARD_SPACING                 40
#define WIZARD_LEFT_SPACING            30
#define WIZARD_ICON_X                  80
#define WIZARD_ICON_Y                  110
#define WIZARD_TEXT_Y                  195

uint8_t getWizardCount()
{
  uint8_t wizCnt=0;
  DIR dir;
  static FILINFO fno;

  FRESULT res = f_opendir(&dir, WIZARD_PATH);
  if (res == FR_OK) {
    for (;;) {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0) {
        break;
      }
      if (fno.fattrib & AM_DIR) {
        wizCnt++;
      }
    }
  }
  f_closedir(&dir);
  return wizCnt;
}

bool menuModelWizard(event_t event)
{
  static uint8_t wizardSelected = 0;
  static uint8_t wizardCnt = getWizardCount();
  bool executeMe = false;
  uint8_t first = 0;
  DIR dir;
  static FILINFO fno;
  char wizpath[MAX_WIZARD_NAME_LEN];

  if (wizardCnt == 0) {
    chainMenu(menuModelSelect);
    return false;
  }

  switch(event) {
   case 0:
     // no need to refresh the screen
     return false;

  case EVT_KEY_FIRST(KEY_EXIT):
    chainMenu(menuModelSelect);
    return false;

  case EVT_KEY_BREAK(KEY_ENTER):
    executeMe = true;
    break;

  case EVT_ROTARY_RIGHT:
    if (wizardSelected < wizardCnt-1) {
      wizardSelected++;
    }
    if (wizardSelected > 3) {
      first = wizardSelected - 3;
    }
    break;

  case EVT_ROTARY_LEFT:
    if (wizardSelected != 0) {
      wizardSelected--;
    }
    if(wizardSelected < first)
    {
      first = wizardSelected;
    }
    break;
  }
  strcpy(wizpath, WIZARD_PATH);
  strcpy(&wizpath[sizeof(WIZARD_PATH)-1], "/");
  lcdDrawSolidFilledRect(0, 0, LCD_W, LCD_H, DEFAULT_BGCOLOR);
  lcd->drawBitmap(0, 0, modelselWizardBackground);
  FRESULT res = f_opendir(&dir, WIZARD_PATH);
  if (res == FR_OK) {
    for (uint8_t wizidx=0;;wizidx++) {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0) {
        break;
      }
      if (fno.fattrib & AM_DIR) {
        if((wizidx >= first) && (wizidx < (first+4))) {
          uint16_t x = WIZARD_LEFT_SPACING + (wizidx - first) * (WIZARD_SPACING + WIZARD_ICON_X);
          strcpy(&wizpath[sizeof(WIZARD_PATH)], fno.fname);
          strcpy(&wizpath[sizeof(WIZARD_PATH) + strlen(fno.fname)], "/icon.png");
          BitmapBuffer * background = BitmapBuffer::load(wizpath);
          lcd->drawBitmap(x, WIZARD_ICON_Y, background);
          if(wizidx == wizardSelected ) {
            if (wizardSelected < 5) {
              lcdDrawRect(x, WIZARD_ICON_Y, 85, 130, 2, SOLID, MAINVIEW_GRAPHICS_COLOR_INDEX);
              lcdDrawRect(x+5, WIZARD_TEXT_Y, 75, 4, 2, SOLID, MAINVIEW_GRAPHICS_COLOR_INDEX);
            }
            if (executeMe) {
              strcpy(&wizpath[sizeof(WIZARD_PATH)+strlen(fno.fname)], "/wizard.lua");
              if (isFileAvailable(wizpath)) {
                wizpath[sizeof(WIZARD_PATH) + strlen(fno.fname)] = 0;
                f_chdir(wizpath);
                luaExec(WIZARD_NAME);
              }
            }
          }
          delete background;
        }
      }
    }
    f_closedir(&dir);
  }
  return true;
}
#endif

void onDeleteModelConfirm(const char * result)
{
  if (result == STR_OK) {
    if (deleteMode == MODE_DELETE_CATEGORY) {
      modelslist.removeCategory(currentCategory);
      modelslist.save();
      setCurrentCategory(currentCategoryIndex > 0 ? currentCategoryIndex-1 : currentCategoryIndex);
    }
    else if (deleteMode == MODE_DELETE_MODEL){
      int modelIndex = MODEL_INDEX();
      modelslist.removeModel(currentCategory, currentModel);
      s_copyMode = 0;
      pushEvent(EVT_REFRESH);
      if (modelIndex > 0) {
        modelIndex--;
      }
      setCurrentModel(modelIndex);
    }
  }
}

void onModelSelectMenu(const char * result)
{
  if (result == STR_SELECT_MODEL) {
    if (!g_eeGeneral.disableRssiPoweroffAlarm) {
      if (!confirmModelChange())
        return;
    }

    // we store the latest changes if any
    storageFlushCurrentModel();
    storageCheck(true);
    memcpy(g_eeGeneral.currModelFilename, currentModel->modelFilename, LEN_MODEL_FILENAME);
    modelslist.setCurrentModel(currentModel);
    modelslist.setCurrentCategory(currentCategory);
    loadModel(g_eeGeneral.currModelFilename, true);
    storageDirty(EE_GENERAL);
    storageCheck(true);
    chainMenu(menuMainView);
  }
  else if (result == STR_DELETE_MODEL) {
    POPUP_CONFIRMATION(STR_DELETEMODEL, onDeleteModelConfirm);
    SET_WARNING_INFO(currentModel->modelName, LEN_MODEL_NAME, 0);
    deleteMode = MODE_DELETE_MODEL;
  }
  else if (result == STR_CREATE_MODEL) {
    if (!g_eeGeneral.disableRssiPoweroffAlarm) {
      if (!confirmModelChange())
        return;
    }
    storageCheck(true);
    modelslist.addModel(currentCategory, createModel());
    selectMode = MODE_SELECT_MODEL;
    setCurrentModel(currentCategory->size() - 1);
    modelslist.setCurrentModel(currentModel);
    modelslist.setCurrentCategory(currentCategory);
    modelslist.onNewModelCreated(currentModel, &g_model);
#if defined(LUA)
    chainMenu(menuModelWizard);
#endif
  }
  else if (result == STR_DUPLICATE_MODEL) {
    char duplicatedFilename[LEN_MODEL_FILENAME+1];
    memcpy(duplicatedFilename, currentModel->modelFilename, sizeof(duplicatedFilename));
    if (findNextFileIndex(duplicatedFilename, LEN_MODEL_FILENAME, MODELS_PATH)) {
      sdCopyFile(currentModel->modelFilename, MODELS_PATH, duplicatedFilename, MODELS_PATH);
      ModelCell * dup_model = modelslist.addModel(currentCategory, duplicatedFilename);
      dup_model->fetchRfData();
      setCurrentModel(currentCategory->size() - 1);
    }
    else {
      POPUP_WARNING(STR_INVALID_FILE);
    }
  }
  else if (result == STR_MOVE_MODEL) {
    selectMode = MODE_MOVE_MODEL;
  }
  else if (result == STR_CREATE_CATEGORY) {
    currentCategory = modelslist.createCategory();
    setCurrentCategory(modelslist.getCategories().size() - 1);
  }
  else if (result == STR_RENAME_CATEGORY) {
    selectMode = MODE_RENAME_CATEGORY;
    s_editMode = EDIT_MODIFY_STRING;
    editNameCursorPos = 0;
  }
  else if (result == STR_DELETE_CATEGORY) {
    if (currentCategory->size() > 0){
      POPUP_WARNING(STR_DELETE_ERROR);
      SET_WARNING_INFO(STR_CAT_NOT_EMPTY, sizeof(TR_CAT_NOT_EMPTY), 0);
    }
    else {
      POPUP_CONFIRMATION(STR_DELETEMODEL, onDeleteModelConfirm);
      SET_WARNING_INFO(currentCategory->name, LEN_MODEL_FILENAME, 0);
      deleteMode = MODE_DELETE_CATEGORY;
    }
  }
}

void initModelsList()
{
  modelslist.load();

  categoriesVerticalOffset = 0;
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
  for (ModelsCategory::iterator it = currentCategory->begin(); it != currentCategory->end(); ++it, ++index) {
    if (*it == modelslist.getCurrentModel()) {
      setCurrentModel(index);
      found = true;
      break;
    }
  }
  if (!found) {
    setCurrentModel(0);
  }
}

bool menuModelSelect(event_t event)
{
  const std::list<ModelsCategory*>& cats = modelslist.getCategories();
  switch (event) {
    case 0:
      // no need to refresh the screen
      return false;

    case EVT_ENTRY:
      selectMode = MODE_SELECT_MODEL;
      initModelsList();
      break;

    case EVT_KEY_BREAK(KEY_ENTER):
      if (selectMode == MODE_MOVE_MODEL)
        selectMode = MODE_SELECT_MODEL;
      break;

    case EVT_KEY_FIRST(KEY_EXIT):
      switch (selectMode) {
        case MODE_MOVE_MODEL:
          selectMode = MODE_SELECT_MODEL;
          break;
        case MODE_SELECT_MODEL:
          chainMenu(menuMainView);
          return false;
      }
      break;

    case EVT_KEY_FIRST(KEY_PGUP):
#if defined(PCBX10)
    case EVT_KEY_LONG(KEY_PGDN):
#endif
      if (selectMode == MODE_SELECT_MODEL) {
        if (categoriesVerticalPosition == 0)
          categoriesVerticalPosition = cats.size() - 1;
        else
          categoriesVerticalPosition -= 1;
        setCurrentCategory(categoriesVerticalPosition);
      }
      else if (selectMode == MODE_MOVE_MODEL && categoriesVerticalPosition > 0) {
        ModelsCategory * previous_category = currentCategory;
        ModelCell * model = currentModel;
        categoriesVerticalPosition -= 1;
        setCurrentCategory(categoriesVerticalPosition);
        modelslist.moveModel(model, previous_category, currentCategory);
        setCurrentModel(currentCategory->size() - 1);
      }
      killEvents(event);
      break;

#if defined(PCBX12S)
    case EVT_KEY_FIRST(KEY_PGDN):
#elif defined(PCBX10)
    case EVT_KEY_BREAK(KEY_PGDN):
#endif
      if (selectMode == MODE_SELECT_MODEL) {
        categoriesVerticalPosition += 1;
        if (categoriesVerticalPosition >= cats.size())
          categoriesVerticalPosition = 0;
        setCurrentCategory(categoriesVerticalPosition);
      }
      else if (selectMode == MODE_MOVE_MODEL && categoriesVerticalPosition < cats.size()-1) {
        ModelsCategory * previous_category = currentCategory;
        ModelCell * model = currentModel;
        categoriesVerticalPosition += 1;
        setCurrentCategory(categoriesVerticalPosition);
        modelslist.moveModel(model, previous_category, currentCategory);
        setCurrentModel(currentCategory->size()-1);
      }
      break;

    case EVT_KEY_LONG(KEY_ENTER):
      if (selectMode == MODE_SELECT_MODEL) {
        killEvents(event);
        if (currentModel && currentModel != modelslist.getCurrentModel()) {
          POPUP_MENU_ADD_ITEM(STR_SELECT_MODEL);
        }
        POPUP_MENU_ADD_ITEM(STR_CREATE_MODEL);
        if (currentModel) {
          POPUP_MENU_ADD_ITEM(STR_DUPLICATE_MODEL);
          POPUP_MENU_ADD_ITEM(STR_MOVE_MODEL);
        }
        // POPUP_MENU_ADD_SD_ITEM(STR_BACKUP_MODEL);
        if (currentModel && currentModel != modelslist.getCurrentModel()) {
          POPUP_MENU_ADD_ITEM(STR_DELETE_MODEL);
        }
        // POPUP_MENU_ADD_ITEM(STR_RESTORE_MODEL);
        POPUP_MENU_ADD_ITEM(STR_CREATE_CATEGORY);
        POPUP_MENU_ADD_ITEM(STR_RENAME_CATEGORY);
        if (cats.size() > 1) {
          POPUP_MENU_ADD_ITEM(STR_DELETE_CATEGORY);
        }
        POPUP_MENU_START(onModelSelectMenu);
      }
      break;

  }

  lcdDrawSolidFilledRect(0, 0, LCD_W, LCD_H, DEFAULT_BGCOLOR);
  lcd->drawBitmap(0, 0, modelselIconBitmap);

  // Categories
  int index = 0;
  coord_t y = 97;
  drawVerticalScrollbar(CATEGORIES_WIDTH-1, y-1, 5*(FH+7)-5, categoriesVerticalOffset, cats.size(), 5);
  for (std::list<ModelsCategory *>::const_iterator it = cats.begin(); it != cats.end(); ++it, ++index) {
    if (index >= categoriesVerticalOffset && index < categoriesVerticalOffset+5) {
      if (index != categoriesVerticalOffset) {
        lcdDrawSolidHorizontalLine(1, y-4, CATEGORIES_WIDTH-10, LINE_COLOR);
      }
      if (selectMode == MODE_RENAME_CATEGORY && currentCategory == *it) {
        lcdDrawSolidFilledRect(0, y-INVERT_VERT_MARGIN+1, CATEGORIES_WIDTH-2, INVERT_LINE_HEIGHT, DEFAULT_BGCOLOR);
        editName(MENUS_MARGIN_LEFT, y, currentCategory->name, LEN_MODEL_FILENAME, event, 1, 0);
        if (s_editMode == 0 || event == EVT_KEY_BREAK(KEY_EXIT)) {
          modelslist.save();
          selectMode = MODE_SELECT_MODEL;
          pushEvent(EVT_REFRESH);
        }
      }
      else {
        drawCategory(y, (*it)->name, currentCategory==*it);
      }
      y += FH+7;
    }
  }

  // Models
  index = 0;
  y = 5;
  for (ModelsCategory::iterator it = currentCategory->begin(); it != currentCategory->end(); ++it, ++index) {
    if (index >= menuVerticalOffset*2 && index < (menuVerticalOffset+4)*2) {
      bool selected = ((selectMode==MODE_SELECT_MODEL || selectMode==MODE_MOVE_MODEL) && index==menuVerticalPosition*2+menuHorizontalPosition);
      bool current = !strncmp((*it)->modelFilename, g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME);
      if (index & 1) {
        drawModel(MODELS_LEFT + MODELS_COLUMN_WIDTH, y, *it, current, selected);
        y += 66;
      }
      else {
        drawModel(MODELS_LEFT, y, *it, current, selected);
      }
      if (selected) {
        lcd->drawBitmap(5, LCD_H-FH, modelselModelNameBitmap);
        lcdDrawText(22, LCD_H-FH-1, (*it)->modelFilename, FONT(XS)|DEFAULT_COLOR);
      }
    }
  }

  // Navigation
  if (currentModel) {
    if (selectMode == MODE_SELECT_MODEL) {
      if (navigate(event, index, 4, 2) != 0) {
        setCurrentModel(MODEL_INDEX());
        pushEvent(EVT_REFRESH);
      }
    }
    else if (selectMode == MODE_MOVE_MODEL) {
      int8_t direction = navigate(event, index, 4, 2, false);
      if (direction) {
        modelslist.moveModel(currentCategory, currentModel, direction);
        setCurrentModel(MODEL_INDEX());
        pushEvent(EVT_REFRESH);
      }
    }
  }
  drawVerticalScrollbar(DEFAULT_SCROLLBAR_X + 4, 7, LCD_H - 15, menuVerticalOffset, (index + 1) / 2, 4);

  // Footer
  lcd->drawBitmap(5, LCD_H-FH-20, modelselSdFreeBitmap);
  uint32_t size = sdGetSize() / 100;
  lcdDrawNumber(22, LCD_H-FH-21, size, PREC1|FONT(XS), 0, nullptr, "GB");
  lcd->drawBitmap(70, LCD_H-FH-20, modelselModelQtyBitmap);
  lcdDrawNumber(92, LCD_H-FH-21, modelslist.getModelsCount(),FONT(XS));

  return true;
}
