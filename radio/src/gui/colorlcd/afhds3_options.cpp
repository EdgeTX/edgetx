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

#include "afhds3_options.h"
#include "opentx.h"

//#include "checkbox.h"

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

#define SET_DIRTY()

static const char* _analog_outputs[] = { "PWM", "PPM" };
static const char* _bus_types[] = { "iBUS", "iBUS2", "SBUS" };
static const char* _v1_bus_types[] = { "PWM", "PPM", "SBUS", "iBUS IN", "iBUS OUT" };

AFHDS3_Options::AFHDS3_Options(uint8_t moduleIdx) : Page(ICON_MODEL_SETUP)
{
  cfg = afhds3::getConfig(moduleIdx);
    
  std::string title =
      moduleIdx == INTERNAL_MODULE ? STR_INTERNALRF : STR_EXTERNALRF;
  title += "\nAFHDS3";

  header.setTitle(title);

  auto form = new FormGroup(&body, rect_t{});
  form->setFlexLayout();
  form->padAll(lv_dpx(8));

  FlexGridLayout grid(col_dsc, row_dsc, 2);

  if (cfg->version == 0) {
    auto vCfg = &cfg->v0;

    auto line = form->newLine(&grid);
    new StaticText(line, rect_t{}, "PWM frequency");
    new NumberEdit(line, rect_t{}, 50, 400, GET_SET_DEFAULT(vCfg->PWMFrequency.Frequency));

    line = form->newLine(&grid);
    new StaticText(line, rect_t{}, "PWM sychronized");
    new CheckBox(line, rect_t{}, GET_SET_DEFAULT(vCfg->PWMFrequency.Synchronized));

    line = form->newLine(&grid);
    new StaticText(line, rect_t{}, "Analog output");
    new Choice(line, rect_t{}, _analog_outputs,
               afhds3::SES_ANALOG_OUTPUT_PWM, afhds3::SES_ANALOG_OUTPUT_PPM,
               GET_SET_DEFAULT(vCfg->AnalogOutput));

    line = form->newLine(&grid);
    new StaticText(line, rect_t{}, "External bus type");
    new Choice(line, rect_t{}, _bus_types,
               afhds3::EB_BT_IBUS1, afhds3::EB_BT_SBUS1,
               GET_SET_DEFAULT(vCfg->ExternalBusType));
    
  } else {
    auto vCfg = &cfg->v1;

    auto line = form->newLine(&grid);
    new StaticText(line, rect_t{}, "PWM frequency");
    new NumberEdit(line, rect_t{}, 50, 400,
                   GET_DEFAULT(vCfg->PWMFrequenciesV1.PWMFrequencies[0]),
                   [=](int16_t newVal) {
                     for (uint8_t i = 0; i < SES_NB_MAX_CHANNELS; i++) {
                       vCfg->PWMFrequenciesV1.PWMFrequencies[i] = newVal;
                     }
                   });

    line = form->newLine(&grid);
    new StaticText(line, rect_t{}, "PWM sychronized");
    new CheckBox(
        line, rect_t{}, GET_DEFAULT(vCfg->PWMFrequenciesV1.Synchronized),
        [=](uint8_t newVal) {
          vCfg->PWMFrequenciesV1.Synchronized = newVal ? 0xFFFFFFFF : 0;
        });

    for (uint8_t i = 0; i < SES_NPT_NB_MAX_PORTS; i++) {
      line = form->newLine(&grid);

      std::string portName = "NP";
      portName += 'A' + i;

      new StaticText(line, rect_t{}, portName.c_str());
      new Choice(line, rect_t{}, _v1_bus_types, afhds3::SES_NPT_PWM,
                 afhds3::SES_NPT_IBUS1_OUT,
                 GET_SET_DEFAULT(vCfg->NewPortTypes[i]));
    }
  }
}
