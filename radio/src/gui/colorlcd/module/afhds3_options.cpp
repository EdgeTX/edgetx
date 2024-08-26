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
#include "edgetx.h"

static const lv_coord_t col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

#define SET_DIRTY()

static const char* const _analog_outputs[] = { "PWM", "PPM" };
static const char* const _bus_types[] = { "iBUS OUT", "iBUS IN", "SBUS" };
static const char* const _v1_bus_types[] = { "PWM", "PPM", "SBUS", "iBUS IN", "iBUS OUT" };
static const char* const _v1_pwmfreq_types[] = { STR_ANALOG_SERVO, STR_DIGITAL_SERVO, "SR833HZ", "SFR1000HZ", STR_MULTI_CUSTOM };
static const char* const _v0_pwmfreq_types[] = { STR_ANALOG_SERVO, STR_DIGITAL_SERVO, STR_MULTI_CUSTOM };

static const uint16_t _v1_index2pwmvalue[] = {50, 333, 2, 1, 50};
static const uint16_t _v0_index2pwmvalue[] = {50, 333, 50};
static uint16_t _v1_pwmvalue_type[2][32] = {0xff};
static uint8_t channel_num[]={18, 10, 18, 8, 12};

struct PWMfrequencyChoice : public Window {
  PWMfrequencyChoice(Window* parent, uint8_t moduleIdx, uint8_t channelIdx);
  PWMfrequencyChoice(Window* parent, uint8_t moduleIdx);

  static LAYOUT_VAL(NUM_W, 60, 60)

 private:
  NumberEdit* num_edit = nullptr;
};

PWMfrequencyChoice::PWMfrequencyChoice(Window* parent, uint8_t moduleIdx, uint8_t channelIdx) :
  Window(parent, rect_t{})
{
  padAll(PAD_TINY);
  setFlexLayout(LV_FLEX_FLOW_ROW, PAD_TINY, LV_SIZE_CONTENT);
  uint16_t &pwmvalue_type = _v1_pwmvalue_type[moduleIdx][channelIdx];
  auto cfg = afhds3::getConfig(moduleIdx);
  auto vCfg = &cfg->v1;
  if( 0xff == pwmvalue_type )
  {
    if ( 50 == vCfg->PWMFrequenciesV1.PWMFrequencies[channelIdx] ) pwmvalue_type = 0;
    else if ( 333 == vCfg->PWMFrequenciesV1.PWMFrequencies[channelIdx] ) pwmvalue_type = 1;
    else if ( 2 == vCfg->PWMFrequenciesV1.PWMFrequencies[channelIdx] ) pwmvalue_type = 2;
    else if ( 1 == vCfg->PWMFrequenciesV1.PWMFrequencies[channelIdx] ) pwmvalue_type = 3;
    else  pwmvalue_type = 4;
  }
  new Choice(this, rect_t{}, _v1_pwmfreq_types, 0, 4,
                [=,&pwmvalue_type]{
                      return pwmvalue_type;
                  },
                [=,&pwmvalue_type](int32_t newValue) {
                      pwmvalue_type = newValue;
                      vCfg->PWMFrequenciesV1.PWMFrequencies[channelIdx] = _v1_index2pwmvalue[newValue];
                      DIRTY_CMD(cfg, afhds3::DirtyConfig::DC_RX_CMD_FREQUENCY_V1);
                      if (num_edit)
                        num_edit->show(pwmvalue_type == 4);
                    });
  num_edit = new NumberEdit(this, {0, 0, NUM_W, 0}, 50, 400,
                  [=,&pwmvalue_type] { return (pwmvalue_type==4?vCfg->PWMFrequenciesV1.PWMFrequencies[channelIdx]:50); },
                  [=](int16_t newVal) {
                    vCfg->PWMFrequenciesV1.PWMFrequencies[channelIdx] = newVal;
                    DIRTY_CMD(cfg, afhds3::DirtyConfig::DC_RX_CMD_FREQUENCY_V1);
                  });
  num_edit->show(pwmvalue_type == 4);
}

