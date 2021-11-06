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

#include "opentx.h"
#include "access_settings.h"

#include <functional>

#define SET_DIRTY()     storageDirty(EE_MODEL)

BindRxChoiceMenu::BindRxChoiceMenu(Window* parent, uint8_t moduleIdx,
                                   uint8_t receiverIdx) :
    Menu(parent), moduleIdx(moduleIdx), receiverIdx(receiverIdx)
{
  const auto& bindInfo = getPXX2BindInformationBuffer();
  auto receiversCount = min<uint8_t>(bindInfo.candidateReceiversCount,
                                     PXX2_MAX_RECEIVERS_PER_MODULE);

  for (uint8_t i = 0; i < receiversCount; i++) {
    const char* receiverName = bindInfo.candidateReceiversNames[i];
    addLine(receiverName, [=]() {
      auto& bindInfo = getPXX2BindInformationBuffer();
      auto& modInfo = getPXX2ModuleSetupBuffer().moduleInformation;
      bindInfo.selectedReceiverIndex = i;
      if (isModuleR9MAccess(moduleIdx) &&
          modInfo.information.variant == PXX2_VARIANT_EU) {
        bindInfo.step = BIND_RX_NAME_SELECTED;
        //              POPUP_MENU_ADD_ITEM(STR_16CH_WITH_TELEMETRY);
        //              POPUP_MENU_ADD_ITEM(STR_16CH_WITHOUT_TELEMETRY);
        //              POPUP_MENU_START(onPXX2R9MBindModeMenu);
      } else if (isModuleR9MAccess(moduleIdx) &&
                 modInfo.information.variant == PXX2_VARIANT_FLEX) {
        bindInfo.step = BIND_RX_NAME_SELECTED;
        //              POPUP_MENU_ADD_ITEM(STR_FLEX_868);
        //              POPUP_MENU_ADD_ITEM(STR_FLEX_915);
        //              POPUP_MENU_START(onPXX2R9MBindModeMenu);
      } else {
#if defined(SIMU)
        memcpy(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx],
               receiverName, PXX2_LEN_RX_NAME);
        storageDirty(EE_MODEL);
        bindInfo.step = BIND_OK;
        moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
        new MessageDialog(parent, STR_BIND, STR_BIND_OK);
#else
        bindInfo.step = BIND_START;
        new BindWaitDialog(parent, moduleIdx, receiverIdx);
#endif
      }
    });
  }

  setCancelHandler([=]() { moduleState[moduleIdx].mode = MODULE_MODE_NORMAL; });
}

BindWaitDialog::BindWaitDialog(Window* parent, uint8_t moduleIdx,
                               uint8_t receiverIdx) :
    Dialog(parent, STR_BIND, {50, 73, LCD_W - 100, LCD_H - 146}),
    moduleIdx(moduleIdx),
    receiverIdx(receiverIdx)
{
  new StaticText(&content->form, {0, height() / 2, width(), PAGE_LINE_HEIGHT},
                 STR_WAITING_FOR_RX, 0, CENTERED | COLOR_THEME_PRIMARY1);
}

void BindWaitDialog::checkEvents()
{
  auto& bindInfo = getPXX2BindInformationBuffer();
  if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL) {
    removePXX2ReceiverIfEmpty(moduleIdx, receiverIdx);
    deleteLater();
    if (bindInfo.step == BIND_OK) {
      new MessageDialog(parent, STR_BIND, STR_BIND_OK);
    } else {
      // TODO: display error???
    }
    return;
  }

  if (bindInfo.step == BIND_INIT && bindInfo.candidateReceiversCount > 0) {
    new BindRxChoiceMenu(parent, moduleIdx, receiverIdx);
    deleteLater();
    return;
  }

  Dialog::checkEvents();
}

#if defined(HARDWARE_KEYS)
void BindWaitDialog::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(),
                event);

  if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
    deleteLater();
  }
}
#endif

ReceiverButton::ReceiverButton(FormGroup* parent, rect_t rect,
                               uint8_t moduleIdx, uint8_t receiverIdx) :
    TextButton(parent, rect, STR_BIND,
               std::bind(&ReceiverButton::pressBind, this)),
    moduleIdx(moduleIdx),
    receiverIdx(receiverIdx)
{
}

uint8_t ReceiverButton::pressBind()
{
  if (g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx][0] == '\0') {
    startBind();
  } else {
    auto menu = new Menu(parent);
    menu->addLine(STR_BIND, [=]() {
      startBind();
      return 0;
    });
    menu->addLine(STR_OPTIONS, [=]() {
      auto hwSettings = getPXX2HardwareAndSettingsBuffer();
      memclear(&hwSettings, sizeof(hwSettings));
      hwSettings.receiverSettings.receiverId = receiverIdx;
      // pushMenu(menuModelReceiverOptions);
      return 0;
    });
    menu->addLine(STR_SHARE, [=]() {
      auto& modSetup = getPXX2ModuleSetupBuffer();
      modSetup.shareReceiverIndex = receiverIdx;
      moduleState[moduleIdx].mode = MODULE_MODE_SHARE;
      return 0;
    });
    menu->addLine(STR_DELETE, [=]() {
      auto& modSetup = getPXX2ModuleSetupBuffer();
      memclear(&modSetup, sizeof(modSetup));
      modSetup.resetReceiverIndex = receiverIdx;
      modSetup.resetReceiverFlags = 0x01;
      new ConfirmDialog(parent, STR_RECEIVER, STR_RECEIVER_DELETE, [=]() {
        moduleState[moduleIdx].mode = MODULE_MODE_RESET;
        removePXX2Receiver(moduleIdx, receiverIdx);
      });
      return 0;
    });
    menu->addLine(STR_RESET, [=]() {
      auto& modSetup = getPXX2ModuleSetupBuffer();
      memclear(&modSetup, sizeof(modSetup));
      modSetup.resetReceiverIndex = receiverIdx;
      modSetup.resetReceiverFlags = 0xFF;
      new ConfirmDialog(parent, STR_RECEIVER, STR_RECEIVER_DELETE, [=]() {
        moduleState[moduleIdx].mode = MODULE_MODE_RESET;
        removePXX2Receiver(moduleIdx, receiverIdx);
      });
      return 0;
    });
    menu->setCloseHandler(
        [=]() { removePXX2ReceiverIfEmpty(moduleIdx, receiverIdx); });
  }
  return 0;
}

