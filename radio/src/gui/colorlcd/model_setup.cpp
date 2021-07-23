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

#include "model_setup.h"
#include "opentx.h"
#include "libopenui.h"
#include "storage/modelslist.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

std::string switchWarninglabel(swsrc_t index)
{
  static const char switchPositions[] = {
    ' ',
    CHAR_UP,
    '-',
    CHAR_DOWN
  };

  return TEXT_AT_INDEX(STR_VSRCRAW,
                       (index + MIXSRC_FIRST_SWITCH - MIXSRC_Rud + 1)) +
         std::string(
             &switchPositions[g_model.switchWarningState >> (3 * index) & 0x07],
             1);
}


class ChannelFailsafeBargraph: public Window {
  public:
    ChannelFailsafeBargraph(Window * parent, const rect_t & rect, uint8_t moduleIdx, uint8_t channel):
      Window(parent, rect),
      moduleIdx(moduleIdx),
      channel(channel)
    {
    }

    void checkEvents() override
    {
      invalidate();
      Window::checkEvents();
    }

    void paint(BitmapBuffer * dc) override
    {
      int32_t failsafeValue = g_model.failsafeChannels[channel];
      int32_t channelValue = channelOutputs[channel];

      const int lim = (g_model.extendedLimits ? (512 * LIMIT_EXT_PERCENT / 100) : 512) * 2;

      coord_t x = 0;
      dc->drawRect(x, 0, width(), height());
      const coord_t lenChannel = limit((uint8_t) 1, uint8_t((abs(channelValue) * width() / 2 + lim / 2) / lim),
                                       uint8_t(width() / 2));
      const coord_t lenFailsafe = limit((uint8_t) 1, uint8_t((abs(failsafeValue) * width() / 2 + lim / 2) / lim),
                                        uint8_t(width() / 2));
      x += width() / 2;
      const coord_t xChannel = (channelValue > 0) ? x : x + 1 - lenChannel;
      const coord_t xFailsafe = (failsafeValue > 0) ? x : x + 1 - lenFailsafe;
      dc->drawSolidFilledRect(xChannel, + 2, lenChannel, (height() / 2) - 3, DEFAULT_COLOR);
      dc->drawSolidFilledRect(xFailsafe, (height() / 2) + 1, lenFailsafe, (height() / 2) - 3, ALARM_COLOR);
    }

  protected:
    uint8_t moduleIdx;
    uint8_t channel;
};

class FailSafeBody : public FormGroup {
  public:
    FailSafeBody(FormGroup * parent, const rect_t & rect, uint8_t moduleIdx) :
      FormGroup(parent, rect, FORM_FORWARD_FOCUS),
      moduleIdx(moduleIdx)
    {
      build();
    }

    void checkEvents() override
    {
      invalidate();
      FormGroup::checkEvents();
    }

    void build()
    {
      FormGridLayout grid;
      grid.setLabelWidth(60);
      grid.spacer(8);

      const int lim = (g_model.extendedLimits ? (512 * LIMIT_EXT_PERCENT / 100) : 512) * 2;

      for (int ch=0; ch < maxModuleChannels(moduleIdx); ch++) {
        // Channel name
        // TODO if (g_model.limitData[ch].name[0] != '\0') { <= add channel name
        new StaticText(this, grid.getLabelSlot(), getSourceString(MIXSRC_CH1 + ch));

        // Channel numeric value
        new NumberEdit(this, grid.getFieldSlot(8, 0), -lim, lim,
                       GET_DEFAULT(g_model.failsafeChannels[ch]),
                       SET_VALUE(g_model.failsafeChannels[ch], newValue),
                       0, PREC1 | RIGHT);

        // Channel bargraph
        new ChannelFailsafeBargraph(this, {180, grid.getWindowHeight(), 150, PAGE_LINE_HEIGHT}, moduleIdx, ch);
        grid.nextLine();
      }

      grid.spacer();
      auto out2fail = new TextButton(this, grid.getLineSlot(), STR_CHANNELS2FAILSAFE);
      out2fail->setPressHandler([=]() {
        setCustomFailsafe(moduleIdx);
        storageDirty(EE_MODEL);
        return 0;
      });

      grid.nextLine();
      setInnerHeight(grid.getWindowHeight() + PAGE_PADDING);
    }

  protected:
    uint8_t moduleIdx;
};

class FailSafePage : public Page {
  public:
    explicit FailSafePage(uint8_t moduleIdx) :
      Page(ICON_STATS_ANALOGS),
      moduleIdx(moduleIdx)
    {
      new FailSafeBody(&body, {0, 0, LCD_W, body.height()}, moduleIdx);
      new StaticText(&header,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                      PAGE_LINE_HEIGHT},
                     STR_FAILSAFESET, 0, FOCUS_COLOR);
    }

  protected:
    static constexpr coord_t footerHeight = 30;
    uint8_t moduleIdx;
};

