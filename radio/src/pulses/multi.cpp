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

#include "edgetx.h"
#include "multi.h"

#include "io/multi_protolist.h"
#include "telemetry/multi.h"
#include "mixer_scheduler.h"
#include "hal/abnormal_reboot.h"

// for the  MULTI protocol definition
// see https://github.com/pascallanger/DIY-Multiprotocol-TX-Module
// file Multiprotocol/multiprotocol.h

#define MULTI_SEND_BIND                     (1 << 7)
#define MULTI_SEND_RANGECHECK               (1 << 5)
#define MULTI_SEND_AUTOBIND                 (1 << 6)

#define MULTI_CHANS                         16
#define MULTI_CHAN_BITS                     11

#define MULTI_NORMAL   0x00
#define MULTI_FAILSAFE 0x01
#define MULTI_DATA     0x02

static void sendFrameProtocolHeader(uint8_t*& p_buf, uint8_t module, bool failsafe);
static void sendChannels(uint8_t*& p_buf, uint8_t module);
static void sendD16BindOption(uint8_t*& p_buf, uint8_t module);
#if defined(LUA)
static void sendSport(uint8_t*& p_buf, uint8_t module);
static void sendHott(uint8_t*& p_buf, uint8_t module);
static void sendConfig(uint8_t*& p_buf, uint8_t module);
static void sendDSM(uint8_t*& p_buf, uint8_t module);
#endif

#include "hal/module_port.h"

etx_serial_init const multiSerialInitParams = {
    .baudrate = MULTIMODULE_BAUDRATE,
    .encoding = ETX_Encoding_8E2,
    .direction = ETX_Dir_TX,
    .polarity = ETX_Pol_Normal,
};

static inline void sendMulti(uint8_t*& p_buf, uint8_t b)
{
  *p_buf++ = b;
}

static void updateMultiSync(uint8_t module)
{
  const auto& status = getMultiModuleStatus(module);
  if (status.isValid() && status.isRXProto) {
    mixerSchedulerSetPeriod(module, 0);
  } else {
    auto& sync = getModuleSyncStatus(module);
    if (sync.isValid())
      mixerSchedulerSetPeriod(module, sync.getAdjustedRefreshRate());
    else
      mixerSchedulerSetPeriod(module, MULTIMODULE_PERIOD);
  }
}

static void sendFailsafeChannels(uint8_t*& p_buf, uint8_t module)
{
  uint32_t bits = 0;
  uint8_t bitsavailable = 0;

  for (int i = 0; i < MULTI_CHANS; i++) {
    int16_t failsafeValue = g_model.failsafeChannels[i];
    int pulseValue;

    if (g_model.moduleData[module].failsafeMode == FAILSAFE_HOLD ||
        failsafeValue == FAILSAFE_CHANNEL_HOLD) {
      pulseValue = 2047;
    } else if (g_model.moduleData[module].failsafeMode ==
                   FAILSAFE_NOPULSES ||
               failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
      pulseValue = 0;
    } else {
      failsafeValue +=
          2 * PPM_CH_CENTER(g_model.moduleData[module].channelsStart + i) -
          2 * PPM_CENTER;
      pulseValue = limit(1, (failsafeValue * 800 / 1000) + 1024, 2046);
    }

    bits |= pulseValue << bitsavailable;
    bitsavailable += MULTI_CHAN_BITS;
    while (bitsavailable >= 8) {
      sendMulti(p_buf, (uint8_t) (bits & 0xff));
      bits >>= 8;
      bitsavailable -= 8;
    }
  }
}