void ReceiverButton::startBind()
{
  auto& bindInfo = getPXX2BindInformationBuffer();
  auto& modSetup = getPXX2ModuleSetupBuffer();

  memclear(&bindInfo, sizeof(bindInfo));
  bindInfo.rxUid = receiverIdx;

  if (isModuleR9MAccess(moduleIdx)) {
    bindInfo.step = BIND_MODULE_TX_INFORMATION_REQUEST;
#if defined(SIMU)
    modSetup.moduleInformation.information.modelID = 1;
    modSetup.moduleInformation.information.variant = 2;
#else
    moduleState[moduleIdx].readModuleInformation(
        &modSetup.moduleInformation, PXX2_HW_INFO_TX_ID, PXX2_HW_INFO_TX_ID);
#endif
  } else {
    moduleState[moduleIdx].startBind(&bindInfo);
  }

  new BindWaitDialog(parent, moduleIdx, receiverIdx);
}

void ReceiverButton::checkEvents()
{
  if (g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx][0] != '\0') {
    char receiverName[PXX2_LEN_RX_NAME + 1];
    memset(receiverName, 0, sizeof(receiverName));
    strncpy(receiverName,
            g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx],
            effectiveLen(
                g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx],
                PXX2_LEN_RX_NAME));
    setText(receiverName);
  } else {
    setText(STR_BIND);
  }

  TextButton::checkEvents();
}

RegisterDialog::RegisterDialog(Window* parent, uint8_t moduleIdx) :
    Dialog(parent, STR_REGISTER, {50, 73, LCD_W - 100, 0}), moduleIdx(moduleIdx)
{
  FormGroup* form = &content->form;
  FormGridLayout grid(content->form.width());
  grid.setLabelWidth(150);
  grid.spacer(PAGE_PADDING);

  // Register ID
  new StaticText(form, grid.getLabelSlot(), STR_REG_ID, 0,
                 COLOR_THEME_PRIMARY1);
  auto edit =
      new ModelTextEdit(form, grid.getFieldSlot(), g_model.modelRegistrationID,
                        sizeof(g_model.modelRegistrationID));
  grid.nextLine();

  // UID
  new StaticText(form, grid.getLabelSlot(), "UID", 0, COLOR_THEME_PRIMARY1);
  uid = new NumberEdit(
      form, grid.getFieldSlot(), 0, 2,
      GET_SET_DEFAULT(getPXX2ModuleSetupBuffer().registerLoopIndex));
  grid.nextLine();

  // RX name
  new StaticText(form, grid.getLabelSlot(), STR_RX_NAME, 0,
                 COLOR_THEME_PRIMARY1);
  waiting = new StaticText(form, grid.getFieldSlot(), STR_WAITING, 0,
                           COLOR_THEME_PRIMARY1);
  grid.nextLine();
  grid.spacer(6);

  // Buttons
  exitButton =
      new TextButton(form, grid.getLabelSlot(), "EXIT", [=]() -> int8_t {
        this->deleteLater();
        return 0;
      });
  exitButton->setFocus(SET_FOCUS_DEFAULT);
  grid.nextLine();
  grid.spacer(PAGE_PADDING);

  FormField::link(exitButton, edit);
  form->setHeight(grid.getWindowHeight());
  content->adjustHeight();

  start();

  setCloseHandler([=]() { moduleState[moduleIdx].mode = MODULE_MODE_NORMAL; });
}

void RegisterDialog::start()
{
    auto& pxx2Setup = getPXX2ModuleSetupBuffer();
    memclear(&pxx2Setup, sizeof(pxx2Setup));
    moduleState[moduleIdx].mode = MODULE_MODE_REGISTER;
}

void RegisterDialog::checkEvents()
{
  if (!rxName && reusableBuffer.moduleSetup.pxx2.registerStep >=
                     REGISTER_RX_NAME_RECEIVED) {
    rect_t rect = waiting->getRect();
    waiting->deleteLater();

    rxName = new ModelTextEdit(&content->form, rect,
                               reusableBuffer.moduleSetup.pxx2.registerRxName,
                               PXX2_LEN_RX_NAME);
    rect = exitButton->getRect();
    auto okButton = new TextButton(&content->form, rect, "OK", [=]() -> int8_t {
      reusableBuffer.moduleSetup.pxx2.registerStep = REGISTER_RX_NAME_SELECTED;
      return 0;
    });
    exitButton->setLeft(left() + rect.w + 10);
    FormField::link(uid, rxName);
    FormField::link(rxName, okButton);
    FormField::link(okButton, exitButton);
    okButton->setFocus(SET_FOCUS_DEFAULT);
  } else if (reusableBuffer.moduleSetup.pxx2.registerStep == REGISTER_OK) {
    deleteLater();
    POPUP_INFORMATION(STR_REG_OK);
  }

  Dialog::checkEvents();
}
