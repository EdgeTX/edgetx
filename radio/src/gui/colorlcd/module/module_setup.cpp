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

#include "module_setup.h"

#include "bind_menu_d16.h"
#include "button.h"
#include "channel_range.h"
#include "choice.h"
#include "custom_failsafe.h"
#include "form.h"
#include "mixer_scheduler.h"
#include "edgetx.h"
#include "ppm_settings.h"
#include "storage/modelslist.h"
#include "etx_lv_theme.h"

#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
#include "pxx1_settings.h"
#endif

#if defined(PXX2)
#include "access_settings.h"
#endif

#if defined(CROSSFIRE)
#include "crossfire_settings.h"
#include "telemetry/crossfire.h"
#endif

#if defined(AFHDS2)
#include "afhds2a_settings.h"
#endif

#if defined(AFHDS3)
#include "afhds3_settings.h"
#endif

#if defined(AFHDS2)
#include "pulses/flysky.h"
#endif

#if defined(MULTIMODULE)
#include "io/multi_protolist.h"
#include "mpm_settings.h"
#include "multi_rfprotos.h"
#endif

#define SET_DIRTY() storageDirty(EE_MODEL)

#define ETX_STATE_UNIQUE_ID_WARN LV_STATE_USER_1

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

struct FailsafeChoice : public Window {
  FailsafeChoice(Window* parent, uint8_t moduleIdx) :
      Window(parent, rect_t{}), moduleIdx(moduleIdx)
  {
    padAll(PAD_TINY);
    setFlexLayout(LV_FLEX_FLOW_ROW, PAD_SMALL, LV_SIZE_CONTENT);

    auto md = &g_model.moduleData[moduleIdx];
    new Choice(this, rect_t{}, STR_VFAILSAFE, 0, FAILSAFE_LAST,
              GET_DEFAULT(md->failsafeMode), [=](int32_t newValue) {
                md->failsafeMode = newValue;
                optsBtn->show(newValue == FAILSAFE_CUSTOM);
                SET_DIRTY();
              });

    optsBtn = new TextButton(this, rect_t{}, STR_SET, [=]() -> uint8_t {
      new FailSafePage(moduleIdx);
      return 0;
    });
    optsBtn->show(md->failsafeMode == FAILSAFE_CUSTOM);
  }

  void update() const
  {
    optsBtn->show(g_model.moduleData[moduleIdx].failsafeMode == FAILSAFE_CUSTOM);
  }

 private:
  uint8_t moduleIdx;
  TextButton* optsBtn;
};

class ModuleWindow : public Window
{
 public:
  ModuleWindow(Window* parent, uint8_t moduleIdx) :
      Window(parent, rect_t{}), moduleIdx(moduleIdx)
  {
    setFlexLayout();
    updateModule();
    lv_obj_add_event_cb(lvobj, ModuleWindow::mw_refresh_cb, LV_EVENT_REFRESH, this);
  }

  void updateModule()
  {
    FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);
    clear();

    modOpts = nullptr;
    chRange = nullptr;
    rxID = nullptr;
    bindButton = nullptr;
    rangeButton = nullptr;
    registerButton = nullptr;
    fsLine = nullptr;
    fsChoice = nullptr;
    rfPower = nullptr;

    // Module parameters
    ModuleData* md = &g_model.moduleData[moduleIdx];

    if (md->type == MODULE_TYPE_NONE) {
      return;
    }
  #if defined(CROSSFIRE)
    else if (isModuleCrossfire(moduleIdx)) {
      modOpts = new CrossfireSettings(this, grid, moduleIdx);
    }
  #endif
  #if defined(AFHDS2)
    else if (isModuleAFHDS2A(moduleIdx)) {
      modOpts = new AFHDS2ASettings(this, grid, moduleIdx);
    }
  #endif
  #if defined(AFHDS3)
    else if (isModuleAFHDS3(moduleIdx)) {
      modOpts = new AFHDS3Settings(this, grid, moduleIdx);
    }
  #endif
  #if defined(MULTIMODULE)
    else if (isModuleMultimodule(moduleIdx)) {
      modOpts = new MultimoduleSettings(this, grid, moduleIdx);
    }
  #endif
  #if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
    else if (moduleIdx == INTERNAL_MODULE && isModuleXJT(moduleIdx) &&
            g_eeGeneral.antennaMode == ANTENNA_MODE_PER_MODEL) {
      modOpts = new PXX1AntennaSettings(this, grid, moduleIdx);
    }
  #endif

