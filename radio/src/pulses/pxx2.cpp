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

#include <stdio.h>
#include "edgetx.h"
#include "io/frsky_firmware_update.h"
#include "lib_file.h"
#include "mixer_scheduler.h"
#include "heartbeat_driver.h"
#include "timers_driver.h"

#include "pxx2.h"
#include "pxx2_transport.h"

static const etx_serial_init pxx2SerialInitParams = {
    .baudrate = PXX2_HIGHSPEED_BAUDRATE,
    .encoding = ETX_Encoding_8N1,
    .direction = ETX_Dir_TX_RX,
    .polarity = ETX_Pol_Normal,
};

#include "hal/module_port.h"

/* Module options order:
 * - External antenna (0x01)
 * - Power (0x02)
 * - Spektrum analyser (0x04)
 * - Power meter (0x08)
 */
static const uint8_t PXX2ModuleOptions[] = {
#if defined(SIMU)
  0b11111111, // None = display all options on SIMU
#else
  0b00000000, // None = no option available on unknown modules
#endif
  0b00000001, // XJT
  0b00000001, // ISRM
  0b00001101, // ISRM-PRO
  0b00000101, // ISRM-S
  0b00000010, // R9M
  0b00000010, // R9MLite
  0b00000110, // R9MLite-PRO
  0b00000100, // ISRM-N
  0b00000100, // ISRM-S-X9
  0b00000101, // ISRM-S-X10E
  0b00000001, // XJT_LITE
  0b00000101, // ISRM-S-X10S
  0b00000100, // ISRM-X9LITES
};

uint8_t getPXX2ModuleOptions(uint8_t modelId)
{
  if (modelId < DIM(PXX2ModuleOptions))
    return PXX2ModuleOptions[modelId];
  else
    return PXX2ModuleOptions[0];
}

bool isPXX2ModuleOptionAvailable(uint8_t modelId, uint8_t option)
{
  return getPXX2ModuleOptions(modelId) & (1 << option);
}

/* Receiver options order:
 * - 2.4G (0x01)
 * - 900M (0x02)
 * - OTA to update self (0x04)
 * - OTA to update sensors (0x08)
 * - Tandem Receiver (0x10)
 * - Twin Receiver (0x20)
 * - Dynamic Telemetry Port (0x40)
 */