PWMfrequencyChoice::PWMfrequencyChoice(Window* parent, uint8_t moduleIdx ) :
  Window(parent, rect_t{})
{
  padAll(PAD_TINY);
  setFlexLayout(LV_FLEX_FLOW_ROW, PAD_TINY, LV_SIZE_CONTENT);
  uint16_t &pwmvalue_type = _v1_pwmvalue_type[moduleIdx][0];
  auto cfg = afhds3::getConfig(moduleIdx);
  auto vCfg = &cfg->v0;
  if( 0xff == pwmvalue_type )
  {
    if ( 50 == (vCfg->PWMFrequency.Frequency&0x7fff) ) pwmvalue_type = 0;
    else if ( 333 == (vCfg->PWMFrequency.Frequency&0x7fff) ) pwmvalue_type = 1;
    else  pwmvalue_type = 2;
  }
  new Choice(this, rect_t{}, _v0_pwmfreq_types, 0, 2,
                [=,&pwmvalue_type]{
                      return pwmvalue_type;
                },
                [=,&pwmvalue_type](int32_t newValue) {
                      pwmvalue_type = newValue;
                      vCfg->PWMFrequency.Frequency = _v0_index2pwmvalue[newValue];
                      DIRTY_CMD(cfg, afhds3::DirtyConfig::DC_RX_CMD_FREQUENCY_V0);
                      if (num_edit)
                        num_edit->show(pwmvalue_type == 2);
                });
  num_edit = new NumberEdit(this, {0, 0, NUM_W, 0}, 50, 400,
                  [=,&pwmvalue_type] { return (pwmvalue_type==2?vCfg->PWMFrequency.Frequency&0x7fff:50); },
                  [=](int16_t newVal) {
                      vCfg->PWMFrequency.Frequency = newVal;
                      DIRTY_CMD(cfg, afhds3::DirtyConfig::DC_RX_CMD_FREQUENCY_V0);
                  });
  num_edit->show(pwmvalue_type == 2);  
}