    // Channel Range
    auto line = newLine(grid);
    new StaticText(line, rect_t{}, STR_CHANNELRANGE);
    chRange = new ModuleChannelRange(line, moduleIdx);

    // Failsafe
    fsLine = newLine(grid);
    new StaticText(fsLine, rect_t{}, STR_FAILSAFE);
    fsChoice = new FailsafeChoice(fsLine, moduleIdx);

    // PPM modules
    if (isModulePPM(moduleIdx)) {
      // PPM frame
      auto line = newLine(grid);
      new StaticText(line, rect_t{}, STR_PPMFRAME);
      auto obj = new PpmFrameSettings<PpmModule>(line, &md->ppm);

      // copy pointer to frame len edit object to channel range
      chRange->setPpmFrameLenEditObject(obj->getPpmFrameLenEditObject());
    }

    // Generic module parameters

    // Bind and Range buttons
    if (!isModuleRFAccess(moduleIdx) && (isModuleModelIndexAvailable(moduleIdx) ||
                                        isModuleBindRangeAvailable(moduleIdx))) {
      // Is Reciever ID Unique
      if (isModuleModelIndexAvailable(moduleIdx)) {
        auto line = newLine(grid);
        new StaticText(line, rect_t{}, "");
        idUnique = new StaticText(line, rect_t{}, "");
        etx_txt_color(idUnique->getLvObj(), COLOR_THEME_WARNING_INDEX,
                      ETX_STATE_UNIQUE_ID_WARN);
        updateIDStaticText(moduleIdx);
      }

      auto line = newLine(grid);
      new StaticText(line, rect_t{}, STR_RECEIVER);

      auto box = new Window(line, rect_t{});
      box->padAll(PAD_TINY);
      box->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_MEDIUM, LV_SIZE_CONTENT);

      // Model index
      auto modelId = &g_model.header.modelId[moduleIdx];
      rxID = new NumberEdit(box, {0, 0, NUM_W, 0}, 0, getMaxRxNum(moduleIdx),
                            GET_DEFAULT(*modelId), [=](int32_t newValue) {
                              if (newValue != *modelId) {
                                *modelId = newValue;
                                modelslist.updateCurrentModelCell();
                                updateIDStaticText(moduleIdx);
  #if defined(CROSSFIRE)
                                if (isModuleCrossfire(moduleIdx)) {
                                  moduleState[moduleIdx].counter =
                                      CRSF_FRAME_MODELID;
                                }
  #endif
                                SET_DIRTY();
                              }
                            });

      if (isModuleBindRangeAvailable(moduleIdx) || isModuleCrossfire(moduleIdx)) {
        bindButton = new TextButton(box, rect_t{}, STR_MODULE_BIND);
        bindButton->setPressHandler([=]() -> uint8_t {
          if (moduleState[moduleIdx].mode == MODULE_MODE_RANGECHECK) {
            if (rangeButton) rangeButton->check(false);
          }
          if (moduleState[moduleIdx].mode == MODULE_MODE_BIND) {
            moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
  #if defined(MULTIMODULE)
            if (isModuleMultimodule(moduleIdx)) {
              setMultiBindStatus(moduleIdx, MULTI_BIND_NONE);
            }
  #endif
  #if defined(AFHDS2)
            if (isModuleAFHDS2A(moduleIdx)) resetPulsesAFHDS2();
  #endif
            if (isModuleDSMP(moduleIdx)) restartModule(moduleIdx);
            return 0;
          } else {
            if (isModuleR9MNonAccess(moduleIdx) || isModuleD16(moduleIdx) ||
                IS_R9_MULTI(moduleIdx)) {
              new BindChoiceMenu(
                  moduleIdx, [=]() { bindButton->check(true); },
                  [=]() { bindButton->check(false); });
              return 0;
            }
  #if defined(MULTIMODULE)
            if (isModuleMultimodule(moduleIdx)) {
              setMultiBindStatus(moduleIdx, MULTI_BIND_INITIATED);
            }
  #endif
            moduleState[moduleIdx].mode = MODULE_MODE_BIND;
            if (isModuleELRS(moduleIdx))
              AUDIO_PLAY(AU_SPECIAL_SOUND_CHEEP); // Since ELRS bind is just one frame, we need to play the sound manually
  #if defined(AFHDS2)
            if (isModuleAFHDS2A(moduleIdx)) {
              resetPulsesAFHDS2();
            }
  #endif
            return 1;
          }
          return 0;
        });
        bindButton->setCheckHandler([=]() {
          if (moduleState[moduleIdx].mode != MODULE_MODE_BIND) {
            if (bindButton->checked()) {
              bindButton->check(false);
            }
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
          rangeButton = new TextButton(box, rect_t{}, STR_MODULE_RANGE);
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
              if (isModuleAFHDS2A(moduleIdx)) {
                resetPulsesAFHDS2();
              }
  #endif
              startRSSIDialog([=]() {
  #if defined(AFHDS2)
                if (isModuleAFHDS2A(moduleIdx)) {
                  resetPulsesAFHDS2();
                }
  #endif
              });
              return 1;
            }
          });
        }