static const uint8_t PXX2ReceiverOptions[] = {
#if defined(SIMU)
    0b11111111, // None = display all options on SIMU
#else
    0b00000000, // None
#endif
    0b00000001, // X8R
    0b00000001, // RX8R
    0b00000001, // RX8R-PRO
    0b00001001, // RX6R, flash sensors by OTA
    0b00001001, // RX4R, flash sensors by OTA
    0b00001001, // G-RX8, flash sensors by OTA
    0b00001001, // G-RX6, flash sensors by OTA
    0b00000001, // X6R
    0b00000001, // X4R
    0b00000001, // X4R-SB
    0b00000001, // XSR
    0b00000001, // XSR-M
    0b00001001, // RXSR, flash sensors by OTA
    0b00000001, // S6R
    0b00000001, // S8R
    0b00000001, // XM
    0b00000001, // XM+
    0b00000001, // XMR
    0b00000010, // R9
    0b00000010, // R9-SLIM
    0b00000010, // R9-SLIM+
    0b00000010, // R9-MINI
    0b00000010, // R9-MM
    0b00001110, // R9-STAB+OTA, flash self + sensors by OTA
    0b00001110, // R9-MINI+OTA, flash self + sensors by OTA
    0b00001110, // R9-MM+OTA, flash self + sensors by OTA
    0b00001110, // R9-SLIM+OTA, flash self + sensors by OTA
    0b00001101, // ARCHER-X, flash self + sensors by OTA
    0b00001110, // R9MX, flash self + sensors by OTA
    0b00001110, // R9SX, flash self + sensors by OTA
    0b00010100, // TDMX, flash self by OTA
    0b01010100, // TDR18, flash self by OTA
    0b01010100, // TDR10, flash self by OTA
    0b00010100, // TDR6, flash self by OTA
    0b01010100, // TDR12
    0b01010100, // TDSR12
    0b01010100, // TDSR18
    0b00010100, // 0x26
    0b00010100,
    0b00010100,
    0b00010100,
    0b00010100, // 0x2A
    0b00010100,
    0b00010100,
    0b00010100,
    0b00010100,
    0b00010100, // 0x2F
    0b00010100, // reserve 0x23 ~ 0x30 for TD Receivers
    0b00100100, // TWMX, flash self by OTA
    0b01100100, // TWSR12, flash self by OTA
    0b01100100, // TWR12, flash self by OTA
    0b00100100, // TWGR6, flash self by OTA
    0b00100100, // TWGR8
    0b00100100, // TWR6
    0b00100100, // TWR8
    0b00100100, // TWGR6FB
    0b00100100, // TWR6FB
    0b00100100, // 0x3A
    0b00100100,
    0b00100100,
    0b00100100,
    0b00100100,
    0b00100100, // reserve 0x38 ~ 0x3F for TW Receivers
    0b01001101, // SR10-plus
    0b01001101, // R10-plus
    0b00001101, // GR8-plus
    0b00001101, // R8-plus
    0b00001101, // SR8-plus
    0b00001101, // GR6-plus
    0b00001101, // R6-plus
    0b00001101, // R6M (ESC DC)
    0b00001101, // RS-plus
    0b00001101, // RS Mini
    0b00001101, // R6FB
    0b00001101, // GR6FB
    0b01001101, // SR12-plus
    0b01001101, // R12-plus
    0b00001101, // R6 Mini E
    0b00001101, // SR6 Mini
    0b00001101, // SR6 Mini E
};

uint8_t getPXX2ReceiverOptions(uint8_t modelId)
{
  if (modelId < DIM(PXX2ReceiverOptions))
    return PXX2ReceiverOptions[modelId];
  else
    return PXX2ReceiverOptions[0];
}

bool isPXX2ReceiverOptionAvailable(uint8_t modelId, uint8_t option)
{
  return getPXX2ReceiverOptions(modelId) & (1 << option);
}

bool isPXX2PowerAvailable(const PXX2HardwareInformation& info, int value)
{
  uint8_t modelId = info.modelID;
  uint8_t variant = info.variant;

  if (modelId == PXX2_MODULE_R9M_LITE) {
    if (variant == PXX2_VARIANT_EU)
      return (value == 14 /* 25 mW with telemetry */ ||
              value == 20 /* 100 mW without telemetry */);
    else
      return value == 20; /* 100 mW */
  }
  else if (modelId == PXX2_MODULE_R9M || modelId == PXX2_MODULE_R9M_LITE_PRO) {
      if (variant == PXX2_VARIANT_EU)
        return (value == 14 /* 25 mW */ ||
                value == 23 /* 200 mW */ ||
                value == 27 /* 500 mW */);
      else
        return (value == 10 /* 10 mW */ ||
                value == 20 /* 100 mW */ ||
                value == 27 /* 500 mW */ ||
                value == 30 /* 1000 mW */);
  }
  else {
    // other modules do not have the power option
    return false;
  }
}

PXX2ModuleSetup& getPXX2ModuleSetupBuffer()
{
  return reusableBuffer.moduleSetup.pxx2;
}

BindInformation& getPXX2BindInformationBuffer()
{
  return reusableBuffer.moduleSetup.bindInformation;
}

PXX2HardwareAndSettings& getPXX2HardwareAndSettingsBuffer()
{
  return reusableBuffer.hardwareAndSettings;
}

