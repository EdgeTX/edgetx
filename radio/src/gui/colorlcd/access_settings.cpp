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
#include "channel_bar.h"

#include <functional>

#define SET_DIRTY()     storageDirty(EE_MODEL)

static void startBindWaitDialog(Window* parent, uint8_t moduleIdx,
                                uint8_t receiverIdx)
{
  auto& bindInfo = getPXX2BindInformationBuffer();
#if defined(SIMU)
  const char* receiverName =
      bindInfo.candidateReceiversNames[bindInfo.selectedReceiverIndex];
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

static void startFlexBindWaitDialog(Window* parent, uint8_t moduleIdx,
                                    uint8_t receiverIdx, uint8_t flex)
{
  auto& bindInfo = getPXX2BindInformationBuffer();
  bindInfo.flexMode = flex;
  startBindWaitDialog(parent, moduleIdx, receiverIdx);
}

static void startLBTBindWaitDialog(Window* parent, uint8_t moduleIdx,
                                   uint8_t receiverIdx, uint8_t lbt)
{
  auto& bindInfo = getPXX2BindInformationBuffer();
  bindInfo.lbtMode = lbt;
  startBindWaitDialog(parent, moduleIdx, receiverIdx);
}

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

        auto& modSetup = getPXX2ModuleSetupBuffer();
        if (modSetup.moduleSettings.txPower <= 14) {
          // with telemetry
          startLBTBindWaitDialog(parent, moduleIdx, receiverIdx, 1);
        } else {
          // without telemetry
          startLBTBindWaitDialog(parent, moduleIdx, receiverIdx, 2);
        }
        return;
      } else if (isModuleR9MAccess(moduleIdx) &&
                 modInfo.information.variant == PXX2_VARIANT_FLEX) {
        bindInfo.step = BIND_RX_NAME_SELECTED;
        auto flexMenu = new Menu(parent);
        flexMenu->addLine(STR_FLEX_868, [=]() {
          startFlexBindWaitDialog(parent, moduleIdx, receiverIdx, 0);
        });
        flexMenu->addLine(STR_FLEX_915, [=]() {
          startFlexBindWaitDialog(parent, moduleIdx, receiverIdx, 1);
        });
        return;
      }

      startBindWaitDialog(parent, moduleIdx, receiverIdx);
    });
  }

  setCancelHandler([=]() { moduleState[moduleIdx].mode = MODULE_MODE_NORMAL; });
}

#if defined(HARDWARE_TOUCH)
bool BindRxChoiceMenu::onTouchEnd(coord_t x, coord_t y)
{
  // Note: onCancel() is not called when the menu is discarded
  //       by clicking outside the menu window and the onCancel
  //       handler is not accessible from here
  moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
  onKeyPress();
  deleteLater();
  return true;
}
#endif

BindWaitDialog::BindWaitDialog(Window* parent, uint8_t moduleIdx,
                               uint8_t receiverIdx) :
    Dialog(parent, STR_BIND, {50, 73, LCD_W - 100, LCD_H - 146}),
    moduleIdx(moduleIdx),
    receiverIdx(receiverIdx)
{
  new StaticText(&content->form, {0, height() / 2, width(), PAGE_LINE_HEIGHT},
                 STR_WAITING_FOR_RX, 0, CENTERED | COLOR_THEME_PRIMARY1);

  setCloseHandler([=]() { moduleState[moduleIdx].mode = MODULE_MODE_NORMAL; });
}

void BindWaitDialog::deleteLater(bool detach, bool trash)
{
  Dialog::deleteLater(detach, trash);
}

