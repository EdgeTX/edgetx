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

struct MPMProtoOption : public FormGroup::Line
{
  StaticText* label;
  Choice* choice;
  NumberEdit* edit;
  CheckBox* cb;
  DynamicNumber<int>* rssi;

  MPMProtoOption(FormGroup* form, FlexGridLayout *layout);
  void update(const MultiRfProtocols::RfProto* rfProto, ModuleData* md);
};

MPMProtoOption::MPMProtoOption(FormGroup* form, FlexGridLayout *layout) :
  FormGroup::Line(form, layout)
{
  if (layout) layout->resetPos();
  label = new StaticText(this, rect_t{}, "", 0, COLOR_THEME_PRIMARY1);

  auto box = new FormGroup(this, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW);

  choice = new Choice(box, rect_t{}, 0, 0, nullptr);
  edit = new NumberEdit(box, rect_t{}, 0, 0, nullptr);
  cb = new CheckBox(box, rect_t{}, nullptr, nullptr);
  rssi = new DynamicNumber<int>(
      box, rect_t{}, [] { return (int)TELEMETRY_RSSI(); }, 0, "RSSI: ", " db");
}

void MPMProtoOption::update(const MultiRfProtocols::RfProto* rfProto, ModuleData* md)
{
  if (!rfProto || !rfProto->getOptionStr()) {
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
    return;
  }

  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_HIDDEN);

  const char *title = rfProto->getOptionStr();
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
  } else if (title == STR_MULTI_TELEMETRY) {
    choice->setValues(STR_MULTI_TELEMETRY_MODE);
    choice->setMin(min);
    choice->setMax(max);
    choice->setGetValueHandler(GET_DEFAULT(md->multi.optionValue));
    choice->setSetValueHandler(SET_DEFAULT(md->multi.optionValue));
    lv_obj_clear_flag(choice->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  } else if (title == STR_MULTI_WBUS) {
    choice->setValues(STR_MULTI_WBUS_MODE);
    choice->setMin(0);
    choice->setMax(1);
    choice->setGetValueHandler(GET_DEFAULT(md->multi.optionValue));
    choice->setSetValueHandler(SET_DEFAULT(md->multi.optionValue));
    lv_obj_clear_flag(choice->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  } else if (rfProto->proto == MODULE_SUBTYPE_MULTI_FS_AFHDS2A) {
    edit->setMin(50);
    edit->setMax(400);
    edit->setGetValueHandler(GET_DEFAULT(50 + 5 * md->multi.optionValue));
    edit->setSetValueHandler(SET_VALUE(md->multi.optionValue, (newValue - 50) / 5));
    edit->setStep(5);
    lv_obj_clear_flag(edit->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  } else if (rfProto->proto == MODULE_SUBTYPE_MULTI_DSM2) {
    cb->setGetValueHandler([=]() { return md->multi.optionValue & 0x01; });
    cb->setSetValueHandler([=](int8_t newValue) {
      md->multi.optionValue = (md->multi.optionValue & 0xFE) + newValue;
    });
    lv_obj_clear_flag(cb->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  } else {
    if (min == 0 && max == 1) {
      cb->setGetValueHandler(GET_DEFAULT(md->multi.optionValue));
      cb->setSetValueHandler(SET_DEFAULT(md->multi.optionValue));
      lv_obj_clear_flag(cb->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    } else {
      edit->setMin(-128);
      edit->setMax(127);
      edit->setGetValueHandler(GET_DEFAULT(md->multi.optionValue));
      edit->setSetValueHandler(SET_DEFAULT(md->multi.optionValue));
      lv_obj_clear_flag(edit->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(rssi->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    }
  }
}

struct MPMSubtype : public FormGroup::Line
{
  Choice* choice;

  MPMSubtype(FormGroup* form, FlexGridLayout *layout, uint8_t moduleIdx);
  void update(const MultiRfProtocols::RfProto* rfProto, ModuleData* md);
};

MPMSubtype::MPMSubtype(FormGroup* form, FlexGridLayout *layout, uint8_t moduleIdx) :
  FormGroup::Line(form, layout)
{
  if (layout) layout->resetPos();
  new StaticText(this, rect_t{}, STR_RF_PROTOCOL, 0, COLOR_THEME_PRIMARY1);

  auto md = &g_model.moduleData[moduleIdx];
  choice = new Choice(
      this, rect_t{}, 0, 0, [=]() { return md->subType; },
      [=](int16_t newValue) {
        md->subType = newValue;
        resetMultiProtocolsOptions(moduleIdx);
        SET_DIRTY();
      });
}

void MPMSubtype::update(const MultiRfProtocols::RfProto* rfProto, ModuleData* md)
{
  if (!rfProto) {
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
    return;
  }

  choice->setValues(rfProto->subProtos);
  choice->setMax(rfProto->subProtos.size() - 1);

  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
  lv_event_send(choice->getLvObj(), LV_EVENT_VALUE_CHANGED, nullptr);
}

struct MPMServoRate : public FormGroup::Line {
  MPMServoRate(FormGroup* form, FlexGridLayout* layout, uint8_t moduleIdx);
};

static const char* _servoRates[] = {"22ms", "11ms"};

MPMServoRate::MPMServoRate(FormGroup* form, FlexGridLayout *layout, uint8_t moduleIdx) :
  FormGroup::Line(form, layout)
{
  if (layout) layout->resetPos();
  new StaticText(this, rect_t{}, STR_MULTI_SERVOFREQ, 0, COLOR_THEME_PRIMARY1);

  auto md = &g_model.moduleData[moduleIdx];
  new Choice(
      this, rect_t{}, _servoRates, 0, 1,
      [=]() { return (md->multi.optionValue & 0x02) >> 1; },
      [=](int16_t newValue) {
        md->multi.optionValue =
            (md->multi.optionValue & 0xFD) + (newValue << 1);
      });
}

struct MPMAutobind : public FormGroup::Line {
  MPMAutobind(FormGroup* form, FlexGridLayout* layout, uint8_t moduleIdx);
};

MPMAutobind::MPMAutobind(FormGroup* form, FlexGridLayout *layout, uint8_t moduleIdx) :
  FormGroup::Line(form, layout)
{
  if (layout) layout->resetPos();
  new StaticText(this, rect_t{}, STR_MULTI_AUTOBIND, 0, COLOR_THEME_PRIMARY1);

  auto md = &g_model.moduleData[moduleIdx];
  new CheckBox(this, rect_t{}, GET_SET_DEFAULT(md->multi.autoBindMode));
}

struct MPMChannelMap : public FormGroup::Line
{
  MPMChannelMap(FormGroup* form, FlexGridLayout *layout, uint8_t moduleIdx);
  void update(const MultiRfProtocols::RfProto* rfProto);
};

MPMChannelMap::MPMChannelMap(FormGroup* form, FlexGridLayout *layout, uint8_t moduleIdx) :
  FormGroup::Line(form, layout)
{
  if (layout) layout->resetPos();
  new StaticText(this, rect_t{}, STR_DISABLE_CH_MAP, 0, COLOR_THEME_PRIMARY1);

  auto md = &g_model.moduleData[moduleIdx];
  new CheckBox(this, rect_t{}, GET_SET_DEFAULT(md->multi.disableMapping));
}  

void MPMChannelMap::update(const MultiRfProtocols::RfProto* rfProto)
{
  if (rfProto && rfProto->supportsDisableMapping()) {
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
  }
}

MultimoduleSettings::MultimoduleSettings(Window *parent,
                                         const FlexGridLayout &g,
                                         uint8_t moduleIdx) :
  FormGroup(parent, rect_t{}), md(&g_model.moduleData[moduleIdx]),
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
  // TODO: set event cb on st_line->choice / LV_EVENT_VALUE_CHANGED ?

  opt_line = new MPMProtoOption(this, &grid);
  sr_line = new MPMServoRate(this, &grid, moduleIdx);
  ab_line = new MPMAutobind(this, &grid, moduleIdx);

  // Low power mode
  line = newLine(&grid);
  new StaticText(line, rect_t{}, STR_MULTI_LOWPOWER, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_DEFAULT(md->multi.lowPowerMode));

  // Disable telemetry
  line = newLine(&grid);
  new StaticText(line, rect_t{}, STR_DISABLE_TELEM, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_DEFAULT(md->multi.disableTelemetry));

  cm_line = new MPMChannelMap(this, &grid, moduleIdx);
}

void MultimoduleSettings::update()
{
  auto mpm_rfprotos = MultiRfProtocols::instance(moduleIdx);
  auto rfProto = mpm_rfprotos->getProto(md->multi.rfProtocol);

  st_line->update(rfProto, md);
  opt_line->update(rfProto, md);

  auto multi_proto = md->multi.rfProtocol;
  if (multi_proto == MODULE_SUBTYPE_MULTI_DSM2) {
    lv_obj_clear_flag(sr_line->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ab_line->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(sr_line->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ab_line->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  }

  cm_line->update(rfProto);
}