AFHDS3_Options::AFHDS3_Options(uint8_t moduleIdx) : Page(ICON_MODEL_SETUP)
{
  cfg = afhds3::getConfig(moduleIdx);
  std::string title =
      moduleIdx == INTERNAL_MODULE ? STR_INTERNALRF : STR_EXTERNALRF;
  header->setTitle(title);

  title = "AFHDS3 (";
  title += (moduleIdx == INTERNAL_MODULE ? "INRM301" : "FRM303");
  title += ")";
  header->setTitle2(title);

  body->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);

  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

  if (cfg->version == 0) {
    auto vCfg = &cfg->v0;

    auto line = body->newLine(grid);
    std::string temp_str = "PWM ";
    temp_str += STR_POWERMETER_FREQ;
    new StaticText(line, rect_t{}, temp_str);
    new PWMfrequencyChoice(line, moduleIdx);
    line = body->newLine(grid);

    temp_str = "PWM ";
    temp_str += STR_SYNC;
    new StaticText(line, rect_t{}, temp_str);
    new ToggleSwitch(line, rect_t{}, GET_SET_AND_SYNC(cfg, vCfg->PWMFrequency.Synchronized,
                 afhds3::DirtyConfig::DC_RX_CMD_FREQUENCY_V0));
    line = body->newLine(grid);

    temp_str = STR_CH;
    temp_str += " 1";
    new StaticText(line, rect_t{}, temp_str );
    new Choice(line, rect_t{}, _analog_outputs,
               afhds3::SES_ANALOG_OUTPUT_PWM, afhds3::SES_ANALOG_OUTPUT_PPM,
               GET_SET_AND_SYNC(cfg, vCfg->AnalogOutput, afhds3::DirtyConfig::DC_RX_CMD_OUT_PWM_PPM_MODE));

    line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_SERIAL_BUS);
    new Choice(line, rect_t{}, _bus_types, 0, 2,
               GET_SET_AND_SYNC(cfg, cfg->others.ExternalBusType, afhds3::DirtyConfig::DC_RX_CMD_BUS_TYPE_V0));
  } else {
    auto vCfg = &cfg->v1;
    for (uint8_t i = 0; i < channel_num[vCfg->PhyMode]; i++) {
      std::string temp_str = STR_CH;
      temp_str += " " + std::to_string(i+1);
      auto line = body->newLine(grid);
      new StaticText(line, rect_t{}, temp_str);
      new PWMfrequencyChoice(line, moduleIdx, i);
      line = body->newLine(grid);

      temp_str = "PWM";
      temp_str += " ";
      temp_str += STR_SYNC;
      new StaticText(line, rect_t{}, temp_str);
      new ToggleSwitch(
          line, rect_t{}, GET_DEFAULT((vCfg->PWMFrequenciesV1.Synchronized&1<<i)>>i),
          [=](uint8_t newVal) {
            vCfg->PWMFrequenciesV1.Synchronized &= ~(1<<i);
            vCfg->PWMFrequenciesV1.Synchronized |= (newVal?1:0)<<i;
            DIRTY_CMD(cfg, afhds3::DirtyConfig::DC_RX_CMD_FREQUENCY_V1);
          });
    }

    for (uint8_t i = 0; i < SES_NPT_NB_MAX_PORTS; i++) {
      auto line = body->newLine(grid);
      std::string portName = "NP";
      portName += 'A' + i;
      new StaticText(line, rect_t{}, portName.c_str());
      new Choice(line, rect_t{}, _v1_bus_types, afhds3::SES_NPT_PWM,
                 afhds3::SES_NPT_IBUS1_OUT,
                 GET_DEFAULT(vCfg->NewPortTypes[i]),
                 [=](int32_t newValue) {
                  if(!newValue)
                  {
                    vCfg->NewPortTypes[i] = newValue;
                    DIRTY_CMD(cfg, afhds3::DirtyConfig::DC_RX_CMD_PORT_TYPE_V1);                    
                  }
                  else {
                    uint8_t j = 0;
                    for ( j = 0; j < SES_NPT_NB_MAX_PORTS; j++) {
                      if ( vCfg->NewPortTypes[j]== newValue && i != j )
                        break;
                    }
                    //The RX does not support two or more ports to output IBUS (the same is true for PPM and SBUS).
                    if(j==SES_NPT_NB_MAX_PORTS )
                    {
                      vCfg->NewPortTypes[i] = newValue;
                      DIRTY_CMD(cfg, afhds3::DirtyConfig::DC_RX_CMD_PORT_TYPE_V1);
                    }
                  }
      });
    }
  }
  auto line = body->newLine(grid);
  new StaticText(line, rect_t{}, STR_SIGNAL_OUTPUT);
  std::vector<std::string> signed_strength_ch;
  signed_strength_ch.emplace_back(STR_OFF);
  for (int i = 0; i < channel_num[cfg->v1.PhyMode]; i++) {
    std::string temstr = STR_CH;
    temstr += " " + std::to_string(i + 1);
    signed_strength_ch.emplace_back(temstr);
  }
  new Choice(line, rect_t{}, signed_strength_ch,
               0, channel_num[cfg->v1.PhyMode],
               [=] { return cfg->v1.SignalStrengthRCChannelNb==0xff?0:cfg->v1.SignalStrengthRCChannelNb+1; },
               [=](int32_t newValue) {
                  newValue?cfg->v1.SignalStrengthRCChannelNb = newValue-1:cfg->v1.SignalStrengthRCChannelNb=0xff;
                  DIRTY_CMD(cfg, afhds3::DirtyConfig::DC_RX_CMD_RSSI_CHANNEL_SETUP);
               });
}