void BindWaitDialog::checkEvents()
{
  auto& bindInfo = getPXX2BindInformationBuffer();

  if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL) {

    // returned to normal after bind
    if (bindInfo.step > BIND_INIT) {
      removePXX2ReceiverIfEmpty(moduleIdx, receiverIdx);
      deleteLater();
      if (bindInfo.step == BIND_OK) {
        POPUP_INFORMATION(STR_REG_OK);
      }
      return;
    }

    // pre-bind phase: fetching info for R9M
    auto& modSetup = getPXX2ModuleSetupBuffer();
    switch(bindInfo.step) {
    case BIND_MODULE_TX_INFORMATION_REQUEST:
      if (modSetup.moduleInformation.information.variant == PXX2_VARIANT_EU) {
        // In EU mode we will need the power of the module to know if telemetry
        // can be proposed
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
    setCloseHandler(nullptr);
    deleteLater();

    // ... and create RX choice dialog
    new BindRxChoiceMenu(Layer::back(), moduleIdx, receiverIdx);
    return;
  }

  Dialog::checkEvents();
}

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
      // pushMenu(menuModelReceiverOptions);
      new RxOptions(this, moduleIdx, receiverIdx);
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
  waiting = new StaticText(form, grid.getFieldSlot(), STR_WAITING_FOR_RX, 0,
                           COLOR_THEME_PRIMARY1);
  grid.nextLine();
  grid.spacer(6);

  // Buttons
  exitButton =
      new TextButton(form, grid.getLabelSlot(), "EXIT", [=]() -> int8_t {
        this->deleteLater();
        return 0;
      });
  // exitButton->setFocus(SET_FOCUS_DEFAULT);
  grid.nextLine();
  grid.spacer(PAGE_PADDING);

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
  auto& modSetup = getPXX2ModuleSetupBuffer();
  if (!rxName && modSetup.registerStep >= REGISTER_RX_NAME_RECEIVED) {
    rect_t rect = waiting->getRect();
    waiting->deleteLater();

    rxName = new ModelTextEdit(&content->form, rect, modSetup.registerRxName,
                               PXX2_LEN_RX_NAME);
    rect = exitButton->getRect();
    auto okButton = new TextButton(&content->form, rect, "OK", [=]() -> int8_t {
      auto& modSetup = getPXX2ModuleSetupBuffer();
      modSetup.registerStep = REGISTER_RX_NAME_SELECTED;
      return 0;
    });
    exitButton->setLeft(left() + rect.w + 10);
  } else if (modSetup.registerStep == REGISTER_OK) {
    deleteLater();
    POPUP_INFORMATION(STR_REG_OK);
  }

  Dialog::checkEvents();
}

ModuleOptions::ModuleOptions(Window* parent, uint8_t moduleIdx):
  Dialog(parent, STR_MODULE_OPTIONS, {50, 73, LCD_W - 100, LCD_H - 146}),
  moduleIdx(moduleIdx)
{
  setCloseWhenClickOutside(true);
  auto form = &content->form;
  new StaticText(form, {0, height() / 2, width(), PAGE_LINE_HEIGHT},
                 STR_WAITING_FOR_MODULE, CENTERED | VCENTERED | COLOR_THEME_PRIMARY1);

#if defined(SIMU)
  auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
  memclear(&hwSettings, sizeof(hwSettings));
  hwSettings.moduleSettings.state = PXX2_SETTINGS_OK;
  moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
  state = MO_ReadModuleSettings;
#endif
  setCloseHandler([=]() { moduleState[moduleIdx].mode = MODULE_MODE_NORMAL; });
  // setFocus();
}

void ModuleOptions::checkEvents()
{
  auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
  switch (state) {
    case MO_Init:
      // kickstart fetching HW settings
      memclear(&hwSettings, sizeof(hwSettings));
      moduleState[moduleIdx].readModuleInformation(
          &hwSettings.modules[moduleIdx], PXX2_HW_INFO_TX_ID,
          PXX2_HW_INFO_TX_ID);
      state = MO_ReadModuleInfo;
      break;
    case MO_ReadModuleInfo:
      // times out after a while (see pxx2.cpp)
      if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL) {
        moduleState[moduleIdx].readModuleSettings(&hwSettings.moduleSettings);
        state = MO_ReadModuleSettings;
      }
      break;
    case MO_ReadModuleSettings:
      // this one does NOT timeout (see pxx2.cpp)
      if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL &&
          hwSettings.moduleSettings.state == PXX2_SETTINGS_OK) {
        state = MO_DisplaySettings;
        update();
      }
      break;
    case MO_WriteSettings:
      // TODO: ask for confirmation ???
      moduleState[moduleIdx].writeModuleSettings(&hwSettings.moduleSettings);
      state = MO_WritingSettings;
      break;

    case MO_WritingSettings:      
