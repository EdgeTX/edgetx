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

#include "dataconstants.h"
#include "hal/module_port.h"
#include "heartbeat_driver.h"
#include "mixer_scheduler.h"

#include "pxx1_transport.h"
#include "pxx1.h"

#include "edgetx.h"

static uint32_t _pxx1_internal_baudrate = PXX1_DEFAULT_SERIAL_BAUDRATE;

void pxx1SetInternalBaudrate(uint32_t baudrate)
{
  _pxx1_internal_baudrate = baudrate;
}

template <class PxxTransport>
void Pxx1Pulses<PxxTransport>::addFlag1(uint8_t module, uint8_t sendFailsafe)
{
  uint8_t flag1 = (g_model.moduleData[module].subType << 6);

  if (moduleState[module].mode == MODULE_MODE_BIND) {
    flag1 |= (g_eeGeneral.countryCode << 1) | PXX_SEND_BIND;
  }
  else if (moduleState[module].mode == MODULE_MODE_RANGECHECK) {
    flag1 |= PXX_SEND_RANGECHECK;
  }

  if (sendFailsafe) {
    flag1 |= PXX_SEND_FAILSAFE;
  }

  PxxTransport::addByte(flag1);
}

template <class PxxTransport>
void Pxx1Pulses<PxxTransport>::addExtraFlags(uint8_t module)
{
  // Ext. flag (holds antenna selection on Horus internal module, 0x00 otherwise)
  uint8_t extraFlags = 0;

#if defined(EXTERNAL_ANTENNA)
  if (module == INTERNAL_MODULE && isExternalAntennaEnabled()) {
    extraFlags |= (1 << 0);
  }
#endif

  extraFlags |= (g_model.moduleData[module].pxx.receiverTelemetryOff << 1);
  extraFlags |= (g_model.moduleData[module].pxx.receiverHigherChannels << 2);
  if (isModuleR9MNonAccess(module)) {
    extraFlags |= (min<uint8_t>(g_model.moduleData[module].pxx.power,
                                isModuleR9M_FCC_VARIANT(module)
                                    ? (uint8_t)R9M_FCC_POWER_MAX
                                    : (uint8_t)R9M_LBT_POWER_MAX)
                   << 3);
    if (isModuleR9M_EUPLUS(module)) extraFlags |= (1 << 6);
  }

#if defined(HARDWARE_EXTERNAL_MODULE) && defined(HARDWARE_INTERNAL_MODULE)
  // Disable S.PORT if port is not used (might be used by the internal module)
  if (module == EXTERNAL_MODULE && !modulePortIsPortUsedByModule(module, ETX_MOD_PORT_SPORT)) {
    extraFlags |= (1 << 5);
  }
#endif

  PxxTransport::addByte(extraFlags);
}

template <class PxxTransport>
void Pxx1Pulses<PxxTransport>::addChannels(uint8_t moduleIdx, uint8_t sendFailsafe, uint8_t sendUpperChannels)
{
  uint16_t pulseValue = 0;
  uint16_t pulseValueLow = 0;

  for (uint8_t i = 0; i < 8; i++) {
    if (sendFailsafe) {
      if (g_model.moduleData[moduleIdx].failsafeMode == FAILSAFE_HOLD) {
        pulseValue = (i < sendUpperChannels ? 4095 : 2047);
      }
      else if (g_model.moduleData[moduleIdx].failsafeMode == FAILSAFE_NOPULSES) {
        pulseValue = (i < sendUpperChannels ? 2048 : 0);
      }
      else {
        if (i < sendUpperChannels) {
          int16_t failsafeValue = g_model.failsafeChannels[8+i];
          if (failsafeValue == FAILSAFE_CHANNEL_HOLD) {
            pulseValue = 4095;
          }
          else if (failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
            pulseValue = 2048;
          }
          else {
            failsafeValue += 2*PPM_CH_CENTER(8+g_model.moduleData[moduleIdx].channelsStart+i) - 2*PPM_CENTER;
            pulseValue = limit(2049, (failsafeValue * 512 / 682) + 3072, 4094);
          }
        }
        else {
          int16_t failsafeValue = g_model.failsafeChannels[i];
          if (failsafeValue == FAILSAFE_CHANNEL_HOLD) {
            pulseValue = 2047;
          }
          else if (failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
            pulseValue = 0;
          }
          else {
            failsafeValue += 2*PPM_CH_CENTER(g_model.moduleData[moduleIdx].channelsStart+i) - 2*PPM_CENTER;
            pulseValue = limit(1, (failsafeValue * 512 / 682) + 1024, 2046);
          }
        }
      }
    }
    else {
      if (i < sendUpperChannels) {
        int channel = 8 + g_model.moduleData[moduleIdx].channelsStart + i;
        int value = channelOutputs[channel] + 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;
        pulseValue = limit(2049, (value * 512 / 682) + 3072, 4094);
      }
      else if (i < sentModulePXXChannels(moduleIdx)) {
        int channel = g_model.moduleData[moduleIdx].channelsStart + i;
        int value = channelOutputs[channel] + 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;
        pulseValue = limit(1, (value * 512 / 682) + 1024, 2046);
      }
      else {
        pulseValue = 1024;
      }
    }

    if (i & 1) {
      PxxTransport::addByte(pulseValueLow); // Low byte of channel
      PxxTransport::addByte(((pulseValueLow >> 8) & 0x0F) | (pulseValue << 4));  // 4 bits each from 2 channels
      PxxTransport::addByte(pulseValue >> 4);  // High byte of channel
    }
    else {
      pulseValueLow = pulseValue;
    }
  }
}