static void setupPulsesMulti(uint8_t*& p_buf, uint8_t module)
{
  static int counter[2] = {0,0}; //TODO
  static uint8_t invert[2] = {
    0x00,       //internal
    0x08        //external
  };
  uint8_t type=MULTI_NORMAL;

  updateMultiSync(module);

  // not scanning protos &&  not spectrum analyser
  if (getModuleMode(module) == MODULE_MODE_NORMAL) {
    // Failsafe packets
    if (counter[module] % 1000 == 0 &&
        g_model.moduleData[module].failsafeMode != FAILSAFE_NOT_SET &&
        g_model.moduleData[module].failsafeMode != FAILSAFE_RECEIVER) {
      type |= MULTI_FAILSAFE;
    }

    counter[module]++;
  }

  // Invert telemetry if needed
  uint8_t disableTelemetry = g_model.moduleData[module].multi.disableTelemetry || (modulePortHasRx(module) ? 0 : 1);
  if (invert[module] & 0x80 && !disableTelemetry) {
    if (getMultiModuleStatus(module).isValid()) {
      invert[module] &= 0x08;  // Telemetry received, stop searching
    } else if (counter[module] % 100 == 0) {
      invert[module] ^= 0x08;  // Try inverting telemetry
    }
  }

  // Send header
  sendFrameProtocolHeader(p_buf, module, type & MULTI_FAILSAFE);

  // Send channels
  if (type & MULTI_FAILSAFE)
    sendFailsafeChannels(p_buf, module);
  else
    sendChannels(p_buf, module);

  // Multi V1.3.X.X -> Send byte 26, Protocol (bits 7 & 6), RX_Num (bits 5 & 4), invert, not used, disable telemetry, disable mapping
  if (moduleState[module].mode == MODULE_MODE_SPECTRUM_ANALYSER
#if defined(MULTI_PROTOLIST)
      || moduleState[module].mode == MODULE_MODE_GET_HARDWARE_INFO
#endif
      ) {
    sendMulti(p_buf, invert[module] & 0x08);
  }
  else {
    sendMulti(p_buf, (uint8_t) (((g_model.moduleData[module].multi.rfProtocol + 1) & 0xC0)
                                | (g_model.header.modelId[module] & 0x30)
                                | (invert[module] & 0x08)
                                //| 0x04 // Future use
                                | (disableTelemetry << 1)
                                | g_model.moduleData[module].multi.disableMapping));
  }

  // Multi V1.3.X.X -> Send protocol additional data: max 9 bytes
  if (getMultiModuleStatus(module).isValid()) {
    MultiModuleStatus &status = getMultiModuleStatus(module);
    if (status.minor >= 3 && !(status.flags & 0x80)) { //Version 1.3.x.x or more and Buffer not full
      if ((IS_D16_MULTI(module) || IS_R9_MULTI(module)) && moduleState[module].mode == MODULE_MODE_BIND) {
        sendD16BindOption(p_buf, module);//1 byte of additional data
      }
#if defined(LUA)
      // SPort send
      if (IS_D16_MULTI(module) && outputTelemetryBuffer.destination == TELEMETRY_ENDPOINT_SPORT && outputTelemetryBuffer.size) {
        sendSport(p_buf, module);       //8 bytes of additional data
      }
      else if (IS_HOTT_MULTI(module)) {
        sendHott(p_buf, module);        //1 byte of additional data
      }
      else if (IS_CONFIG_MULTI(module)) {
        sendConfig(p_buf, module);      //7 bytes of additional data
      }
      else if (IS_DSM_MULTI(module)) {
        sendDSM(p_buf, module);         //7 bytes of additional data
      }
#endif
    }
  }
}

static void* multiInit(uint8_t module)
{
  etx_module_state_t* mod_st = nullptr;
  etx_serial_init cfg(multiSerialInitParams);

#if defined(INTERNAL_MODULE_MULTI)
  if (module == INTERNAL_MODULE) {
    // serial port setup
    // TODO: error handling
    cfg.direction = ETX_Dir_TX_RX;
    mod_st = modulePortInitSerial(module, ETX_MOD_PORT_UART, &cfg, false);
  }
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
  if (module == EXTERNAL_MODULE) {
    // serial port setup
    cfg.direction = ETX_Dir_TX;
    cfg.polarity = ETX_Pol_Inverted;

    mod_st = modulePortInitSerial(module, ETX_MOD_PORT_UART, &cfg, true);
    if (!mod_st) return nullptr;

    // Init S.PORT RX channel
    cfg.direction = ETX_Dir_RX;
    cfg.polarity = ETX_Pol_Normal;
    modulePortInitSerial(module, ETX_MOD_PORT_SPORT, &cfg, false);
  }
#endif

  if (!mod_st) return nullptr;
  
  // mixer setup
  mixerSchedulerSetPeriod(module, MULTIMODULE_PERIOD);

  // reset status
  getMultiModuleStatus(module).failsafeChecked = false;
  getMultiModuleStatus(module).flags = 0;

#if defined(MULTI_PROTOLIST)
  if (!UNEXPECTED_SHUTDOWN()) {
    TRACE("enablePulsesInternalModule(): trigger scan");
    MultiRfProtocols::instance(module)->triggerScan();
  }
#endif

  return mod_st;
}

static void multiDeInit(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;
  modulePortDeInit(mod_st);
}