uint8_t Pxx2Pulses::addFlag0(uint8_t module)
{
  uint8_t flag0 = g_model.header.modelId[module] & 0x3F;
  if (g_model.moduleData[module].failsafeMode != FAILSAFE_NOT_SET && g_model.moduleData[module].failsafeMode != FAILSAFE_RECEIVER) {
    if (moduleState[module].counter == 0) {
      flag0 |= PXX2_CHANNELS_FLAG0_FAILSAFE;
    }
  }
  if (moduleState[module].mode == MODULE_MODE_RANGECHECK) {
    flag0 |= PXX2_CHANNELS_FLAG0_RANGECHECK;
  }
  Pxx2Transport::addByte(flag0);
  return flag0;
}

void Pxx2Pulses::addFlag1(uint8_t module)
{
  uint8_t subType = 0;
  if (isModuleXJT(module)) {
    static const uint8_t PXX2_XJT_MODULE_SUBTYPES[] = {0x01, 0x03, 0x02};
    subType = PXX2_XJT_MODULE_SUBTYPES[min<uint8_t>(g_model.moduleData[module].subType, 2)];
  }
  else if (isModuleISRM(module)) {
    subType = g_model.moduleData[module].subType;
  }

  uint8_t flag1 = subType << 4u;
  if (isRacingModeEnabled() && isFunctionActive(FUNCTION_RACING_MODE)) {
    flag1 |= PXX2_CHANNELS_FLAG1_RACING_MODE;
  }

  Pxx2Transport::addByte(flag1);
}

void Pxx2Pulses::addPulsesValues(uint16_t low, uint16_t high)
{
  Pxx2Transport::addByte(low); // Low byte of channel
  Pxx2Transport::addByte(((low >> 8u) & 0x0Fu) | (high << 4u));  // 4 bits each from 2 channels
  Pxx2Transport::addByte(high >> 4u);  // High byte of channel
}

void Pxx2Pulses::addChannels(uint8_t module, int16_t* channels, uint8_t nChannels)
{
  uint16_t pulseValue = 0;
  uint16_t pulseValueLow = 0;

  uint8_t channel = g_model.moduleData[module].channelsStart;
  uint8_t count = sentModuleChannels(module);

  for (int8_t i = 0; i < count; i++, channel++) {
    int value = channels[i] + 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;
    pulseValue = limit(1, (value * 512 / 682) + 1024, 2046);
#if defined(DEBUG_LATENCY_RF_ONLY)
    if (latencyToggleSwitch)
      pulseValue = 1;
    else
      pulseValue = 2046;
#endif
    if (i & 1)
      addPulsesValues(pulseValueLow, pulseValue);
    else
      pulseValueLow = pulseValue;
  }
}

void Pxx2Pulses::addFailsafe(uint8_t module)
{
  uint16_t pulseValue = 0;
  uint16_t pulseValueLow = 0;

  uint8_t channel = g_model.moduleData[module].channelsStart;
  uint8_t count = sentModuleChannels(module);

  for (int8_t i = 0; i < count; i++, channel++) {
    if (g_model.moduleData[module].failsafeMode == FAILSAFE_HOLD) {
      pulseValue = 2047;
    }
    else if (g_model.moduleData[module].failsafeMode == FAILSAFE_NOPULSES) {
      pulseValue = 0;
    }
    else {
      int16_t failsafeValue = g_model.failsafeChannels[channel];
      if (failsafeValue == FAILSAFE_CHANNEL_HOLD) {
        pulseValue = 2047;
      }
      else if (failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
        pulseValue = 0;
      }
      else {
        failsafeValue += 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;
        pulseValue = limit(1, (failsafeValue * 512 / 682) + 1024, 2046);
      }
    }
    if (i & 1)
      addPulsesValues(pulseValueLow, pulseValue);
    else
      pulseValueLow = pulseValue;
  }
}

void Pxx2Pulses::setupChannelsFrame(uint8_t module, int16_t* channels, uint8_t nChannels)
{
  addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_CHANNELS);

  // Flag0
  uint8_t flag0 = addFlag0(module);

  // Flag1
  addFlag1(module);

  // Failsafe / Channels
  if (flag0 & PXX2_CHANNELS_FLAG0_FAILSAFE)
    addFailsafe(module);
  else
    addChannels(module, channels, nChannels);
}