class RegisterDialog: public Dialog {
  public:
    RegisterDialog(Window * parent, uint8_t moduleIdx):
      Dialog(parent, STR_REGISTER, {50, 73, LCD_W - 100, 0}),
      moduleIdx(moduleIdx)
    {
      FormGroup * form = &content->form;
      FormGridLayout grid(content->form.width());
      grid.setLabelWidth(150);
      grid.spacer(PAGE_PADDING);

      // Register ID
      new StaticText(form, grid.getLabelSlot(), STR_REG_ID);
      auto edit = new RadioTextEdit(form, grid.getFieldSlot(), g_model.modelRegistrationID, sizeof(g_model.modelRegistrationID));
      grid.nextLine();

      // UID
      new StaticText(form, grid.getLabelSlot(), "UID");
      uid = new NumberEdit(form, grid.getFieldSlot(), 0, 2, GET_SET_DEFAULT(reusableBuffer.moduleSetup.pxx2.registerLoopIndex));
      grid.nextLine();

      // RX name
      new StaticText(form, grid.getLabelSlot(), STR_RX_NAME);
      waiting = new StaticText(form, grid.getFieldSlot(), STR_WAITING);
      grid.nextLine();
      grid.spacer(6);

      // Buttons
      exitButton = new TextButton(form, grid.getLabelSlot(), "EXIT",
                                   [=]() -> int8_t {
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

      setCloseHandler([=]() {
          moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
      });
    }

    void start()
    {
      memclear(&reusableBuffer.moduleSetup.pxx2, sizeof(reusableBuffer.moduleSetup.pxx2));
      moduleState[moduleIdx].mode = MODULE_MODE_REGISTER;
    }

    void checkEvents() override
    {
      if (!rxName && reusableBuffer.moduleSetup.pxx2.registerStep >= REGISTER_RX_NAME_RECEIVED) {
        rect_t rect = waiting->getRect();
        waiting->deleteLater();

        rxName = new RadioTextEdit(&content->form, rect, reusableBuffer.moduleSetup.pxx2.registerRxName, PXX2_LEN_RX_NAME);
        rect = exitButton->getRect();
        auto okButton = new TextButton(&content->form, rect, "OK",
                                    [=]() -> int8_t {
                                        reusableBuffer.moduleSetup.pxx2.registerStep = REGISTER_RX_NAME_SELECTED;
                                        return 0;
                                    });
        exitButton->setLeft(left() + rect.w + 10);
        FormField::link(uid, rxName);
        FormField::link(rxName, okButton);
        FormField::link(okButton, exitButton);
        okButton->setFocus(SET_FOCUS_DEFAULT);
      }
      else if (reusableBuffer.moduleSetup.pxx2.registerStep == REGISTER_OK) {
        deleteLater();
        POPUP_INFORMATION(STR_REG_OK);
      }

      Dialog::checkEvents();
    }

  protected:
    uint8_t moduleIdx;
    NumberEdit * uid;
    StaticText * waiting;
    TextEdit * rxName = nullptr;
    TextButton * exitButton;
};

class BindWaitDialog: public Dialog {
  public:
    BindWaitDialog(Window * parent, uint8_t moduleIdx, uint8_t receiverIdx):
      Dialog(parent, STR_BIND, {50, 73, LCD_W - 100, LCD_H - 146}),
      moduleIdx(moduleIdx),
      receiverIdx(receiverIdx)
    {
      new StaticText(&content->form, {0, height() / 2, width(), PAGE_LINE_HEIGHT}, STR_WAITING_FOR_RX, CENTERED);
    }

    void checkEvents() override;

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override
    {
      TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

      if (event == EVT_KEY_BREAK(KEY_EXIT)) {
        moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
        deleteLater();
      }
    }
#endif

  protected:
    uint8_t moduleIdx;
    uint8_t receiverIdx;
};

class BindRxChoiceMenu: public Menu {
  public:
    BindRxChoiceMenu(Window * parent, uint8_t moduleIdx, uint8_t receiverIdx):
      Menu(parent),
      moduleIdx(moduleIdx),
      receiverIdx(receiverIdx)
    {
      auto receiversCount = min<uint8_t>(reusableBuffer.moduleSetup.bindInformation.candidateReceiversCount, PXX2_MAX_RECEIVERS_PER_MODULE);
      for (uint8_t i = 0; i < receiversCount; i++) {
        const char * receiverName = reusableBuffer.moduleSetup.bindInformation.candidateReceiversNames[i];
        addLine(receiverName, [=]() {
            reusableBuffer.moduleSetup.bindInformation.selectedReceiverIndex = i;
            if (isModuleR9MAccess(moduleIdx) && reusableBuffer.moduleSetup.pxx2.moduleInformation.information.variant == PXX2_VARIANT_EU) {
              reusableBuffer.moduleSetup.bindInformation.step = BIND_RX_NAME_SELECTED;
//              POPUP_MENU_ADD_ITEM(STR_16CH_WITH_TELEMETRY);
//              POPUP_MENU_ADD_ITEM(STR_16CH_WITHOUT_TELEMETRY);
//              POPUP_MENU_START(onPXX2R9MBindModeMenu);
            }
            else if (isModuleR9MAccess(moduleIdx) && reusableBuffer.moduleSetup.pxx2.moduleInformation.information.variant == PXX2_VARIANT_FLEX) {
              reusableBuffer.moduleSetup.bindInformation.step = BIND_RX_NAME_SELECTED;
//              POPUP_MENU_ADD_ITEM(STR_FLEX_868);
//              POPUP_MENU_ADD_ITEM(STR_FLEX_915);
//              POPUP_MENU_START(onPXX2R9MBindModeMenu);
            }
            else {
#if defined(SIMU)
              memcpy(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], receiverName, PXX2_LEN_RX_NAME);
              storageDirty(EE_MODEL);
              reusableBuffer.moduleSetup.bindInformation.step = BIND_OK;
              new MessageDialog(parent, STR_BIND, STR_BIND_OK);
#else
              reusableBuffer.moduleSetup.bindInformation.step = BIND_START;
              new BindWaitDialog(parent, moduleIdx, receiverIdx);
#endif
            }
        });
      }

      setCancelHandler([=]() {
          moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
      });
    }

  protected:
    uint8_t moduleIdx;
    uint8_t receiverIdx;
};

void BindWaitDialog::checkEvents()
{
  if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL) {
    removePXX2ReceiverIfEmpty(moduleIdx, receiverIdx);
    deleteLater();
    if (reusableBuffer.moduleSetup.bindInformation.step == BIND_OK)
      POPUP_INFORMATION(STR_BIND_OK);
    return;
  }

  if (reusableBuffer.moduleSetup.bindInformation.step == BIND_INIT && reusableBuffer.moduleSetup.bindInformation.candidateReceiversCount > 0) {
    deleteLater();
    new BindRxChoiceMenu(this, moduleIdx, receiverIdx);
    return;
  }

  Dialog::checkEvents();
}

class ReceiverButton: public TextButton
{
  public:
    ReceiverButton(FormGroup * parent, rect_t rect, uint8_t moduleIdx, uint8_t receiverIdx):
      TextButton(parent, rect, STR_BIND, [=]() {
          if (g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx][0] == '\0') {
            startBind();
          }
          else {
            auto menu = new Menu(parent);
            menu->addLine(STR_BIND, [=]() {
                startBind();
                return 0;
            });
            menu->addLine(STR_OPTIONS, [=]() {
                memclear(&reusableBuffer.hardwareAndSettings, sizeof(reusableBuffer.hardwareAndSettings));
                reusableBuffer.hardwareAndSettings.receiverSettings.receiverId = receiverIdx;
                // g_moduleIdx = moduleIdx;
                // pushMenu(menuModelReceiverOptions);
                return 0;
            });
            menu->addLine(STR_SHARE, [=]() {
                reusableBuffer.moduleSetup.pxx2.shareReceiverIndex = receiverIdx;
                moduleState[moduleIdx].mode = MODULE_MODE_SHARE;
                return 0;
            });
            menu->addLine(STR_DELETE, [=]() {
                memclear(&reusableBuffer.moduleSetup.pxx2, sizeof(reusableBuffer.moduleSetup.pxx2));
                reusableBuffer.moduleSetup.pxx2.resetReceiverIndex = receiverIdx;
                reusableBuffer.moduleSetup.pxx2.resetReceiverFlags = 0x01;
                new ConfirmDialog(parent, STR_RECEIVER, STR_RECEIVER_DELETE, [=]() {
                    moduleState[moduleIdx].mode = MODULE_MODE_RESET;
                    removePXX2Receiver(moduleIdx, receiverIdx);
                });
                return 0;
            });
            menu->addLine(STR_RESET, [=]() {
                memclear(&reusableBuffer.moduleSetup.pxx2, sizeof(reusableBuffer.moduleSetup.pxx2));
                reusableBuffer.moduleSetup.pxx2.resetReceiverIndex = receiverIdx;
                reusableBuffer.moduleSetup.pxx2.resetReceiverFlags = 0xFF;
                new ConfirmDialog(parent, STR_RECEIVER, STR_RECEIVER_DELETE, [=]() {
                    moduleState[moduleIdx].mode = MODULE_MODE_RESET;
                    removePXX2Receiver(moduleIdx, receiverIdx);
                });
                return 0;
            });
            menu->setCloseHandler([=]() {
                removePXX2ReceiverIfEmpty(moduleIdx, receiverIdx);
            });
          }
          return 0;
      }),
      moduleIdx(moduleIdx),
      receiverIdx(receiverIdx)
    {
    }

    void startBind()
    {
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

      new BindWaitDialog(parent, moduleIdx, receiverIdx);
    }

    void checkEvents() override
    {
      if (g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx][0] != '\0') {
        char receiverName[PXX2_LEN_RX_NAME + 1];
        memset(receiverName, 0, sizeof(receiverName));
        strncpy(receiverName, g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx],
                effectiveLen(g_model.moduleData[moduleIdx].pxx2.receiverName[receiverIdx], PXX2_LEN_RX_NAME));
        setText(receiverName);
      }
      else {
        setText(STR_BIND);
      }

      TextButton::checkEvents();
    }

  protected:
    uint8_t moduleIdx;
    uint8_t receiverIdx;
};

