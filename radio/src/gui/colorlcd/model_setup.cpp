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

#include "model_setup.h"
#include "multi_rfprotos.h"
#include "io/multi_protolist.h"
#include "pulses/flysky.h"

#include "opentx.h"
#include "libopenui.h"
#include "storage/modelslist.h"
#include "algorithm"

#if defined(PXX2)
#include "access_settings.h"
#endif

#define SET_DIRTY()     storageDirty(EE_MODEL)

std::string switchWarninglabel(swsrc_t index)
{
  static const char *switchPositions[] = {
      " ",
      STR_CHAR_UP,
      "-",
      STR_CHAR_DOWN,
  };

  return TEXT_AT_INDEX(STR_VSRCRAW,
                       (index + MIXSRC_FIRST_SWITCH - MIXSRC_Rud + 1)) +
         std::string(
             switchPositions[g_model.switchWarningState >> (3 * index) & 0x07]);
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
      dc->drawSolidFilledRect(xChannel, + 2, lenChannel, (height() / 2) - 3, COLOR_THEME_SECONDARY1);
      dc->drawSolidFilledRect(xFailsafe, (height() / 2) + 1, lenFailsafe, (height() / 2) - 3, COLOR_THEME_WARNING);
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

      const int lim = calcRESXto1000(
          (g_model.extendedLimits ? (512 * LIMIT_EXT_PERCENT / 100) : 512) * 2);

      for (int ch=0; ch < maxModuleChannels(moduleIdx); ch++) {
        // Channel name
        // TODO if (g_model.limitData[ch].name[0] != '\0') { <= add channel name
        new StaticText(this, grid.getLabelSlot(), getSourceString(MIXSRC_CH1 + ch), 0, COLOR_THEME_PRIMARY1);

        // Channel numeric value
        new NumberEdit(this, grid.getFieldSlot(8, 0), -lim, lim,
                       [=]() { return calcRESXto1000(g_model.failsafeChannels[ch]); },
                       [=](int32_t newValue) {
                         g_model.failsafeChannels[ch] = calc1000toRESX(newValue);
                         SET_DIRTY();
                       },
                       0, PREC1 | RIGHT);

        // Channel bargraph
        new ChannelFailsafeBargraph(this, {180, grid.getWindowHeight(), 150, PAGE_LINE_HEIGHT}, moduleIdx, ch);
        grid.nextLine();
      }

      grid.spacer();
      auto out2fail = new TextButton(this, grid.getLineSlot(), STR_CHANNELS2FAILSAFE);
      out2fail->setPressHandler([=]() {
        setCustomFailsafe(moduleIdx);
        AUDIO_WARNING1();
        SET_DIRTY();
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
                     STR_FAILSAFESET, 0, COLOR_THEME_PRIMARY2);
    }

  protected:
    static constexpr coord_t footerHeight = 30;
    uint8_t moduleIdx;
};

class BindChoiceMenu : public Menu
{
 protected:
  enum BindChanMode {
    Bind_1_8_TELEM_ON,
    Bind_1_8_TELEM_OFF,
    Bind_9_16_TELEM_ON,
    Bind_9_16_TELEM_OFF,
  };

  uint8_t moduleIdx;
  std::function<void()> onPressHandler;

  void onSelect(BindChanMode mode)
  {
    bool receiverTelemetry = false;
    bool receiverHigherChannels = false;

    switch (mode) {
      case Bind_1_8_TELEM_ON:
        receiverTelemetry = true;
        break;
      case Bind_1_8_TELEM_OFF:
        break;
      case Bind_9_16_TELEM_ON:
        receiverTelemetry = true;
        receiverHigherChannels = true;
        break;
      case Bind_9_16_TELEM_OFF:
        receiverHigherChannels = true;
        break;
    }

    if (isModuleMultimodule(moduleIdx)) {
      g_model.moduleData[moduleIdx].multi.receiverTelemetryOff =
          !receiverTelemetry;
      g_model.moduleData[moduleIdx].multi.receiverHigherChannels =
          receiverHigherChannels;
    } else {
      g_model.moduleData[moduleIdx].pxx.receiverTelemetryOff =
          !receiverTelemetry;
      g_model.moduleData[moduleIdx].pxx.receiverHigherChannels =
          receiverHigherChannels;
    }

    if (onPressHandler) onPressHandler();
  }

 public:
  BindChoiceMenu(Window *parent, uint8_t moduleIdx,
                 std::function<void()> onPress,
                 std::function<void()> onCancel) :
      Menu(parent), moduleIdx(moduleIdx), onPressHandler(std::move(onPress))
  {
    if (isTelemAllowedOnBind(moduleIdx)) {
      addLine(STR_BINDING_1_8_TELEM_ON, [=]() { onSelect(Bind_1_8_TELEM_ON); });
    }
    addLine(STR_BINDING_1_8_TELEM_OFF, [=]() { onSelect(Bind_1_8_TELEM_OFF); });

    if (isBindCh9To16Allowed(moduleIdx)) {
      if (isTelemAllowedOnBind(moduleIdx)) {
        addLine(STR_BINDING_9_16_TELEM_ON,
                [=]() { onSelect(Bind_9_16_TELEM_ON); });
      }
      addLine(STR_BINDING_9_16_TELEM_OFF,
              [=]() { onSelect(Bind_9_16_TELEM_OFF); });
    }
    setTitle(STR_SELECT_MODE);
    setCancelHandler(onCancel);
  }
};

class TrChoice : public Choice
{
 public:
  TrChoice(FormGroup *parent, const rect_t &rect, const char **values, int vmin,
           int vmax, std::function<int()> getValue,
           std::function<void(int)> setValue = nullptr,
           bool *menuOpen = nullptr) :
      Choice(parent, rect, values, vmin, vmax, getValue, setValue),
      menuOpen(menuOpen)
  {
  }

 protected:
  void openMenu()
  {
    if (menuOpen) *menuOpen = true;
    Choice::openMenu();
  }

 private:
  bool *menuOpen;
};

class TrainerModuleWindow : public FormGroup
{
 public:
  TrainerModuleWindow(FormWindow *parent, const rect_t &rect) :
      FormGroup(parent, rect, FORWARD_SCROLL | FORM_FORWARD_FOCUS)
  {
    update();
  }

  void checkEvents() override
  {
#if defined(BLUETOOTH)
    if (popupopen) {
      if (bluetooth.state == BLUETOOTH_STATE_DISCOVER_START ||
          bluetooth.state == BLUETOOTH_STATE_DISCOVER_END) {
        int cnt = min<uint8_t>(reusableBuffer.moduleSetup.bt.devicesCount,
                               MAX_BLUETOOTH_DISTANT_ADDR);
        if (devicecount < cnt) {
          for (int i = 0; i < cnt - devicecount; i++) {
            int index = devicecount + i;
            btDiscoverMenuAddItem(reusableBuffer.moduleSetup.bt.devices[index]);
          }
          devicecount = cnt;
        }
      }
    }
    if (bluetooth.state != lastbluetoothstate) {
      if (!popupopen && !trChoiceOpen) update();
      lastbluetoothstate = bluetooth.state;
    }
#endif
    FormGroup::checkEvents();
  }

