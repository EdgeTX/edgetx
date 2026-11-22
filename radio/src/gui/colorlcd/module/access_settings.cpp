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

#include "access_settings.h"

#include <functional>

#include "choice.h"
#include "channel_bar.h"
#include "edgetx.h"
#include "getset_helpers.h"
#include "menu.h"
#include "standalone_lua.h"
#include "toggleswitch.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

namespace pxx2
{

void runAccessTool(const char* path)
{
#if defined(LUA)
  if (isFileAvailable(path)) {
    luaExecStandalone(path);
  } else {
    new MessageDialog(STR_NEEDS_FILE, path);
  }
#endif
}

class BindRxChoiceMenu : public Menu
{
 public:
  BindRxChoiceMenu(uint8_t moduleIdx, uint8_t receiverIdx);

 protected:
  uint8_t moduleIdx;
  uint8_t receiverIdx;
};

class BindWaitDialog : public BaseDialog
{
 public:
  BindWaitDialog(uint8_t moduleIdx, uint8_t receiverIdx) :
      BaseDialog(STR_BIND, true),
      moduleIdx(moduleIdx),
      receiverIdx(receiverIdx)
  {
    new StaticText(form, rect_t{}, STR_WAITING_FOR_RX);

    onClosing([=]() { if (resetOnClose) moduleState[moduleIdx].mode = MODULE_MODE_NORMAL; });
  }

  void checkEvents() override
  {
    auto& bindInfo = getPXX2BindInformationBuffer();

    if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL) {
      // returned to normal after bind
      if (bindInfo.step > BIND_INIT) {
        removePXX2ReceiverIfEmpty(moduleIdx, receiverIdx);
        closeWindow();
        if (bindInfo.step == BIND_OK) {
          POPUP_INFORMATION(STR_REG_OK);
          setPXX2ReceiverUsed(moduleIdx, receiverIdx);
        }
        return;
      }

      // pre-bind phase: fetching info for R9M
      auto& modSetup = getPXX2ModuleSetupBuffer();
      switch (bindInfo.step) {
        case BIND_MODULE_TX_INFORMATION_REQUEST:
          if (modSetup.moduleInformation.information.variant == PXX2_VARIANT_EU) {
            // In EU mode we will need the power of the module to know if
            // telemetry can be proposed
            bindInfo.step = BIND_MODULE_TX_SETTINGS_REQUEST;
  #if defined(SIMU)
            modSetup.moduleSettings.txPower = 14;
  #else
            moduleState[moduleIdx].readModuleSettings(&modSetup.moduleSettings);
  #endif
          } else {
            bindInfo.step = 0;
            moduleState[moduleIdx].startBind(&bindInfo);
          }
          break;
        case BIND_MODULE_TX_SETTINGS_REQUEST:
          // We just receive the module settings (for TX power)
          bindInfo.step = 0;
          moduleState[moduleIdx].startBind(&bindInfo);
          break;
      }
      return;
    }

    if (bindInfo.step == BIND_INIT && bindInfo.candidateReceiversCount > 0) {
      // prevent module mode being reset to NORMAL before exiting
      resetOnClose = false;
      closeWindow();

      // ... and create RX choice dialog
      new BindRxChoiceMenu(moduleIdx, receiverIdx);
      return;
    }

    BaseDialog::checkEvents();
  }

 protected:
  uint8_t moduleIdx;
  uint8_t receiverIdx;
  bool resetOnClose = true;
};

static void startBindWaitDialog(uint8_t moduleIdx, uint8_t receiverIdx)
{
  auto& bindInfo = getPXX2BindInformationBuffer();
#if defined(SIMU)
  const char* receiverName =
      bindInfo.candidateReceiversNames[bindInfo.selectedReceiverIndex];
  memcpy(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx],
         receiverName, PXX2_LEN_RX_NAME);
  bindInfo.step = BIND_OK;
  setPXX2ReceiverUsed(moduleIdx, receiverIdx);
  moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
  new MessageDialog(STR_BIND, STR_BIND_OK);
#else
  bindInfo.step = BIND_START;
  new BindWaitDialog(moduleIdx, receiverIdx);
#endif
}

static void startFlexBindWaitDialog(uint8_t moduleIdx,
                                    uint8_t receiverIdx, uint8_t flex)
{
  auto& bindInfo = getPXX2BindInformationBuffer();
  bindInfo.flexMode = flex;
  startBindWaitDialog(moduleIdx, receiverIdx);
}

static void startLBTBindWaitDialog(uint8_t moduleIdx,
                                   uint8_t receiverIdx, uint8_t lbt)
{
  auto& bindInfo = getPXX2BindInformationBuffer();
  bindInfo.lbtMode = lbt;
  startBindWaitDialog(moduleIdx, receiverIdx);
}