class TrainerModuleWindow  : public FormGroup {
  public:
    TrainerModuleWindow(FormWindow * parent, const rect_t & rect) :
      FormGroup(parent, rect, FORWARD_SCROLL | FORM_FORWARD_FOCUS)
    {
      update();
    }

    void update()
    {
      FormGridLayout grid;
      clear();

      new StaticText(this, grid.getLabelSlot(true), STR_MODE);
      trainerChoice = new Choice(this, grid.getFieldSlot(), STR_VTRAINERMODES, 0, TRAINER_MODE_MAX(), GET_DEFAULT(g_model.trainerData.mode), [=](int32_t newValue) {
          g_model.trainerData.mode = newValue;
          SET_DIRTY();
          update();
          trainerChoice->setFocus(SET_FOCUS_DEFAULT);
      });
      trainerChoice->setAvailableHandler(isTrainerModeAvailable);
      grid.nextLine();

      if (g_model.isTrainerTraineeEnable()) {
        new StaticText(this, grid.getLabelSlot(true), STR_CHANNELRANGE);
        channelStart = new NumberEdit(this, grid.getFieldSlot(2, 0), 1,
                                      MAX_OUTPUT_CHANNELS - 8 + g_model.trainerData.channelsCount + 1,
                                      GET_DEFAULT(1 + g_model.trainerData.channelsStart));
        channelEnd = new NumberEdit(this, grid.getFieldSlot(2, 1),
                                    g_model.trainerData.channelsStart + 1,
                                    min<int8_t>(MAX_TRAINER_CHANNELS, g_model.trainerData.channelsStart + MAX_TRAINER_CHANNELS_M8),
                                    GET_DEFAULT(g_model.trainerData.channelsStart + 8 + g_model.trainerData.channelsCount));
        channelStart->setPrefix(STR_CH);
        channelEnd->setPrefix(STR_CH);
        channelStart->setSetValueHandler([=](int32_t newValue) {
            g_model.trainerData.channelsStart = newValue - 1;
            SET_DIRTY();
            channelEnd->setMin(g_model.trainerData.channelsStart + 1);
            channelEnd->setMax(min<int8_t>(MAX_TRAINER_CHANNELS, g_model.trainerData.channelsStart + MAX_TRAINER_CHANNELS_M8));
            channelEnd->invalidate();
        });
        channelEnd->setSetValueHandler([=](int32_t newValue) {
            g_model.trainerData.channelsCount = newValue - g_model.trainerData.channelsStart - 8;
            SET_DIRTY();
            channelStart->setMax(MAX_TRAINER_CHANNELS - 8 + g_model.trainerData.channelsCount + 1);
        });
        grid.nextLine();
      }

      if (g_model.trainerData.mode == TRAINER_MODE_SLAVE) {
        // PPM frame
        new StaticText(this, grid.getLabelSlot(true), STR_PPMFRAME);

        // PPM frame length
        auto edit = new NumberEdit(this, grid.getFieldSlot(3, 0), 125, 35 * 5 + 225,
                                   GET_DEFAULT(g_model.trainerData.frameLength * 5 + 225),
                                   SET_VALUE(g_model.trainerData.frameLength, (newValue - 225) / 5),
                                   0, PREC1);
        edit->setStep(5);
        edit->setSuffix(STR_MS);

        // PPM frame delay
        edit = new NumberEdit(this, grid.getFieldSlot(3, 1), 100, 800,
                              GET_DEFAULT(g_model.trainerData.delay * 50 + 300),
                              SET_VALUE(g_model.trainerData.delay, (newValue - 300) / 50));
        edit->setStep(50);
        edit->setSuffix(STR_US);

        // PPM Polarity
        new Choice(this, grid.getFieldSlot(3, 2), STR_PPM_POL, 0, 1, GET_SET_DEFAULT(g_model.trainerData.pulsePol ));
        grid.nextLine();
      }
      getParent()->moveWindowsTop(top() + 1, adjustHeight());
    }

  protected:
    Choice * trainerChoice = nullptr;
    NumberEdit * channelStart = nullptr;
    NumberEdit * channelEnd = nullptr;
};

class ModuleWindow : public FormGroup {
  public:
    ModuleWindow(FormWindow * parent, const rect_t &rect, uint8_t moduleIdx) :
      FormGroup(parent, rect, FORWARD_SCROLL | FORM_FORWARD_FOCUS),
      moduleIdx(moduleIdx)
    {
      update();
    }

  protected:
    uint8_t moduleIdx;
    Choice * moduleChoice = nullptr;
    Choice * rfChoice = nullptr;
    TextButton * bindButton = nullptr;
    TextButton * rangeButton = nullptr;
    TextButton * registerButton = nullptr;
    Choice * failSafeChoice = nullptr;

    void addChannelRange(FormGridLayout &grid)
    {
      new StaticText(this, grid.getLabelSlot(true), STR_CHANNELRANGE);
      auto channelStart = new NumberEdit(this, grid.getFieldSlot(2, 0), 1,
                                         MAX_OUTPUT_CHANNELS - sentModuleChannels(moduleIdx) + 1,
                                         GET_DEFAULT(1 + g_model.moduleData[moduleIdx].channelsStart));
      auto channelEnd = new NumberEdit(this, grid.getFieldSlot(2, 1),
                                       g_model.moduleData[moduleIdx].channelsStart + minModuleChannels(moduleIdx),
                                       min<int8_t>(MAX_OUTPUT_CHANNELS, g_model.moduleData[moduleIdx].channelsStart + maxModuleChannels(moduleIdx)),
                                       GET_DEFAULT(g_model.moduleData[moduleIdx].channelsStart + 8 + g_model.moduleData[moduleIdx].channelsCount));
      if (isModulePXX2(moduleIdx)) {
        channelEnd->setAvailableHandler([=](int value) {
            return isPxx2IsrmChannelsCountAllowed(value - 8);
        });
      }
      channelStart->setPrefix(STR_CH);
      channelEnd->setPrefix(STR_CH);
      channelStart->setSetValueHandler([=](int32_t newValue) {
        g_model.moduleData[moduleIdx].channelsStart = newValue - 1;
        SET_DIRTY();
        channelEnd->setMin(g_model.moduleData[moduleIdx].channelsStart + minModuleChannels(moduleIdx));
        channelEnd->setMax(min<int8_t>(MAX_OUTPUT_CHANNELS, g_model.moduleData[moduleIdx].channelsStart + maxModuleChannels(moduleIdx)));
        channelEnd->invalidate();
      });
      channelEnd->setSetValueHandler([=](int32_t newValue) {
        g_model.moduleData[moduleIdx].channelsCount = newValue - g_model.moduleData[moduleIdx].channelsStart - 8;
        SET_DIRTY();
        channelStart->setMax(MAX_OUTPUT_CHANNELS - sentModuleChannels(moduleIdx) + 1);
      });
      channelEnd->enable(minModuleChannels(moduleIdx) < maxModuleChannels(moduleIdx));
    }

