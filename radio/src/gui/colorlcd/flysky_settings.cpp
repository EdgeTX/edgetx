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

#include "flysky_settings.h"
#include "opentx.h"

#include "pulses/flysky.h"
#include "pulses/afhds3.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

static const char* _afhds3_region[] = { "CE", "FCC" };

static const char* _afhds3_phy_mode[] = {
  // V0
  "Classic 18ch",
  "Classic 10ch",
  // V1
  "Routine 18ch",
  "Fast 8ch",
  "Lora 12ch",
};

class FSProtoOpts : public FormGroup
{
  std::function<uint8_t()> _getMode;
  std::function<void(uint8_t)> _setMode;
  
public:
  FSProtoOpts(Window* parent, std::function<uint8_t()> getMode,
              std::function<void(uint8_t)> setMode);
};

FSProtoOpts::FSProtoOpts(Window* parent, std::function<uint8_t()> getMode,
                         std::function<void(uint8_t)> setMode) :
  FormGroup(parent, rect_t{}),
  _getMode(std::move(getMode)),
  _setMode(std::move(setMode))
{
  setFlexLayout(LV_FLEX_FLOW_ROW);

  // PPM / PWM
  new Choice(
      this, rect_t{}, STR_FLYSKY_PULSE_PROTO, 0, 1,
      [=]() -> int { return _getMode() >> 1; },
      [=](int v) {
        _setMode((_getMode() & 1) | ((v & 1) << 1));
        SET_DIRTY();
      });

  // SBUS / iBUS
  new Choice(
      this, rect_t{}, STR_FLYSKY_SERIAL_PROTO, 0, 1,
      [=]() -> int { return _getMode() & 1; },
      [=](int v) {
        _setMode((_getMode() & 2) | (v & 1));
        SET_DIRTY();
      });
}

FlySkySettings::FlySkySettings(Window* parent, const FlexGridLayout& g,
                               uint8_t moduleIdx) :
    FormGroup(parent, rect_t{}),
    moduleIdx(moduleIdx),
    md(&g_model.moduleData[moduleIdx]),
    grid(g)
{
  setFlexLayout();
}

void FlySkySettings::update()
{
  clear();
  
#if defined(AFHDS2)
  if (isModuleAFHDS2A(moduleIdx)) {
    // RX options:
    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, STR_OPTIONS, 0, COLOR_THEME_PRIMARY1);

    new FSProtoOpts(
        line, [=]() { return md->flysky.mode; },
        [=](uint8_t v) { md->flysky.mode = v; });

#if defined(AFHDS2) && defined(PCBNV14)
    if (getNV14RfFwVersion() >= 0x1000E) {
      line = newLine(&grid);
      static const char* _rf_power[] = {"Default", "High"};
      new StaticText(line, rect_t{}, STR_MULTI_RFPOWER);
      new Choice(line, rect_t{}, _rf_power, 0, 1,
                 GET_DEFAULT(md->flysky.rfPower),
                 [=](int32_t newValue) -> void {
                   md->flysky.rfPower = newValue;
                   resetPulsesAFHDS2();
                 });
    }
#endif
  }
#endif
#if defined(AFHDS3)
  if (isModuleAFHDS3(moduleIdx)) {

    // Status
    auto line = newLine(&grid);
    new StaticText(line, rect_t{}, STR_MODULE_STATUS, 0, COLOR_THEME_PRIMARY1);
    new DynamicText(line, rect_t{}, [=] {
      char msg[64] = "";
      getModuleStatusString(moduleIdx, msg);
      return std::string(msg);
    });

    // TYPE
    line = newLine(&grid);
    new StaticText(line, rect_t{}, STR_TYPE, 0, COLOR_THEME_PRIMARY1);

    auto box = new FormGroup(line, rect_t{});
    box->setFlexLayout(LV_FLEX_FLOW_ROW);

    new Choice(box, rect_t{}, _afhds3_region, 1, afhds3::LNK_ES_FCC,
               GET_SET_DEFAULT(md->afhds3.emi));

    new Choice(box, rect_t{}, _afhds3_phy_mode, 0, afhds3::PHYMODE_MAX,
               GET_SET_DEFAULT(md->afhds3.phyMode));

    // // Power source
    // line = newLine(&grid);
    // new StaticText(line, rect_t{}, STR_AFHDS3_POWER_SOURCE, 0,
    //                COLOR_THEME_PRIMARY1);
    // new DynamicText(line, rect_t{}, [=] {
    //   char msg[64] = "";
    //   getModuleSyncStatusString(moduleIdx, msg);
    //   return std::string(msg);
    // });

    // // RX Freq
    // line = newLine(&grid);
    // new StaticText(line, rect_t{}, STR_AFHDS3_RX_FREQ, 0, COLOR_THEME_PRIMARY1);
    // auto edit = new NumberEdit(line, rect_t{}, AFHDS3_MIN_FREQ, AFHDS3_MAX_FREQ,
    //                            GET_DEFAULT(md->afhds3.rxFreq()));
    // edit->setSetValueHandler(
    //     [=](int32_t newValue) { md->afhds3.setRxFreq((uint16_t)newValue); });
    // edit->setSuffix(STR_HZ);

    // // Module actual power
    // line = newLine(&grid);
    // new StaticText(line, rect_t{}, STR_AFHDS3_ACTUAL_POWER, 0,
    //                COLOR_THEME_PRIMARY1);
    // new DynamicText(line, rect_t{}, [=] {
    //   char msg[64] = "";
    //   getStringAtIndex(msg, STR_AFHDS3_POWERS, actualAfhdsRunPower(moduleIdx));
    //   return std::string(msg);
    // });

    // // Module power
    // line = newLine(&grid);
    // new StaticText(line, rect_t{}, STR_RF_POWER, 0, COLOR_THEME_PRIMARY1);
    // new Choice(line, rect_t{}, STR_AFHDS3_POWERS,
    //            afhds3::RUN_POWER::RUN_POWER_FIRST,
    //            afhds3::RUN_POWER::RUN_POWER_LAST,
    //            GET_SET_DEFAULT(md->afhds3.runPower));
  }
#endif
}