BindRxChoiceMenu::BindRxChoiceMenu(uint8_t moduleIdx,
                                   uint8_t receiverIdx) :
    Menu(), moduleIdx(moduleIdx), receiverIdx(receiverIdx)
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
        auto& modSetup = getPXX2ModuleSetupBuffer();
        if (modSetup.moduleSettings.txPower <= 14) {
          // with telemetry
          startLBTBindWaitDialog(moduleIdx, receiverIdx, 1);
        } else {
          // without telemetry
          startLBTBindWaitDialog(moduleIdx, receiverIdx, 2);
        }
        return;
      } else if (isModuleR9MAccess(moduleIdx) &&
                 modInfo.information.variant == PXX2_VARIANT_FLEX) {
        bindInfo.step = BIND_RX_NAME_SELECTED;
        auto flexMenu = new Menu();
        flexMenu->addLine(STR_FLEX_868, [=]() {
          startFlexBindWaitDialog(moduleIdx, receiverIdx, 0);
        });
        flexMenu->addLine(STR_FLEX_915, [=]() {
          startFlexBindWaitDialog(moduleIdx, receiverIdx, 1);
        });
        return;
      }

      startBindWaitDialog(moduleIdx, receiverIdx);
    });
  }

  setCancelHandler([=]() { moduleState[moduleIdx].mode = MODULE_MODE_NORMAL; });
}

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};

static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

ReceiverButton::ReceiverButton(Window* parent, rect_t rect, uint8_t moduleIdx,
                               uint8_t receiverIdx) :
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
    auto menu = new Menu();
    menu->addLine(STR_BIND, [=]() {
      startBind();
      return 0;
    });
#if defined(LUA)
    menu->addLine(STR_OPTIONS, [=]() {
      runAccessTool(PXX2_RX_OPTIONS_SCRIPT);
      return 0;
    });
#endif
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
      new ConfirmDialog(STR_RECEIVER, STR_RECEIVER_DELETE, [=]() {
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
      new ConfirmDialog(STR_RECEIVER, STR_RECEIVER_RESET, [=]() {
        moduleState[moduleIdx].mode = MODULE_MODE_RESET;
        removePXX2Receiver(moduleIdx, receiverIdx);
      });
      return 0;
    });
    menu->onClosing(
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
  bindInfo.step = BIND_INIT;

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

  new BindWaitDialog(moduleIdx, receiverIdx);
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

RegisterDialog::RegisterDialog(uint8_t moduleIdx) :
    BaseDialog(STR_REGISTER, true, DIALOG_DEFAULT_WIDTH,
               LV_SIZE_CONTENT),
    moduleIdx(moduleIdx)
{
  FlexGridLayout grid(line_col_dsc, line_row_dsc);

  // Register ID
  auto line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_REG_ID);
  reg_id = new ModelTextEdit(line, rect_t{}, g_model.modelRegistrationID,
                             sizeof(g_model.modelRegistrationID));

  // UID
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, "UID");

  auto* modSetup = &(getPXX2ModuleSetupBuffer());
  uid = new NumberEdit(line, rect_t{}, 0, 2,
                       GET_SET_DEFAULT(modSetup->registerLoopIndex));

  // RX name
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_RX_NAME);

  start();  // clears registration data buffer
  rx_name = new ModelTextEdit(line, rect_t{}, modSetup->registerRxName,
                              PXX2_LEN_RX_NAME);
  rx_name->disable();

  // Status
  // line = form->newLine(grid);
  // new StaticText(line, rect_t{}, STR_STATUS);
  // status = new StaticText(line, rect_t{}, STR_WAITING_FOR_RX);

  auto box = new Window(form, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, PAD_MEDIUM);
  lv_obj_set_style_flex_main_place(box->getLvObj(), LV_FLEX_ALIGN_SPACE_EVENLY,
                                   0);
  box->padAll(PAD_MEDIUM);

  new TextButton(box, rect_t{}, STR_CANCEL, [=]() -> int8_t {
    this->closeWindow();
    return 0;
  });

  btn_ok = new TextButton(box, rect_t{}, STR_SAVE, [=]() -> int8_t {
    modSetup->registerStep = REGISTER_RX_NAME_SELECTED;
    return 0;
  });
  btn_ok->hide();

  onClosing([=]() { moduleState[moduleIdx].mode = MODULE_MODE_NORMAL; });
}

void RegisterDialog::start()
{
  auto& pxx2Setup = getPXX2ModuleSetupBuffer();
  memclear(&pxx2Setup, sizeof(pxx2Setup));
  moduleState[moduleIdx].mode = MODULE_MODE_REGISTER;
  old_registerStep = REGISTER_INIT;
}

void RegisterDialog::checkEvents()
{
  auto& modSetup = getPXX2ModuleSetupBuffer();

#if defined(SIMU)
  if (modSetup.registerStep == REGISTER_INIT) {
    memcpy(modSetup.registerRxName, "SimuRx   ", PXX2_LEN_RX_NAME);
    modSetup.registerStep = REGISTER_RX_NAME_RECEIVED;
  } else if (modSetup.registerStep == REGISTER_RX_NAME_SELECTED) {
    modSetup.registerStep = REGISTER_OK;
    moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
  }
#endif

  if (old_registerStep != modSetup.registerStep) {
    old_registerStep = modSetup.registerStep;

    if (modSetup.registerStep == REGISTER_RX_NAME_RECEIVED) {
      rx_name->enable();
      btn_ok->show();
      // status->hide();
      rx_name->update();
    } else if (modSetup.registerStep == REGISTER_OK) {
      closeWindow();
      POPUP_INFORMATION(STR_REG_OK);
      // pop-up call garbage collector,
      // so that the dialog is alread destroyed
      return;
    }
  }

  BaseDialog::checkEvents();
}

};  // namespace pxx2