    void update()
    {
      FormGridLayout grid;
      clear();

      // Module Type
      new StaticText(this, grid.getLabelSlot(true), STR_MODE);
      moduleChoice = new Choice(this, grid.getFieldSlot(2, 0), STR_INTERNAL_MODULE_PROTOCOLS,
                                MODULE_TYPE_NONE, MODULE_TYPE_COUNT - 1,
                                GET_DEFAULT(g_model.moduleData[moduleIdx].type),
                                [=](int32_t newValue) {
                                  g_model.moduleData[moduleIdx].type = newValue;
                                  SET_DIRTY();
                                  // TODO resetModuleSettings(moduleIdx);
                                  update();
                                  moduleChoice->setFocus(SET_FOCUS_DEFAULT);
                                });
      moduleChoice->setAvailableHandler([=](int8_t moduleType) {
          return moduleIdx == INTERNAL_MODULE ? isInternalModuleAvailable(moduleType) : isExternalModuleAvailable(moduleType);
      });

      // Module parameters
      if (isModuleXJT(moduleIdx)) {
        auto xjtChoice = new Choice(this, grid.getFieldSlot(2, 1), STR_XJT_ACCST_RF_PROTOCOLS, MODULE_SUBTYPE_PXX1_OFF, MODULE_SUBTYPE_PXX1_ACCST_LR12,
                                    GET_SET_DEFAULT(g_model.moduleData[moduleIdx].rfProtocol));
        xjtChoice->setAvailableHandler([](int index) {
          return index != MODULE_SUBTYPE_PXX1_OFF;
        });
      }
      else if (isModuleDSM2(moduleIdx)) {
        new Choice(this, grid.getFieldSlot(2, 1), STR_DSM_PROTOCOLS, DSM2_PROTO_LP45, DSM2_PROTO_DSMX,
                   GET_SET_DEFAULT(g_model.moduleData[moduleIdx].rfProtocol));
      }
      else if (isModuleR9M(moduleIdx)) {
        rfChoice = new Choice(this, grid.getFieldSlot(2, 1), STR_R9M_REGION, MODULE_SUBTYPE_R9M_FCC, MODULE_SUBTYPE_R9M_EU,
                              GET_DEFAULT(g_model.moduleData[moduleIdx].subType),
                              [=](int32_t newValue) {
                                  g_model.moduleData[moduleIdx].subType = newValue;
                                  SET_DIRTY();
                                  update();
                                  rfChoice->setFocus(SET_FOCUS_DEFAULT);
                              });
      }
      else if (isModulePXX2(moduleIdx)) {
        rfChoice = new Choice(this, grid.getFieldSlot(2, 1), STR_ISRM_RF_PROTOCOLS, 0, MODULE_SUBTYPE_ISRM_PXX2_ACCST_LR12,
                              GET_DEFAULT(g_model.moduleData[moduleIdx].subType),
                              [=](int32_t newValue) {
                                  g_model.moduleData[moduleIdx].subType = newValue;
                                  SET_DIRTY();
                                  update();
                                  rfChoice->setFocus(SET_FOCUS_DEFAULT);
                              });
      }
#if defined(MULTIMODULE)
      else if (isModuleMultimodule(moduleIdx)) {
        Choice * mmSubProtocol = nullptr;
        grid.nextLine();
        new StaticText(this, grid.getLabelSlot(true), STR_RF_PROTOCOL);

        // Multi type (CUSTOM, brand A, brand B,...)
        int multiRfProto = g_model.moduleData[moduleIdx].getMultiProtocol();
        rfChoice = new Choice(
            this,
            grid.getFieldSlot(
                g_model.moduleData[moduleIdx].multi.customProto ? 3 : 2, 0),
            STR_MULTI_PROTOCOLS, MODULE_SUBTYPE_MULTI_FIRST,
            MODULE_SUBTYPE_MULTI_LAST, GET_DEFAULT(multiRfProto),
            [=](int32_t newValue) {
              g_model.moduleData[moduleIdx].setMultiProtocol(newValue);
              g_model.moduleData[moduleIdx].subType = 0;
              if (mmSubProtocol != nullptr) mmSubProtocol->invalidate(); // always null ???
              resetMultiProtocolsOptions(moduleIdx);
              SET_DIRTY();
              update();
              rfChoice->setFocus(SET_FOCUS_DEFAULT);// really valid ???
            });

        if (MULTIMODULE_HAS_SUBTYPE(moduleIdx)) {
          // Subtype (D16, DSMX,...)
          const mm_protocol_definition *pdef = getMultiProtocolDefinition(
              g_model.moduleData[moduleIdx].getMultiProtocol());
          if (pdef->maxSubtype > 0) {
            mmSubProtocol = new Choice(
                this, grid.getFieldSlot(2, 1), pdef->subTypeString, 0,
                pdef->maxSubtype,
                [=]() { return g_model.moduleData[moduleIdx].subType; },
                [=](int16_t newValue) {
                  g_model.moduleData[moduleIdx].subType = newValue;
                  resetMultiProtocolsOptions(moduleIdx);
                  SET_DIRTY();
                  // update(); ??? how to set focus properly ???
                  update();
                  //mmSubProtocol->setFocus(SET_FOCUS_DEFAULT);
                });

          }
        }
        grid.nextLine();

        // Multimodule status
        new StaticText(this, grid.getLabelSlot(true), STR_MODULE_STATUS);
        new DynamicText(this, grid.getFieldSlot(), [=] {
            char msg[64] = "";
            getModuleStatusString(moduleIdx, msg);
            return std::string(msg);
        });

        // Multimodule sync
        /*if (multiSyncStatus.isValid()) {
          grid.nextLine();
          new StaticText(this, grid.getLabelSlot(true), STR_MODULE_SYNC);
          multiSyncStatus.getRefreshString(statusText);
          new StaticText(this, grid.getFieldSlot(), statusText);
        }*/

        const uint8_t multi_proto = g_model.moduleData[moduleIdx].getMultiProtocol();
        if (MULTIMODULE_PROTOCOL_KNOWN(moduleIdx)) {
          // Multi optional feature row
          const char *title = getMultiOptionTitleStatic(moduleIdx);
          if (title != nullptr) {
            grid.nextLine();
            new StaticText(this, grid.getLabelSlot(true), title);

            // int optionValue =
            // g_model.moduleData[moduleIdx].multi.optionValue;
            int8_t min, max;
            getMultiOptionValues(multi_proto, min, max);

            if (title == STR_MULTI_RFPOWER) {
              new Choice(this, grid.getFieldSlot(2, 0), STR_MULTI_POWER, 0, 15,
                         GET_SET_DEFAULT(
                             g_model.moduleData[moduleIdx].multi.optionValue));
            } else if (title == STR_MULTI_TELEMETRY) {
              new Choice(this, grid.getFieldSlot(2, 0),
                         STR_MULTI_TELEMETRY_MODE, min, max,
                         GET_SET_DEFAULT(
                             g_model.moduleData[moduleIdx].multi.optionValue));
            } else if (title == STR_MULTI_WBUS) {
              new Choice(this, grid.getFieldSlot(2, 0), STR_MULTI_WBUS_MODE, 0,
                         1,
                         GET_SET_DEFAULT(
                             g_model.moduleData[moduleIdx].multi.optionValue));
            } else if (multi_proto == MODULE_SUBTYPE_MULTI_FS_AFHDS2A) {
              auto edit = new NumberEdit(
                  this, grid.getFieldSlot(2, 0), 50, 400,
                  GET_DEFAULT(
                      50 + 5 * g_model.moduleData[moduleIdx].multi.optionValue),
                  SET_VALUE(g_model.moduleData[moduleIdx].multi.optionValue,
                            (newValue - 50) / 5));
              edit->setStep(5);
            } else if (multi_proto == MODULE_SUBTYPE_MULTI_DSM2) {
              new CheckBox(
                  this, grid.getFieldSlot(2, 0),
                  [=]() {
                    return g_model.moduleData[moduleIdx].multi.optionValue &
                           0x01;
                  },
                  [=](int16_t newValue) {
                    g_model.moduleData[moduleIdx].multi.optionValue =
                        (g_model.moduleData[moduleIdx].multi.optionValue &
                         0xFE) +
                        newValue;
                  });
            } else {
              if (min == 0 && max == 1) {
                new CheckBox(
                    this, grid.getFieldSlot(2, 0),
                    GET_SET_DEFAULT(
                        g_model.moduleData[moduleIdx].multi.optionValue));
              } else {
                new NumberEdit(
                    this, grid.getFieldSlot(2, 0), -128, 127,
                    GET_SET_DEFAULT(
                        g_model.moduleData[moduleIdx].multi.optionValue));

                // Show RSSI next to RF Freq Fine Tune
                if (title == STR_MULTI_RFTUNE) {
                  new DynamicNumber<int>(
                      this, grid.getFieldSlot(2, 1),
                      [] { return (int)TELEMETRY_RSSI(); }, 0, "RSSI: ", " db");
                }
              }
            }
          }
        }
        grid.nextLine();

        if (multi_proto == MODULE_SUBTYPE_MULTI_DSM2) {

          const char* servoRates[] = { "22ms", "11ms" };
          
          new StaticText(this, grid.getLabelSlot(true), STR_MULTI_SERVOFREQ);
          new Choice(
              this, grid.getFieldSlot(), servoRates, 0, 1,
              [=]() {
                return (g_model.moduleData[moduleIdx].multi.optionValue & 0x02) >> 1;
              },
              [=](int16_t newValue) {
                g_model.moduleData[moduleIdx].multi.optionValue =
                    (g_model.moduleData[moduleIdx].multi.optionValue & 0xFD) +
                    (newValue << 1);
              });
        } else {
          // Bind on power up
          new StaticText(this, grid.getLabelSlot(true), STR_MULTI_AUTOBIND);
          new CheckBox(this, grid.getFieldSlot(),
                       GET_SET_DEFAULT(
                           g_model.moduleData[moduleIdx].multi.autoBindMode));
        }

        // Low power mode
        grid.nextLine();
        new StaticText(this, grid.getLabelSlot(true), STR_MULTI_LOWPOWER);
        new CheckBox(
            this, grid.getFieldSlot(),
            GET_SET_DEFAULT(g_model.moduleData[moduleIdx].multi.lowPowerMode));

        // Disable telemetry
        grid.nextLine();
        new StaticText(this, grid.getLabelSlot(true), STR_DISABLE_TELEM);
        new CheckBox(this, grid.getFieldSlot(),
                     GET_SET_DEFAULT(
                         g_model.moduleData[moduleIdx].multi.disableTelemetry));

        if (MULTI_DISABLE_CHAN_MAP_ROW_STATIC(moduleIdx) != HIDDEN_ROW) {
          // Disable channel mapping
          grid.nextLine();
          new StaticText(this, grid.getLabelSlot(true), STR_DISABLE_CH_MAP);
          new CheckBox(this, grid.getFieldSlot(),
                       GET_SET_DEFAULT(g_model.moduleData[moduleIdx].multi.disableMapping));
        }
      }
#endif
#if defined(AFHDS3)
      else if (isModuleAFHDS3(moduleIdx)) {
        rfChoice = new Choice(this, grid.getFieldSlot(2, 1), STR_AFHDS3_PROTOCOLS, AFHDS_SUBTYPE_FIRST, AFHDS_SUBTYPE_LAST,
                              GET_SET_DEFAULT(g_model.moduleData[moduleIdx].subType));

        // TYPE
        grid.nextLine();
        new StaticText(this, grid.getLabelSlot(true), STR_TYPE);
        new StaticText(this, grid.getFieldSlot(),
                       g_model.moduleData[EXTERNAL_MODULE].afhds3.telemetry ? STR_AFHDS3_ONE_TO_ONE_TELEMETRY : TR_AFHDS3_ONE_TO_MANY);

        // Status
        grid.nextLine();
        new StaticText(this, grid.getLabelSlot(true), STR_MODULE_STATUS);
        new DynamicText(this, grid.getFieldSlot(), [=] {
            char msg[64] = "";
            getModuleStatusString(moduleIdx, msg);
            return std::string(msg);
        });


        // Power source
        grid.nextLine();
        new StaticText(this, grid.getLabelSlot(true), STR_AFHDS3_POWER_SOURCE);
        new DynamicText(this, grid.getFieldSlot(), [=] {
            char msg[64] = "";
            getModuleSyncStatusString(moduleIdx, msg);
            return std::string(msg);
        });

        // RX Freq
        grid.nextLine();
        new StaticText(this, grid.getLabelSlot(true), STR_AFHDS3_RX_FREQ);
        auto edit = new NumberEdit(this, grid.getFieldSlot(2,0), MIN_FREQ, MAX_FREQ, GET_DEFAULT(g_model.moduleData[moduleIdx].afhds3.rxFreq()));
        edit->setSetValueHandler([=](int32_t newValue) {
            g_model.moduleData[EXTERNAL_MODULE].afhds3.setRxFreq((uint16_t)newValue);
        });
        edit->setSuffix(STR_HZ);

        // Module actual power
        grid.nextLine();
        new StaticText(this, grid.getLabelSlot(true), STR_AFHDS3_ACTUAL_POWER);
        new DynamicText(this, grid.getFieldSlot(), [=] {
            char msg[64] = "";
            getStringAtIndex(msg, STR_AFHDS3_POWERS, actualAfhdsRunPower(moduleIdx));
            return std::string(msg);
        });

        // Module power
        grid.nextLine();
        new StaticText(this, grid.getLabelSlot(true), STR_RF_POWER);
        new Choice(this, grid.getFieldSlot(2, 0), STR_AFHDS3_POWERS, afhds3::RUN_POWER::RUN_POWER_FIRST, afhds3::RUN_POWER::RUN_POWER_LAST,
                    GET_SET_DEFAULT(g_model.moduleData[moduleIdx].afhds3.runPower));
      }
#endif
      grid.nextLine();

      // Channel Range
      if (g_model.moduleData[moduleIdx].type != MODULE_TYPE_NONE) {
        addChannelRange(grid); //TODO XJT2 should only set channel count of 8/16/24
        grid.nextLine();
      }

      // PPM modules
      if (isModulePPM(moduleIdx)) {
        // PPM frame
        new StaticText(this, grid.getLabelSlot(true), STR_PPMFRAME);

        // PPM frame length
        auto edit = new NumberEdit(this, grid.getFieldSlot(3, 0), 125, 35 * 5 + 225,
                                   GET_DEFAULT(g_model.moduleData[moduleIdx].ppm.frameLength * 5 + 225),
                                   SET_VALUE(g_model.moduleData[moduleIdx].ppm.frameLength, (newValue - 225) / 5),
                                   0, PREC1);
        edit->setStep(5);
        edit->setSuffix(STR_MS);

        // PPM frame delay
        edit = new NumberEdit(this, grid.getFieldSlot(3, 1), 100, 800,
                              GET_DEFAULT(g_model.moduleData[moduleIdx].ppm.delay * 50 + 300),
                              SET_VALUE(g_model.moduleData[moduleIdx].ppm.delay, (newValue - 300) / 50));
        edit->setStep(50);
        edit->setSuffix(STR_US);

        // PPM Polarity
        new Choice(this, grid.getFieldSlot(3, 2), STR_PPM_POL, 0, 1, GET_SET_DEFAULT(g_model.moduleData[moduleIdx].ppm.pulsePol ));
        grid.nextLine();
      }

      // Module parameters

      // Bind and Range buttons
      if (!isModuleRFAccess(moduleIdx) && (isModuleModelIndexAvailable(moduleIdx)|| isModuleBindRangeAvailable(moduleIdx))) {
        uint8_t thirdColumn = 0;
        new StaticText(this, grid.getLabelSlot(true), STR_RECEIVER);

        // Model index
        if (isModuleModelIndexAvailable(moduleIdx)) {
          thirdColumn++;
          new NumberEdit(this, grid.getFieldSlot(3, 0), 0, getMaxRxNum(moduleIdx), GET_SET_DEFAULT(g_model.header.modelId[moduleIdx]));
        }

        if (isModuleBindRangeAvailable(moduleIdx)) {
          bindButton = new TextButton(this, grid.getFieldSlot(2 + thirdColumn, 0 + thirdColumn), STR_MODULE_BIND);
          bindButton->setPressHandler([=]() -> uint8_t {
              if (moduleState[moduleIdx].mode == MODULE_MODE_RANGECHECK) {
                rangeButton->check(false);
              }
              if (moduleState[moduleIdx].mode == MODULE_MODE_BIND) {
                moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
                return 0;
              }
#if defined(MULTIMODULE)
              else {
                setMultiBindStatus(moduleIdx, MULTI_BIND_INITIATED);
                moduleState[moduleIdx].mode = MODULE_MODE_BIND;
                return 1;
              }
#endif
              return 0;
          });
          bindButton->setCheckHandler([=]() {
              if (moduleState[moduleIdx].mode != MODULE_MODE_BIND) {
                bindButton->check(false);
              }
#if defined(MULTIMODULE)
              if (getMultiBindStatus(moduleIdx) == MULTI_BIND_FINISHED) {
                setMultiBindStatus(moduleIdx, MULTI_BIND_NONE);
                moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
                bindButton->check(false);
              }
#endif
          });

          rangeButton = new TextButton(this, grid.getFieldSlot(2 + thirdColumn, 1 + thirdColumn), STR_MODULE_RANGE);
          rangeButton->setPressHandler([=]() -> uint8_t {
              if (moduleState[moduleIdx].mode == MODULE_MODE_BIND) {
                bindButton->check(false);
                moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
              }
              if (moduleState[moduleIdx].mode == MODULE_MODE_RANGECHECK) {
                moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
                return 0;
              }
              else {
                moduleState[moduleIdx].mode = MODULE_MODE_RANGECHECK;
                auto rssiDialog = new DynamicMessageDialog(
                    this, "Range Test",
                    [=]() {
                      char buf[16];
                      sprintf(buf, "%d db", (int)TELEMETRY_RSSI());
                      return std::string(buf);
                    },
                    "RSSI:", 50,
                    DEFAULT_COLOR | CENTERED | FONT(BOLD) | FONT(XL));
                rssiDialog->setCloseHandler([this]() {
                  rangeButton->check(false);
                  moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
                });
                return 1;
              }
          });
        }

        grid.nextLine();
      }

      // Failsafe
      if (isModuleFailsafeAvailable(moduleIdx)) {
        hasFailsafe = true;
        new StaticText(this, grid.getLabelSlot(true), STR_FAILSAFE);
        failSafeChoice = new Choice(this, grid.getFieldSlot(2, 0), STR_VFAILSAFE, 0, FAILSAFE_LAST,
                                    GET_DEFAULT(g_model.moduleData[moduleIdx].failsafeMode),
                                    [=](int32_t newValue) {
                                      g_model.moduleData[moduleIdx].failsafeMode = newValue;
                                      SET_DIRTY();
                                      update();
                                      failSafeChoice->setFocus(SET_FOCUS_DEFAULT);
                                    });
        if (g_model.moduleData[moduleIdx].failsafeMode == FAILSAFE_CUSTOM) {
          new TextButton(this, grid.getFieldSlot(2, 1), STR_SET,
                         [=]() -> uint8_t {
                           new FailSafePage(moduleIdx);
                           return 1;
                         });
        }
        grid.nextLine();
      }

      // Register and Range buttons
      if (isModuleRFAccess(moduleIdx)) {
        new StaticText(this, grid.getLabelSlot(true), STR_MODULE);
        registerButton = new TextButton(this, grid.getFieldSlot(2, 0), STR_REGISTER);
        registerButton->setPressHandler([=]() -> uint8_t {
            new RegisterDialog(this, moduleIdx);
            return 0;
        });

        rangeButton = new TextButton(this, grid.getFieldSlot(2, 1), STR_MODULE_RANGE);
        rangeButton->setPressHandler([=]() -> uint8_t {
            if (moduleState[moduleIdx].mode == MODULE_MODE_RANGECHECK) {
              moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
              return 0;
            }
            else {
              moduleState[moduleIdx].mode = MODULE_MODE_RANGECHECK;
              return 1;
            }
        });

        grid.nextLine();
      }

      // R9M Power
      if (isModuleR9M_FCC(moduleIdx)) {
        new StaticText(this, grid.getLabelSlot(true), STR_RF_POWER);
        new Choice(this, grid.getFieldSlot(), STR_R9M_FCC_POWER_VALUES, 0, R9M_FCC_POWER_MAX,
                   GET_SET_DEFAULT(g_model.moduleData[moduleIdx].pxx.power));
      }

      if (isModuleR9M_LBT(moduleIdx)) {
        new StaticText(this, grid.getLabelSlot(true), STR_RF_POWER);
        new Choice(this, grid.getFieldSlot(), STR_R9M_LBT_POWER_VALUES, 0, R9M_LBT_POWER_MAX,
                   GET_DEFAULT(min<uint8_t>(g_model.moduleData[moduleIdx].pxx.power, R9M_LBT_POWER_MAX)),
                   SET_DEFAULT(g_model.moduleData[moduleIdx].pxx.power));
      }

      // Receivers
      if (isModulePXX2(moduleIdx)) {
        for (uint8_t receiverIdx = 0; receiverIdx < PXX2_MAX_RECEIVERS_PER_MODULE; receiverIdx++) {
          char label[] = TR_RECEIVER " X";
          label[sizeof(label) - 2] = '1' + receiverIdx;
          new StaticText(this, grid.getLabelSlot(true), label);
          new ReceiverButton(this, grid.getFieldSlot(2, 0), moduleIdx, receiverIdx);
          grid.nextLine();
        }
      }

      // SBUS refresh rate
      if (isModuleSBUS(moduleIdx)) {
        new StaticText(this, grid.getLabelSlot(true), STR_REFRESHRATE);
        auto edit = new NumberEdit(this, grid.getFieldSlot(2, 0), 60, 400,
                                           GET_DEFAULT((int16_t)g_model.moduleData[moduleIdx].ppm.frameLength*5 + 225),
                                           SET_VALUE(g_model.moduleData[moduleIdx].ppm.frameLength, (newValue - 225)/5),
                                           0, PREC1);
        edit->setSuffix(STR_MS);
        new Choice(this, grid.getFieldSlot(2, 1), STR_SBUS_INVERSION_VALUES, 0, 1, GET_SET_DEFAULT(g_model.moduleData[moduleIdx].sbus.noninverted));
#if defined(RADIO_TX16S)
        grid.nextLine();
        new StaticText(this, grid.getFieldSlot(1, 0), STR_WARN_5VOLTS);
#endif
        grid.nextLine();
      }

      getParent()->moveWindowsTop(top() + 1, adjustHeight());
    }