  #if defined(PXX2)
        if (isModuleISRM(moduleIdx)) {
          auto options = new TextButton(box, rect_t{}, LV_SYMBOL_SETTINGS);
          options->setPressHandler([=]() {
            new pxx2::ModuleOptions(moduleIdx);
            return 0;
          });
        }
  #endif
      }
    }
  #if defined(PXX2)
    else if (isModuleRFAccess(moduleIdx)) {

      // Register and Range buttons
      auto line = newLine(grid);
      new StaticText(line, rect_t{}, STR_MODULE);

      auto box = new Window(line, rect_t{});
      box->padAll(PAD_TINY);
      box->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_LARGE);

      registerButton = new TextButton(box, rect_t{}, STR_REGISTER);
      registerButton->setPressHandler([=]() -> uint8_t {
        new pxx2::RegisterDialog(moduleIdx);
        return 0;
      });

      rangeButton = new TextButton(box, rect_t{}, STR_MODULE_RANGE);
      rangeButton->setPressHandler([=]() -> uint8_t {
        if (moduleState[moduleIdx].mode == MODULE_MODE_RANGECHECK) {
          moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
          return 0;
        } else {
          moduleState[moduleIdx].mode = MODULE_MODE_RANGECHECK;
          startRSSIDialog();
          return 1;
        }
      });

      auto options = new TextButton(box, rect_t{}, LV_SYMBOL_SETTINGS);
      options->setPressHandler([=]() {
        new pxx2::ModuleOptions(moduleIdx);
        return 0;
      });

      // Model index
      line = newLine(grid);
      new StaticText(line, rect_t{}, STR_RECEIVER_NUM);
      auto modelId = &g_model.header.modelId[moduleIdx];
      new NumberEdit(line, rect_t{}, 0, getMaxRxNum(moduleIdx),
                    GET_SET_DEFAULT(*modelId));
    }
  #endif

    // R9M Power
    if (isModuleR9MNonAccess(moduleIdx)) {
      auto line = newLine(grid);
      new StaticText(line, rect_t{}, STR_RF_POWER);
      rfPower = new Choice(line, rect_t{}, 0, 0, GET_SET_DEFAULT(md->pxx.power));
      line = newLine(grid);
      new StaticText(line, rect_t{}, STR_MODULE_TELEMETRY);
      new DynamicText(line, rect_t{}, [=]() {
        if (modulePortHasRx(moduleIdx)) {
          return std::string(STR_MODULE_TELEM_ON);
        } else {
          return std::string(STR_DISABLE_INTERNAL);
        }
      });
    }

  #if defined(PXX2)
    // Receivers
    if (isModuleRFAccess(moduleIdx)) {
      for (uint8_t receiverIdx = 0; receiverIdx < PXX2_MAX_RECEIVERS_PER_MODULE;
          receiverIdx++) {
        char label[] = TR_RECEIVER " X";
        label[sizeof(label) - 2] = '1' + receiverIdx;

        auto line = newLine(grid);
        new StaticText(line, rect_t{}, label);
        new pxx2::ReceiverButton(line, rect_t{}, moduleIdx, receiverIdx);
      }
    }
  #endif
    // SBUS refresh rate
    if (isModuleSBUS(moduleIdx)) {
      auto line = newLine(grid);
      new StaticText(line, rect_t{}, STR_REFRESHRATE);

      auto box = new Window(line, rect_t{});
      box->padAll(PAD_TINY);
      box->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_SMALL);

      auto edit = new NumberEdit(
          box, rect_t{}, SBUS_MIN_PERIOD, SBUS_MAX_PERIOD,
          GET_DEFAULT((int16_t)md->sbus.refreshRate * SBUS_STEPSIZE +
                      SBUS_DEF_PERIOD),
          SET_VALUE(md->sbus.refreshRate,
                    (newValue - SBUS_DEF_PERIOD) / SBUS_STEPSIZE),
          PREC1);
      edit->setSuffix(STR_MS);
      edit->setStep(SBUS_STEPSIZE);
      new Choice(box, rect_t{}, STR_SBUS_INVERSION_VALUES, 0, 1,
                GET_SET_DEFAULT(md->sbus.noninverted));
  #if defined(RADIO_TX16S)
      new StaticText(this, rect_t{}, STR_WARN_5VOLTS);
  #endif
    }

    if (isModuleGhost(moduleIdx)) {
      auto line = newLine(grid);
      new StaticText(line, rect_t{}, "Raw 12 bits");
      new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(md->ghost.raw12bits));
    }

    updateSubType();
  }

  void updateSubType()
  {
    if (modOpts) modOpts->update();
    if (chRange) chRange->update();

    updateRxID();
    updateFailsafe();

    if (rfPower) {
      if (isModuleR9M_LBT(moduleIdx)) {
        rfPower->setMax(R9M_LBT_POWER_MAX);
        rfPower->setValues(STR_R9M_LBT_POWER_VALUES);
      } else {
        rfPower->setMax(R9M_FCC_POWER_MAX);
        rfPower->setValues(STR_R9M_FCC_POWER_VALUES);
      }
      rfPower->update();
    }
  }

  void updateRxID()
  {
    if (rxID) {
      if (isModuleModelIndexAvailable(moduleIdx)) {
        rxID->show();
        rxID->update();
      } else {
        rxID->hide();
      }
    }
  }

  void updateFailsafe()
  {
    if (fsLine) {
      if (isModuleFailsafeAvailable(moduleIdx)) {
        fsLine->show();
        fsChoice->update();
      } else {
        fsLine->hide();
      }
    }
  }

  void updateLayout()
  {
    if (isModuleISRM(moduleIdx))
      updateModule();
    else
      updateSubType();

    pulsesModuleSettingsUpdate(moduleIdx);
  }

  uint8_t getModuleIdx() const { return moduleIdx; }

  static LAYOUT_VAL(NUM_W, 60, 60)

 protected:
  uint8_t moduleIdx;

  ModuleOptions* modOpts = nullptr;
  ChannelRange* chRange = nullptr;
  NumberEdit* rxID = nullptr;
  TextButton* bindButton = nullptr;
  TextButton* rangeButton = nullptr;
  TextButton* registerButton = nullptr;
  Window* fsLine = nullptr;
  FailsafeChoice* fsChoice = nullptr;
  Choice* rfPower = nullptr;
  StaticText* idUnique = nullptr;

  void startRSSIDialog(std::function<void()> closeHandler = nullptr)
  {
    auto rssiDialog = new DynamicMessageDialog(
        STR_RANGE_TEST,
        [=]() {
          return std::to_string((int)TELEMETRY_RSSI()) + getRxStatLabels()->unit;
        },
        getRxStatLabels()->label, 50,
        COLOR_THEME_SECONDARY1_INDEX, CENTERED | FONT(XL));

    rssiDialog->setCloseHandler([this, closeHandler]() {
      rangeButton->check(false);
      moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
      if (closeHandler) closeHandler();
    });
  }

  void updateIDStaticText(int mdIdx)
  {
    if (idUnique == nullptr) return;
    char buffer[50];
    std::string idStr = STR_MODELIDUNIQUE;
    if (!modelslist.isModelIdUnique(mdIdx, buffer, sizeof(buffer))) {
      idStr = STR_MODELIDUSED;
      idStr = idStr + buffer;
      lv_obj_add_state(idUnique->getLvObj(), ETX_STATE_UNIQUE_ID_WARN);
    } else {
      lv_obj_clear_state(idUnique->getLvObj(), ETX_STATE_UNIQUE_ID_WARN);
    }
    idUnique->setText(idStr);
  }

  void checkEvents() override
  {
    if (bindButton != nullptr) {
      if (TELEMETRY_STREAMING() && isModuleELRS(moduleIdx))
        bindButton->setText(STR_MODULE_UNBIND);
      else if (isModuleELRS(moduleIdx))
        bindButton->setText(STR_MODULE_BIND);

      bindButton->show(isModuleBindRangeAvailable(moduleIdx));
    }
    Window::checkEvents();
  }

  static void mw_refresh_cb(lv_event_t* e)
  {
    auto mw = (ModuleWindow*)lv_event_get_user_data(e);
    if (mw) {
      mw->updateRxID();
      mw->updateFailsafe();
    }
  }
};