#if defined(SIMU)
      statusText.clear();
      deleteLater();
#else
      if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL &&
          hwSettings.moduleSettings.state == PXX2_SETTINGS_OK) {
        statusText.clear();
        deleteLater();
      }
#endif
      break;

    default: // MO_DisplaySettings
      break;
  }

  Dialog::checkEvents();
}

uint8_t ModuleOptions::getModuleSettingsState()
{
  const auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
  return hwSettings.moduleSettings.state;
}

void ModuleOptions::update()
{
  auto form = &content->form;
  form->clear();

  auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
  uint8_t modelId = hwSettings.modules[moduleIdx].information.modelID;

#if defined(SIMU)
  if (modelId == 0) {
    modelId = PXX2_MODULE_R9M_LITE_PRO;
    hwSettings.modules[moduleIdx].information.modelID = modelId;
    hwSettings.modules[moduleIdx].information.variant = PXX2_VARIANT_FCC;
  }
#endif
  
  uint8_t optionsAvailable =
      getPXX2ModuleOptions(modelId) &
      ((1 << MODULE_OPTION_EXTERNAL_ANTENNA) | (1 << MODULE_OPTION_POWER));

  FormGridLayout grid(content->form.width(), 10);
  grid.setLabelWidth(width() / 3);

  new StaticText(form, grid.getLabelSlot(), STR_MODULE);
  new StaticText(form, grid.getFieldSlot(), getPXX2ModuleName(modelId));
  grid.nextLine();

  if (!optionsAvailable) {
    // no options available
    new StaticText(form, grid.getCenteredSlot(), STR_NO_TX_OPTIONS, 0, CENTERED);
  } else {
    // some options available
    if (optionsAvailable & (1 << MODULE_OPTION_EXTERNAL_ANTENNA)) {
      new StaticText(form, grid.getLabelSlot(), STR_EXT_ANTENNA, 0,
                     COLOR_THEME_PRIMARY1);
      new CheckBox(form, grid.getFieldSlot(),
                   []() {
                     const auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
                     return hwSettings.moduleSettings.externalAntenna;
                   },
                   [&](uint8_t val) {
                     auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
                     hwSettings.moduleSettings.externalAntenna = val;
                   });
      grid.nextLine();
    }

    if (optionsAvailable & (1 << MODULE_OPTION_POWER)) {

      // TODO: use isTelemetryAvailable() to check if rebind is necessary
      new StaticText(form, grid.getLabelSlot(), STR_POWER, 0,
                     COLOR_THEME_PRIMARY1);
      auto txPower = new Choice(
          form, grid.getFieldSlot(), 0, 30,
          []() {
            const auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
            return hwSettings.moduleSettings.txPower;
          },
          [&](int val) {
            auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
            hwSettings.moduleSettings.txPower = val;
            //state = MO_WriteSettings;
          });

      txPower->setTextHandler([](int val) {
        switch (val) {
          case 10:
            return std::string("10 mW");
          case 14:
            return std::string("25 mW");
          case 20:
            return std::string("100 mW");
          case 23:
            return std::string("200 mW");
          case 27:
            return std::string("500 mW");
          case 30:
            return std::string("1000 mW");
          default:
            return std::string("---");
        }
      });

      txPower->setAvailableHandler([=](int val) {
        const auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
        return isPXX2PowerAvailable(hwSettings.modules[moduleIdx].information, val);
      });
      grid.nextLine();
    }
  }

  new DynamicText(form, grid.getFieldSlot(), [=]() { return statusText; });
  grid.setLabelWidth(15);
  grid.setMarginRight(15);
  grid.nextLine();

  auto exitButton =
    new TextButton(form, grid.getFieldSlot(2, 0), STR_EXIT, [=]() -> int8_t {
        this->deleteLater();
        return 0;
      });

  new TextButton(form, grid.getFieldSlot(2, 1), STR_OK, [=]() -> int8_t {
    this->writeSettings();
    return 0;
  });

  // exitButton->setFocus(SET_FOCUS_DEFAULT);
  grid.nextLine();
  grid.spacer(PAGE_PADDING);


  content->adjustHeight();
  content->setWindowCentered();
}