static void multiSendPulses(void* ctx, uint8_t* buffer, int16_t* channels, uint8_t nChannels)
{
  // TODO:
  (void)channels;
  (void)nChannels;

  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);

  auto data = buffer;
  setupPulsesMulti(data, module);

  auto drv = modulePortGetSerialDrv(mod_st->tx);
  auto drv_ctx = modulePortGetCtx(mod_st->tx);
  drv->sendBuffer(drv_ctx, buffer, data - buffer);
}

static void multiProcessData(void* ctx, uint8_t data, uint8_t* buffer, uint8_t* len)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto module = modulePortGetModule(mod_st);

  processMultiTelemetryData(data, module);
}

#include "hal/module_driver.h"

const etx_proto_driver_t MultiDriver = {
  .protocol = PROTOCOL_CHANNELS_MULTIMODULE,
  .init = multiInit,
  .deinit = multiDeInit,
  .sendPulses = multiSendPulses,
  .processData = multiProcessData,
  .processFrame = nullptr,
  .onConfigChange = nullptr,
};

static void sendChannels(uint8_t*& p_buf, uint8_t module)
{
  uint32_t bits = 0;
  uint8_t bitsavailable = 0;

  // byte 4-25, channels 0..2047
  // Range for pulses (channelsOutputs) is [-1024:+1024] for [-100%;100%]
  // Multi uses [204;1843] as [-100%;100%]
  for (int i = 0; i < MULTI_CHANS; i++) {
    int channel = g_model.moduleData[module].channelsStart + i;
    int value = channelOutputs[channel] + 2 * PPM_CH_CENTER(channel) - 2 * PPM_CENTER;

    // Scale to 80%
    value = value * 800 / 1000 + 1024;
    value = limit(0, value, 2047);

    bits |= value << bitsavailable;
    bitsavailable += MULTI_CHAN_BITS;
    while (bitsavailable >= 8) {
      sendMulti(p_buf, (uint8_t) (bits & 0xff));
      bits >>= 8;
      bitsavailable -= 8;
    }
  }
}

void sendFrameProtocolHeader(uint8_t*& p_buf, uint8_t module, bool failsafe)
{// byte 1+2, protocol information

  // Our enumeration starts at 0
  int type = g_model.moduleData[module].multi.rfProtocol + 1;
  int subtype = g_model.moduleData[module].subType;
  int8_t optionValue = g_model.moduleData[module].multi.optionValue;

  uint8_t protoByte = 0;

  uint8_t moduleMode = getModuleMode(module);
  if (moduleMode == MODULE_MODE_SPECTRUM_ANALYSER) {
    sendMulti(p_buf, (uint8_t) 0x54);  // Header byte
    sendMulti(p_buf, (uint8_t) 54);    // Spectrum custom protocol
    sendMulti(p_buf, (uint8_t) 0);
    sendMulti(p_buf, (uint8_t) 0);
    return;
  }

#if defined(MULTI_PROTOLIST)
  if (moduleMode == MODULE_MODE_GET_HARDWARE_INFO) {
    sendMulti(p_buf, (uint8_t) 0x55); // Header byte
    sendMulti(p_buf, (uint8_t) 0);    // PROTOLIST custom protocol
    sendMulti(p_buf, (uint8_t) 0);

    // proto array item
    uint8_t protoIdx = MultiRfProtocols::instance(module)->getScanProto();
    TRACE("scan [%d]", protoIdx);
    sendMulti(p_buf, protoIdx);
    return;
  }
#endif

  if (moduleMode == MODULE_MODE_BIND)
    protoByte |= MULTI_SEND_BIND;
  else if (moduleMode ==  MODULE_MODE_RANGECHECK)
    protoByte |= MULTI_SEND_RANGECHECK;

  // rfProtocol
  if (type == MODULE_SUBTYPE_MULTI_DSM2 +1 ) {
    // Multi module in DSM mode wants the number of channels to be used as option value along with other flags
    if (optionValue & 0x01)
      optionValue = 0x80; // Max throw
    else
      optionValue = 0x00;
    if (g_model.moduleData[module].multi.optionValue & 0x02)
      optionValue |= 0x40; // 11ms servo refresh
    if (g_model.moduleData[module].multi.optionValue & 0x04)
      optionValue |= 0x20; // Cloned
    optionValue |= sentModuleChannels(module); //add number of channels
  }

  // Set the highest bit of option byte in AFHDS2A protocol to instruct MULTI to passthrough telemetry bytes instead
  // of sending Frsky D telemetry
  if (g_model.moduleData[module].multi.rfProtocol == MODULE_SUBTYPE_MULTI_FS_AFHDS2A)
    optionValue = optionValue | 0x80;

  // For custom protocol send unmodified type byte
  if (g_model.moduleData[module].multi.rfProtocol == MM_RF_CUSTOM_SELECTED)
    type = g_model.moduleData[module].multi.rfProtocol;

  uint8_t headerByte = 0x55;
  // header, byte 0,  0x55 for proto 0-31, 0x54 for proto 32-63
  if (type & 0x20)
    headerByte &= 0xFE;

  if (failsafe)
    headerByte |= 0x02;

  sendMulti(p_buf, headerByte);

  // protocol byte
  protoByte |= (type & 0x1f);
  if (g_model.moduleData[module].multi.rfProtocol != MODULE_SUBTYPE_MULTI_DSM2)
    protoByte |= (g_model.moduleData[module].multi.autoBindMode << 6);

  sendMulti(p_buf, protoByte);

  // byte 2, subtype, powermode, model id
  sendMulti(p_buf, (uint8_t) ((g_model.header.modelId[module] & 0x0f)
                           | ((subtype & 0x7) << 4)
                           | (g_model.moduleData[module].multi.lowPowerMode << 7))
  );

  // byte 3
  sendMulti(p_buf, (uint8_t) optionValue);
}

