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

#define MODELSEL_W                     LCD_W

void onDeleteModelConfirm(const char * result)
{
  if (result == STR_OK) {
    storageCheck(true);
    eeDeleteModel(menuVerticalPosition); // delete file
    s_copyMode = 0;
  }
}

void onModelSelectMenu(const char * result)
{
  int8_t sub = menuVerticalPosition;

  if (result == STR_SELECT_MODEL || result == STR_CREATE_MODEL) {
    if (!g_eeGeneral.disableRssiPoweroffAlarm) {
      if (!confirmModelChange())
        return;
    }
    selectModel(sub);
  }
  else if (result == STR_COPY_MODEL) {
    s_copyMode = COPY_MODE;
    s_copyTgtOfs = 0;
    s_copySrcRow = -1;
  }
  else if (result == STR_MOVE_MODEL) {
    s_copyMode = MOVE_MODE;
    s_copyTgtOfs = 0;
    s_copySrcRow = -1;
  }
#if defined(SDCARD)
  else if (result == STR_BACKUP_MODEL) {
    storageCheck(true); // force writing of current model data before this is changed
    POPUP_WARNING(eeBackupModel(sub));
  }
  else if (result == STR_RESTORE_MODEL || result == STR_UPDATE_LIST) {
    if (sdListFiles(MODELS_PATH, MODELS_EXT, MENU_LINE_LENGTH-1, nullptr))
      POPUP_MENU_START(onModelSelectMenu);
    else
      POPUP_WARNING(STR_NO_MODELS_ON_SD);
  }
#endif
  else if (result == STR_DELETE_MODEL) {
    POPUP_CONFIRMATION(STR_DELETEMODEL, onDeleteModelConfirm);
    SET_WARNING_INFO(modelHeaders[sub].name, sizeof(g_model.header.name), ZCHAR);
  }
#if defined(SDCARD)
  else if (result != STR_EXIT) {
    // The user choosed a file on SD to restore
    storageCheck(true);
    POPUP_WARNING(eeRestoreModel(sub, (char *)result));
    if (!warningText && g_eeGeneral.currModel == sub) {
      eeLoadModel(sub);
    }
  }
#endif
}

