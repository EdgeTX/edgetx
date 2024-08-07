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

#include "edgetx.h"

void onPXX2R9MBindModeMenu(const char * result)
{
  if (result == STR_16CH_WITH_TELEMETRY) {
    reusableBuffer.moduleSetup.bindInformation.lbtMode = 1;
  }
  else if (result == STR_16CH_WITHOUT_TELEMETRY) {
    reusableBuffer.moduleSetup.bindInformation.lbtMode = 2;
  }
  else if (result == STR_FLEX_915) {
    reusableBuffer.moduleSetup.bindInformation.flexMode = 0;
  }
  else if (result == STR_FLEX_868) {
    reusableBuffer.moduleSetup.bindInformation.flexMode = 1;
  }
  else {
    // the user pressed [Exit]
    uint8_t moduleIdx = CURRENT_MODULE_EDITED(menuVerticalPosition - HEADER_LINE);
    uint8_t receiverIdx = CURRENT_RECEIVER_EDITED(menuVerticalPosition - HEADER_LINE);
    moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
    reusableBuffer.moduleSetup.bindInformation.step = 0;
    removePXX2ReceiverIfEmpty(moduleIdx, receiverIdx);
    return;
  }

#if defined(SIMU)
  uint8_t moduleIdx = CURRENT_MODULE_EDITED(menuVerticalPosition - HEADER_LINE);
  uint8_t receiverIdx = CURRENT_RECEIVER_EDITED(menuVerticalPosition - HEADER_LINE);
  memcpy(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], reusableBuffer.moduleSetup.bindInformation.candidateReceiversNames[reusableBuffer.moduleSetup.bindInformation.selectedReceiverIndex], PXX2_LEN_RX_NAME);
  storageDirty(EE_MODEL);
  moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
  reusableBuffer.moduleSetup.bindInformation.step = BIND_OK;
  POPUP_INFORMATION(STR_BIND_OK);
#else
  reusableBuffer.moduleSetup.bindInformation.step = BIND_START;
#endif
}

void onPXX2BindMenu(const char * result)
{
  if (result != STR_EXIT) {
    uint8_t moduleIdx = CURRENT_MODULE_EDITED(menuVerticalPosition - HEADER_LINE);
    reusableBuffer.moduleSetup.bindInformation.selectedReceiverIndex = (result - reusableBuffer.moduleSetup.bindInformation.candidateReceiversNames[0]) / sizeof(reusableBuffer.moduleSetup.bindInformation.candidateReceiversNames[0]);
    if (isModuleR9MAccess(moduleIdx) && reusableBuffer.moduleSetup.pxx2.moduleInformation.information.variant == PXX2_VARIANT_EU) {
      reusableBuffer.moduleSetup.bindInformation.step = BIND_RX_NAME_SELECTED;
      if (reusableBuffer.moduleSetup.pxx2.moduleSettings.txPower <= 14)
        onPXX2R9MBindModeMenu(STR_16CH_WITH_TELEMETRY);
      else
        onPXX2R9MBindModeMenu(STR_16CH_WITHOUT_TELEMETRY);
    }
    else if (isModuleR9MAccess(moduleIdx) && reusableBuffer.moduleSetup.pxx2.moduleInformation.information.variant == PXX2_VARIANT_FLEX) {
      reusableBuffer.moduleSetup.bindInformation.step = BIND_RX_NAME_SELECTED;
      POPUP_MENU_START(onPXX2R9MBindModeMenu, 2, STR_FLEX_868, STR_FLEX_915);
    }
    else {
#if defined(SIMU)
      uint8_t receiverIdx = CURRENT_RECEIVER_EDITED(menuVerticalPosition - HEADER_LINE);
      memcpy(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], result, PXX2_LEN_RX_NAME);
      storageDirty(EE_MODEL);
      moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
      reusableBuffer.moduleSetup.bindInformation.step = BIND_OK;
      POPUP_INFORMATION(STR_BIND_OK);
#else
      reusableBuffer.moduleSetup.bindInformation.step = BIND_START;
#endif
    }
  }
  else {
    // the user pressed [Exit]
    uint8_t moduleIdx = CURRENT_MODULE_EDITED(menuVerticalPosition - HEADER_LINE);
    uint8_t receiverIdx = CURRENT_RECEIVER_EDITED(menuVerticalPosition - HEADER_LINE);
    moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
    removePXX2ReceiverIfEmpty(moduleIdx, receiverIdx);
    s_editMode = 0;
  }
}