void sendD16BindOption(uint8_t*& p_buf, uint8_t module)
{
  uint8_t bind_opt = g_model.moduleData[module].multi.receiverTelemetryOff ? 1 : 0;
  bind_opt |= g_model.moduleData[module].multi.receiverHigherChannels ? 2 : 0;
  sendMulti(p_buf, bind_opt);
}

#if defined(LUA)
void sendSport(uint8_t*& p_buf, uint8_t module)
{
  // example: B7 30 30 0C 80 00 00 00 13
  uint8_t j=0;

  // unstuff and remove crc
  for (uint8_t i = 0; i < outputTelemetryBuffer.size - 1 && j < 8; i++, j++) {
    if (outputTelemetryBuffer.data[i] == BYTE_STUFF) {
      i++;
      sendMulti(p_buf, outputTelemetryBuffer.data[i] ^ STUFF_MASK);
    }
    else {
      sendMulti(p_buf, outputTelemetryBuffer.data[i]);
    }
  }

  outputTelemetryBuffer.reset(); // empty buffer
}

void sendHott(uint8_t*& p_buf, uint8_t module)
{
  if (Multi_Buffer && memcmp(Multi_Buffer, "HoTT", 4) == 0 && (Multi_Buffer[5] & 0x80) && (Multi_Buffer[5] & 0x0F) >= 0x07) {
    // HoTT Lua script is running
    sendMulti(p_buf, Multi_Buffer[5]);
  }
}

void sendConfig(uint8_t*& p_buf, uint8_t module)
{
  // Multi_Buffer[0..3]=="Conf" -> Lua script is running
  // Multi_Buffer[4]==0x01 -> TX to Module data ready to be sent
  // Multi_Buffer[4]==0xFF -> Clear buffer data
  // Multi_Buffer[5..11]=7 bytes of TX to Module data
  // Multi_Buffer[12] -> Current page
  // Multi_Buffer[13..172]=8*20=160 bytes of Module to TX data
  if (Multi_Buffer && memcmp(Multi_Buffer, "Conf", 4) == 0 && Multi_Buffer[4] == 0x01) {
    // Config Lua script is running and sending
    for (uint8_t i = 0; i < 7; i++) {
      sendMulti(p_buf, Multi_Buffer[5 + i]);
    }
    Multi_Buffer[4] = 0x00;   // Send data only once
  }
}

void sendDSM(uint8_t*& p_buf, uint8_t module)
{
  // Multi_Buffer[0..2]=="DSM" -> Lua script is running
  // Multi_Buffer[3]==0x70 + len -> TX to RX data ready to be sent
  // Multi_Buffer[4..9]=6 bytes of TX to RX data
  // Multi_Buffer[10..25]=16 bytes of RX to TX data
  if (Multi_Buffer && memcmp(Multi_Buffer, "DSM", 3) == 0 && (Multi_Buffer[3] & 0xF8) == 0x70) {
    for(uint8_t i = 0; i < 7; i++) {
        sendMulti(p_buf, Multi_Buffer[3+i]);
    }
    Multi_Buffer[3] = 0x00;    // Data sent
  }
}
#endif