void menuModelSelect(event_t event)
{
  event_t _event_ = event;
  if ((s_copyMode && IS_KEY_EVT(event, KEY_EXIT)) || event == EVT_KEY_BREAK(KEY_EXIT) || IS_ROTARY_BREAK(event) || IS_ROTARY_LONG(event)) {
    _event_ = 0;
  }

  int8_t oldSub = menuVerticalPosition;

  check_submenu_simple(_event_, MAX_MODELS - HEADER_LINE);

  if (s_editMode > 0) s_editMode = 0;


  int8_t sub = menuVerticalPosition;

  switch (event) {
    case EVT_ENTRY:
      menuVerticalPosition = sub = g_eeGeneral.currModel;
      if (sub >= NUM_BODY_LINES)
        menuVerticalOffset = sub-(NUM_BODY_LINES-1);
      s_copyMode = 0;
      s_editMode = EDIT_MODE_INIT;
      break;

    case EVT_KEY_LONG(KEY_EXIT):
      killEvents(event);
      if (s_copyMode && s_copyTgtOfs == 0 && g_eeGeneral.currModel != sub && eeModelExists(sub)) {
        POPUP_CONFIRMATION(STR_DELETEMODEL, nullptr);
        SET_WARNING_INFO(modelHeaders[sub].name, sizeof(g_model.header.name), ZCHAR);
      }
      else {
        s_copyMode = 0;
        menuVerticalPosition = g_eeGeneral.currModel;
      }
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      if (s_copyMode) {
        sub = menuVerticalPosition = (s_copyMode == MOVE_MODE || s_copySrcRow<0) ? (MAX_MODELS+sub+s_copyTgtOfs) % MAX_MODELS : s_copySrcRow;
        s_copyMode = 0;
      }
      else if (uint8_t(menuVerticalPosition) != g_eeGeneral.currModel) {
        menuVerticalPosition = g_eeGeneral.currModel;
      }
      else {
        popMenu();
      }
      break;

    case EVT_KEY_LONG(KEY_ENTER):
    case EVT_KEY_BREAK(KEY_ENTER):
      s_editMode = 0;
      if (READ_ONLY()) {
        if (g_eeGeneral.currModel != sub && eeModelExists(sub)) {
          selectModel(sub);
        }
      }
      else if (s_copyMode && (s_copyTgtOfs || s_copySrcRow>=0)) {
        showMessageBox(s_copyMode==COPY_MODE ? STR_COPYINGMODEL : STR_MOVINGMODEL);
        storageCheck(true); // force writing of current model data before this is changed

        uint8_t cur = (MAX_MODELS + sub + s_copyTgtOfs) % MAX_MODELS;

        if (s_copyMode == COPY_MODE) {
          if (!eeCopyModel(cur, s_copySrcRow)) {
            cur = sub;
          }
        }

        s_copySrcRow = g_eeGeneral.currModel; // to update the currModel value
        while (sub != cur) {
          uint8_t src = cur;
          cur = (s_copyTgtOfs > 0 ? cur+MAX_MODELS-1 : cur+1) % MAX_MODELS;
          eeSwapModels(src, cur);
          if (src == s_copySrcRow)
            s_copySrcRow = cur;
          else if (cur == s_copySrcRow)
            s_copySrcRow = src;
        }

        if (s_copySrcRow != g_eeGeneral.currModel) {
          g_eeGeneral.currModel = s_copySrcRow;
          storageDirty(EE_GENERAL);
        }

        s_copyMode = 0;
        event = EVT_ENTRY_UP;
      }
      else if (event == EVT_KEY_LONG(KEY_ENTER) || IS_ROTARY_BREAK(event)) {
        s_copyMode = 0;
        killEvents(event);
        if (g_eeGeneral.currModel != sub) {
          if (eeModelExists(sub)) {
            POPUP_MENU_ADD_ITEM(STR_SELECT_MODEL);
            POPUP_MENU_ADD_SD_ITEM(STR_BACKUP_MODEL);
            POPUP_MENU_ADD_ITEM(STR_COPY_MODEL);
            POPUP_MENU_ADD_ITEM(STR_MOVE_MODEL);
            POPUP_MENU_ADD_ITEM(STR_DELETE_MODEL);
          }
          else {
#if defined(SDCARD)
            POPUP_MENU_ADD_ITEM(STR_CREATE_MODEL);
            POPUP_MENU_ADD_ITEM(STR_RESTORE_MODEL);
#else
            selectModel(sub);
#endif
          }
        }
        else {
          POPUP_MENU_ADD_SD_ITEM(STR_BACKUP_MODEL);
          POPUP_MENU_ADD_ITEM(STR_COPY_MODEL);
          POPUP_MENU_ADD_ITEM(STR_MOVE_MODEL);
        }
        POPUP_MENU_START(onModelSelectMenu);
      }
      else if (eeModelExists(sub)) {
        s_copyMode = (s_copyMode == COPY_MODE ? MOVE_MODE : COPY_MODE);
        s_copyTgtOfs = 0;
        s_copySrcRow = -1;
      }
      break;

#if defined(KEYS_GPIO_REG_PAGEDN)
    case EVT_KEY_FIRST(KEY_PAGEUP):
      chainMenu(menuTabModel[DIM(menuTabModel)-1]);
      killEvents(event);
      break;

    case EVT_KEY_FIRST(KEY_PAGEDN):
      chainMenu(menuModelSetup);
      break;
#elif defined(KEYS_GPIO_REG_PAGE)
    case EVT_KEY_LONG(KEY_PAGE):
      chainMenu(menuTabModel[DIM(menuTabModel)-1]);
      killEvents(event);
      break;

    case EVT_KEY_BREAK(KEY_PAGE):
      chainMenu(menuModelSetup);
      break;
#else
#if defined(ROTARY_ENCODER_NAVIGATION)
    case EVT_ROTARY_LEFT:
    case EVT_ROTARY_RIGHT:
#endif
    case EVT_KEY_FIRST(KEY_LEFT):
    case EVT_KEY_FIRST(KEY_RIGHT):
#if defined(ROTARY_ENCODER_NAVIGATION)
      if ((!IS_ROTARY_RIGHT(event) && !IS_ROTARY_LEFT(event)) || s_editMode < 0) {
#endif
      if (sub == g_eeGeneral.currModel) {
        chainMenu((IS_ROTARY_RIGHT(event) || event == EVT_KEY_FIRST(KEY_RIGHT)) ? menuModelSetup : menuTabModel[DIM(menuTabModel)-1]);
      }
      else {
        AUDIO_WARNING2();
      }
      break;
#if defined(ROTARY_ENCODER_NAVIGATION)
      }
      // no break
#endif
#endif

#if defined(ROTARY_ENCODER_NAVIGATION) && defined(NAVIGATION_X7)
    case EVT_ROTARY_LEFT:
    case EVT_ROTARY_RIGHT:
#endif
    case EVT_KEY_FIRST(KEY_UP):
    case EVT_KEY_REPT(KEY_UP):
    case EVT_KEY_FIRST(KEY_DOWN):
    case EVT_KEY_REPT(KEY_DOWN):
      if (s_copyMode) {
        int8_t next_ofs = s_copyTgtOfs + oldSub - menuVerticalPosition;
        if (next_ofs == MAX_MODELS || next_ofs == -MAX_MODELS)
          next_ofs = 0;

        if (s_copySrcRow < 0 && s_copyMode==COPY_MODE) {
          s_copySrcRow = oldSub;
          // find a hole (in the first empty slot above / below)
          sub = eeFindEmptyModel(s_copySrcRow, IS_ROTARY_RIGHT(event) || event==EVT_KEY_FIRST(KEY_DOWN));
          if (sub < 0) {
            // no free room for duplicating the model
            AUDIO_ERROR();
            sub = oldSub;
            s_copyMode = 0;
          }
          next_ofs = 0;
          menuVerticalPosition = sub;
        }
        s_copyTgtOfs = next_ofs;
      }
      break;
  }

#if defined(EEPROM_RLC)
  lcdDrawText(9*FW-(LEN_FREE-4)*FW-4, 0, STR_FREE);
  if (event) reusableBuffer.modelsel.eepromfree = EeFsGetFree();
  lcdDrawNumber(lcdLastRightPos+3, 0, reusableBuffer.modelsel.eepromfree, LEFT);
#elif defined(EEPROM_RLC)
  lcdDrawText(9*FW-(LEN_FREE-4)*FW, 0, STR_FREE);
  if (event) reusableBuffer.modelsel.eepromfree = EeFsGetFree();
  lcdDrawNumber(17*FW, 0, reusableBuffer.modelsel.eepromfree, RIGHT);
#endif

#if defined(NAVIGATION_X7)
  drawScreenIndex(MENU_MODEL_SELECT, DIM(menuTabModel), 0);
#elif defined(ROTARY_ENCODER_NAVIGATION)
  drawScreenIndex(MENU_MODEL_SELECT, DIM(menuTabModel), (sub == g_eeGeneral.currModel) ? ((IS_ROTARY_ENCODER_NAVIGATION_ENABLE() && s_editMode < 0) ? INVERS|BLINK : INVERS) : 0);
#else
  drawScreenIndex(MENU_MODEL_SELECT, DIM(menuTabModel), (sub == g_eeGeneral.currModel) ? INVERS : 0);
#endif

  title(STR_MENUMODELSEL);

  for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i+menuVerticalOffset;

    lcdDrawNumber(3*FW+2, y, k+1, RIGHT|LEADING0|((!s_copyMode && sub==k) ? INVERS : 0), 2);

    if (s_copyMode == MOVE_MODE || (s_copyMode == COPY_MODE && s_copySrcRow >= 0)) {
      if (k == sub) {
        if (s_copyMode == COPY_MODE) {
          k = s_copySrcRow;
          lcdDrawChar(MODELSEL_W-FW, y, '+');
        }
        else {
          k = sub + s_copyTgtOfs;
        }
      }
      else if (s_copyTgtOfs < 0 && ((k < sub && k >= sub+s_copyTgtOfs) || (k-MAX_MODELS < sub && k-MAX_MODELS >= sub+s_copyTgtOfs)))
        k += 1;
      else if (s_copyTgtOfs > 0 && ((k > sub && k <= sub+s_copyTgtOfs) || (k+MAX_MODELS > sub && k+MAX_MODELS <= sub+s_copyTgtOfs)))
        k += MAX_MODELS-1;
    }

    k %= MAX_MODELS;

    if (eeModelExists(k)) {
      putsModelName(4*FW, y, modelHeaders[k].name, k, 0);
      if (k==g_eeGeneral.currModel && (s_copyMode!=COPY_MODE || s_copySrcRow<0 || i+menuVerticalOffset!=(vertpos_t)sub)) {
        lcdDrawChar(1, y, '*');
      }
    }

    if (s_copyMode && (vertpos_t)sub==i+menuVerticalOffset) {
      lcdDrawSolidFilledRect(9, y, MODELSEL_W-1-9, 7);
      lcdDrawRect(8, y-1, MODELSEL_W-1-7, 9, s_copyMode == COPY_MODE ? SOLID : DOTTED);
    }
  }
}