    void checkEvents() override
    {
      if (isModuleFailsafeAvailable(moduleIdx) != hasFailsafe) {
        hasFailsafe = isModuleFailsafeAvailable(moduleIdx);
        update();
      }

      FormGroup::checkEvents();
    }

  protected:
    bool hasFailsafe = false;
};

ModelSetupPage::ModelSetupPage() :
  PageTab(STR_MENU_MODEL_SETUP, ICON_MODEL_SETUP)
{
}

uint8_t g_moduleIdx;

void onBindMenu(const char * result)
{
  uint8_t moduleIdx = 0; // TODO (menuVerticalPosition >= ITEM_MODEL_EXTERNAL_MODULE_LABEL ? EXTERNAL_MODULE : INTERNAL_MODULE);

  if (result == STR_BINDING_1_8_TELEM_ON) {
    g_model.moduleData[moduleIdx].pxx.receiverTelemetryOff = false;
    g_model.moduleData[moduleIdx].pxx.receiverHigherChannels = false;
  }
  else if (result == STR_BINDING_1_8_TELEM_OFF) {
    g_model.moduleData[moduleIdx].pxx.receiverTelemetryOff = true;
    g_model.moduleData[moduleIdx].pxx.receiverHigherChannels = false;
  }
  else if (result == STR_BINDING_9_16_TELEM_ON) {
    g_model.moduleData[moduleIdx].pxx.receiverTelemetryOff = false;
    g_model.moduleData[moduleIdx].pxx.receiverHigherChannels = true;
  }
  else if (result == STR_BINDING_9_16_TELEM_OFF) {
    g_model.moduleData[moduleIdx].pxx.receiverTelemetryOff = true;
    g_model.moduleData[moduleIdx].pxx.receiverHigherChannels = true;
  }
  else {
    return;
  }

  moduleState[moduleIdx].mode  = MODULE_MODE_BIND;
}