void Pxx2Pulses::setupTelemetryFrame(uint8_t module)
{
  addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_TELEMETRY);
  Pxx2Transport::addByte(outputTelemetryBuffer.destination & 0x03);
  for (uint8_t i = 0; i < sizeof(SportTelemetryPacket); i++) {
    Pxx2Transport::addByte(outputTelemetryBuffer.data[i]);
  }
}

void Pxx2Pulses::setupHardwareInfoFrame(uint8_t module, int16_t* channels, uint8_t nChannels)
{
  ModuleInformation * destination = moduleState[module].moduleInformation;

  if (destination->timeout == 0) {
    if (destination->current <= destination->maximum) {
      addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_HW_INFO);
      Pxx2Transport::addByte(destination->current);
      destination->timeout = 60; /* 300ms */
      destination->current++;
    }
    else {
      moduleState[module].mode = MODULE_MODE_NORMAL;
      setupChannelsFrame(module, channels, nChannels);
    }
  }
  else {
    destination->timeout--;
    setupChannelsFrame(module, channels, nChannels);
  }
}

void Pxx2Pulses::setupRegisterFrame(uint8_t module)
{
  addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_REGISTER);

  auto& modSetup = getPXX2ModuleSetupBuffer();
  if (modSetup.registerStep == REGISTER_RX_NAME_SELECTED) {
    Pxx2Transport::addByte(0x01);
    for (uint8_t i=0; i<PXX2_LEN_RX_NAME; i++) {
      Pxx2Transport::addByte(modSetup.registerRxName[i]);
    }
    for (uint8_t i=0; i<PXX2_LEN_REGISTRATION_ID; i++) {
      Pxx2Transport::addByte(g_model.modelRegistrationID[i]);
    }
    Pxx2Transport::addByte(modSetup.registerLoopIndex);
  }
  else {
    Pxx2Transport::addByte(0);
  }

#if defined(SIMU)
  if (modSetup.registerStep == REGISTER_INIT) {
    memcpy(modSetup.registerRxName, "SimuRx   ", PXX2_LEN_RX_NAME);
    modSetup.registerStep = REGISTER_RX_NAME_RECEIVED;
  } else if (modSetup.registerStep == REGISTER_RX_NAME_SELECTED) {
    modSetup.registerStep = REGISTER_OK;
    moduleState[module].mode = MODULE_MODE_NORMAL;
  }
#endif
}

void Pxx2Pulses::setupBindFrame(uint8_t module)
{
  if ((g_model.moduleData[module].type == MODULE_TYPE_ISRM_PXX2 &&
       g_model.moduleData[module].subType !=
       MODULE_SUBTYPE_ISRM_PXX2_ACCESS) ||
      (g_model.moduleData[module].type == MODULE_TYPE_XJT_LITE_PXX2)) {
    setupAccstBindFrame(module);
  } else {
    setupAccessBindFrame(module);
  }  
}

void Pxx2Pulses::setupModuleSettingsFrame(uint8_t module, int16_t* channels, uint8_t nChannels)
{
  ModuleSettings * destination = moduleState[module].moduleSettings;

  if (get_tmr10ms() > destination->timeout) {
    addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_TX_SETTINGS);
    uint8_t flag0 = 0;
    if (destination->state == PXX2_SETTINGS_WRITE)
      flag0 |= PXX2_TX_SETTINGS_FLAG0_WRITE;
    Pxx2Transport::addByte(flag0);
    if (destination->state == PXX2_SETTINGS_WRITE) {
      uint8_t flag1 = 0;
      if (destination->externalAntenna)
        flag1 |= PXX2_TX_SETTINGS_FLAG1_EXTERNAL_ANTENNA;
      Pxx2Transport::addByte(flag1);
      Pxx2Transport::addByte(destination->txPower);
    }
    destination->timeout = get_tmr10ms() + 200/*next try in 2s*/;
  }
  else {
    setupChannelsFrame(module, channels, nChannels);
  }
}