class ModuleSubTypeChoice : public Choice
{
 public:
  ModuleSubTypeChoice(Window* parent, uint8_t moduleIdx) :
      Choice(parent, rect_t{}, 0, 0,
            [=]() { return getSubTypeValue(); },
            [=](int32_t newValue) { setSubTypeValue(newValue); }),
      moduleIdx(moduleIdx)
  {
  }

  int getSubTypeValue()
  {
    if (isModuleXJT(moduleIdx) || isModuleDSM2(moduleIdx) || isModuleR9MNonAccess(moduleIdx)
#if defined(PPM)
        || isModulePPM(moduleIdx)
#endif
#if defined(PXX2)
        || isModuleISRM(moduleIdx)
#endif
       ) {
      return g_model.moduleData[moduleIdx].subType;
    } else {
      return g_model.moduleData[moduleIdx].multi.rfProtocol;
    }
  }

  void setSubTypeValue(int32_t newValue)
  {
    if (isModuleXJT(moduleIdx) || isModuleDSM2(moduleIdx) ||
        isModuleR9MNonAccess(moduleIdx) || isModuleSBUS(moduleIdx)
#if defined(PPM)
        || isModulePPM(moduleIdx)
#endif
#if defined(PXX2)
        || isModuleISRM(moduleIdx)
#endif
    ) {
      if (isModuleXJT(moduleIdx)) {
        g_model.moduleData[moduleIdx].channelsStart = 0;
        g_model.moduleData[moduleIdx].channelsCount = defaultModuleChannels_M8(moduleIdx);
      }
      g_model.moduleData[moduleIdx].subType = newValue;
      SET_DIRTY();
    } else {
      g_model.moduleData[moduleIdx].multi.rfProtocol = newValue;
      g_model.moduleData[moduleIdx].subType = 0;
      resetMultiProtocolsOptions(moduleIdx);

      MultiModuleStatus& status = getMultiModuleStatus(moduleIdx);
      status.invalidate();

      uint32_t startUpdate = RTOS_GET_MS();
      while (!status.isValid() && (RTOS_GET_MS() - startUpdate < 250))
        ;
      SET_DIRTY();
    }

    if (moduleWindow)
      moduleWindow->updateLayout();
  }

