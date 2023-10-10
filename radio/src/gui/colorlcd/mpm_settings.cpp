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

#include "mpm_settings.h"
#include "opentx.h"

#include "multi_rfprotos.h"
#include "io/multi_protolist.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

static void update_mpm_settings(lv_event_t* e)
{
  MultimoduleSettings* ms = (MultimoduleSettings*)lv_event_get_user_data(e);
  if (!ms) return;

  ms->update();
  lv_event_send(ms->getParent()->getLvObj(), LV_EVENT_REFRESH, nullptr);
}

struct MPMProtoOption : public FormWindow::Line
{
  StaticText* label;
  Choice* choice;
  NumberEdit* edit;
  ToggleSwitch* cb;
  DynamicNumber<uint16_t>* rssi;

  MPMProtoOption(FormWindow* form, FlexGridLayout *layout);
  void update(const MultiRfProtocols::RfProto* rfProto, ModuleData* md, uint8_t moduleIdx);
};

MPMProtoOption::MPMProtoOption(FormWindow* form, FlexGridLayout *layout) :
  FormWindow::Line(form, layout)
{
  if (layout) layout->resetPos();
  label = new StaticText(this, rect_t{}, "", 0, COLOR_THEME_PRIMARY1);

  auto box = new FormWindow(this, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW, 4);
  box->setWidth((LCD_W - 20) * 2 / 3);

  choice = new Choice(box, rect_t{}, 0, 0, nullptr);
  edit = new NumberEdit(box, rect_t{}, 0, 0, nullptr);
  cb = new ToggleSwitch(box, rect_t{}, nullptr, nullptr);
  rssi = new DynamicNumber<uint16_t>(
      box, rect_t{}, [] { return (uint16_t)TELEMETRY_RSSI(); }, 0, getRxStatLabels()->label, getRxStatLabels()->unit);
  rssi->padTop(5);
}