const char * STR_TIMER_MODES[] = {"OFF", "ON", "Start", "Throttle", "Throttle %", "Throttle Start"};

const char MODEL_NAME_EXTRA_CHARS[] = "_-.,:;<=>";

void ModelSetupPage::build(FormWindow * window)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);

  // Model name
  new StaticText(window, grid.getLabelSlot(), STR_MODELNAME);
  auto text =
      new ModelTextEdit(window, grid.getFieldSlot(), g_model.header.name,
                        sizeof(g_model.header.name), 0, MODEL_NAME_EXTRA_CHARS);
  text->setChangeHandler([=] {
    modelslist.load();
    auto model = modelslist.getCurrentModel();
    if (model) {
      model->setModelName(g_model.header.name);
    }
    SET_DIRTY();
  });
  grid.nextLine();

  // Bitmap
  new StaticText(window, grid.getLabelSlot(), STR_BITMAP);
  new FileChoice(window, grid.getFieldSlot(), BITMAPS_PATH, BITMAPS_EXT, sizeof(g_model.header.bitmap),
                 [=]() {
                   return std::string(g_model.header.bitmap, sizeof(g_model.header.bitmap));
                 },
                 [=](std::string newValue) {
                   strncpy(g_model.header.bitmap, newValue.c_str(), sizeof(g_model.header.bitmap));
                   SET_DIRTY();
                 });
  grid.nextLine();

  for (uint8_t i = 0; i < TIMERS; i++) {
    TimerData * timer = &g_model.timers[i];

    // Timer label
    strAppendStringWithIndex(reusableBuffer.moduleSetup.msg, STR_TIMER, i + 1);
    new Subtitle(window, grid.getLineSlot(), reusableBuffer.moduleSetup.msg);
    grid.nextLine();

    auto group = new FormGroup(window, grid.getFieldSlot(), FORM_BORDER_FOCUS_ONLY | PAINT_CHILDREN_FIRST);
    GridLayout timerGrid(group);

    // Timer name
    new StaticText(window, grid.getLabelSlot(true), STR_NAME);
    grid.nextLine();
    new RadioTextEdit(group, timerGrid.getSlot(), timer->name, LEN_TIMER_NAME);
    timerGrid.nextLine();

    // Timer mode
    new StaticText(window, grid.getLabelSlot(true), STR_MODE);
    grid.nextLine();
    new Choice(group, timerGrid.getSlot(2, 0), STR_TIMER_MODES, 0, TMRMODE_MAX, GET_SET_DEFAULT(timer->mode));
    new SwitchChoice(group, timerGrid.getSlot(2, 1), SWSRC_FIRST, SWSRC_LAST, GET_SET_DEFAULT(timer->swtch));
    timerGrid.nextLine();

    // Timer start value
    new StaticText(window, grid.getLabelSlot(true), "Start");
    grid.nextLine();
    new TimeEdit(group, timerGrid.getSlot(), 0, TIMER_MAX, GET_SET_DEFAULT(timer->start));
    timerGrid.nextLine();

    // Timer minute beep
    new StaticText(window, grid.getLabelSlot(true), STR_MINUTEBEEP);
    grid.nextLine();
    new CheckBox(group, timerGrid.getSlot(), GET_SET_DEFAULT(timer->minuteBeep));
    timerGrid.nextLine();

    // Timer countdown
    new StaticText(window, grid.getLabelSlot(true), STR_BEEPCOUNTDOWN);
    grid.nextLine();
    new Choice(group, timerGrid.getSlot(2, 0), STR_VBEEPCOUNTDOWN, COUNTDOWN_SILENT, COUNTDOWN_COUNT - 1, GET_SET_DEFAULT(timer->countdownBeep));
    new Choice(group, timerGrid.getSlot(2, 1), STR_COUNTDOWNVALUES, 0, 3, GET_SET_WITH_OFFSET(timer->countdownStart, 2));
    timerGrid.nextLine();

    // Timer persistent
    new StaticText(window, grid.getLabelSlot(true), STR_PERSISTENT);
    grid.nextLine();
    new Choice(group, timerGrid.getSlot(), STR_VPERSISTENT, 0, 2, GET_SET_DEFAULT(timer->persistent));
    timerGrid.nextLine();

    coord_t height = timerGrid.getWindowHeight() - 1;
    group->setHeight(height);
  }

  // Extended limits
  new StaticText(window, grid.getLabelSlot(), STR_ELIMITS);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.extendedLimits));
  grid.nextLine();

  // Extended trims
  new StaticText(window, grid.getLabelSlot(), STR_ETRIMS);
  new CheckBox(window, grid.getFieldSlot(2, 0), GET_SET_DEFAULT(g_model.extendedTrims));
  new TextButton(window, grid.getFieldSlot(2, 1), STR_RESET_BTN,
                 []() -> uint8_t {
                   for (auto &flightModeData : g_model.flightModeData) {
                     memclear(&flightModeData, TRIMS_ARRAY_SIZE);
                   }
                   storageDirty(EE_MODEL);
                   AUDIO_WARNING1();
                   return 0;
                 });
  grid.nextLine();

  // Display trims
  new StaticText(window, grid.getLabelSlot(), STR_DISPLAY_TRIMS);
  new Choice(window, grid.getFieldSlot(), "\006No\0   ChangeYes", 0, 2, GET_SET_DEFAULT(g_model.displayTrims));
  grid.nextLine();

  // Trim step
  new StaticText(window, grid.getLabelSlot(), STR_TRIMINC);
  new Choice(window, grid.getFieldSlot(), STR_VTRIMINC, -2, 2, GET_SET_DEFAULT(g_model.trimInc));
  grid.nextLine();

  // Throttle parameters
  {
    new Subtitle(window, grid.getLineSlot(), STR_THROTTLE_LABEL);
    grid.nextLine();

    // Throttle reversed
    new StaticText(window, grid.getLabelSlot(true), STR_THROTTLEREVERSE);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.throttleReversed));
    grid.nextLine();

    // Throttle source
    new StaticText(window, grid.getLabelSlot(true), STR_TTRACE);
    auto sc = new SourceChoice(
        window, grid.getFieldSlot(), 0, MIXSRC_LAST_CH,
        [=]() { return throttleSource2Source(g_model.thrTraceSrc); },
        [=](int16_t src) {
          int16_t val = source2ThrottleSource(src);
          if (val >= 0) {
            g_model.thrTraceSrc = val;
            SET_DIRTY();
          }
        });
    sc->setAvailableHandler(isThrottleSourceAvailable);
    grid.nextLine();

    // Throttle trim
    new StaticText(window, grid.getLabelSlot(true), STR_TTRIM);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.thrTrim));
    grid.nextLine();

    // Throttle trim source
    new StaticText(window, grid.getLabelSlot(true), STR_TTRIM_SW);
    new SourceChoice(
        window, grid.getFieldSlot(), MIXSRC_FIRST_TRIM, MIXSRC_LAST_TRIM,
        [=]() { return g_model.getThrottleStickTrimSource(); },
        [=](int16_t src) { g_model.setThrottleStickTrimSource(src); });
    grid.nextLine();
  }

  // Preflight parameters
  {
    new Subtitle(window, grid.getLineSlot(), STR_PREFLIGHT);
    grid.nextLine();

    // Display checklist
    new StaticText(window, grid.getLabelSlot(true), STR_CHECKLIST);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.displayChecklist));
    grid.nextLine();

    // Throttle warning
    new StaticText(window, grid.getLabelSlot(true), STR_THROTTLE_WARNING);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_INVERTED(g_model.disableThrottleWarning));
    grid.nextLine();

    // Switches warning
    new StaticText(window, grid.getLabelSlot(true), STR_SWITCHWARNING);
    auto group = new FormGroup(window, grid.getFieldSlot(), FORM_BORDER_FOCUS_ONLY | PAINT_CHILDREN_FIRST);
    GridLayout switchesGrid(group);
    for (int i = 0, j = 0; i < NUM_SWITCHES; i++) {
      if (SWITCH_EXISTS(i)) {
        if (j > 0 && (j % 3) == 0)
          switchesGrid.nextLine();
        auto button = new TextButton(group, switchesGrid.getSlot(3, j % 3), switchWarninglabel(i), nullptr,
                                     OPAQUE | (bfGet(g_model.switchWarningState, 3 * i, 3) == 0 ? 0 : BUTTON_CHECKED));
        button->setPressHandler([button, i] {
            swarnstate_t newstate = bfGet(g_model.switchWarningState, 3 * i, 3);
            if (newstate == 1 && SWITCH_CONFIG(i) != SWITCH_3POS)
              newstate = 3;
            else
              newstate = (newstate + 1) % 4;
            g_model.switchWarningState = bfSet(g_model.switchWarningState, newstate, 3 * i, 3);
            SET_DIRTY();
            button->setText(switchWarninglabel(i));
            return newstate > 0;
        });
        j++;
      }
    }
    grid.addWindow(group);
  }

  // Center beeps
  {
    new StaticText(window, grid.getLabelSlot(false), STR_BEEPCTR);
    auto group = new FormGroup(window, grid.getFieldSlot(), FORM_BORDER_FOCUS_ONLY | PAINT_CHILDREN_FIRST);
    GridLayout centerGrid(group);
    for (int i = 0, j = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
      char s[2];
      if (i < NUM_STICKS ||  (IS_POT_SLIDER_AVAILABLE(i) && !IS_POT_MULTIPOS(i))) { // multipos cannot be centered
        if (j > 0 && (j % 6) == 0)
          centerGrid.nextLine();

        new TextButton(group, centerGrid.getSlot(6, j % 6), getStringAtIndex(s, STR_RETA123, i),
                       [=]() -> uint8_t {
                           BFBIT_FLIP(g_model.beepANACenter, bfBit<BeepANACenter>(i));
                           SET_DIRTY();
                           return bfSingleBitGet<BeepANACenter>(g_model.beepANACenter, i);
                       },
                       OPAQUE | (bfSingleBitGet(g_model.beepANACenter, i) ? BUTTON_CHECKED : 0));
        j++;
      }
    }
    grid.addWindow(group);
  }

  // Global functions
  new StaticText(window, grid.getLabelSlot(), STR_USE_GLOBAL_FUNCS);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_INVERTED(g_model.noGlobalFunctions));
  grid.nextLine();

  // Internal module
  {
    new Subtitle(window, grid.getLineSlot(), TR_INTERNALRF);
    grid.nextLine();
    grid.addWindow(new ModuleWindow(window, {0, grid.getWindowHeight(), LCD_W, 0}, INTERNAL_MODULE));
  }

  // External module
  {
    new Subtitle(window, grid.getLineSlot(), TR_EXTERNALRF);
    grid.nextLine();
    grid.addWindow(new ModuleWindow(window, {0, grid.getWindowHeight(), LCD_W, 0}, EXTERNAL_MODULE));
  }

  // Trainer
  {
    new Subtitle(window, grid.getLineSlot(), STR_TRAINER);
    grid.nextLine();
    grid.addWindow(new TrainerModuleWindow(window, {0, grid.getWindowHeight(), LCD_W, 0}));
  }

  window->setInnerHeight(grid.getWindowHeight());
}

// Switch to external antenna confirmation
//  bool newAntennaSel;
//  if (warningResult) {
//    warningResult = 0;
//    g_model.moduleData[INTERNAL_MODULE].pxx.external_antenna = XJT_EXTERNAL_ANTENNA;
//  }

