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

#pragma once

#define AFHDS3_MAX_CHANNELS 18

namespace afhds3
{

PACK(struct sSES_PWMFrequencyV0 {
  uint16_t Frequency : 15;    // From 50 to 400Hz
  uint16_t Synchronized : 1;  // 1=Synchronize the PWM output to the RF cycle
                              // (lower latency but unstable frequency)
});

enum eSES_PA_SetAnalogOutput {
  SES_ANALOG_OUTPUT_PWM = 0,
  SES_ANALOG_OUTPUT_PPM
};

enum eEB_BusType : uint8_t {
  EB_BT_IBUS1=0,
  EB_BT_IBUS1_OUT = EB_BT_IBUS1,
  EB_BT_IBUS2,
  EB_BT_IBUS1_IN = EB_BT_IBUS2,
  EB_BT_SBUS1
};

enum BUS_DIR
{
	BUS_OUT,
	BUS_IN,
};

// 48 bytes
PACK(struct sDATA_ConfigV0 {
  uint8_t Version;     // =0
  uint8_t EMIStandard; // eLNK_EMIStandard
  uint8_t IsTwoWay;
  uint8_t PhyMode;     // eDATA_PHYMODE
  uint8_t SignalStrengthRCChannelNb; // 0xFF if not used, 0`18
  uint16_t FailsafeTimeout;  // in unit of ms
  int16_t FailSafe[AFHDS3_MAX_CHANNELS];
  uint8_t FailsafeOutputMode; //TRUE Or FALSE
  sSES_PWMFrequencyV0 PWMFrequency;
  uint8_t AnalogOutput; // eSES_PA_SetAnalogOutput
  eEB_BusType ExternalBusType; // eEB_BusType
});

#define SES_NB_MAX_CHANNELS (32)
#define SES_NPT_NB_MAX_PORTS (4)

enum eSES_NewPortType {
  SES_NPT_PWM,
  SES_NPT_PPM,
  SES_NPT_SBUS,
  SES_NPT_IBUS1_IN,
  SES_NPT_IBUS1_OUT,
  SES_NPT_IBUS2,
  SES_NPT_IBUS2_HUB_PORT,
  SES_NPT_WSTX,
  SES_NPT_WSRX,
  SES_NPT_NONE=0xFF
};

enum DirtyConfig {
  DC_RX_CMD_TX_PWR,
  DC_RX_CMD_FAILSAFE_VALUE,
  DC_RX_CMD_FAILSAFE_TIME,
  DC_RX_CMD_RSSI_CHANNEL_SETUP,
  DC_RX_CMD_RANGE,
  DC_RX_CMD_GET_CAPABILITIES,
  DC_RX_CMD_OUT_PWM_PPM_MODE,
  DC_RX_CMD_FREQUENCY_V0,
  DC_RX_CMD_PORT_TYPE_V1,
  DC_RX_CMD_FREQUENCY_V1,
  DC_RX_CMD_FREQUENCY_V1_2,
  DC_RX_CMD_BUS_TYPE_V0,
  DC_RX_CMD_BUS_TYPE_V0_2,
  DC_RX_CMD_IBUS_SETUP,
  DC_RX_CMD_BUS_DIRECTION,
  DC_RX_CMD_BUS_FAILSAFE,
  DC_RX_CMD_GET_VERSION,
  DC_RX_CMD_GET_RX_VERSION
};

#define SET_AND_SYNC(cfg, value, dirtyCmd)  [=](int32_t newValue) { value = newValue; cfg->others.dirtyFlag |= (uint32_t) 1 << dirtyCmd; }
#define GET_SET_AND_SYNC(cfg, value, dirtyCmd)  GET_DEFAULT(value), SET_AND_SYNC(cfg, value, dirtyCmd)
#define DIRTY_CMD(cfg, dirtyCmd) cfg->others.dirtyFlag |= (uint32_t) 1 << dirtyCmd;

PACK(struct sSES_PWMFrequenciesAPPV1 {
  // One unsigned short per
  // channel, From 50 to 400Hz,
  // 1:1000Hz,2:833Hz
  uint16_t PWMFrequencies[SES_NB_MAX_CHANNELS];

  // 1 bit per channel, 32 channels total
  uint32_t Synchronized;
});

// 116 bytes
PACK(struct sDATA_ConfigV1 {
  uint8_t Version;      // =1
  uint8_t EMIStandard;  // eLNK_EMIStandard
  uint8_t IsTwoWay;
  uint8_t PhyMode;                    // eDATA_PHYMODE
  uint8_t SignalStrengthRCChannelNb;  // 0xFF if not used , 0`18
  uint16_t FailsafeTimeout;           // in unit of ms
  int16_t FailSafe[AFHDS3_MAX_CHANNELS];
  uint8_t FailsafeOutputMode;                  // TRUE Or FALSE
  uint8_t NewPortTypes[SES_NPT_NB_MAX_PORTS];  // eSES_NewPortType
  sSES_PWMFrequenciesAPPV1 PWMFrequenciesV1;
});

PACK(struct sDATA_Others {
  uint8_t buffer[sizeof(sDATA_ConfigV1)];
  uint8_t ExternalBusType;  // eEB_BusType IBUS1:0;IBUS2:1(Not supported yet);SBUS:2
  tmr10ms_t lastUpdated;    // last updated time
  bool isConnected;         // specify if receiver is connected
  uint32_t dirtyFlag;       // mapped to commands that need to be issued to sync settings
});

union Config_u
{
  uint8_t version;
  sDATA_ConfigV0 v0;
  sDATA_ConfigV1 v1;
  uint8_t buffer[sizeof(sDATA_ConfigV1)];
  sDATA_Others others;
};

Config_u* getConfig(uint8_t moduleIdx);
void applyModelConfig(uint8_t moduleIdx);

};