void Pxx2Pulses::setupReceiverSettingsFrame(uint8_t module, int16_t* channels, uint8_t nChannels)
{
  if (get_tmr10ms() > reusableBuffer.hardwareAndSettings.receiverSettings.timeout) {
    addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_RX_SETTINGS);
    uint8_t flag0 = reusableBuffer.hardwareAndSettings.receiverSettings.receiverId;
    if (reusableBuffer.hardwareAndSettings.receiverSettings.state == PXX2_SETTINGS_WRITE)
      flag0 |= PXX2_RX_SETTINGS_FLAG0_WRITE;
    Pxx2Transport::addByte(flag0);
    if (reusableBuffer.hardwareAndSettings.receiverSettings.state == PXX2_SETTINGS_WRITE) {
      uint8_t flag1 = 0;
      if (reusableBuffer.hardwareAndSettings.receiverSettings.telemetryDisabled)
        flag1 |= PXX2_RX_SETTINGS_FLAG1_TELEMETRY_DISABLED;
      if (reusableBuffer.hardwareAndSettings.receiverSettings.pwmRate)
        flag1 |= PXX2_RX_SETTINGS_FLAG1_FASTPWM;
      if (reusableBuffer.hardwareAndSettings.receiverSettings.fport)
        flag1 |= PXX2_RX_SETTINGS_FLAG1_FPORT;
      if (reusableBuffer.hardwareAndSettings.receiverSettings.telemetry25mw)
        flag1 |= PXX2_RX_SETTINGS_FLAG1_TELEMETRY_25MW;
      if (reusableBuffer.hardwareAndSettings.receiverSettings.enablePwmCh5Ch6)
        flag1 |= PXX2_RX_SETTINGS_FLAG1_ENABLE_PWM_CH5_CH6;
      if (reusableBuffer.hardwareAndSettings.receiverSettings.fport2)
        flag1 |= PXX2_RX_SETTINGS_FLAG1_FPORT2;
      if (reusableBuffer.hardwareAndSettings.receiverSettings.sbus24)
        flag1 |= PXX2_RX_SETTINGS_FLAG1_SBUS24;
      Pxx2Transport::addByte(flag1);
      uint8_t outputsCount = min<uint8_t>(PXX2_MAX_CHANNELS, reusableBuffer.hardwareAndSettings.receiverSettings.outputsCount);
      for (int i = 0; i < outputsCount; i++) {
        Pxx2Transport::addByte(reusableBuffer.hardwareAndSettings.receiverSettings.outputsMapping[i]);
      }
    }
    reusableBuffer.hardwareAndSettings.receiverSettings.timeout = get_tmr10ms() + 200/*next try in 2s*/;
  }
  else {
    setupChannelsFrame(module, channels, nChannels);
  }
}

void Pxx2Pulses::setupAccstBindFrame(uint8_t module)
{
  addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_BIND);
  Pxx2Transport::addByte(0x01); // DATA0
  for (uint8_t i=0; i<PXX2_LEN_RX_NAME; i++) {
    Pxx2Transport::addByte(0x00);
  }
  Pxx2Transport::addByte((g_model.moduleData[module].pxx.receiverHigherChannels << 7) + (g_model.moduleData[module].pxx.receiverTelemetryOff << 6));
  Pxx2Transport::addByte(g_model.header.modelId[module]);
}