void onResetReceiverConfirm(const char * result)
{
  if (result == STR_OK) {
    uint8_t moduleIdx = CURRENT_MODULE_EDITED(menuVerticalPosition - HEADER_LINE);
    uint8_t receiverIdx = CURRENT_RECEIVER_EDITED(menuVerticalPosition - HEADER_LINE);
    moduleState[moduleIdx].mode = MODULE_MODE_RESET;
    removePXX2Receiver(moduleIdx, receiverIdx);
  }
}

void onPXX2ReceiverMenu(const char * result)
{
  uint8_t moduleIdx = CURRENT_MODULE_EDITED(menuVerticalPosition - HEADER_LINE);
  uint8_t receiverIdx = CURRENT_RECEIVER_EDITED(menuVerticalPosition - HEADER_LINE);

  if (result == STR_OPTIONS) {
    memclear(&reusableBuffer.hardwareAndSettings, sizeof(reusableBuffer.hardwareAndSettings));
    reusableBuffer.hardwareAndSettings.receiverSettings.receiverId = receiverIdx;
    g_moduleIdx = moduleIdx;
    pushMenu(menuModelReceiverOptions);
  }
  else if (result == STR_BIND) {
    memclear(&reusableBuffer.moduleSetup.bindInformation, sizeof(BindInformation));
    reusableBuffer.moduleSetup.bindInformation.rxUid = receiverIdx;
    if (isModuleR9MAccess(moduleIdx)) {
      reusableBuffer.moduleSetup.bindInformation.step = BIND_MODULE_TX_INFORMATION_REQUEST;
#if defined(SIMU)
      reusableBuffer.moduleSetup.pxx2.moduleInformation.information.modelID = 1;
      reusableBuffer.moduleSetup.pxx2.moduleInformation.information.variant = 2;
#else
      moduleState[moduleIdx].readModuleInformation(&reusableBuffer.moduleSetup.pxx2.moduleInformation, PXX2_HW_INFO_TX_ID, PXX2_HW_INFO_TX_ID);
#endif
    }
    else {
      moduleState[moduleIdx].startBind(&reusableBuffer.moduleSetup.bindInformation);
    }
    s_editMode = 1;
  }
  else if (result == STR_SHARE) {
    reusableBuffer.moduleSetup.pxx2.shareReceiverIndex = receiverIdx;
    moduleState[moduleIdx].mode = MODULE_MODE_SHARE;
    s_editMode = 1;
  }
  else if (result == STR_DELETE || result == STR_RESET) {
    memclear(&reusableBuffer.moduleSetup.pxx2, sizeof(reusableBuffer.moduleSetup.pxx2));
    reusableBuffer.moduleSetup.pxx2.resetReceiverIndex = receiverIdx;
    reusableBuffer.moduleSetup.pxx2.resetReceiverFlags = (result == STR_RESET ? 0xFF : 0x01);
    POPUP_CONFIRMATION(result == STR_RESET ? STR_RECEIVER_RESET : STR_RECEIVER_DELETE, onResetReceiverConfirm);
  }
  else {
    removePXX2ReceiverIfEmpty(moduleIdx, receiverIdx);
  }
}

enum PopupRegisterItems {
  ITEM_REGISTER_PASSWORD,
  ITEM_REGISTER_MODULE_INDEX,
  ITEM_REGISTER_RECEIVER_NAME,
  ITEM_REGISTER_BUTTONS
};