void MPMProtoOption::update(const MultiRfProtocols::RfProto* rfProto, ModuleData* md, uint8_t moduleIdx)
{
  if (!rfProto || !getMultiOptionTitle(moduleIdx)) {
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
    return;
  }

  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_HIDDEN);

  const char *title = getMultiOptionTitle(moduleIdx);
  label->setText(title);

  lv_obj_add_flag(choice->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(edit->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(cb->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(rssi->getLvObj(), LV_OBJ_FLAG_HIDDEN);

  int8_t min, max;
  getMultiOptionValues(rfProto->proto, min, max);

  if (title == STR_MULTI_RFPOWER) {
    choice->setValues(STR_MULTI_POWER);
    choice->setMin(0);
    choice->setMax(15);
    choice->setGetValueHandler(GET_DEFAULT(md->multi.optionValue));
    choice->setSetValueHandler(SET_DEFAULT(md->multi.optionValue));
    lv_obj_clear_flag(choice->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    lv_event_send(choice->getLvObj(), LV_EVENT_VALUE_CHANGED, nullptr);
  } else if (title == STR_MULTI_TELEMETRY) { // Bayang
    choice->setValues(STR_MULTI_BAYANG_OPTIONS);
    choice->setMin(min);
    choice->setMax(max);
    choice->setGetValueHandler(GET_DEFAULT(md->multi.optionValue));
    choice->setSetValueHandler(SET_DEFAULT(md->multi.optionValue));
    lv_obj_clear_flag(choice->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    lv_event_send(choice->getLvObj(), LV_EVENT_VALUE_CHANGED, nullptr);
  } else if (title == STR_MULTI_WBUS) { // e.g. WFLY2 but may not be used anymore
    choice->setValues(STR_MULTI_WBUS_MODE);
    choice->setMin(0);
    choice->setMax(1);
    choice->setGetValueHandler(GET_DEFAULT(md->multi.optionValue));
    choice->setSetValueHandler(SET_DEFAULT(md->multi.optionValue));
    lv_obj_clear_flag(choice->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    lv_event_send(choice->getLvObj(), LV_EVENT_VALUE_CHANGED, nullptr);
  } else if (rfProto->proto == MODULE_SUBTYPE_MULTI_FS_AFHDS2A) {
    edit->setMin(50);
    edit->setMax(400);
    edit->setGetValueHandler(GET_DEFAULT(50 + 5 * md->multi.optionValue));
    edit->setSetValueHandler(SET_VALUE(md->multi.optionValue, (newValue - 50) / 5));
    edit->setStep(5);
    edit->update();
    lv_obj_clear_flag(edit->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  } else if (rfProto->proto == MODULE_SUBTYPE_MULTI_DSM2) {
    cb->setGetValueHandler([=]() { return md->multi.optionValue & 0x01; });
    cb->setSetValueHandler([=](int8_t newValue) {
      md->multi.optionValue = (md->multi.optionValue & 0xFE) + newValue;
      SET_DIRTY();
    });
    cb->update();
    lv_obj_clear_flag(cb->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  } else {
    if (min == 0 && max == 1) {
      cb->setGetValueHandler(GET_DEFAULT(md->multi.optionValue));
      cb->setSetValueHandler(SET_DEFAULT(md->multi.optionValue));
      cb->update();
      lv_obj_clear_flag(cb->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    } else {
      edit->setMin(min);
      edit->setMax(max);
      edit->setGetValueHandler(GET_DEFAULT(md->multi.optionValue));
      edit->setSetValueHandler(SET_DEFAULT(md->multi.optionValue));
      lv_obj_clear_flag(edit->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      edit->update();
      if (title == STR_MULTI_RFTUNE) {
        rssi->setPrefix(getRxStatLabels()->label);
        rssi->setSuffix(getRxStatLabels()->unit);
        lv_obj_clear_flag(rssi->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      }
    }
  }
}

struct MPMSubtype : public FormWindow::Line
{
  Choice* choice;

  MPMSubtype(FormWindow* form, FlexGridLayout *layout, uint8_t moduleIdx);
  void update(const MultiRfProtocols::RfProto* rfProto, uint8_t moduleIdx);
  void checkEvents();

  protected:                                 
    uint8_t moduleIdx;
    uint8_t DSM2lastSubType;
    bool DSM2autoUpdated;
};

void MPMSubtype::checkEvents() { 
  //
  // DSM2: successful bind in auto mode changes DSM2 subType
  //
  ModuleData* md = &g_model.moduleData[this->moduleIdx];

  if(md->multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2) {           // do this only for DSM2 
    uint8_t subType = md->subType;                                  // fetch DSM2 subType

    if(subType != DSM2lastSubType) {                                // if DSM2 subType has auto changed
      DSM2autoUpdated = true;                                       // indicate this was not user triggered
      DSM2lastSubType = subType;                                    // memorize new DSM2 subType
      choice->setValue(subType);                                    // set new DSM2 subType
      lv_event_send(choice->getLvObj(), LV_EVENT_REFRESH, nullptr); // refresh subType field
    }
  }
}

static void subtype_event_cb(lv_event_t* e)
{
  if (lv_event_get_param(e)) return;
  auto obj = (lv_obj_t*)lv_event_get_user_data(e);
  if (obj) lv_event_send(obj, LV_EVENT_VALUE_CHANGED, nullptr);
}

MPMSubtype::MPMSubtype(FormWindow* form, FlexGridLayout *layout, uint8_t moduleIdx) :
  FormWindow::Line(form, layout)
{
  this->moduleIdx = moduleIdx;
  this->DSM2lastSubType = g_model.moduleData[this->moduleIdx].subType;
  this->DSM2autoUpdated = false;

  if (layout) layout->resetPos();
  new StaticText(this, rect_t{}, STR_RF_PROTOCOL, 0, COLOR_THEME_PRIMARY1);

  auto md = &g_model.moduleData[moduleIdx];
  choice = new Choice(
      this, rect_t{}, 0, 0, [=]() { return md->subType; },
      [=](int16_t newValue) {
        md->subType = newValue;
        if(!DSM2autoUpdated)                        // reset MPM options only if user triggered
          resetMultiProtocolsOptions(moduleIdx);
        DSM2autoUpdated = false; 
        SET_DIRTY();
      });

  lv_obj_add_event_cb(choice->getLvObj(), subtype_event_cb, LV_EVENT_VALUE_CHANGED, lvobj);
}

void MPMSubtype::update(const MultiRfProtocols::RfProto* rfProto, uint8_t moduleIdx)
{
  if (!rfProto || rfProto->subProtos.size() == 0) {
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
    return;
  }

  choice->setValues(rfProto->subProtos);
  choice->setMax(rfProto->subProtos.size() - 1);

  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_HIDDEN);

  bool stop=true;
  lv_event_send(choice->getLvObj(), LV_EVENT_VALUE_CHANGED, &stop);
}

struct MPMDSMCloned : public FormWindow::Line {
  MPMDSMCloned(FormWindow* form, FlexGridLayout* layout, uint8_t moduleIdx);
  void update() const { lv_event_send(choice->getLvObj(), LV_EVENT_VALUE_CHANGED, nullptr); }

 private:
  Choice* choice;
};

MPMDSMCloned::MPMDSMCloned(FormWindow* form, FlexGridLayout *layout, uint8_t moduleIdx) :
    FormWindow::Line(form, layout)
{
  if (layout) layout->resetPos();
  new StaticText(this, rect_t{}, STR_SUBTYPE, 0, COLOR_THEME_PRIMARY1);

  auto md = &g_model.moduleData[moduleIdx];
  choice = new Choice(this, rect_t{}, STR_MULTI_DSM_CLONE, 0, 1, 0, 0);

  choice->setGetValueHandler(GET_DEFAULT((md->multi.optionValue & 0x04) >> 2));
  choice->setSetValueHandler(SET_VALUE(
      md->multi.optionValue, (md->multi.optionValue & 0xFB) + (newValue << 2)));
}

struct MPMServoRate : public FormWindow::Line {
  MPMServoRate(FormWindow* form, FlexGridLayout* layout, uint8_t moduleIdx);
  void update() const { lv_event_send(choice->getLvObj(), LV_EVENT_VALUE_CHANGED, nullptr); }

 private:
  Choice* choice;
};

MPMServoRate::MPMServoRate(FormWindow* form, FlexGridLayout *layout, uint8_t moduleIdx) :
  FormWindow::Line(form, layout)
{
  if (layout) layout->resetPos();
  new StaticText(this, rect_t{}, STR_MULTI_SERVOFREQ, 0, COLOR_THEME_PRIMARY1);

  auto md = &g_model.moduleData[moduleIdx];
  choice = new Choice(this, rect_t{}, STR_MULTI_DSM_OPTIONS, 0, 1, 0, 0);

  choice->setGetValueHandler(GET_DEFAULT((md->multi.optionValue & 0x02) >> 1));
  choice->setSetValueHandler(SET_VALUE(
      md->multi.optionValue, (md->multi.optionValue & 0xFD) + (newValue << 1)));
}

struct MPMAutobind : public FormWindow::Line {
  MPMAutobind(FormWindow* form, FlexGridLayout* layout, uint8_t moduleIdx);
  void update() const { cb->update(); }

 private:
  ToggleSwitch* cb;
};

MPMAutobind::MPMAutobind(FormWindow* form, FlexGridLayout *layout, uint8_t moduleIdx) :
  FormWindow::Line(form, layout)
{
  if (layout) layout->resetPos();
  new StaticText(this, rect_t{}, STR_MULTI_AUTOBIND, 0, COLOR_THEME_PRIMARY1);

  auto md = &g_model.moduleData[moduleIdx];
  cb = new ToggleSwitch(this, rect_t{}, GET_SET_DEFAULT(md->multi.autoBindMode));
}

struct MPMChannelMap : public FormWindow::Line
{
  MPMChannelMap(FormWindow* form, FlexGridLayout *layout, uint8_t moduleIdx);
  void update(const MultiRfProtocols::RfProto* rfProto);
 private:
  ToggleSwitch* cb;
};

MPMChannelMap::MPMChannelMap(FormWindow* form, FlexGridLayout *layout, uint8_t moduleIdx) :
  FormWindow::Line(form, layout)
{
  if (layout) layout->resetPos();
  new StaticText(this, rect_t{}, STR_DISABLE_CH_MAP, 0, COLOR_THEME_PRIMARY1);

  auto md = &g_model.moduleData[moduleIdx];
  cb = new ToggleSwitch(this, rect_t{}, GET_SET_DEFAULT(md->multi.disableMapping));
}

void MPMChannelMap::update(const MultiRfProtocols::RfProto* rfProto)
{
  if (rfProto && rfProto->supportsDisableMapping()) {
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
    cb->update();
  } else {
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
  }
}

MultimoduleSettings::MultimoduleSettings(Window *parent,
                                         const FlexGridLayout &g,
                                         uint8_t moduleIdx) :
  FormWindow(parent, rect_t{}), md(&g_model.moduleData[moduleIdx]),
  moduleIdx(moduleIdx)
{
  FlexGridLayout grid(g);
  setFlexLayout();

  // TODO: needs to be placed differently
  // MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);
  // if (status.protocolName[0] && status.isValid()) {
  //   new StaticText(this, grid.getFieldSlot(2, 1), status.protocolName, 0,
  //                  COLOR_THEME_PRIMARY1);
  //   grid.nextLine();
  // }

  // Multimodule status
  auto line = newLine(&grid);
  new StaticText(line, rect_t{}, STR_MODULE_STATUS, 0, COLOR_THEME_PRIMARY1);
  new DynamicText(
      line, rect_t{},
      [=] {
        char msg[64] = "";
        getModuleStatusString(moduleIdx, msg);
        return std::string(msg);
      },
      COLOR_THEME_PRIMARY1);

  st_line = new MPMSubtype(this, &grid, moduleIdx);

  lv_obj_add_event_cb(st_line->getLvObj(), update_mpm_settings,
                      LV_EVENT_VALUE_CHANGED, this);

  cl_line = new MPMDSMCloned(this, &grid, moduleIdx);
  opt_line = new MPMProtoOption(this, &grid);
  sr_line = new MPMServoRate(this, &grid, moduleIdx);
  ab_line = new MPMAutobind(this, &grid, moduleIdx);

  // Low power mode
  line = newLine(&grid);
  new StaticText(line, rect_t{}, STR_MULTI_LOWPOWER, 0, COLOR_THEME_PRIMARY1);
  lp_mode = new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(md->multi.lowPowerMode));

#if defined(MANUFACTURER_FRSKY)
  // Disable telemetry
  line = newLine(&grid);
  new StaticText(line, rect_t{}, STR_DISABLE_TELEM, 0, COLOR_THEME_PRIMARY1);
  disable_telem = new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(md->multi.disableTelemetry));
#endif

  cm_line = new MPMChannelMap(this, &grid, moduleIdx);

  // Ensure elements properly initalised
  update();
}

void MultimoduleSettings::update()
{
  auto mpm_rfprotos = MultiRfProtocols::instance(moduleIdx);
  auto rfProto = mpm_rfprotos->getProto(md->multi.rfProtocol);

  st_line->update(rfProto, moduleIdx);
  opt_line->update(rfProto, md, moduleIdx);

  auto multi_proto = md->multi.rfProtocol;
  if (multi_proto == MODULE_SUBTYPE_MULTI_DSM2) {
    lv_obj_clear_flag(sr_line->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    sr_line->update();
    lv_obj_add_flag(ab_line->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(sr_line->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ab_line->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    ab_line->update();
  }

  if (isMultiProtocolDSMCloneAvailable(moduleIdx)) {
    lv_obj_clear_flag(cl_line->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    cl_line->update();
  } else {
    lv_obj_add_flag(cl_line->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  }

  lp_mode->update();
#if defined(MANUFACTURER_FRSKY)
  disable_telem->update();
#endif
  cm_line->update(rfProto);
}