  void update()
  {
    FormGridLayout grid;
    clear();

    new StaticText(this, grid.getLabelSlot(true), STR_MODE, 0,
                   COLOR_THEME_PRIMARY1);
    trainerChoice = new TrChoice(
        this, grid.getFieldSlot(), STR_VTRAINERMODES, 0, TRAINER_MODE_MAX(),
        GET_DEFAULT(g_model.trainerData.mode),
        [=](int32_t newValue) {
#if defined(BLUETOOTH)
          memclear(bluetooth.distantAddr, sizeof(bluetooth.distantAddr));
          bluetooth.state = BLUETOOTH_STATE_OFF;
#endif
          g_model.trainerData.mode = newValue;
          SET_DIRTY();
          update();
          trainerChoice->setFocus(SET_FOCUS_DEFAULT);
          trChoiceOpen = false;
        },
        &trChoiceOpen);
    trainerChoice->setAvailableHandler(isTrainerModeAvailable);

    grid.nextLine();

    if (g_model.isTrainerTraineeEnable()) {
#if defined(BLUETOOTH)
      if (g_model.trainerData.mode == TRAINER_MODE_MASTER_BLUETOOTH) {
        btDistAddress = new StaticText(this, grid.getFieldSlot(true), "---", 0,
                                       COLOR_THEME_PRIMARY1);
        if (bluetooth.state == BLUETOOTH_STATE_CONNECTED)
          new StaticText(this, grid.getLabelSlot(true), STR_CONNECTED, 0,
                         COLOR_THEME_PRIMARY1);
        else
          new StaticText(this, grid.getLabelSlot(true), STR_NOT_CONNECTED, 0,
                         COLOR_THEME_PRIMARY1);

        grid.nextLine();
        btMasterButton =
            new TextButton(this, grid.getFieldSlot(), "", [=]() -> uint8_t {
              if (bluetooth.distantAddr[0]) {
                bluetooth.state = BLUETOOTH_STATE_CLEAR_REQUESTED;
                memclear(bluetooth.distantAddr, sizeof(bluetooth.distantAddr));
              } else if (bluetooth.state < BLUETOOTH_STATE_IDLE) {
                bluetooth.state = BLUETOOTH_STATE_OFF;
              } else {
                reusableBuffer.moduleSetup.bt.devicesCount = 0;
                devicecount = 0;
                bluetooth.state = BLUETOOTH_STATE_DISCOVER_REQUESTED;
                btPopUpMenu = new Menu(parent);
                btPopUpMenu->setTitle(STR_BT_SELECT_DEVICE);
                popupopen = true;
                btPopUpMenu->setCloseHandler([=]() { popupopen = false; });
              }
              return 0;
            });

        if (bluetooth.distantAddr[0]) {
          btDistAddress->setText(bluetooth.distantAddr);
          btMasterButton->setText(STR_CLEAR);
        } else if (bluetooth.state < BLUETOOTH_STATE_IDLE) {
          btMasterButton->setText(STR_BLUETOOTH_INIT);
        } else {
          btMasterButton->setText(STR_DISCOVER);
        }

        grid.nextLine();

      } else if (g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH) {
        if (bluetooth.state == BLUETOOTH_STATE_CONNECTED) {
          new StaticText(this, grid.getLabelSlot(true), STR_CONNECTED, 0,
                         COLOR_THEME_PRIMARY1);
          new StaticText(this, grid.getFieldSlot(), bluetooth.distantAddr, 0,
                         COLOR_THEME_PRIMARY1);
        } else
          new StaticText(this, grid.getLabelSlot(true), STR_NOT_CONNECTED, 0,
                         COLOR_THEME_PRIMARY1);

        grid.nextLine();

        new StaticText(this, grid.getLabelSlot(true), STR_CHANNELRANGE, 0,
                       COLOR_THEME_PRIMARY1);
        channelStart = new NumberEdit(
            this, grid.getFieldSlot(2, 0), 1,
            MAX_OUTPUT_CHANNELS - BLUETOOTH_TRAINER_CHANNELS + 1,
            GET_DEFAULT(1 + g_model.trainerData.channelsStart));
        char chend[6];
        snprintf(chend, sizeof(chend), "%s%d", STR_CH,
                 g_model.trainerData.channelsStart + BLUETOOTH_TRAINER_CHANNELS);

        btChannelEnd = new StaticText(this, grid.getFieldSlot(2, 1), chend, 0,
                                      COLOR_THEME_PRIMARY1);
        channelStart->setPrefix(STR_CH);
        channelStart->setSetValueHandler([=](int32_t newValue) {
          g_model.trainerData.channelsStart = newValue - 1;
          g_model.trainerData.channelsCount = BLUETOOTH_TRAINER_CHANNELS - DEF_TRAINER_CHANNELS;
          char chend[6];
          snprintf(chend, sizeof(chend), "%s%d", STR_CH,
                   g_model.trainerData.channelsStart + BLUETOOTH_TRAINER_CHANNELS);
          SET_DIRTY();
          btChannelEnd->setText(chend);
        });

        grid.nextLine();
      } else
#endif
          if (g_model.trainerData.mode == TRAINER_MODE_SLAVE) {
        #define PPMCH_START_MIN 1
        #define PPMCH_START_MAX (MAX_OUTPUT_CHANNELS - g_model.trainerData.channelsCount + 1 - DEF_TRAINER_CHANNELS)
        #define PPMCH_END_MIN (g_model.trainerData.channelsStart + MIN_TRAINER_CHANNELS)
        #define PPMCH_END_MAX min<uint8_t>(g_model.trainerData.channelsStart + MAX_TRAINER_CHANNELS, MAX_OUTPUT_CHANNELS)

        new StaticText(this, grid.getLabelSlot(true), STR_CHANNELRANGE, 0,
                       COLOR_THEME_PRIMARY1);
        channelStart = new NumberEdit(
            this, grid.getFieldSlot(2, 0), PPMCH_START_MIN, PPMCH_START_MAX,
            GET_DEFAULT(1 + g_model.trainerData.channelsStart));
        channelEnd = new NumberEdit(
            this, grid.getFieldSlot(2, 1), PPMCH_END_MIN, PPMCH_END_MAX,
            GET_DEFAULT((g_model.trainerData.channelsStart +
                         DEF_TRAINER_CHANNELS +
                         g_model.trainerData.channelsCount)));
        channelStart->setPrefix(STR_CH);
        channelEnd->setPrefix(STR_CH);
        channelStart->setSetValueHandler([=](int32_t newValue) {
          g_model.trainerData.channelsStart = newValue - 1;
          SET_DIRTY();
          channelEnd->setMin(PPMCH_END_MIN);
          channelEnd->setMax(PPMCH_END_MAX);
          channelEnd->invalidate();
        });
        channelEnd->setSetValueHandler([=](int32_t newValue) {
          g_model.trainerData.channelsCount =
              newValue - g_model.trainerData.channelsStart -
              DEF_TRAINER_CHANNELS;
          SET_DIRTY();
          channelStart->setMax(PPMCH_START_MAX);
        });

        grid.nextLine();

        // PPM frame
        new StaticText(this, grid.getLabelSlot(true), STR_PPMFRAME, 0,
                       COLOR_THEME_PRIMARY1);

        // PPM frame length
        auto edit = new NumberEdit(
            this, grid.getFieldSlot(3, 0), 125, 35 * 5 + 225,
            GET_DEFAULT(g_model.trainerData.frameLength * 5 + 225),
            SET_VALUE(g_model.trainerData.frameLength, (newValue - 225) / 5), 0,
            PREC1);
        edit->setStep(5);
        edit->setSuffix(STR_MS);

        // PPM frame delay
        edit = new NumberEdit(
            this, grid.getFieldSlot(3, 1), 100, 800,
            GET_DEFAULT(g_model.trainerData.delay * 50 + 300),
            SET_VALUE(g_model.trainerData.delay, (newValue - 300) / 50));
        edit->setStep(50);
        edit->setSuffix(STR_US);

        // PPM Polarity
        new Choice(this, grid.getFieldSlot(3, 2), STR_PPM_POL, 0, 1,
                   GET_SET_DEFAULT(g_model.trainerData.pulsePol));
        grid.nextLine();
      }
    }
#if defined(HARDWARE_TOUCH)
    new StaticText(this, grid.getLabelSlot(true));
#endif
    auto par = getParent();
    par->moveWindowsTop(top() + 1, adjustHeight());
    par->adjustInnerHeight();
  }