void runPopupRegister(event_t event)
{
  uint8_t backupVerticalPosition = menuVerticalPosition;
  uint8_t backupHorizontalPosition = menuHorizontalPosition;
  uint8_t backupVerticalOffset = menuVerticalOffset;
  int8_t backupEditMode = s_editMode;

  menuVerticalPosition = reusableBuffer.moduleSetup.pxx2.registerPopupVerticalPosition;
  menuHorizontalPosition = reusableBuffer.moduleSetup.pxx2.registerPopupHorizontalPosition;
  s_editMode = reusableBuffer.moduleSetup.pxx2.registerPopupEditMode;

  switch (event) {
    case EVT_KEY_BREAK(KEY_ENTER):
      if (menuVerticalPosition != ITEM_REGISTER_BUTTONS) {
        break;
      }
      else if (reusableBuffer.moduleSetup.pxx2.registerStep >= REGISTER_RX_NAME_RECEIVED && menuHorizontalPosition == 0) {
        // [Enter] pressed
        reusableBuffer.moduleSetup.pxx2.registerStep = REGISTER_RX_NAME_SELECTED;
        backupEditMode = EDIT_MODIFY_FIELD; // so that the [Register] button blinks and the REGISTER process can continue
      }
      // no break

    case EVT_KEY_LONG(KEY_EXIT):
      killEvents(event);
      s_editMode = 0;
      // no break;

    case EVT_KEY_BREAK(KEY_EXIT):
      if (s_editMode <= 0) {
        warningText = nullptr;
      }
      break;
  }

  if (warningText) {
    const uint8_t dialogRows[] = { 0, 0, uint8_t(reusableBuffer.moduleSetup.pxx2.registerStep < REGISTER_RX_NAME_RECEIVED ? READONLY_ROW : 0), uint8_t(reusableBuffer.moduleSetup.pxx2.registerStep < REGISTER_RX_NAME_RECEIVED ? 0 : 1)};
    check(event, 0, nullptr, 0, dialogRows, 3, 4 - HEADER_LINE); // TODO add a comment for 3 - HEADER_LINE once understood

    drawMessageBox(warningText);

    // registration password
    lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y - 4, STR_REG_ID);
    editName(WARNING_LINE_X + 8 * FW, WARNING_LINE_Y - 4,
             g_model.modelRegistrationID, PXX2_LEN_REGISTRATION_ID, event,
             menuVerticalPosition == ITEM_REGISTER_PASSWORD, 0, backupEditMode);

    // loop index (will be removed in future)
    lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y - 4 + FH, "UID");
    lcdDrawNumber(WARNING_LINE_X + 8*FW, WARNING_LINE_Y - 4 + FH, reusableBuffer.moduleSetup.pxx2.registerLoopIndex, menuVerticalPosition == ITEM_REGISTER_MODULE_INDEX ? (s_editMode ? INVERS + BLINK : INVERS) : 0);
    if (menuVerticalPosition == ITEM_REGISTER_MODULE_INDEX && s_editMode) {
      CHECK_INCDEC_MODELVAR_ZERO(event, reusableBuffer.moduleSetup.pxx2.registerLoopIndex, 2);
    }

    // RX name
    if (reusableBuffer.moduleSetup.pxx2.registerStep < REGISTER_RX_NAME_RECEIVED) {
      lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y - 4 + 2 * FH, STR_WAITING);
      lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y - 2 + 3 * FH, TR_EXIT, menuVerticalPosition == ITEM_REGISTER_BUTTONS ? INVERS : 0);
    }
    else {
      lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y - 4 + 2 * FH, STR_RX_NAME);

      editName(WARNING_LINE_X + 8 * FW, WARNING_LINE_Y - 4 + 2 * FH,
               reusableBuffer.moduleSetup.pxx2.registerRxName, PXX2_LEN_RX_NAME,
               event, menuVerticalPosition == ITEM_REGISTER_RECEIVER_NAME, 0,
               backupEditMode);

      lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y - 2 + 3 * FH, TR_ENTER,
                  menuVerticalPosition == ITEM_REGISTER_BUTTONS &&
                          menuHorizontalPosition == 0
                      ? INVERS
                      : 0);

      lcdDrawText(WARNING_LINE_X + 8 * FW, WARNING_LINE_Y - 2 + 3 * FH, TR_EXIT,
                  menuVerticalPosition == ITEM_REGISTER_BUTTONS &&
                          menuHorizontalPosition == 1
                      ? INVERS
                      : 0);
    }

    reusableBuffer.moduleSetup.pxx2.registerPopupVerticalPosition = menuVerticalPosition;
    reusableBuffer.moduleSetup.pxx2.registerPopupHorizontalPosition = menuHorizontalPosition;
    reusableBuffer.moduleSetup.pxx2.registerPopupEditMode = s_editMode;
  }

  menuVerticalPosition = backupVerticalPosition;
  menuHorizontalPosition = backupHorizontalPosition;
  menuVerticalOffset = backupVerticalOffset;
  s_editMode = backupEditMode;
}

void startRegisterDialog(uint8_t module)
{
  memclear(&reusableBuffer.moduleSetup.pxx2, sizeof(reusableBuffer.moduleSetup.pxx2));
  reusableBuffer.moduleSetup.pxx2.registerPopupVerticalPosition = ITEM_REGISTER_BUTTONS;
  moduleState[module].mode = MODULE_MODE_REGISTER;
  s_editMode = 0;
  killAllEvents();
  POPUP_INPUT("", runPopupRegister);
}

