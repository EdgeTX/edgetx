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
#include "opentx.h"
#include "pulses/pxx2.h"
#include "io/frsky_firmware_update.h"
#include "libopenui/src/libopenui_file.h"
#include "mixer_scheduler.h"
#include "heartbeat_driver.h"

#if defined(INTMODULE_USART)
#include "intmodule_serial_driver.h"

const etx_serial_init pxx2SerialInitParams = {
    .baudrate = PXX2_HIGHSPEED_BAUDRATE,
    .parity = ETX_Parity_None,
    .stop_bits = ETX_StopBits_One,
    .word_length = ETX_WordLength_8,
    .rx_enable = true,
    .rx_dma_buf = nullptr,
    .rx_dma_buf_len = 0,
    .on_receive = intmoduleFifoReceive,
    .on_error = intmoduleFifoError,
};
#endif

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
  uint8_t subType;
  if (isModuleXJT(module)) {
    static const uint8_t PXX2_XJT_MODULE_SUBTYPES[] = {0x01, 0x03, 0x02};
    subType = PXX2_XJT_MODULE_SUBTYPES[min<uint8_t>(g_model.moduleData[module].subType, 2)];
  }
  else {
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

  if (reusableBuffer.moduleSetup.pxx2.registerStep == REGISTER_RX_NAME_SELECTED) {
    Pxx2Transport::addByte(0x01);
    for (uint8_t i=0; i<PXX2_LEN_RX_NAME; i++) {
      Pxx2Transport::addByte(reusableBuffer.moduleSetup.pxx2.registerRxName[i]);
    }
    for (uint8_t i=0; i<PXX2_LEN_REGISTRATION_ID; i++) {
      Pxx2Transport::addByte(g_model.modelRegistrationID[i]);
    }
    Pxx2Transport::addByte(reusableBuffer.moduleSetup.pxx2.registerLoopIndex);
  }
  else {
    Pxx2Transport::addByte(0);
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
      Pxx2Transport::addByte(flag1);
      uint8_t outputsCount = min<uint8_t>(24, reusableBuffer.hardwareAndSettings.receiverSettings.outputsCount);
      for (int i = 0; i < outputsCount; i++) {
        Pxx2Transport::addByte(min<uint8_t>(23, reusableBuffer.hardwareAndSettings.receiverSettings.outputsMapping[i]));
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

void Pxx2Pulses::sendOtaUpdate(uint8_t module, const char * rxName, uint32_t address, const char * data)
{
  initFrame();

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

#if defined(HARDWARE_INTERNAL_MODULE)
  if (module == INTERNAL_MODULE) {
    intmoduleSendNextFrame();
  }
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
  if (module == EXTERNAL_MODULE) {
    extmoduleSendNextFrame();
  }
#endif
}

void Pxx2Pulses::setupAuthenticationFrame(uint8_t module, uint8_t mode, const uint8_t * outputMessage)
{
  initFrame();

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

  initFrame();

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
      if ((g_model.moduleData[module].type == MODULE_TYPE_ISRM_PXX2 &&
           g_model.moduleData[module].subType !=
               MODULE_SUBTYPE_ISRM_PXX2_ACCESS) ||
          (g_model.moduleData[module].type == MODULE_TYPE_XJT_LITE_PXX2)) {
        setupAccstBindFrame(module);
      } else {
        setupAccessBindFrame(module);
      }
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

bool Pxx2OtaUpdate::waitStep(uint8_t step, uint8_t timeout)
{
  OtaUpdateInformation * destination = moduleState[module].otaUpdateInformation;
  uint8_t elapsed = 0;

  watchdogSuspend(100 /*1s*/);

  while (step != destination->step) {
    if (elapsed++ > timeout) {
      return false;
    }
    RTOS_WAIT_MS(1);
    telemetryWakeup();
  }

  return true;
}

const char * Pxx2OtaUpdate::nextStep(uint8_t step, const char * rxName, uint32_t address, const uint8_t * buffer)
{
  OtaUpdateInformation * destination = moduleState[module].otaUpdateInformation;

  destination->step = step;
  destination->address = address;

  for (uint8_t retry = 0;; retry++) {
    if (module == EXTERNAL_MODULE) {
      extmodulePulsesData.pxx2.sendOtaUpdate(module, rxName, address, (const char *) buffer);
    }
    else if (module == INTERNAL_MODULE) {
      intmodulePulsesData.pxx2.sendOtaUpdate(module, rxName, address, (const char *) buffer);
    }
    if (waitStep(step + 1, 20)) {
      return nullptr;
    }
    else if (retry == 100) {
      return "Transfer failed";
    }
  }
}

const char * Pxx2OtaUpdate::doFlashFirmware(const char * filename, ProgressHandler progressHandler)
{
  FIL file;
  uint8_t buffer[32];
  UINT count;
  const char * result;

  result = nextStep(OTA_UPDATE_START, rxName, 0, nullptr);
  if (result) {
    return result;
  }

  if (f_open(&file, filename, FA_READ) != FR_OK) {
    return "Open file failed";
  }

  uint32_t size;
  const char * ext = getFileExtension(filename);
  if (ext && !strcasecmp(ext, FRSKY_FIRMWARE_EXT)) {
    FrSkyFirmwareInformation * information = (FrSkyFirmwareInformation *) buffer;
    if (f_read(&file, buffer, sizeof(FrSkyFirmwareInformation), &count) != FR_OK || count != sizeof(FrSkyFirmwareInformation)) {
      f_close(&file);
      return "Format error";
    }
    size = information->size;
  }
  else {
    size = f_size(&file);
  }

  uint32_t done = 0;
  while (1) {
    progressHandler(getBasename(filename), STR_OTA_UPDATE, done, size);
    if (f_read(&file, buffer, sizeof(buffer), &count) != FR_OK) {
      f_close(&file);
      return "Read file failed";
    }

    result = nextStep(OTA_UPDATE_TRANSFER, nullptr, done, buffer);
    if (result) {
      return result;
    }

    if (count < sizeof(buffer)) {
      f_close(&file);
      break;
    }

    done += count;
  }

  return nextStep(OTA_UPDATE_EOF, nullptr, done, nullptr);
}

void Pxx2OtaUpdate::flashFirmware(const char * filename, ProgressHandler progressHandler)
{
  pausePulses();

  watchdogSuspend(100 /*1s*/);
  RTOS_WAIT_MS(100);

  moduleState[module].mode = MODULE_MODE_OTA_UPDATE;
  const char * result = doFlashFirmware(filename, progressHandler);
  moduleState[module].mode = MODULE_MODE_NORMAL;

  AUDIO_PLAY(AU_SPECIAL_SOUND_BEEP1 );
  BACKLIGHT_ENABLE();

  if (result) {
    POPUP_WARNING(STR_FIRMWARE_UPDATE_ERROR, result);
  }
  else {
    POPUP_INFORMATION(STR_FIRMWARE_UPDATE_SUCCESS);
  }

  watchdogSuspend(100);
  RTOS_WAIT_MS(100);

  resumePulses();
}

static void* pxx2InitInternal(uint8_t moduleType)
{
  intmoduleFifo.clear();
  IntmoduleSerialDriver.init(&pxx2SerialInitParams);
  resetAccessAuthenticationCount();

#if defined(INTMODULE_HEARTBEAT)
  // use backup trigger (1 ms later)
  init_intmodule_heartbeat();
#endif
  mixerSchedulerSetPeriod(INTERNAL_MODULE, PXX2_PERIOD);
  INTERNAL_MODULE_ON();

  return nullptr;
}

static void pxx2DeInitInternal(void* context)
{
  INTERNAL_MODULE_OFF();
  IntmoduleSerialDriver.deinit();
#if defined(INTMODULE_HEARTBEAT)
  stop_intmodule_heartbeat();
#endif
  mixerSchedulerSetPeriod(INTERNAL_MODULE, 0);
}

static bool pxx2InternalSendNextFrame = true;

static void pxx2SetupPulsesInternal(void* context, int16_t* channels, uint8_t nChannels)
{
  (void)context;

  pxx2InternalSendNextFrame =
      intmodulePulsesData.pxx2.setupFrame(INTERNAL_MODULE, channels, nChannels);

  if (moduleState[INTERNAL_MODULE].mode == MODULE_MODE_SPECTRUM_ANALYSER ||
      moduleState[INTERNAL_MODULE].mode == MODULE_MODE_POWER_METER) {
    mixerSchedulerSetPeriod(INTERNAL_MODULE, PXX2_TOOLS_PERIOD);
  } else {
    mixerSchedulerSetPeriod(INTERNAL_MODULE, PXX2_PERIOD);
  }
}

static void pxx2SendPulsesInternal(void* context)
{
  (void)context;

  if (pxx2InternalSendNextFrame) {
    IntmoduleSerialDriver.sendBuffer(intmodulePulsesData.pxx2.getData(),
                                     intmodulePulsesData.pxx2.getSize());
  }
}

#include "hal/module_driver.h"

const etx_module_driver_t Pxx2InternalDriver = {
  .protocol = PROTOCOL_CHANNELS_PXX2_HIGHSPEED,
  .init = pxx2InitInternal,
  .deinit = pxx2DeInitInternal,
  .setupPulses = pxx2SetupPulsesInternal,
  .sendPulses = pxx2SendPulsesInternal,
};

#if defined(EXTMODULE_USART)
#include "extmodule_serial_driver.h"

static const etx_serial_init pxx2ExtSerialInitParams = {
  .baudrate = 0,
  .parity = ETX_Parity_None,
  .stop_bits = ETX_StopBits_One,
  .word_length = ETX_WordLength_8,
  .rx_enable = true,
  .rx_dma_buf = nullptr,
  .rx_dma_buf_len = 0,
  .on_receive = extmoduleFifoReceive,
  .on_error = extmoduleFifoError,
};

static void* pxx2InitLowSpeed(uint8_t module)
{
  etx_serial_init params(pxx2ExtSerialInitParams);
  params.baudrate = PXX2_LOWSPEED_BAUDRATE;
  ExtmoduleSerialDriver.init(&params);

  mixerSchedulerSetPeriod(EXTERNAL_MODULE, PXX2_NO_HEARTBEAT_PERIOD);
  EXTERNAL_MODULE_ON();

  return nullptr;
}

static void* pxx2InitExternal(uint8_t module)
{
  etx_serial_init params(pxx2ExtSerialInitParams);
  params.baudrate = PXX2_HIGHSPEED_BAUDRATE;
  ExtmoduleSerialDriver.init(&params);

  mixerSchedulerSetPeriod(EXTERNAL_MODULE, PXX2_NO_HEARTBEAT_PERIOD);
  EXTERNAL_MODULE_ON();

  return nullptr;
}

static void pxx2DeInitExternal(void* context)
{
  (void)context;

  EXTERNAL_MODULE_OFF();
  mixerSchedulerSetPeriod(EXTERNAL_MODULE, 0);
  ExtmoduleSerialDriver.deinit();
}

static void pxx2SetupPulsesExternal(void* context, int16_t* channels, uint8_t nChannels)
{
  (void)context;
  extmodulePulsesData.pxx2.setupFrame(EXTERNAL_MODULE, channels, nChannels);
}

static void pxx2SendPulsesExternal(void* context)
{
  (void)context;
  ExtmoduleSerialDriver.sendBuffer(extmodulePulsesData.pxx2.getData(),
                                   extmodulePulsesData.pxx2.getSize());
}

const etx_module_driver_t Pxx2ExternalDriver = {
  .protocol = PROTOCOL_CHANNELS_PXX2_HIGHSPEED,
  .init = pxx2InitExternal,
  .deinit = pxx2DeInitExternal,
  .setupPulses = pxx2SetupPulsesExternal,
  .sendPulses = pxx2SendPulsesExternal,
};

const etx_module_driver_t Pxx2LowSpeedExternalDriver = {
  .protocol = PROTOCOL_CHANNELS_PXX2_LOWSPEED,
  .init = pxx2InitLowSpeed,
  .deinit = pxx2DeInitExternal,
  .setupPulses = pxx2SetupPulsesExternal,
  .sendPulses = pxx2SendPulsesExternal,
};
#endif