 protected:
  TrChoice *trainerChoice = nullptr;
  NumberEdit *channelStart = nullptr;
  NumberEdit *channelEnd = nullptr;
  bool trChoiceOpen = false;
#if defined(BLUETOOTH)
  StaticText *btChannelEnd = nullptr;
  StaticText *btDistAddress = nullptr;
  TextButton *btMasterButton = nullptr;
  Menu *btPopUpMenu = nullptr;
  bool btCanceled = false;

 private:
  bool popupopen = false;
  int devicecount = 0;
  uint8_t lastbluetoothstate = BLUETOOTH_STATE_OFF;

  void btDiscoverMenuItemChosen()
  {
    if (bluetooth.state == BLUETOOTH_STATE_DISCOVER_SENT ||
        bluetooth.state == BLUETOOTH_STATE_DISCOVER_END) {
      int index = btPopUpMenu->selection();
      if (index >= 0 && index < reusableBuffer.moduleSetup.bt.devicesCount) {
        strncpy(bluetooth.distantAddr,
                reusableBuffer.moduleSetup.bt.devices[index],
                LEN_BLUETOOTH_ADDR);
        bluetooth.state = BLUETOOTH_STATE_BIND_REQUESTED;
        SET_DIRTY();
      }
    }
  }

  void btDiscoverMenuAddItem(const char *itm)
  {
    if (btPopUpMenu != nullptr && !btPopUpMenu->deleted())
      btPopUpMenu->addLine(
          itm, std::bind(&TrainerModuleWindow::btDiscoverMenuItemChosen, this));
  }

#endif
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
      new StaticText(this, grid.getLabelSlot(true), STR_CHANNELRANGE, 0,
                     COLOR_THEME_PRIMARY1);
      auto channelStart = new NumberEdit(
          this, grid.getFieldSlot(2, 0), 1,
          MAX_OUTPUT_CHANNELS - sentModuleChannels(moduleIdx) + 1,
          GET_DEFAULT(1 + g_model.moduleData[moduleIdx].channelsStart));
      auto channelEnd = new NumberEdit(
          this, grid.getFieldSlot(2, 1),
          g_model.moduleData[moduleIdx].channelsStart +
              minModuleChannels(moduleIdx),
          min<int8_t>(MAX_OUTPUT_CHANNELS,
                      g_model.moduleData[moduleIdx].channelsStart +
                          maxModuleChannels(moduleIdx)),
          GET_DEFAULT(g_model.moduleData[moduleIdx].channelsStart + 8 +
                      g_model.moduleData[moduleIdx].channelsCount));
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
        channelEnd->setMin(g_model.moduleData[moduleIdx].channelsStart +
                           minModuleChannels(moduleIdx));
        channelEnd->setMax(min<int8_t>(
            MAX_OUTPUT_CHANNELS, g_model.moduleData[moduleIdx].channelsStart +
                                     maxModuleChannels(moduleIdx)));
        channelEnd->invalidate();
      });
      channelEnd->setSetValueHandler([=](int32_t newValue) {
        g_model.moduleData[moduleIdx].channelsCount =
            newValue - g_model.moduleData[moduleIdx].channelsStart - 8;
        SET_DIRTY();
        channelStart->setMax(MAX_OUTPUT_CHANNELS -
                             sentModuleChannels(moduleIdx) + 1);
      });
      channelEnd->enable(minModuleChannels(moduleIdx) <
                         maxModuleChannels(moduleIdx));
      if (channelEnd->getValue() > channelEnd->getMax())
        channelEnd->setValue(channelEnd->getMax());
    }

    void update()
    {
      FormGridLayout grid;
      clear();

      // Module Type
      new StaticText(this, grid.getLabelSlot(true), STR_MODE, 0,
                     COLOR_THEME_PRIMARY1);
      moduleChoice = new Choice(
          this, grid.getFieldSlot(2, 0), STR_INTERNAL_MODULE_PROTOCOLS,
          MODULE_TYPE_NONE, MODULE_TYPE_COUNT - 1,
          GET_DEFAULT(g_model.moduleData[moduleIdx].type),
          [=](int32_t newValue) {
            setModuleType(moduleIdx, newValue);
            update();
            moduleChoice->setFocus(SET_FOCUS_DEFAULT);
            SET_DIRTY();
          });
      moduleChoice->setAvailableHandler([=](int8_t moduleType) {
        return moduleIdx == INTERNAL_MODULE
                   ? isInternalModuleAvailable(moduleType)
                   : isExternalModuleAvailable(moduleType);
      });

      // Module parameters
      if (isModuleXJT(moduleIdx)) {
        rfChoice = new Choice(
            this, grid.getFieldSlot(2, 1), STR_XJT_ACCST_RF_PROTOCOLS,
            MODULE_SUBTYPE_PXX1_OFF, MODULE_SUBTYPE_PXX1_LAST,
            GET_DEFAULT(g_model.moduleData[moduleIdx].subType),
            [=](int32_t newValue) {
              g_model.moduleData[moduleIdx].subType = newValue;
              g_model.moduleData[moduleIdx].channelsStart = 0;
              g_model.moduleData[moduleIdx].channelsCount =
                  defaultModuleChannels_M8(moduleIdx);
              SET_DIRTY();
              update();
              rfChoice->setFocus(SET_FOCUS_DEFAULT);
            });

        rfChoice->setAvailableHandler(
            [](int index) { return index != MODULE_SUBTYPE_PXX1_OFF; });

      } else if (isModuleDSM2(moduleIdx)) {
        new Choice(this, grid.getFieldSlot(2, 1), STR_DSM_PROTOCOLS,
                   DSM2_PROTO_LP45, DSM2_PROTO_DSMX,
                   GET_SET_DEFAULT(g_model.moduleData[moduleIdx].rfProtocol));
      } else if (isModuleR9M(moduleIdx)) {
        rfChoice =
            new Choice(this, grid.getFieldSlot(2, 1), STR_R9M_REGION,
                       MODULE_SUBTYPE_R9M_FCC, MODULE_SUBTYPE_R9M_LAST,
                       GET_DEFAULT(g_model.moduleData[moduleIdx].subType),
                       [=](int32_t newValue) {
                         g_model.moduleData[moduleIdx].subType = newValue;
                         SET_DIRTY();
                         update();
                         rfChoice->setFocus(SET_FOCUS_DEFAULT);
                       });
      }
#if defined(PXX2)
      else if (isModulePXX2(moduleIdx)) {
        rfChoice =
            new Choice(this, grid.getFieldSlot(2, 1), STR_ISRM_RF_PROTOCOLS, 0,
                       MODULE_SUBTYPE_ISRM_PXX2_ACCST_LR12,
                       GET_DEFAULT(g_model.moduleData[moduleIdx].subType),
                       [=](int32_t newValue) {
                         g_model.moduleData[moduleIdx].subType = newValue;
                         SET_DIRTY();
                         update();
                         rfChoice->setFocus(SET_FOCUS_DEFAULT);
                       });
      }
#endif
#if defined(AFHDS2) || defined(AFHDS3)
      else if (isModuleFlySky(moduleIdx)) {
        rfChoice =
            new Choice(this, grid.getFieldSlot(2, 1),
                       STR_FLYSKY_PROTOCOLS, 0, FLYSKY_SUBTYPE_AFHDS2A,
                       GET_DEFAULT(g_model.moduleData[moduleIdx].subType),
                       [=](int32_t newValue) {
                         g_model.moduleData[moduleIdx].subType = newValue;
                         SET_DIRTY();
                         update();
                         rfChoice->setFocus(SET_FOCUS_DEFAULT);
                       });

        if (moduleIdx == INTERNAL_MODULE) {
          g_model.moduleData[moduleIdx].subType = FLYSKY_SUBTYPE_AFHDS2A;
          rfChoice->setAvailableHandler(
              [](int v) { return v == FLYSKY_SUBTYPE_AFHDS2A; });
        } else {
          g_model.moduleData[moduleIdx].subType = FLYSKY_SUBTYPE_AFHDS3;
          rfChoice->setAvailableHandler(
              [](int v) { return v == FLYSKY_SUBTYPE_AFHDS3; });
        }

        // RX options:
        grid.nextLine();
        new StaticText(this, grid.getLabelSlot(true), STR_OPTIONS, 0,
                       COLOR_THEME_PRIMARY1);

#if defined(AFHDS2)
        if (isModuleAFHDS2A(moduleIdx)) {
          // PPM / PWM
          new Choice(
              this, grid.getFieldSlot(2, 0), STR_FLYSKY_PULSE_PROTO, 0, 1,
              [=]() { return g_model.moduleData[moduleIdx].flysky.mode >> 1; },
              [=](int v) {
                g_model.moduleData[moduleIdx].flysky.mode =
                    (g_model.moduleData[moduleIdx].flysky.mode & 1) |
                    ((v & 1) << 1);
              });
          // SBUS / iBUS
          new Choice(
              this, grid.getFieldSlot(2, 1), STR_FLYSKY_SERIAL_PROTO, 0, 1,
              [=]() { return g_model.moduleData[moduleIdx].flysky.mode & 1; },
              [=](int v) {
                g_model.moduleData[moduleIdx].flysky.mode =
                    (g_model.moduleData[moduleIdx].flysky.mode & 2) | (v & 1);
              });
        }
#endif
#if defined(AFHDS3)
        if (isModuleAFHDS3(moduleIdx)) {

          // PPM / PWM
          new Choice(
              this, grid.getFieldSlot(2, 0), STR_FLYSKY_PULSE_PROTO, 0, 1,
              [=]() { return g_model.moduleData[moduleIdx].afhds3.mode >> 1; },
              [=](int v) {
                g_model.moduleData[moduleIdx].afhds3.mode =
                    (g_model.moduleData[moduleIdx].afhds3.mode & 1) |
                    ((v & 1) << 1);
              });
          // SBUS / iBUS
          new Choice(
              this, grid.getFieldSlot(2, 1), STR_FLYSKY_SERIAL_PROTO, 0, 1,
              [=]() { return g_model.moduleData[moduleIdx].afhds3.mode & 1; },
              [=](int v) {
                g_model.moduleData[moduleIdx].afhds3.mode =
                    (g_model.moduleData[moduleIdx].afhds3.mode & 2) | (v & 1);
              });

          // TYPE
          grid.nextLine();
          new StaticText(this, grid.getLabelSlot(true), STR_TYPE, 0,
                         COLOR_THEME_PRIMARY1);

          // This is chosen when binding (menu? see stdlcd/model_setup_afhds3.cpp)
          new StaticText(this, grid.getFieldSlot(),
                         g_model.moduleData[moduleIdx].afhds3.telemetry
                             ? STR_AFHDS3_ONE_TO_ONE_TELEMETRY
                             : TR_AFHDS3_ONE_TO_MANY,
                         0, COLOR_THEME_PRIMARY1);

          // Status
          grid.nextLine();
          new StaticText(this, grid.getLabelSlot(true), STR_MODULE_STATUS, 0,
                         COLOR_THEME_PRIMARY1);
          new DynamicText(this, grid.getFieldSlot(), [=] {
            char msg[64] = "";
            getModuleStatusString(moduleIdx, msg);
            return std::string(msg);
          });

          // Power source
          grid.nextLine();
          new StaticText(this, grid.getLabelSlot(true), STR_AFHDS3_POWER_SOURCE,
                         0, COLOR_THEME_PRIMARY1);
          new DynamicText(this, grid.getFieldSlot(), [=] {
            char msg[64] = "";
            getModuleSyncStatusString(moduleIdx, msg);
            return std::string(msg);
          });

          // RX Freq
          grid.nextLine();
          new StaticText(this, grid.getLabelSlot(true), STR_AFHDS3_RX_FREQ, 0,
                         COLOR_THEME_PRIMARY1);
          auto edit = new NumberEdit(
              this, grid.getFieldSlot(2, 0), MIN_FREQ, MAX_FREQ,
              GET_DEFAULT(g_model.moduleData[moduleIdx].afhds3.rxFreq()));
          edit->setSetValueHandler([=](int32_t newValue) {
            g_model.moduleData[moduleIdx].afhds3.setRxFreq(
                (uint16_t)newValue);
          });
          edit->setSuffix(STR_HZ);

          // Module actual power
          grid.nextLine();
          new StaticText(this, grid.getLabelSlot(true), STR_AFHDS3_ACTUAL_POWER,
                         0, COLOR_THEME_PRIMARY1);
          new DynamicText(this, grid.getFieldSlot(), [=] {
            char msg[64] = "";
            getStringAtIndex(msg, STR_AFHDS3_POWERS,
                             actualAfhdsRunPower(moduleIdx));
            return std::string(msg);
          });

          // Module power
          grid.nextLine();
          new StaticText(this, grid.getLabelSlot(true), STR_RF_POWER, 0,
                         COLOR_THEME_PRIMARY1);
          new Choice(
              this, grid.getFieldSlot(2, 0), STR_AFHDS3_POWERS,
              afhds3::RUN_POWER::RUN_POWER_FIRST,
              afhds3::RUN_POWER::RUN_POWER_LAST,
              GET_SET_DEFAULT(g_model.moduleData[moduleIdx].afhds3.runPower));
        }
#endif
      }
#endif
#if defined(MULTIMODULE)
      else if (isModuleMultimodule(moduleIdx)) {
        MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);

        if (status.protocolName[0] && status.isValid()) {
             new StaticText(this, grid.getFieldSlot(2, 1), status.protocolName,
                            0, COLOR_THEME_PRIMARY1);
        }

        Choice * mmSubProto = nullptr;
        grid.nextLine();
        new StaticText(this, grid.getLabelSlot(true), STR_RF_PROTOCOL, 0,
                       COLOR_THEME_PRIMARY1);

        // Grid count for narrow/wide screen
        int count =
            LCD_W < LCD_H
                ? 1
                : (g_model.moduleData[moduleIdx].multi.customProto ? 3 : 2);

        rfChoice = new MultiProtoChoice(
            this, grid.getFieldSlot(count, 0), moduleIdx,
            [=](int32_t newValue) {

              g_model.moduleData[moduleIdx].setMultiProtocol(newValue);
              g_model.moduleData[moduleIdx].subType = 0;
              resetMultiProtocolsOptions(moduleIdx);

              MultiModuleStatus& status = getMultiModuleStatus(moduleIdx);
              status.invalidate();

              uint32_t startUpdate = RTOS_GET_MS();
              while(!status.isValid() && (RTOS_GET_MS() - startUpdate < 250));

              SET_DIRTY();
              update();

              if (rfChoice)
                rfChoice->setFocus(SET_FOCUS_DEFAULT);
            },
            [=]() { update(); });

        auto *rfProto =
            MultiRfProtocols::instance(moduleIdx)->getProto(
                g_model.moduleData[moduleIdx].getMultiProtocol());

        if (rfProto && !rfProto->subProtos.empty()) {
          // Subtype (D16, DSMX,...)

          // Grid count for narrow/wide screen
          count = LCD_W < LCD_H ? 1 : 2;
          int index = 1;
          if (count == 1) {
            grid.nextLine();
            index = 0;
          }

          mmSubProto = new Choice(
                this, grid.getFieldSlot(count, index),
                rfProto->subProtos, 0, rfProto->subProtos.size() - 1,
                [=]() { return g_model.moduleData[moduleIdx].subType; },
                [=](int16_t newValue) {
                  g_model.moduleData[moduleIdx].subType = newValue;
                  resetMultiProtocolsOptions(moduleIdx);
                  SET_DIRTY();
                  update();
                  if (mmSubProto != nullptr) mmSubProto->setFocus(SET_FOCUS_DEFAULT);
                });
        }
        grid.nextLine();

        // Multimodule status
        new StaticText(this, grid.getLabelSlot(true), STR_MODULE_STATUS, 0, COLOR_THEME_PRIMARY1);
        new DynamicText(this, grid.getFieldSlot(), [=] {
            char msg[64] = "";
            getModuleStatusString(moduleIdx, msg);
            return std::string(msg);
        }, COLOR_THEME_PRIMARY1);

        const uint8_t multi_proto = g_model.moduleData[moduleIdx].getMultiProtocol();
        if (rfProto) {
          // Multi optional feature row
          const char *title = rfProto->getOptionStr();
          if (title != nullptr) {
            grid.nextLine();
            new StaticText(this, grid.getLabelSlot(true), title, 0, COLOR_THEME_PRIMARY1);

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

          new StaticText(this, grid.getLabelSlot(true), STR_MULTI_SERVOFREQ, 0, COLOR_THEME_PRIMARY1);
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
          new StaticText(this, grid.getLabelSlot(true), STR_MULTI_AUTOBIND, 0, COLOR_THEME_PRIMARY1);
          new CheckBox(this, grid.getFieldSlot(),
                       GET_SET_DEFAULT(
                           g_model.moduleData[moduleIdx].multi.autoBindMode));
        }

        // Low power mode
        grid.nextLine();
        new StaticText(this, grid.getLabelSlot(true), STR_MULTI_LOWPOWER, 0, COLOR_THEME_PRIMARY1);
        new CheckBox(
            this, grid.getFieldSlot(),
            GET_SET_DEFAULT(g_model.moduleData[moduleIdx].multi.lowPowerMode));

        // Disable telemetry
        grid.nextLine();
        new StaticText(this, grid.getLabelSlot(true), STR_DISABLE_TELEM, 0, COLOR_THEME_PRIMARY1);
        new CheckBox(this, grid.getFieldSlot(),
                     GET_SET_DEFAULT(
                         g_model.moduleData[moduleIdx].multi.disableTelemetry));

        if (rfProto && rfProto->supportsDisableMapping()) {
          // Disable channel mapping
          grid.nextLine();
          new StaticText(this, grid.getLabelSlot(true), STR_DISABLE_CH_MAP, 0, COLOR_THEME_PRIMARY1);
          new CheckBox(this, grid.getFieldSlot(),
                       GET_SET_DEFAULT(g_model.moduleData[moduleIdx].multi.disableMapping));
        }
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
        new StaticText(this, grid.getLabelSlot(true), STR_PPMFRAME, 0, COLOR_THEME_PRIMARY1);

        // PPM frame length
        auto edit = new NumberEdit(
            this, grid.getFieldSlot(3, 0), 125, 35 * PPM_STEP_SIZE + PPM_DEF_PERIOD,
            GET_DEFAULT(g_model.moduleData[moduleIdx].ppm.frameLength * PPM_STEP_SIZE +
                        PPM_DEF_PERIOD),
            SET_VALUE(g_model.moduleData[moduleIdx].ppm.frameLength,
                      (newValue - PPM_DEF_PERIOD) / PPM_STEP_SIZE),
            0, PREC1);
        edit->setStep(PPM_STEP_SIZE);
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
      if (!isModuleRFAccess(moduleIdx) &&
          (isModuleModelIndexAvailable(moduleIdx) ||
           isModuleBindRangeAvailable(moduleIdx))) {
        uint8_t thirdColumn = 0;
        new StaticText(this, grid.getLabelSlot(true), STR_RECEIVER, 0, COLOR_THEME_PRIMARY1);

        // Model index
        if (isModuleModelIndexAvailable(moduleIdx)) {
          thirdColumn++;
          new NumberEdit(
              this, grid.getFieldSlot(3, 0), 0, getMaxRxNum(moduleIdx),
              GET_DEFAULT(g_model.header.modelId[moduleIdx]),
              [=](int32_t newValue) {
                if (newValue != g_model.header.modelId[moduleIdx]) {
                  g_model.header.modelId[moduleIdx] = newValue;
                  if (isModuleCrossfire(moduleIdx)) {
                    moduleState[moduleIdx].counter = CRSF_FRAME_MODELID;
                  }
                  SET_DIRTY();
                }
              });
        }

        if (isModuleBindRangeAvailable(moduleIdx)) {
          bindButton = new TextButton(this, grid.getFieldSlot(2 + thirdColumn, 0 + thirdColumn), STR_MODULE_BIND);
          bindButton->setPressHandler([=]() -> uint8_t {
              if (moduleState[moduleIdx].mode == MODULE_MODE_RANGECHECK) {
                if (rangeButton) rangeButton->check(false);
              }
              if (moduleState[moduleIdx].mode == MODULE_MODE_BIND) {
                moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
#if defined(AFHDS2)
                if (isModuleFlySky(moduleIdx)) resetPulsesAFHDS2();
#endif
                return 0;
              } else {
                if (isModuleR9MNonAccess(moduleIdx) || isModuleD16(moduleIdx) ||
                    IS_R9_MULTI(moduleIdx)) {
                  new BindChoiceMenu(
                      this, moduleIdx,
                      [=]() {
#if defined(MULTIMODULE)
                        if (isModuleMultimodule(moduleIdx)) {
                          setMultiBindStatus(moduleIdx, MULTI_BIND_INITIATED);
                        }
#endif
                        moduleState[moduleIdx].mode = MODULE_MODE_BIND;
                        bindButton->check(true);
                      },
                      [=]() {
                        moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
                        bindButton->check(false);
                      });

                  return 0;
                }
#if defined(MULTIMODULE)
                if (isModuleMultimodule(moduleIdx)) {
                  setMultiBindStatus(moduleIdx, MULTI_BIND_INITIATED);
                }
#endif
                moduleState[moduleIdx].mode = MODULE_MODE_BIND;
#if defined(AFHDS2)
                if (isModuleFlySky(moduleIdx)) {
                  resetPulsesAFHDS2();
                }
#endif
                return 1;
              }
              return 0;
          });
          bindButton->setCheckHandler([=]() {
              if (moduleState[moduleIdx].mode != MODULE_MODE_BIND) {
                bindButton->check(false);
              }
#if defined(MULTIMODULE)
              if (isModuleMultimodule(moduleIdx) &&
                  getMultiBindStatus(moduleIdx) == MULTI_BIND_FINISHED) {
                setMultiBindStatus(moduleIdx, MULTI_BIND_NONE);
                moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
                bindButton->check(false);
              }
#endif
          });

          if (isModuleRangeAvailable(moduleIdx)) {
            rangeButton = new TextButton(
                this, grid.getFieldSlot(2 + thirdColumn, 1 + thirdColumn),
                STR_MODULE_RANGE);
            rangeButton->setPressHandler([=]() -> uint8_t {
              if (moduleState[moduleIdx].mode == MODULE_MODE_BIND) {
                bindButton->check(false);
                moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
              }
              if (moduleState[moduleIdx].mode == MODULE_MODE_RANGECHECK) {
                moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
                return 0;
              } else {
                moduleState[moduleIdx].mode = MODULE_MODE_RANGECHECK;
#if defined(AFHDS2)
                if (isModuleFlySky(moduleIdx)) {
                  resetPulsesAFHDS2();
                }
#endif
                startRSSIDialog([=]() {
#if defined(AFHDS2)
                  if (isModuleFlySky(moduleIdx)) {
                    resetPulsesAFHDS2();
                  }
#endif
                });
                return 1;
              }
            });
          }
        }

        grid.nextLine();
      }

#if defined(AFHDS2) && (defined(PCBNV14) || defined(PCBPL18))
  #if defined(PCBNV14)
      if (isModuleAFHDS2A(moduleIdx) && getNV14RfFwVersion() >= 0x1000E) {
  #else
      // PCBPL18
      if (isModuleAFHDS2A(moduleIdx) && getPL18RfFwVersion() >= 0x1000E) {
  #endif
        new StaticText(this, grid.getLabelSlot(true), STR_MULTI_RFPOWER);
        new Choice(this, grid.getFieldSlot(), "\007""Default""High", 0, 1,
            GET_DEFAULT(g_model.moduleData[moduleIdx].flysky.rfPower),
            [=](int32_t newValue) -> void {
          g_model.moduleData[moduleIdx].flysky.rfPower = newValue;
          resetPulsesAFHDS2();
        });
        grid.nextLine();
      }
#endif

      // Failsafe
      if (isModuleFailsafeAvailable(moduleIdx)) {
        hasFailsafe = true;
        new StaticText(this, grid.getLabelSlot(true), STR_FAILSAFE, 0, COLOR_THEME_PRIMARY1);
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

#if defined(PXX2)
      // Register and Range buttons
      if (isModuleRFAccess(moduleIdx)) {
        new StaticText(this, grid.getLabelSlot(true), STR_MODULE, 0, COLOR_THEME_PRIMARY1);
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
              startRSSIDialog();
              return 1;
            }
        });

        grid.nextLine();

        new StaticText(this, grid.getLabelSlot(true), TR_OPTIONS, 0,
                       COLOR_THEME_PRIMARY1);
        auto options = new TextButton(this, grid.getFieldSlot(2, 0), TR_SET);
        options->setPressHandler([=]() {
            new ModuleOptions(this, moduleIdx);
            return 0;
          });
        grid.nextLine();
      }
#endif

      // R9M Power
      if (isModuleR9M_FCC(moduleIdx)) {
        new StaticText(this, grid.getLabelSlot(true), STR_RF_POWER, 0, COLOR_THEME_PRIMARY1);
        new Choice(this, grid.getFieldSlot(), STR_R9M_FCC_POWER_VALUES, 0, R9M_FCC_POWER_MAX,
                   GET_SET_DEFAULT(g_model.moduleData[moduleIdx].pxx.power));
      }

      if (isModuleR9M_LBT(moduleIdx)) {
        new StaticText(this, grid.getLabelSlot(true), STR_RF_POWER, 0, COLOR_THEME_PRIMARY1);
        new Choice(this, grid.getFieldSlot(), STR_R9M_LBT_POWER_VALUES, 0, R9M_LBT_POWER_MAX,
                   GET_DEFAULT(min<uint8_t>(g_model.moduleData[moduleIdx].pxx.power, R9M_LBT_POWER_MAX)),
                   SET_DEFAULT(g_model.moduleData[moduleIdx].pxx.power));
      }
#if defined(PXX2)
      // Receivers
      if (isModuleRFAccess(moduleIdx)) {
        for (uint8_t receiverIdx = 0; receiverIdx < PXX2_MAX_RECEIVERS_PER_MODULE; receiverIdx++) {
          char label[] = TR_RECEIVER " X";
          label[sizeof(label) - 2] = '1' + receiverIdx;
          new StaticText(this, grid.getLabelSlot(true), label, 0, COLOR_THEME_PRIMARY1);
          new ReceiverButton(this, grid.getFieldSlot(2, 0), moduleIdx, receiverIdx);
          grid.nextLine();
        }
      }
#endif
      // SBUS refresh rate
      if (isModuleSBUS(moduleIdx)) {
        new StaticText(this, grid.getLabelSlot(true), STR_REFRESHRATE, 0, COLOR_THEME_PRIMARY1);
        auto edit = new NumberEdit(this, grid.getFieldSlot(2, 0), SBUS_MIN_PERIOD, SBUS_MAX_PERIOD,
                                           GET_DEFAULT((int16_t)g_model.moduleData[moduleIdx].sbus.refreshRate * SBUS_STEPSIZE + SBUS_DEF_PERIOD),
                                           SET_VALUE(g_model.moduleData[moduleIdx].sbus.refreshRate, (newValue - SBUS_DEF_PERIOD) / SBUS_STEPSIZE),
                                           0, PREC1);
        edit->setSuffix(STR_MS);
        edit->setStep(SBUS_STEPSIZE);
        new Choice(this, grid.getFieldSlot(2, 1), STR_SBUS_INVERSION_VALUES, 0, 1, GET_SET_DEFAULT(g_model.moduleData[moduleIdx].sbus.noninverted));
#if defined(RADIO_TX16S)
        grid.nextLine();
        new StaticText(this, grid.getFieldSlot(1, 0), STR_WARN_5VOLTS, 0, COLOR_THEME_PRIMARY1);
#endif
        grid.nextLine();
      }

      if (isModuleGhost(moduleIdx)) {
          new StaticText(this, grid.getLabelSlot(true), "Raw 12 bits", 0, COLOR_THEME_PRIMARY1);
          new CheckBox(this, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.moduleData[moduleIdx].ghost.raw12bits));
      }

      auto par = getParent();
      par->moveWindowsTop(top() + 1, adjustHeight());
      par->adjustInnerHeight();
    }