void modelSetupModulePxx2ReceiverLine(uint8_t moduleIdx, uint8_t receiverIdx, coord_t y, event_t event, LcdFlags attr)
{
  ModuleInformation & moduleInformation = reusableBuffer.moduleSetup.pxx2.moduleInformation;

  drawStringWithIndex(INDENT_WIDTH, y, STR_RECEIVER, receiverIdx + 1);

  if (!isPXX2ReceiverUsed(moduleIdx, receiverIdx)) {
    lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_MODULE_BIND, attr);
    if (attr && event == EVT_KEY_BREAK(KEY_ENTER)) {
      setPXX2ReceiverUsed(moduleIdx, receiverIdx);
      memclear(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], PXX2_LEN_RX_NAME);
      onPXX2ReceiverMenu(STR_BIND);
    }
    return;
  }

  drawReceiverName(MODEL_SETUP_2ND_COLUMN, y, moduleIdx, receiverIdx, attr);

  if (s_editMode && isModuleR9MAccess(moduleIdx) && moduleState[moduleIdx].mode == MODULE_MODE_NORMAL && reusableBuffer.moduleSetup.bindInformation.step < 0) {
    if (reusableBuffer.moduleSetup.bindInformation.step == BIND_MODULE_TX_INFORMATION_REQUEST && moduleInformation.information.modelID) {
      // For R9M ACCESS the module information has been requested to know if we are in EU mode. We just receive it here and continue
      if (moduleInformation.information.variant == PXX2_VARIANT_EU) {
        // In EU mode we will need the power of the module to know if telemetry can be proposed
        reusableBuffer.moduleSetup.bindInformation.step = BIND_MODULE_TX_SETTINGS_REQUEST;
#if defined(SIMU)
        reusableBuffer.moduleSetup.pxx2.moduleSettings.txPower = 14;
#else
        moduleState[moduleIdx].readModuleSettings(&reusableBuffer.moduleSetup.pxx2.moduleSettings);
#endif
      }
      else {
        reusableBuffer.moduleSetup.bindInformation.step = 0;
        moduleState[moduleIdx].startBind(&reusableBuffer.moduleSetup.bindInformation);
      }
    }
    else if (reusableBuffer.moduleSetup.bindInformation.step == BIND_MODULE_TX_SETTINGS_REQUEST && reusableBuffer.moduleSetup.pxx2.moduleSettings.txPower > 0) {
      // We just receive the module settings (for TX power)
      reusableBuffer.moduleSetup.bindInformation.step = 0;
      moduleState[moduleIdx].startBind(&reusableBuffer.moduleSetup.bindInformation);
    }
  }
  else if (attr && (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL || s_editMode == 0)) {
    if (moduleState[moduleIdx].mode) {
      moduleState[moduleIdx].mode = 0;
      removePXX2ReceiverIfEmpty(moduleIdx, receiverIdx);
      killEvents(event); // we stopped BIND / SHARE, we don't want to re-open the menu
      event = 0;
      CLEAR_POPUP();
    }
    s_editMode = 0;
  }

  if (moduleState[moduleIdx].mode == MODULE_MODE_BIND) {
    if (reusableBuffer.moduleSetup.bindInformation.step == BIND_INIT) {
      if (reusableBuffer.moduleSetup.bindInformation.candidateReceiversCount > 0) {
        if (reusableBuffer.moduleSetup.bindInformation.candidateReceiversCount != popupMenuItemsCount) {
          CLEAR_POPUP();
          popupMenuItemsCount = reusableBuffer.moduleSetup.bindInformation.candidateReceiversCount;
          for (auto rx = 0; rx < popupMenuItemsCount; rx++) {
            popupMenuItems[rx] = reusableBuffer.moduleSetup.bindInformation.candidateReceiversNames[rx];
          }
          POPUP_MENU_TITLE(STR_PXX2_SELECT_RX);
          POPUP_MENU_START(onPXX2BindMenu);
        }
      }
      else {
        POPUP_WAIT(STR_WAITING_FOR_RX);
      }
    }
  }

  if (attr && event == EVT_KEY_BREAK(KEY_ENTER)) {
    POPUP_MENU_START(onPXX2ReceiverMenu, 5, STR_BIND, STR_OPTIONS, STR_SHARE, STR_DELETE, STR_RESET);
  }
}