void ModuleOptions::writeSettings()
{
  if (state == MO_DisplaySettings) {
    statusText = STR_WRITING;
    state = MO_WriteSettings;
  }
}

RxOptions::RxOptions(Window* parent, uint8_t moduleIdx, uint8_t rxIdx):
  Dialog(parent, STR_RECEIVER_OPTIONS, {50, 73, LCD_W - 100, LCD_H - 146}),
  moduleIdx(moduleIdx),
  receiverIdx(rxIdx)
{
  setCloseWhenClickOutside(true);
  auto form = &content->form;
  new StaticText(
      form,
      {0, static_cast<coord_t>(form->height() / 2 - PAGE_LINE_HEIGHT),
       form->width(), PAGE_LINE_HEIGHT},
      STR_WAITING_FOR_RX, 0, CENTERED | VCENTERED | COLOR_THEME_PRIMARY1);

#if defined(SIMU)
  auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
  memclear(&hwSettings, sizeof(hwSettings));
  hwSettings.moduleSettings.state = PXX2_SETTINGS_OK;
  hwSettings.receiverSettings.state = PXX2_SETTINGS_OK;
  moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
  state = RO_ReadReceiverSettings;
  auto& rxInfo = hwSettings.modules[moduleIdx].receivers[receiverIdx].information;
  rxInfo.capabilities = 0xFFFFFFFF;
  hwSettings.receiverSettings.outputsCount = 6;
  for (int i=0; i<6; i++) {
    hwSettings.receiverSettings.outputsMapping[i] = i;
  }
#endif
  setCloseHandler([=]() { moduleState[moduleIdx].mode = MODULE_MODE_NORMAL; });
  // setFocus();
}

void RxOptions::checkEvents()
{
  auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
  auto& module = hwSettings.modules[moduleIdx];
  auto& rxInfo = module.receivers[receiverIdx].information;

  switch (state) {
    case RO_Init:
      // kickstart fetching RX settings
      memclear(&hwSettings, sizeof(hwSettings));
      hwSettings.receiverSettings.receiverId = receiverIdx;
      moduleState[moduleIdx].readModuleInformation(
          &hwSettings.modules[moduleIdx], receiverIdx, receiverIdx);
      state = RO_ReadModuleInfo;
      break;
    case RO_ReadModuleInfo:
      // times out after a while (see pxx2.cpp)
      if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL) {
        // uint8_t rxModelId = rxInfo.modelID;
        uint8_t rxVariant = rxInfo.variant;
        if (isModuleR9MAccess(moduleIdx) && rxVariant == PXX2_VARIANT_EU &&
            !hwSettings.moduleSettings.txPower) {
          moduleState[moduleIdx].readModuleSettings(&hwSettings.moduleSettings);
          state = RO_ReadModuleSettings;
        } else {
          moduleState[moduleIdx].readReceiverSettings(&hwSettings.receiverSettings);
          state = RO_ReadReceiverSettings;
        }
      }
      break;
    case RO_ReadModuleSettings:
      // this one does NOT timeout (see pxx2.cpp)
      if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL &&
          hwSettings.moduleSettings.state == PXX2_SETTINGS_OK) {
        moduleState[moduleIdx].readReceiverSettings(&hwSettings.receiverSettings);
        state = RO_ReadReceiverSettings;
      }
      break;
    case RO_ReadReceiverSettings:
      if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL &&
          hwSettings.receiverSettings.state == PXX2_SETTINGS_OK) {
        state = RO_DisplaySettings;
        update();
      }
      break;
    case RO_WriteSettings:
      // TODO: ask for confirmation ???
      moduleState[moduleIdx].writeReceiverSettings(&hwSettings.receiverSettings);
      state = RO_WritingSettings;
      break;

    case RO_WritingSettings:
#if defined(SIMU)
      statusText.clear();
      deleteLater();
#else
      if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL &&
          hwSettings.receiverSettings.state == PXX2_SETTINGS_OK) {
        statusText.clear();
        deleteLater();
      }
#endif
      break;

    default: // RO_DisplaySettings
      break;
  }

  Dialog::checkEvents();
}

#define CH_ENABLE_SPORT 4
#define CH_ENABLE_SBUS  5

static uint8_t getShiftedChannel(int8_t moduleIdx, int ch)
{
  return g_model.moduleData[moduleIdx].channelsStart + ch;
}

static std::string getChannelText(int8_t moduleIdx, uint8_t pin, int val)
{
  uint8_t ch = getShiftedChannel(moduleIdx, val);
  uint8_t channelsMax = sentModuleChannels(moduleIdx) - 1;
  if (val <= channelsMax) {
    return std::string("CH") + std::to_string(ch + 1);
  } else if (pin == CH_ENABLE_SPORT) {
    return std::string("S.PORT");
  } else if (pin == CH_ENABLE_SBUS) {
    return std::string("SBUS");
  }
  return std::string();
}

void RxOptions::update()
{
  auto form = &content->form;
  form->clear();

  auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
  auto& rxInfo = hwSettings.modules[moduleIdx].receivers[receiverIdx].information;
  // uint8_t receiverModelId = rxInfo.modelID;
  uint8_t receiverVariant = rxInfo.variant;
  uint8_t capabilities = rxInfo.capabilities;

  FormGridLayout grid(content->form.width(), 10);
  grid.setLabelWidth(width() / 3);

  new StaticText(form, grid.getLabelSlot(), STR_RECEIVER);
  new StaticText(form, grid.getFieldSlot(),
                 g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx]);
  grid.nextLine();

  // PWM rate
  new StaticText(form, grid.getLabelSlot(),
                 isModuleR9MAccess(moduleIdx) ? "6.67ms PWM" : "7ms PWM");
  new CheckBox(
      form, grid.getFieldSlot(),
      []() {
        auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
        return hwSettings.receiverSettings.pwmRate;
      },
      [](int val) {
        auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
        hwSettings.receiverSettings.pwmRate = val;
      });
  grid.nextLine();

  // telemetry disabled
  new StaticText(form, grid.getLabelSlot(), STR_TELEMETRY_DISABLED);
  auto tele25mw = new CheckBox(
        form, grid.getFieldSlot(),
        []() {
          auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
          return hwSettings.receiverSettings.telemetryDisabled;
        },
        [](int val) {
          auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
          hwSettings.receiverSettings.telemetryDisabled = val;
        });

  if (isModuleR9MAccess(moduleIdx) && receiverVariant == PXX2_VARIANT_EU &&
      hwSettings.moduleSettings.txPower > 14 /*25mW*/) {
    // read only field in this case
    tele25mw->disable();
  }
  grid.nextLine();

  if (capabilities & (1 << RECEIVER_CAPABILITY_TELEMETRY_25MW)) {
    // telemetry 25 mW
    new StaticText(form, grid.getLabelSlot(), "25mw Tele");
    new CheckBox(
        form, grid.getFieldSlot(),
        []() {
          auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
          return hwSettings.receiverSettings.telemetry25mw;
        },
        [](int val) {
          auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
          hwSettings.receiverSettings.telemetry25mw = val;
        });
    grid.nextLine();
  }

  if (capabilities &
      ((1 << RECEIVER_CAPABILITY_FPORT) | (1 << RECEIVER_CAPABILITY_FPORT2))) {

    // SPORT modes
    new StaticText(form, grid.getLabelSlot(), STR_PROTOCOL);
    auto sportModes = new Choice(
        form, grid.getFieldSlot(2, 0), STR_SPORT_MODES, 0, 2,
        []() {
          const auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
          const auto& rxSettings = hwSettings.receiverSettings;
          return rxSettings.fport | (rxSettings.fport2 << 1);
        },
        [](int val) {
          auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
          auto& rxSettings = hwSettings.receiverSettings;
          rxSettings.fport = val & 0x01;
          rxSettings.fport2 = (val & 0x02) >> 1;
        });

    sportModes->setAvailableHandler([=](int val) {
      switch (val) {
        case 1:  // FPORT
          return (bool)(capabilities & (1 << RECEIVER_CAPABILITY_FPORT));
        case 2:  // FPORT2
          return (bool)(capabilities & (1 << RECEIVER_CAPABILITY_FPORT2));
      }
      return true;
    });
    grid.nextLine();
  }

  auto outputsCount = min<uint8_t>(16, hwSettings.receiverSettings.outputsCount);
  for (uint8_t i = 0; i < outputsCount; i++) {
    std::string i_str = std::to_string(i+1);
    new StaticText(form, grid.getLabelSlot(), std::string(STR_PIN) + i_str);

    uint8_t channelsMax = sentModuleChannels(moduleIdx) - 1;
    uint8_t selectionMax = channelsMax;
    if (capabilities & (1 << RECEIVER_CAPABILITY_ENABLE_PWM_CH5_CH6)
        && (CH_ENABLE_SPORT == i || CH_ENABLE_SBUS == i)) {
          selectionMax++;
    }

    uint8_t mapping = hwSettings.receiverSettings.outputsMapping[i];
    uint8_t channel = getShiftedChannel(moduleIdx, mapping);

    auto r = grid.getFieldSlot(2, 1);
    if (r.h > BAR_HEIGHT) {
      r.y += (r.h - BAR_HEIGHT)/2;
      r.h = BAR_HEIGHT;
    }
    auto chBar = new OutputChannelBar(form, r, channel);

    auto chDn = new Choice(
        form, grid.getFieldSlot(2, 0), 0, selectionMax,
        [=]() {
          auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
          return hwSettings.receiverSettings.outputsMapping[i];
        },
        [=](int val) {
          auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
          hwSettings.receiverSettings.outputsMapping[i] = val;
          if (val <= channelsMax) {
            chBar->setChannel(getShiftedChannel(moduleIdx, val));
            lv_obj_clear_flag(chBar->getLvObj(), LV_OBJ_FLAG_HIDDEN);
          } else {
            lv_obj_add_flag(chBar->getLvObj(), LV_OBJ_FLAG_HIDDEN);
          }
        });
    chDn->setTextHandler(
        [=](int val) { return getChannelText(moduleIdx, i, val); });

    grid.nextLine();
  }

  new DynamicText(form, grid.getFieldSlot(), [=]() { return statusText; });
  grid.setLabelWidth(15);
  grid.setMarginRight(15);
  grid.nextLine();

  auto exitButton =
    new TextButton(form, grid.getFieldSlot(2, 0), STR_EXIT, [=]() -> int8_t {
        this->deleteLater();
        return 0;
      });

  new TextButton(form, grid.getFieldSlot(2, 1), STR_OK, [=]() -> int8_t {
    this->writeSettings();
    return 0;
  });

  // exitButton->setFocus(SET_FOCUS_DEFAULT);
  grid.nextLine();
  grid.spacer(PAGE_PADDING);


  content->adjustHeight();
  content->setWindowCentered();
}

void RxOptions::writeSettings()
{
  if (state == RO_DisplaySettings) {
    statusText = STR_WRITING;
    state = RO_WriteSettings;
  }
}