void Pxx2Pulses::setupAccessBindFrame(uint8_t module)
{
  BindInformation * destination = moduleState[module].bindInformation;

  if (destination->step == BIND_WAIT) {
    if (get_tmr10ms() > destination->timeout) {
      destination->step = BIND_OK;
      moduleState[module].mode = MODULE_MODE_NORMAL;
#if !defined(COLORLCD)
      POPUP_INFORMATION(STR_BIND_OK); // TODO rather use the new callback
#endif
    }
    return;
  }

  addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_BIND);

  if (destination->step == BIND_INFO_REQUEST) {
    Pxx2Transport::addByte(0x02); // DATA0
    for (uint8_t i=0; i<PXX2_LEN_RX_NAME; i++) {
      Pxx2Transport::addByte(destination->candidateReceiversNames[destination->selectedReceiverIndex][i]);
    }
  }
  else if (destination->step == BIND_START) {
    Pxx2Transport::addByte(0x01); // DATA0
    for (uint8_t i=0; i<PXX2_LEN_RX_NAME; i++) {
      Pxx2Transport::addByte(destination->candidateReceiversNames[destination->selectedReceiverIndex][i]);
    }
    if (isModuleR9MAccess(module)) {
      Pxx2Transport::addByte((destination->lbtMode << 6) + (destination->flexMode << 4) + destination->rxUid); // RX_UID is the slot index (which is unique and never moved)
    }
    else {
      Pxx2Transport::addByte(destination->rxUid); // RX_UID is the slot index (which is unique and never moved)
    }
    Pxx2Transport::addByte(g_model.header.modelId[module]);
  }
  else {
    Pxx2Transport::addByte(0x00); // DATA0
    for (uint8_t i=0; i<PXX2_LEN_REGISTRATION_ID; i++) {
      Pxx2Transport::addByte(g_model.modelRegistrationID[i]);
    }
  }

#if defined(SIMU)
  if (moduleState[module].mode == MODULE_MODE_BIND) {
    destination->candidateReceiversCount = 2;
    strcpy(destination->candidateReceiversNames[0], "SimuRX1");
    strcpy(destination->candidateReceiversNames[1], "SimuRX2");
  }
#endif
}

void Pxx2Pulses::setupResetFrame(uint8_t module)
{
  addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_RESET);
  Pxx2Transport::addByte(reusableBuffer.moduleSetup.pxx2.resetReceiverIndex);
  Pxx2Transport::addByte(reusableBuffer.moduleSetup.pxx2.resetReceiverFlags);
  moduleState[module].mode = MODULE_MODE_NORMAL;
}

void Pxx2Pulses::setupSpectrumAnalyser(uint8_t module)
{
  if (reusableBuffer.spectrumAnalyser.dirty) {
    reusableBuffer.spectrumAnalyser.dirty = false;
#if defined(PCBHORUS)
    memclear(&reusableBuffer.spectrumAnalyser.max, sizeof(reusableBuffer.spectrumAnalyser.max));
#endif
    addFrameType(PXX2_TYPE_C_POWER_METER, PXX2_TYPE_ID_SPECTRUM);
    Pxx2Transport::addByte(0x00);
    Pxx2Transport::addWord(reusableBuffer.spectrumAnalyser.freq);
    Pxx2Transport::addWord(reusableBuffer.spectrumAnalyser.span);
    Pxx2Transport::addWord(reusableBuffer.spectrumAnalyser.step);
  }
}

void Pxx2Pulses::setupPowerMeter(uint8_t module)
{
  if (reusableBuffer.powerMeter.dirty) {
    reusableBuffer.powerMeter.dirty = false;
    addFrameType(PXX2_TYPE_C_POWER_METER, PXX2_TYPE_ID_POWER_METER);
    Pxx2Transport::addByte(0x00);
    Pxx2Transport::addWord(reusableBuffer.powerMeter.freq);
  }
}

void Pxx2Pulses::setupShareMode(uint8_t module)
{
  addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_SHARE);
  Pxx2Transport::addByte(reusableBuffer.moduleSetup.pxx2.shareReceiverIndex);
}

// TODO: move up
static void _send_frame(etx_module_state_t* mod_st, uint8_t* buffer, uint32_t len);