  void updateLayout()
  {
    if (isModuleXJT(moduleIdx)) {
      setMin(MODULE_SUBTYPE_PXX1_ACCST_D16);
      setMax(MODULE_SUBTYPE_PXX1_LAST);
      setValues(STR_XJT_ACCST_RF_PROTOCOLS);
      setTextHandler(nullptr);
    } else if (isModuleDSM2(moduleIdx)) {
      setMin(DSM2_PROTO_LP45);
      setMax(DSM2_PROTO_DSMX);
      setValues(STR_DSM_PROTOCOLS);
      setTextHandler(nullptr);
    }
    else if (isModuleSBUS(moduleIdx)) {
      setMin(SBUS_PROTO_TLM_NONE);
      setMax(SBUS_PROTO_TLM_SPORT);
      setValues(STR_SBUS_PROTOCOLS);
      setTextHandler(nullptr);
    }
#if defined(PPM)
    else if (isModulePPM(moduleIdx)) {
      setMin(PPM_PROTO_TLM_NONE);
      setMax(PPM_PROTO_TLM_SPORT);
      setValues(STR_PPM_PROTOCOLS);
      setTextHandler(nullptr);
    }
#endif
    else if (isModuleR9MNonAccess(moduleIdx)) {
      setMin(MODULE_SUBTYPE_R9M_FCC);
      setMax(MODULE_SUBTYPE_R9M_LAST);
      setValues(STR_R9M_REGION);
      setTextHandler(nullptr);
    }
#if defined(PXX2)
    else if (isModuleISRM(moduleIdx)) {
      setMin(MODULE_SUBTYPE_ISRM_PXX2_ACCESS);
      setMax(MODULE_SUBTYPE_ISRM_PXX2_ACCST_D16);
      setValues(STR_ISRM_RF_PROTOCOLS);
      setTextHandler(nullptr);
    }
#endif
#if defined(MULTIMODULE)
    else if (isModuleMultimodule(moduleIdx)) {
      setMin(0);
      setMax(0);
      values.clear();

      auto protos = MultiRfProtocols::instance(moduleIdx);
      protos->triggerScan();

      if (protos->isScanning()) {
        new RfScanDialog(protos, [=]() { updateLayout(); });
      } else {
        TRACE("!protos->isScanning()");
      }

      setTextHandler([=](int value) { return protos->getProtoLabel(value); });
    }
#endif
    else {
      hide();
      return;
    }

    update();
    show();
  }