template <class PxxTransport>
void Pxx1Pulses<PxxTransport>::add8ChannelsFrame(uint8_t module, uint8_t sendUpperChannels, uint8_t sendFailsafe)
{
  PxxTransport::initCrc();

  // Sync
  addHead();

  // RX Number
  PxxTransport::addByte(g_model.header.modelId[module]);

  // Flag1
  addFlag1(module, sendFailsafe);

  // Flag2
  PxxTransport::addByte(0);

  // Channels
  addChannels(module, sendFailsafe, sendUpperChannels);

  // Extra flags
  addExtraFlags(module);

  // CRC
  addCrc();

  // Sync = HEAD
  addHead();

  // Tail
  PxxTransport::addTail();
}

template <class PxxTransport>
Pxx1Pulses<PxxTransport>::Pxx1Pulses(uint8_t* buffer)
  : PxxTransport(buffer)
{
}

template <class PxxTransport>
void Pxx1Pulses<PxxTransport>::setupFrame(uint8_t module, Pxx1Type type)
{
  uint8_t sendUpperChannels = 0;
  uint8_t sendFailsafe = 0;


  // Fast serial PXX1 (either X-lite internal or R9M-lite):
  // - up to 16 channels sent in the same frame
  //
  if (type == Pxx1Type::FAST_SERIAL) {
    if (moduleState[module].counter-- == 0) {
      sendFailsafe = (g_model.moduleData[module].failsafeMode != FAILSAFE_NOT_SET &&
                      g_model.moduleData[module].failsafeMode != FAILSAFE_RECEIVER);
      moduleState[module].counter = 1000;
    }
    add8ChannelsFrame(module, 0, sendFailsafe);
    if (sentModulePXXChannels(module) > 8) {
      add8ChannelsFrame(module, 8, sendFailsafe);
    }
    return;
  }

  // Slow PXX1:
  // - if more than 8 channels shall be sent, it happens
  //   in alternating frames
  //
  if (moduleState[module].counter & 0x01) {
    // channelsCount is shifted by 8
    sendUpperChannels = g_model.moduleData[module].channelsCount;
    // if real channels count > 8
    if (sendUpperChannels && moduleState[module].counter == 1) {
      sendFailsafe =
          (g_model.moduleData[module].failsafeMode != FAILSAFE_NOT_SET &&
           g_model.moduleData[module].failsafeMode != FAILSAFE_RECEIVER);
    }
  } else {
    if (moduleState[module].counter == 0) {
      sendFailsafe = (g_model.moduleData[module].failsafeMode != FAILSAFE_NOT_SET &&
                      g_model.moduleData[module].failsafeMode != FAILSAFE_RECEIVER);
    }
  }

  add8ChannelsFrame(module, sendUpperChannels, sendFailsafe);

  if (moduleState[module].counter-- == 0) {
    moduleState[module].counter = 999;
  }
}

template class Pxx1Pulses<StandardPxx1Transport>;
template class Pxx1Pulses<UartPxx1Transport>;

static const etx_serial_init pxx1SerialCfg = {
  .baudrate = 0,
  .encoding = ETX_Encoding_8N1,
  .direction = ETX_Dir_TX,
  .polarity = ETX_Pol_Normal,
};

static void pxx1SportSensorPolling(void* param)
{
  if (outputTelemetryBuffer.destination != TELEMETRY_ENDPOINT_SPORT)
    return;

  auto mod_st = (etx_module_state_t*)param;
  auto drv = modulePortGetSerialDrv(mod_st->rx);
  auto ctx = modulePortGetCtx(mod_st->rx);

  // Match sensor polling from the module
  // -> detect <0x7E [Physical ID]> as the last sequence
  uint8_t b;
  if (drv->getBufferedBytes(ctx) != 2 ||
      drv->getLastByte(ctx, 2, &b) <= 0 || b != START_STOP ||
      drv->getLastByte(ctx, 1, &b) <= 0 ||
      b != outputTelemetryBuffer.sport.physicalId)
    return;

  drv->sendBuffer(ctx, outputTelemetryBuffer.data + 1,
                  outputTelemetryBuffer.size - 1);

  outputTelemetryBuffer.reset();
}

