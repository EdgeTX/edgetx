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
#include "opentx.h"

#include "form.h"
#include "choice.h"
#include "button.h"

#include "mixer_scheduler.h"
#include "bind_menu_d16.h"
#include "custom_failsafe.h"
#include "ppm_settings.h"
#include "channel_range.h"

#if defined(PXX2)
#include "access_settings.h"
#endif

#if defined(CROSSFIRE)
#include "telemetry/crossfire.h"
#include "crossfire_settings.h"
#endif

#if defined(AFHDS2) || defined(AFHDS3)
#include "flysky_settings.h"
#endif

#if defined(AFHDS2)
#include "pulses/flysky.h"
#endif

#if defined(MULTIMODULE)
#include "mpm_settings.h"
#include "multi_rfprotos.h"
#include "io/multi_protolist.h"
#endif

#define SET_DIRTY()     storageDirty(EE_MODEL)

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

class ModuleWindow : public FormGroup
{
 public:
  ModuleWindow(Window* parent, uint8_t moduleIdx);
  void updateModule();
  void updateSubType();

 protected:
  uint8_t moduleIdx;

  ModuleOptions* modOpts = nullptr;
  ChannelRange* chRange = nullptr;
  Window *rxID = nullptr;
  TextButton *bindButton = nullptr;
  TextButton *rangeButton = nullptr;
  TextButton *registerButton = nullptr;
  Window *fsLine = nullptr;

  void startRSSIDialog(std::function<void()> closeHandler = nullptr);
};

struct FailsafeChoice : public FormGroup {
  FailsafeChoice(Window* parent, uint8_t moduleIdx);
};