void Pxx2Pulses::sendOtaUpdate(uint8_t module, const char* rxName,
                               uint32_t address, const char* data)
{
  addFrameType(PXX2_TYPE_C_OTA, PXX2_TYPE_ID_OTA);

  if (rxName) {
    Pxx2Transport::addByte(0x00);
    for (uint8_t i=0; i<PXX2_LEN_RX_NAME; i++) {
      Pxx2Transport::addByte(rxName[i]);
    }
  }
  else if (data) {
    Pxx2Transport::addByte(0x01);
    Pxx2Transport::addWord(address);
    for (uint8_t i=0; i<32; i++) {
      Pxx2Transport::addByte(data[i]);
    }
  }
  else {
    Pxx2Transport::addByte(0x02);
  }

  endFrame();
}

void Pxx2Pulses::setupAuthenticationFrame(uint8_t module, uint8_t mode, const uint8_t * outputMessage)
{
  // initFrame();

  addFrameType(PXX2_TYPE_C_MODULE, PXX2_TYPE_ID_AUTHENTICATION);

  Pxx2Transport::addByte(mode);
  if (outputMessage) {
    for (uint8_t i = 0; i < 16; i++) {
      Pxx2Transport::addByte(outputMessage[i]);
    }
  }

  endFrame();
}

bool Pxx2Pulses::setupFrame(uint8_t module, int16_t* channels, uint8_t nChannels)
{
  if (moduleState[module].mode == MODULE_MODE_OTA_UPDATE)
    return false;

  if (moduleState[module].mode == MODULE_MODE_AUTHENTICATION) {
    moduleState[module].mode = MODULE_MODE_NORMAL;
    return false;
  }

  // initFrame();

  switch (moduleState[module].mode) {
    case MODULE_MODE_GET_HARDWARE_INFO:
      setupHardwareInfoFrame(module, channels, nChannels);
      break;
    case MODULE_MODE_MODULE_SETTINGS:
      setupModuleSettingsFrame(module, channels, nChannels);
      break;
    case MODULE_MODE_RECEIVER_SETTINGS:
      setupReceiverSettingsFrame(module, channels, nChannels);
      break;
    case MODULE_MODE_REGISTER:
      setupRegisterFrame(module);
      break;
    case MODULE_MODE_BIND:
      setupBindFrame(module);
      break;
    case MODULE_MODE_RESET:
      setupResetFrame(module);
      break;
    case MODULE_MODE_SPECTRUM_ANALYSER:
      setupSpectrumAnalyser(module);
      break;
    case MODULE_MODE_POWER_METER:
      setupPowerMeter(module);
      break;
    case MODULE_MODE_SHARE:
      setupShareMode(module);
      break;
    default:
      if (outputTelemetryBuffer.isModuleDestination(module)) {
        setupTelemetryFrame(module);
        outputTelemetryBuffer.reset();
      }
      else {
        setupChannelsFrame(module, channels, nChannels);
      }
      break;
  }

  if (moduleState[module].counter-- == 0) {
    moduleState[module].counter = 2500;
  }

  endFrame();

  return true;
}

static void* pxx2Init(uint8_t module)
{
  etx_module_state_t* mod_st = nullptr;
  etx_serial_init params(pxx2SerialInitParams);

  if (module == INTERNAL_MODULE) {

    params.baudrate = PXX2_HIGHSPEED_BAUDRATE;
    mod_st = modulePortInitSerial(module, ETX_MOD_PORT_UART, &params, false);
    if (!mod_st) return nullptr;
    resetAccessAuthenticationCount();
  }

  if (module == EXTERNAL_MODULE) {

    uint8_t type = g_model.moduleData[module].type;
    switch(type) {

    case MODULE_TYPE_R9M_LITE_PXX2:
      params.baudrate = PXX2_LOWSPEED_BAUDRATE;
      break;

    case MODULE_TYPE_ISRM_PXX2:
    case MODULE_TYPE_R9M_PXX2:
    case MODULE_TYPE_XJT_LITE_PXX2:
    case MODULE_TYPE_R9M_LITE_PRO_PXX2:
      params.baudrate = PXX2_HIGHSPEED_BAUDRATE;
      break;

    default:
      return nullptr;
    }

    mod_st = modulePortInitSerial(module, ETX_MOD_PORT_UART, &params, false);
    if (!mod_st) return nullptr;
  }

  mixerSchedulerSetPeriod(module, PXX2_DEFAULT_PERIOD);
  return mod_st;
}