static void* pxx1Init(uint8_t module)
{
  etx_module_state_t* mod_st = nullptr;
  etx_serial_init txCfg(pxx1SerialCfg);

  if (module == INTERNAL_MODULE) {

    if (!pxxClearSPort()) return nullptr;
    
    txCfg.baudrate = _pxx1_internal_baudrate;
    mod_st = modulePortInitSerial(module, ETX_MOD_PORT_UART, &txCfg, false);

    if (!mod_st) {
      // assume that radios that don't have an internal UART
      // will have a module that uses legacy PXX1 (PWM)
      txCfg.encoding = ETX_Encoding_PXX1_PWM;
      mod_st = modulePortInitSerial(module, ETX_MOD_PORT_SOFT_INV, &txCfg, false);
    }

    if (!mod_st) return nullptr;
  }

  if (module == EXTERNAL_MODULE) {

    // Init driver (timer / serial) based on module type
    uint8_t type = g_model.moduleData[module].type;
    switch(type) {

    case MODULE_TYPE_R9M_LITE_PXX1: {
      txCfg.baudrate = EXTMODULE_PXX1_SERIAL_BAUDRATE;
      mod_st = modulePortInitSerial(module, ETX_MOD_PORT_UART, &txCfg, false);
      if (!mod_st) return nullptr;
    } break;

    case MODULE_TYPE_XJT_PXX1:
    case MODULE_TYPE_R9M_PXX1: {
      txCfg.encoding = ETX_Encoding_PXX1_PWM;
      mod_st = modulePortInitSerial(module, ETX_MOD_PORT_SOFT_INV, &txCfg, false);
      if (!mod_st) return nullptr;
    } break;

    default:
      // unknown module: bail out!
      return nullptr;
    }
  }

  // Init telemetry RX
  etx_serial_init rxCfg(pxx1SerialCfg);
  rxCfg.baudrate = FRSKY_SPORT_BAUDRATE;
  rxCfg.direction = ETX_Dir_TX_RX;

  if (modulePortInitSerial(module, ETX_MOD_PORT_SPORT, &rxCfg, false) != nullptr) {
    auto drv = modulePortGetSerialDrv(mod_st->rx);
    auto ctx = modulePortGetCtx(mod_st->rx);
    if (drv && ctx && drv->setIdleCb) {
      drv->setIdleCb(ctx, pxx1SportSensorPolling, mod_st);
    }
  }

  // Store PXX1 type in 'user_data'
  if (txCfg.encoding == ETX_Encoding_PXX1_PWM ||
      txCfg.baudrate == PXX1_DEFAULT_SERIAL_BAUDRATE) {

    // Legacy / slow PXX1
    mixerSchedulerSetPeriod(module, PXX1_DEFAULT_PERIOD);
    if (txCfg.encoding == ETX_Encoding_PXX1_PWM) {
      mod_st->user_data = (void*)Pxx1Type::PWM;
    } else {
      mod_st->user_data = (void*)Pxx1Type::SLOW_SERIAL;
    }
  } else {
    // Fast PXX1
    mixerSchedulerSetPeriod(module, PXX1_FAST_PERIOD);
    mod_st->user_data = (void*)Pxx1Type::FAST_SERIAL;
  }

  return mod_st;
}

static void pxx1DeInit(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);
  modulePortDeInit(mod_st);

  if (module == INTERNAL_MODULE) {
    pulsesRestartModuleUnsafe(EXTERNAL_MODULE);
  }
}

static void pxx1SendPulses(void* ctx, uint8_t* buffer, int16_t* channels, uint8_t nChannels)
{
  // TODO:
  (void)channels;
  (void)nChannels;

  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);
  auto pxx1_type = (Pxx1Type)(uintptr_t)mod_st->user_data;

  uint32_t frame_len = 0;
  if (pxx1_type == Pxx1Type::PWM) {
    PwmPxx1Pulses frame(buffer);
    frame.setupFrame(module, pxx1_type);
    frame_len = frame.getSize();
  } else {
    UartPxx1Pulses frame(buffer);
    frame.setupFrame(module, pxx1_type);
    frame_len = frame.getSize();
  }

  if (frame_len == 0) return;

  auto drv = modulePortGetSerialDrv(mod_st->tx);
  auto drv_ctx = modulePortGetCtx(mod_st->tx);
  drv->sendBuffer(drv_ctx, buffer, frame_len);
}

static void pxx1ProcessData(void* ctx, uint8_t data, uint8_t* buffer, uint8_t* len)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);

  processFrskySportTelemetryData(module, data, buffer, len);
}

const etx_proto_driver_t Pxx1Driver = {
  .protocol = PROTOCOL_CHANNELS_PXX1,
  .init = pxx1Init,
  .deinit = pxx1DeInit,
  .sendPulses = pxx1SendPulses,
  .processData = pxx1ProcessData,
  .processFrame = nullptr,
  .onConfigChange = nullptr,
};
