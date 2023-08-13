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

enum eEB_BusType {
  EB_BT_IBUS1=0,
  EB_BT_IBUS2,
  EB_BT_SBUS1
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
  uint8_t ExternalBusType; // eEB_BusType
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

union Config_u
{
  uint8_t version;
  sDATA_ConfigV0 v0;
  sDATA_ConfigV1 v1;
  uint8_t buffer[sizeof(sDATA_ConfigV1)];
};

Config_u* getConfig(uint8_t moduleIdx);
void applyModelConfig(uint8_t moduleIdx);

};