static void pxx2DeInit(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;
  modulePortDeInit(mod_st);
}

static void _send_frame(etx_module_state_t* mod_st, uint8_t* buffer, uint32_t len)
{
  auto drv = modulePortGetSerialDrv(mod_st->tx);
  auto ctx = modulePortGetCtx(mod_st->tx);
  drv->sendBuffer(ctx, buffer, len);
}

static void pxx2SendPulses(void* ctx, uint8_t* buffer, int16_t* channels, uint8_t nChannels)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);

  if (module == INTERNAL_MODULE) {
    Pxx2Pulses pxx2(buffer);
    bool should_send = pxx2.setupFrame(module, channels, nChannels);

    if (moduleState[module].mode == MODULE_MODE_SPECTRUM_ANALYSER ||
        moduleState[module].mode == MODULE_MODE_POWER_METER) {

      // backup normal period into 'user_data'
      if (!mod_st->user_data) {
        mod_st->user_data = (void*)(uintptr_t)mixerSchedulerGetPeriod(module);
      }
      mixerSchedulerSetPeriod(module, PXX2_TOOLS_PERIOD);
    } else if (mod_st->user_data) {
      // restore old scheduling period
      uintptr_t period = (uintptr_t)mod_st->user_data;
      mixerSchedulerSetPeriod(module, period);
      mod_st->user_data = nullptr;
    }

    if (!should_send) return;
    _send_frame(mod_st, buffer, pxx2.getSize());
  }

  if (module == EXTERNAL_MODULE) {
    Pxx2Pulses pxx2(buffer);
    pxx2.setupFrame(module, channels, nChannels);
    _send_frame(mod_st, buffer, pxx2.getSize());
  }
}

static void pxx2ProcessData(void* ctx, uint8_t data, uint8_t* buffer, uint8_t* len)
{
  if (*len == 0 && data != START_STOP) {
    return;
  }

  if (*len == 1 && data > 40) {
    *len = 0;
    return;
  }

  if (*len < TELEMETRY_RX_PACKET_SIZE) {
    buffer[(*len)++] = data;
  }
  else {
    TRACE("[PXX2] array size %d error", *len);
    *len = 0;
  }

  // no size yet
  if (*len < 2) return;

  uint8_t frame_len = buffer[1];

  /* 1 byte start + 1 byte len + 2 bytes CRC = 4 */
  if (*len < unsigned(frame_len + 4)) {
    return;
  }

  uint16_t crc = 0xFFFF;
  uint8_t* frame = buffer + 1;

  for (uint32_t i = 1; i <= frame_len; i++) {
    crc -= frame[i];
  }

  uint8_t crcHigh = frame[frame_len + 1];
  uint8_t crcLow = frame[frame_len + 2];
  
  if (crc != (crcHigh << 8 | crcLow)) {
    TRACE("[PXX2] crc error [%02x/%02x]", crc, crcHigh << 8 | crcLow);
    *len = 0;
    return;
  }

  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);
  auto drv = modulePortGetSerialDrv(mod_st->rx);
  auto drv_ctx = modulePortGetCtx(mod_st->rx);
  processPXX2Frame(module, frame, drv, drv_ctx);
  *len = 0;
}

#include "hal/module_driver.h"
// #include "extmodule_serial_driver.h"

const etx_proto_driver_t Pxx2Driver = {
  .protocol = PROTOCOL_CHANNELS_PXX2,
  .init = pxx2Init,
  .deinit = pxx2DeInit,
  .sendPulses = pxx2SendPulses,
  .processData = pxx2ProcessData,
  .processFrame = nullptr,
  .onConfigChange = nullptr,
};