static void fs_changed(lv_event_t* e)
{
  auto obj = lv_event_get_target(e);
  auto fs = (Choice*)lv_obj_get_user_data(obj);
  if (!fs) return;

  auto btn_obj = (lv_obj_t*)lv_event_get_user_data(e);
  if (!btn_obj) return;

  if (fs->getIntValue() == FAILSAFE_CUSTOM) {
    lv_obj_clear_flag(btn_obj, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(btn_obj, LV_OBJ_FLAG_HIDDEN);
  }
}

FailsafeChoice::FailsafeChoice(Window* parent, uint8_t moduleIdx) :
  FormGroup(parent, rect_t{})
{
  setFlexLayout(LV_FLEX_FLOW_ROW);

  auto md = &g_model.moduleData[moduleIdx];
  auto choice =
      new Choice(this, rect_t{}, STR_VFAILSAFE, 0, FAILSAFE_LAST,
                 GET_DEFAULT(md->failsafeMode), [=](int32_t newValue) {
                   md->failsafeMode = newValue;
                   SET_DIRTY();
                 });

  auto btn = new TextButton(this, rect_t{}, STR_SET, [=]() -> uint8_t {
    new FailSafePage(moduleIdx);
    return 0;
  });

  auto c_obj = choice->getLvObj();
  auto btn_obj = btn->getLvObj();
  lv_obj_add_event_cb(c_obj, fs_changed, LV_EVENT_VALUE_CHANGED, btn_obj);
  lv_event_send(c_obj, LV_EVENT_VALUE_CHANGED, nullptr);
}

ModuleWindow::ModuleWindow(Window* parent, uint8_t moduleIdx) :
    FormGroup(parent, rect_t{}),
    moduleIdx(moduleIdx)
{
  setFlexLayout();
  updateModule();
}

void ModuleWindow::updateModule()
{
  FlexGridLayout grid(col_dsc, row_dsc, 2);
  clear();

  modOpts = nullptr;
  chRange = nullptr;
  rxID = nullptr;
  bindButton = nullptr;
  rangeButton = nullptr;
  registerButton = nullptr;
  fsLine = nullptr;

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
#if defined(AFHDS2) || defined(AFHDS3)
  else if (isModuleFlySky(moduleIdx)) {
    modOpts = new FlySkySettings(this, grid, moduleIdx);
  }
#endif
#if defined(MULTIMODULE)
  else if (isModuleMultimodule(moduleIdx)) {
    modOpts = new MultimoduleSettings(this, grid, moduleIdx);
  }
#endif

  // Channel Range
  auto line = newLine(&grid);
  new StaticText(line, rect_t{}, STR_CHANNELRANGE, 0, COLOR_THEME_PRIMARY1);
  chRange = new ModuleChannelRange(line, moduleIdx);

  // PPM modules
  if (isModulePPM(moduleIdx)) {
    new PpmSettings(this, grid, moduleIdx);
  }

  // Generic module parameters

  // Bind and Range buttons
  if (!isModuleRFAccess(moduleIdx) && (isModuleModelIndexAvailable(moduleIdx) ||
                                       isModuleBindRangeAvailable(moduleIdx))) {

    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, STR_RECEIVER, 0, COLOR_THEME_PRIMARY1);

    auto box = new FormGroup(line, rect_t{});
    box->setFlexLayout(LV_FLEX_FLOW_ROW);

    // Model index
    auto modelId = &g_model.header.modelId[moduleIdx];
    rxID = new NumberEdit(box, rect_t{}, 0, getMaxRxNum(moduleIdx),
                          GET_DEFAULT(*modelId), [=](int32_t newValue) {
                            if (newValue != *modelId) {
                              *modelId = newValue;
#if defined(CROSSFIRE)
                              if (isModuleCrossfire(moduleIdx)) {
                                moduleState[moduleIdx].counter =
                                    CRSF_FRAME_MODELID;
                              }
#endif
                              SET_DIRTY();
                            }
                          });

    if (isModuleBindRangeAvailable(moduleIdx)) {
      bindButton = new TextButton(box, rect_t{},STR_MODULE_BIND);
      bindButton->setPressHandler([=]() -> uint8_t {
        if (moduleState[moduleIdx].mode == MODULE_MODE_RANGECHECK) {
          if (rangeButton) rangeButton->check(false);
        }
        if (moduleState[moduleIdx].mode == MODULE_MODE_BIND) {
          moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
#if defined(AFHDS2)
          if (isModuleFlySky(moduleIdx)) resetPulsesAFHDS2();
#endif
          if (isModuleDSMP(moduleIdx)) restartModule(moduleIdx);
          return 0;
        } else {
          if (isModuleR9MNonAccess(moduleIdx) || isModuleD16(moduleIdx) ||
              IS_R9_MULTI(moduleIdx)) {
            new BindChoiceMenu(
                Layer::back(), moduleIdx, [=]() { bindButton->check(true); },
                [=]() { bindButton->check(false); });
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
          if (bindButton->checked()) {
            bindButton->check(false);
            this->invalidate();
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
  }

  // Failsafe
  fsLine = newLine(&grid);
  new StaticText(fsLine, rect_t{}, STR_FAILSAFE, 0, COLOR_THEME_PRIMARY1);
  new FailsafeChoice(fsLine, moduleIdx);

#if defined(PXX2)
  // Register and Range buttons
  if (isModuleRFAccess(moduleIdx)) {
    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, STR_MODULE, 0, COLOR_THEME_PRIMARY1);
    registerButton = new TextButton(line, rect_t{}, STR_REGISTER);
    registerButton->setPressHandler([=]() -> uint8_t {
      new pxx2::RegisterDialog(Layer::back(), moduleIdx);
      return 0;
    });

    rangeButton = new TextButton(line, rect_t{}, STR_MODULE_RANGE);
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

    line = newLine(&grid);
    new StaticText(line, rect_t{}, STR_OPTIONS, 0, COLOR_THEME_PRIMARY1);
    auto options = new TextButton(line, rect_t{}, STR_SET);
    options->setPressHandler([=]() {
      new pxx2::ModuleOptions(Layer::back(), moduleIdx);
      return 0;
    });
  }
#endif

  // R9M Power
  if (isModuleR9M_FCC_VARIANT(moduleIdx)) {
    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, STR_RF_POWER, 0, COLOR_THEME_PRIMARY1);
    new Choice(line, rect_t{}, STR_R9M_FCC_POWER_VALUES, 0, R9M_FCC_POWER_MAX,
               GET_SET_DEFAULT(md->pxx.power));
  }

  if (isModuleR9M_LBT(moduleIdx)) {
    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, STR_RF_POWER, 0, COLOR_THEME_PRIMARY1);
    new Choice(line, rect_t{}, STR_R9M_LBT_POWER_VALUES, 0,
               R9M_LBT_POWER_MAX,
               GET_DEFAULT(min<uint8_t>(md->pxx.power,
                                        R9M_LBT_POWER_MAX)),
               SET_DEFAULT(md->pxx.power));
  }
#if defined(PXX2)
  // Receivers
  if (isModuleRFAccess(moduleIdx)) {
    for (uint8_t receiverIdx = 0; receiverIdx < PXX2_MAX_RECEIVERS_PER_MODULE;
         receiverIdx++) {

      char label[] = TR_RECEIVER " X";
      label[sizeof(label) - 2] = '1' + receiverIdx;

      auto line = newLine(&grid);
      new StaticText(line, rect_t{}, label, 0, COLOR_THEME_PRIMARY1);
      new pxx2::ReceiverButton(line, rect_t{}, moduleIdx, receiverIdx);
    }
  }
#endif
  // SBUS refresh rate
  if (isModuleSBUS(moduleIdx)) {
    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, STR_REFRESHRATE, 0, COLOR_THEME_PRIMARY1);

    auto box = new FormGroup(line, rect_t{});
    box->setFlexLayout(LV_FLEX_FLOW_ROW);

    auto edit = new NumberEdit(
        box, rect_t{}, SBUS_MIN_PERIOD, SBUS_MAX_PERIOD,
        GET_DEFAULT((int16_t)md->sbus.refreshRate *
                        SBUS_STEPSIZE +
                    SBUS_DEF_PERIOD),
        SET_VALUE(md->sbus.refreshRate,
                  (newValue - SBUS_DEF_PERIOD) / SBUS_STEPSIZE),
        0, PREC1);
    edit->setSuffix(STR_MS);
    edit->setStep(SBUS_STEPSIZE);
    new Choice(box, rect_t{}, STR_SBUS_INVERSION_VALUES, 0, 1,
               GET_SET_DEFAULT(md->sbus.noninverted));
#if defined(RADIO_TX16S)
    new StaticText(this, rect_t{}, STR_WARN_5VOLTS, 0, COLOR_THEME_PRIMARY1);
#endif
  }

  if (isModuleGhost(moduleIdx)) {
    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, "Raw 12 bits", 0, COLOR_THEME_PRIMARY1);
    new CheckBox(line, rect_t{}, GET_SET_DEFAULT(md->ghost.raw12bits));
  }

  updateSubType();
}

void ModuleWindow::updateSubType()
{
  if (modOpts) modOpts->update();
  if (chRange) chRange->update();

  if (rxID) {
    if (isModuleModelIndexAvailable(moduleIdx)) {
      lv_obj_clear_flag(rxID->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(rxID->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    }
  }

  if (fsLine) {
    if (isModuleFailsafeAvailable(moduleIdx)) {
      lv_obj_clear_flag(fsLine->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(fsLine->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    }
  }
}

#if defined(PCBNV14)
#define SIGNAL_POSTFIX
#define SIGNAL_MESSAGE "SGNL"
#else
#define SIGNAL_POSTFIX " db"
#define SIGNAL_MESSAGE "RSSI"
#endif

void ModuleWindow::startRSSIDialog(std::function<void()> closeHandler)
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

// void ModuleWindow::checkEvents()
// {
//   if (isModuleFailsafeAvailable(moduleIdx) != hasFailsafe && rfChoice &&
//       !rfChoice->isEditMode()) {
//     hasFailsafe = isModuleFailsafeAvailable(moduleIdx);
//     update();
//   }

//   FormGroup::checkEvents();
// }

class ModuleSubTypeChoice: public Choice
{
  uint8_t moduleIdx;

public:
  ModuleSubTypeChoice(Window* parent, uint8_t moduleIdx);
  void update();
  void openMenu() override;
};

ModuleSubTypeChoice::ModuleSubTypeChoice(Window *parent, uint8_t moduleIdx) :
    Choice(parent, rect_t{}, 0, 0, nullptr), moduleIdx(moduleIdx)
{
  ModuleData *md = &g_model.moduleData[moduleIdx];
  setGetValueHandler(GET_DEFAULT(md->subType));
  update();
}

void ModuleSubTypeChoice::update()
{
  ModuleData* md = &g_model.moduleData[moduleIdx];

  if (isModuleXJT(moduleIdx)) {
    setMin(MODULE_SUBTYPE_PXX1_ACCST_D16);
    setMax(MODULE_SUBTYPE_PXX1_LAST);
    setValues(STR_XJT_ACCST_RF_PROTOCOLS);
    setGetValueHandler(GET_DEFAULT(md->subType));
    setSetValueHandler([=](int32_t newValue) {
      md->subType = newValue;
      md->channelsStart = 0;
      md->channelsCount = defaultModuleChannels_M8(moduleIdx);
      SET_DIRTY();
    });
    setAvailableHandler(nullptr);
  }
  else if (isModuleDSM2(moduleIdx)) {
    setMin(DSM2_PROTO_LP45);
    setMax(DSM2_PROTO_DSMX);
    setValues(STR_DSM_PROTOCOLS);
    setGetValueHandler(GET_DEFAULT(md->subType));
    setSetValueHandler(SET_DEFAULT(md->subType));
    setAvailableHandler(nullptr);
  }
  else if (isModuleR9M(moduleIdx)) {
    setMin(MODULE_SUBTYPE_R9M_FCC);
    setMax(MODULE_SUBTYPE_R9M_LAST);
    setValues(STR_R9M_REGION);
    setGetValueHandler(GET_DEFAULT(md->subType));
    setSetValueHandler(SET_DEFAULT(md->subType));
    setAvailableHandler(nullptr);    
  }
#if defined(PXX2)
  else if (isModulePXX2(moduleIdx)) {
    setMin(MODULE_SUBTYPE_ISRM_PXX2_ACCESS);
    setMax(MODULE_SUBTYPE_ISRM_PXX2_ACCST_D16);
    setValues(STR_ISRM_RF_PROTOCOLS);
    setGetValueHandler(GET_DEFAULT(md->subType));
    setSetValueHandler(SET_DEFAULT(md->subType));
    setAvailableHandler(nullptr);    
  }
#endif
#if defined(AFHDS2) || defined(AFHDS3)
  else if (isModuleFlySky(moduleIdx)) {
    setMin(0);
    setMax(FLYSKY_SUBTYPE_AFHDS2A);
    setValues(STR_FLYSKY_PROTOCOLS);
    setGetValueHandler(GET_DEFAULT(md->subType));
    setSetValueHandler(SET_DEFAULT(md->subType));

#if defined(PCBNV14) && !defined(SIMU)
    if (moduleIdx == INTERNAL_MODULE) {
      if (hardwareOptions.pcbrev == PCBREV_NV14) {
        md->subType = FLYSKY_SUBTYPE_AFHDS2A;
        setAvailableHandler([](int v) { return v == FLYSKY_SUBTYPE_AFHDS2A; });
      } else {
        md->subType = FLYSKY_SUBTYPE_AFHDS3;
        setAvailableHandler([](int v) { return v == FLYSKY_SUBTYPE_AFHDS3; });
      }
    }
#elif !defined(SIMU)
    md->subType = FLYSKY_SUBTYPE_AFHDS3;
    setAvailableHandler([](int v) { return v == FLYSKY_SUBTYPE_AFHDS3; });
#else
    setAvailableHandler(nullptr);
#endif
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
      new RfScanDialog(parent, protos, [=](){ update(); });
    } else {
      TRACE("!protos->isScanning()");
    }

    setTextHandler([=](int value) { return protos->getProtoLabel(value); });

    setGetValueHandler(GET_DEFAULT(md->multi.rfProtocol));
    setSetValueHandler([=](int newValue) {
        md->multi.rfProtocol = newValue;
        md->subType = 0;
        resetMultiProtocolsOptions(moduleIdx);

        MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);
        status.invalidate();

        uint32_t startUpdate = RTOS_GET_MS();
        while (!status.isValid() && (RTOS_GET_MS() - startUpdate < 250));
      });
  }
#endif
  else {
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
    return;
  }
  
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
  
  // update choice value
  lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
}

void ModuleSubTypeChoice::openMenu()
{
#if defined(MULTIMODULE)
  if (isModuleMultimodule(moduleIdx)) {
    auto menu = new Menu(this);

    if (!menuTitle.empty()) menu->setTitle(menuTitle);
    menu->setCloseHandler([=]() { setEditMode(false); });

    setEditMode(true);
    invalidate();

    auto protos = MultiRfProtocols::instance(moduleIdx);
    protos->fillList([=](const MultiRfProtocols::RfProto &p) {
        addValue(p.label.c_str());
        menu->addLine(p.label.c_str(), [=]() {
            setValue(p.proto);
            lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
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

static void update_module_window(lv_event_t* e)
{
  ModuleWindow* mw = (ModuleWindow*)lv_event_get_user_data(e);
  if (!mw) return;

  mw->updateSubType();
}

ModulePage::ModulePage(uint8_t moduleIdx) : Page(ICON_MODEL_SETUP)
{
  const char* title = moduleIdx == INTERNAL_MODULE ?
    STR_INTERNALRF : STR_EXTERNALRF;
  header.setTitle(title);

  auto form = new FormGroup(&body, rect_t{});
  form->setFlexLayout();
  form->padAll(lv_dpx(8));

  FlexGridLayout grid(col_dsc, row_dsc, 2);

  // Module Type
  auto line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_MODE, 0, COLOR_THEME_PRIMARY1);

  auto box = new FormGroup(line, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW);

  ModuleData* md = &g_model.moduleData[moduleIdx];
  auto moduleChoice = new Choice(box, rect_t{}, STR_INTERNAL_MODULE_PROTOCOLS,
                                 MODULE_TYPE_NONE, MODULE_TYPE_COUNT - 1,
                                 GET_DEFAULT(md->type));

  moduleChoice->setAvailableHandler([=](int8_t moduleType) {
    return moduleIdx == INTERNAL_MODULE ? isInternalModuleAvailable(moduleType)
                                        : isExternalModuleAvailable(moduleType);
  });

  auto subTypeChoice = new ModuleSubTypeChoice(box, moduleIdx);  
  auto moduleWindow = new ModuleWindow(form, moduleIdx);

  // This needs to be after moduleWindow has been created
  moduleChoice->setSetValueHandler([=](int32_t newValue) {
    setModuleType(moduleIdx, newValue);

    moduleWindow->updateModule();
    subTypeChoice->update();

    SET_DIRTY();
  });

  lv_obj_add_event_cb(subTypeChoice->getLvObj(), update_module_window,
                      LV_EVENT_VALUE_CHANGED, moduleWindow);
}