  void openMenu() override
  {
#if defined(MULTIMODULE)
    if (isModuleMultimodule(moduleIdx)) {
      auto menu = new Menu();

      if (menuTitle) menu->setTitle(menuTitle);
      menu->setCloseHandler([=]() { setEditMode(false); });

      setEditMode(true);

      auto protos = MultiRfProtocols::instance(moduleIdx);
      protos->fillList([=](const MultiRfProtocols::RfProto& p) {
        addValue(p.label.c_str());
        menu->addLine(p.label.c_str(), [=]() {
          setValue(p.proto);
        });
      });

      ModuleData* md = &g_model.moduleData[moduleIdx];
      int idx = protos->getIndex(md->multi.rfProtocol);
      if (idx >= 0) menu->select(idx);
    } else
#endif
    {
      Choice::openMenu();
    }
  }

  void setModuleWindow(ModuleWindow* w) { moduleWindow = w; }

 protected:
  uint8_t moduleIdx;
  ModuleWindow* moduleWindow = nullptr;
};

ModulePage::ModulePage(uint8_t moduleIdx) : Page(ICON_MODEL_SETUP)
{
  const char* title2 =
      moduleIdx == INTERNAL_MODULE ? STR_INTERNALRF : STR_EXTERNALRF;
  header->setTitle(STR_MENU_MODEL_SETUP);
  header->setTitle2(title2);

  body->setFlexLayout();

  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

  // Module Type
  auto line = body->newLine(grid);
  new StaticText(line, rect_t{}, STR_MODE);

  auto box = new Window(line, rect_t{});
  box->padAll(PAD_TINY);
  box->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_SMALL, LV_SIZE_CONTENT);

  ModuleData* md = &g_model.moduleData[moduleIdx];
  auto moduleChoice =
      new Choice(box, rect_t{}, STR_MODULE_PROTOCOLS, MODULE_TYPE_NONE,
                 MODULE_TYPE_COUNT - 1, GET_DEFAULT(md->type));

  moduleChoice->setAvailableHandler([=](int8_t moduleType) {
    return moduleIdx == INTERNAL_MODULE ? isInternalModuleAvailable(moduleType)
                                        : isExternalModuleAvailable(moduleType);
  });

  auto subTypeChoice = new ModuleSubTypeChoice(box, moduleIdx);
  auto moduleWindow = new ModuleWindow(body, moduleIdx);

  subTypeChoice->setModuleWindow(moduleWindow);

  // This needs to be after moduleWindow has been created
  moduleChoice->setSetValueHandler([=](int32_t newValue) {
    setModuleType(moduleIdx, newValue);

    moduleWindow->updateModule();
    subTypeChoice->updateLayout();

    SET_DIRTY();
  });

  // Call this last in case it opens the 'Scanning' popup.
  subTypeChoice->updateLayout();
}