#if defined (PCBNV14)
#define SIGNAL_POSTFIX 
#define SIGNAL_MESSAGE "SGNL"
#else
#define SIGNAL_POSTFIX  " db"
#define SIGNAL_MESSAGE  "RSSI"
#endif

    void startRSSIDialog(std::function<void()> closeHandler = nullptr)
    {
      auto rssiDialog = new DynamicMessageDialog(
          parent, "Range Test",
          [=]() {
            return std::to_string((int)TELEMETRY_RSSI()) +
                   std::string(SIGNAL_POSTFIX);
          },
          SIGNAL_MESSAGE, 50,
          COLOR_THEME_SECONDARY1 | CENTERED | FONT(BOLD) | FONT(XL));

      rssiDialog->setCloseHandler([this, closeHandler]() {
        rangeButton->check(false);
        moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
        if (closeHandler) closeHandler();
      });
    }

    void checkEvents() override
    {
      if (isModuleFailsafeAvailable(moduleIdx) != hasFailsafe
          && rfChoice && !rfChoice->isEditMode()) {

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

const char * STR_TIMER_MODES[] = {"OFF", "ON", "Start", "Throttle", "Throttle %", "Throttle Start"};

const char MODEL_NAME_EXTRA_CHARS[] = "_-.,:;<=>";

void ModelSetupPage::build(FormWindow * window)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);

  // Model name
  new StaticText(window, grid.getLabelSlot(), STR_MODELNAME, 0, COLOR_THEME_PRIMARY1);
  auto text =
      new ModelTextEdit(window, grid.getFieldSlot(), g_model.header.name,
                        sizeof(g_model.header.name), 0, MODEL_NAME_EXTRA_CHARS);
  text->setChangeHandler([=] {
    modelslist.load();
    auto model = modelslist.getCurrentModel();
    if (model) {
      model->setModelName(g_model.header.name);
      modelslist.save();
    }
    SET_DIRTY();
  });
  grid.nextLine();

  // Bitmap
  new StaticText(window, grid.getLabelSlot(), STR_BITMAP, 0, COLOR_THEME_PRIMARY1);
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
    new Subtitle(window, grid.getLineSlot(), reusableBuffer.moduleSetup.msg, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();

    auto group = new FormGroup(window, grid.getFieldSlot(), FORM_BORDER_FOCUS_ONLY | PAINT_CHILDREN_FIRST);
    GridLayout timerGrid(group);

    // Timer name
    new StaticText(window, grid.getLabelSlot(true), STR_NAME, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();
    new ModelTextEdit(group, timerGrid.getSlot(), timer->name, LEN_TIMER_NAME);
    timerGrid.nextLine();

    // Timer mode
    new StaticText(window, grid.getLabelSlot(true), STR_MODE, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();
    new Choice(group, timerGrid.getSlot(2, 0), STR_TIMER_MODES, 0, TMRMODE_MAX, GET_SET_DEFAULT(timer->mode));
    new SwitchChoice(group, timerGrid.getSlot(2, 1), SWSRC_FIRST, SWSRC_LAST, GET_SET_DEFAULT(timer->swtch));
    timerGrid.nextLine();

    // Timer start value
    new StaticText(window, grid.getLabelSlot(true), "Start", 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();
    new TimeEdit(group, timerGrid.getSlot(), 0, TIMER_MAX, GET_DEFAULT(timer->start),
                  [timer, i](int32_t value) {
                    timer->start = value;
                    timerSet(i, value);
    });
    timerGrid.nextLine();

    // Timer minute beep
    new StaticText(window, grid.getLabelSlot(true), STR_MINUTEBEEP, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();
    new CheckBox(group, timerGrid.getSlot(), GET_SET_DEFAULT(timer->minuteBeep));
    timerGrid.nextLine();

    // Timer countdown
    new StaticText(window, grid.getLabelSlot(true), STR_BEEPCOUNTDOWN, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();
    new Choice(group, timerGrid.getSlot(2, 0), STR_VBEEPCOUNTDOWN, COUNTDOWN_SILENT, COUNTDOWN_COUNT - 1, GET_SET_DEFAULT(timer->countdownBeep));
    new Choice(group, timerGrid.getSlot(2, 1), STR_COUNTDOWNVALUES, 0, 3, GET_SET_WITH_OFFSET(timer->countdownStart, 2));
    timerGrid.nextLine();

    // Timer persistent
    new StaticText(window, grid.getLabelSlot(true), STR_PERSISTENT, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();
    new Choice(group, timerGrid.getSlot(), STR_VPERSISTENT, 0, 2, GET_SET_DEFAULT(timer->persistent));
    timerGrid.nextLine();

    coord_t height = timerGrid.getWindowHeight() - 1;
    group->setHeight(height);
  }

  // Extended limits
  new StaticText(window, grid.getLabelSlot(), STR_ELIMITS, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.extendedLimits));
  grid.nextLine();

  // Extended trims
  new StaticText(window, grid.getLabelSlot(), STR_ETRIMS, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(window, grid.getFieldSlot(2, 0), GET_SET_DEFAULT(g_model.extendedTrims));
  new TextButton(window, grid.getFieldSlot(2, 1), STR_RESET_BTN,
                 []() -> uint8_t {
                   for (auto &flightModeData : g_model.flightModeData) {
                     memclear(&flightModeData, TRIMS_ARRAY_SIZE);
                   }
                   SET_DIRTY();
                   AUDIO_WARNING1();
                   return 0;
                 });
  grid.nextLine();

  // Display trims
  new StaticText(window, grid.getLabelSlot(), STR_DISPLAY_TRIMS, 0, COLOR_THEME_PRIMARY1);
  new Choice(window, grid.getFieldSlot(), "\006No\0   ChangeYes", 0, 2, GET_SET_DEFAULT(g_model.displayTrims));
  grid.nextLine();

  // Trim step
  new StaticText(window, grid.getLabelSlot(), STR_TRIMINC, 0, COLOR_THEME_PRIMARY1);
  new Choice(window, grid.getFieldSlot(), STR_VTRIMINC, -2, 2, GET_SET_DEFAULT(g_model.trimInc));
  grid.nextLine();

  // Throttle parameters
  {
    new Subtitle(window, grid.getLineSlot(), STR_THROTTLE_LABEL, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();

    // Throttle reversed
    new StaticText(window, grid.getLabelSlot(true), STR_THROTTLEREVERSE, 0, COLOR_THEME_PRIMARY1);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.throttleReversed));
    grid.nextLine();

    // Throttle source
    new StaticText(window, grid.getLabelSlot(true), STR_TTRACE, 0, COLOR_THEME_PRIMARY1);
    auto sc = new SourceChoice(
        window, grid.getFieldSlot(), 0, MIXSRC_LAST_CH,
        [=]() { return throttleSource2Source(g_model.thrTraceSrc); },
        [=](int16_t src) {
          int16_t val = source2ThrottleSource(src);
          if (val >= 0) {
            g_model.thrTraceSrc = val;
            SET_DIRTY();
          }
        }, 0, COLOR_THEME_PRIMARY1);
    sc->setAvailableHandler(isThrottleSourceAvailable);
    grid.nextLine();

    // Throttle trim
    new StaticText(window, grid.getLabelSlot(true), STR_TTRIM, 0, COLOR_THEME_PRIMARY1);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.thrTrim));
    grid.nextLine();

    // Throttle trim source
    new StaticText(window, grid.getLabelSlot(true), STR_TTRIM_SW, 0, COLOR_THEME_PRIMARY1);
    new SourceChoice(
        window, grid.getFieldSlot(), MIXSRC_FIRST_TRIM, MIXSRC_LAST_TRIM,
        [=]() { return g_model.getThrottleStickTrimSource(); },
        [=](int16_t src) { g_model.setThrottleStickTrimSource(src); }, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();
  }

  // Preflight parameters
  {
    new Subtitle(window, grid.getLineSlot(), STR_PREFLIGHT, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();

    // Display checklist
    new StaticText(window, grid.getLabelSlot(true), STR_CHECKLIST, 0, COLOR_THEME_PRIMARY1);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.displayChecklist));
    grid.nextLine();

    // Throttle warning
    new StaticText(window, grid.getLabelSlot(true), STR_THROTTLE_WARNING, 0, COLOR_THEME_PRIMARY1);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_INVERTED(g_model.disableThrottleWarning));
    grid.nextLine();

    // Custom Throttle warning
    new StaticText(window, grid.getLabelSlot(true), STR_CUSTOM_THROTTLE_WARNING, 0, COLOR_THEME_PRIMARY1);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.enableCustomThrottleWarning));
    grid.nextLine();
    // Custom Throttle warning value
    new StaticText(window, grid.getLabelSlot(true), STR_CUSTOM_THROTTLE_WARNING_VAL, 0, COLOR_THEME_PRIMARY1);
    new NumberEdit(window, grid.getFieldSlot(), -100, 100, GET_SET_DEFAULT(g_model.customThrottleWarningPosition));
    grid.nextLine();

    // Switches warning
    new StaticText(window, grid.getLabelSlot(true), STR_SWITCHWARNING, 0, COLOR_THEME_PRIMARY1);
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

    // Pots and sliders warning
#if NUM_POTS + NUM_SLIDERS
    {
      new StaticText(window, grid.getLabelSlot(true), STR_POTWARNINGSTATE, 0, COLOR_THEME_PRIMARY1);
      new Choice(window, grid.getFieldSlot(), {"OFF", "ON", "AUTO"}, 0, 2,
                 GET_SET_DEFAULT(g_model.potsWarnMode));
      grid.nextLine();

#if (NUM_POTS)
      {
        new StaticText(window, grid.getLabelSlot(true), STR_POTWARNING, 0, COLOR_THEME_PRIMARY1);
        auto group =
            new FormGroup(window, grid.getFieldSlot(),
                          FORM_BORDER_FOCUS_ONLY | PAINT_CHILDREN_FIRST);
        GridLayout centerGrid(group);
        for (int i = POT_FIRST, j = 0, k = 0; i <= POT_LAST; i++, k++) {
          char s[8];
          if ((IS_POT(i) || IS_POT_MULTIPOS(i)) && IS_POT_AVAILABLE(i)) {
            if (j > 0 && ((j % 4) == 0)) centerGrid.nextLine();

            auto button = new TextButton(
                group, centerGrid.getSlot(4, j % 4),
                getStringAtIndex(s, STR_VSRCRAW, i + 1), nullptr,
                OPAQUE | ((g_model.potsWarnEnabled & (1 << k)) ? BUTTON_CHECKED
                                                               : 0));
            button->setPressHandler([button, k] {
              g_model.potsWarnEnabled ^= (1 << k);
              if ((g_model.potsWarnMode == POTS_WARN_MANUAL) &&
                  (g_model.potsWarnEnabled & (1 << k))) {
                SAVE_POT_POSITION(k);
              }
              button->check(g_model.potsWarnEnabled & (1 << k) ? true : false);
              SET_DIRTY();
              return (g_model.potsWarnEnabled & (1 << k) ? 1 : 0);
            });
            j++;
          } else {
            g_model.potsWarnEnabled &= ~(1 << k);
          }
        }
        grid.addWindow(group);
      }
#endif

#if (NUM_SLIDERS)
      {
        new StaticText(window, grid.getLabelSlot(true), STR_SLIDERWARNING, 0, COLOR_THEME_PRIMARY1);
        auto group =
            new FormGroup(window, grid.getFieldSlot(),
                          FORM_BORDER_FOCUS_ONLY | PAINT_CHILDREN_FIRST);
        GridLayout centerGrid(group);
        for (int i = SLIDER_FIRST, j = 0, k = NUM_POTS; i <= SLIDER_LAST; i++, k++) {
          char s[8];
          if ((IS_SLIDER(i))) {
            if (j > 0 && ((j % 4) == 0)) centerGrid.nextLine();

            auto *button = new TextButton(
                group, centerGrid.getSlot(4, j % 4),
                getStringAtIndex(s, STR_VSRCRAW, i + 1), nullptr,
                OPAQUE | ((g_model.potsWarnEnabled & (1 << k)) ? BUTTON_CHECKED
                                                               : 0));
            button->setPressHandler([button, k] {
              g_model.potsWarnEnabled ^= (1 << (k));
              if ((g_model.potsWarnMode == POTS_WARN_MANUAL) &&
                  (g_model.potsWarnEnabled & (1 << k))) {
                SAVE_POT_POSITION(k);
              }
              button->check(g_model.potsWarnEnabled & (1 << k) ? true : false);
              SET_DIRTY();
              return (g_model.potsWarnEnabled & (1 << k) ? 1 : 0);
            });
            j++;
          }
        }
        grid.addWindow(group);
      }
#endif
    }
  #endif
  }

  grid.nextLine();

  // Center beeps
  {
    new StaticText(window, grid.getLabelSlot(false), STR_BEEPCTR, 0, COLOR_THEME_PRIMARY1);
    auto group = new FormGroup(window, grid.getFieldSlot(), FORM_BORDER_FOCUS_ONLY | PAINT_CHILDREN_FIRST);
    GridLayout centerGrid(group);
    for (int i = 0, j = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
      char s[2];
      if (i < NUM_STICKS ||  (IS_POT_SLIDER_AVAILABLE(i) && !IS_POT_MULTIPOS(i))) { // multipos cannot be centered
        if (j > 0 && (j % 6) == 0)
          centerGrid.nextLine();

        new TextButton(
            group, centerGrid.getSlot(6, j % 6),
            getStringAtIndex(s, STR_RETA123, i),
            [=]() -> uint8_t {
              BFBIT_FLIP(g_model.beepANACenter, bfBit<BeepANACenter>(i));
              SET_DIRTY();
              return (bfSingleBitGet<BeepANACenter>(g_model.beepANACenter, i) ? 1 : 0);
            },
            OPAQUE | (bfSingleBitGet<BeepANACenter>(g_model.beepANACenter, i)
                          ? BUTTON_CHECKED
                          : 0));
        j++;
      }
    }
    grid.addWindow(group);
  }

  // Global functions
  new StaticText(window, grid.getLabelSlot(), STR_USE_GLOBAL_FUNCS, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_INVERTED(g_model.noGlobalFunctions));
  grid.nextLine();

  {
     // Model ADC jitter filter
    new StaticText(window, grid.getLabelSlot(), STR_JITTER_FILTER, 0, COLOR_THEME_PRIMARY1);
    new Choice(window, grid.getFieldSlot(), STR_ADCFILTERVALUES, 0, 2, GET_SET_DEFAULT(g_model.jitterFilter));
    grid.nextLine();
  }

  // Internal module
  {
    new Subtitle(window, grid.getLineSlot(), TR_INTERNALRF, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();
    grid.addWindow(new ModuleWindow(window, {0, grid.getWindowHeight(), LCD_W, 0}, INTERNAL_MODULE));
  }

  // External module
  {
    new Subtitle(window, grid.getLineSlot(), TR_EXTERNALRF, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();
    grid.addWindow(new ModuleWindow(window, {0, grid.getWindowHeight(), LCD_W, 0}, EXTERNAL_MODULE));
  }

  // Trainer
  {
    new Subtitle(window, grid.getLineSlot(), STR_TRAINER, 0, COLOR_THEME_PRIMARY1);
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

