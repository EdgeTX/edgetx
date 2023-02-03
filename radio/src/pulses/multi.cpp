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

#include "opentx.h"
#include "multi.h"

#include "io/multi_protolist.h"
#include "telemetry/multi.h"
#include "mixer_scheduler.h"

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

static void sendFrameProtocolHeader(uint8_t moduleIdx, bool failsafe);
void sendChannels(uint8_t moduleIdx);
static void sendD16BindOption(uint8_t moduleIdx);
#if defined(LUA)
static void sendSport(uint8_t moduleIdx);
static void sendHott(uint8_t moduleIdx);
static void sendConfig(uint8_t moduleIdx);
static void sendDSM(uint8_t moduleIdx);
#endif

#if defined(INTMODULE_USART) && defined(INTERNAL_MODULE_MULTI)
#include "intmodule_serial_driver.h"

etx_serial_init multiSerialInitParams = {
    .baudrate = MULTIMODULE_BAUDRATE,
    .parity = ETX_Parity_Even,
    .stop_bits = ETX_StopBits_Two,
    .word_length = ETX_WordLength_9,
    .rx_enable = true,
};
#endif

static void sendMulti(uint8_t moduleIdx, uint8_t b)
{
#if defined(INTERNAL_MODULE_MULTI)
  if (moduleIdx == INTERNAL_MODULE) {
    intmodulePulsesData.multi.sendByte(b);
  }
  else
#endif
    sendByteSbus(b);
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

static void sendFailsafeChannels(uint8_t moduleIdx)
{
  uint32_t bits = 0;
  uint8_t bitsavailable = 0;

  for (int i = 0; i < MULTI_CHANS; i++) {
    int16_t failsafeValue = g_model.failsafeChannels[i];
    int pulseValue;

    if (g_model.moduleData[moduleIdx].failsafeMode == FAILSAFE_HOLD ||
        failsafeValue == FAILSAFE_CHANNEL_HOLD) {
      pulseValue = 2047;
    } else if (g_model.moduleData[moduleIdx].failsafeMode ==
                   FAILSAFE_NOPULSES ||
               failsafeValue == FAILSAFE_CHANNEL_NOPULSE) {
      pulseValue = 0;
    } else {
      failsafeValue +=
          2 * PPM_CH_CENTER(g_model.moduleData[moduleIdx].channelsStart + i) -
          2 * PPM_CENTER;
      pulseValue = limit(1, (failsafeValue * 800 / 1000) + 1024, 2046);
    }

    bits |= pulseValue << bitsavailable;
    bitsavailable += MULTI_CHAN_BITS;
    while (bitsavailable >= 8) {
      sendMulti(moduleIdx, (uint8_t) (bits & 0xff));
      bits >>= 8;
      bitsavailable -= 8;
    }
  }
}

void setupPulsesMulti(uint8_t moduleIdx)
{
  static int counter[2] = {0,0}; //TODO
  static uint8_t invert[2] = {0x00,        //internal
#if defined(PCBTARANIS) || defined(PCBHORUS) || defined(PCBNV14)
    0x08        //external
#else
    0x00	//external
#endif
  };
  uint8_t type=MULTI_NORMAL;

  updateMultiSync(moduleIdx);

  // not scanning protos &&  not spectrum analyser
  if (getModuleMode(moduleIdx) == MODULE_MODE_NORMAL) {
    // Failsafe packets
    if (counter[moduleIdx] % 1000 == 0 &&
        g_model.moduleData[moduleIdx].failsafeMode != FAILSAFE_NOT_SET &&
        g_model.moduleData[moduleIdx].failsafeMode != FAILSAFE_RECEIVER) {
      type |= MULTI_FAILSAFE;
    }

    counter[moduleIdx]++;
  }

  // Invert telemetry if needed
  if (invert[moduleIdx] & 0x80 &&
      !g_model.moduleData[moduleIdx].multi.disableTelemetry) {
    if (getMultiModuleStatus(moduleIdx).isValid()) {
      invert[moduleIdx] &= 0x08;  // Telemetry received, stop searching
    } else if (counter[moduleIdx] % 100 == 0) {
      invert[moduleIdx] ^= 0x08;  // Try inverting telemetry
    }
  }

  // Send header
  sendFrameProtocolHeader(moduleIdx, type & MULTI_FAILSAFE);

  // Send channels
  if (type & MULTI_FAILSAFE)
    sendFailsafeChannels(moduleIdx);
  else
    sendChannels(moduleIdx);

  // Multi V1.3.X.X -> Send byte 26, Protocol (bits 7 & 6), RX_Num (bits 5 & 4), invert, not used, disable telemetry, disable mapping
  if ((moduleState[moduleIdx].mode == MODULE_MODE_SPECTRUM_ANALYSER)
#if defined(MULTI_PROTOLIST)
      || (moduleState[moduleIdx].mode == MODULE_MODE_GET_HARDWARE_INFO)
#endif
      ) {
    sendMulti(moduleIdx, invert[moduleIdx] & 0x08);
  }
  else {
    sendMulti(moduleIdx, (uint8_t) (((g_model.moduleData[moduleIdx].multi.rfProtocol + 3) & 0xC0)
                                    | (g_model.header.modelId[moduleIdx] & 0x30)
                                    | (invert[moduleIdx] & 0x08)
                                    //| 0x04 // Future use
                                    | (g_model.moduleData[moduleIdx].multi.disableTelemetry << 1)
                                    | g_model.moduleData[moduleIdx].multi.disableMapping));
  }

  // Multi V1.3.X.X -> Send protocol additional data: max 9 bytes
  if (getMultiModuleStatus(moduleIdx).isValid()) {
    MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);
    if (status.minor >= 3 && !(status.flags & 0x80)) { //Version 1.3.x.x or more and Buffer not full
      if ((IS_D16_MULTI(moduleIdx) || IS_R9_MULTI(moduleIdx)) && moduleState[moduleIdx].mode == MODULE_MODE_BIND) {
        sendD16BindOption(moduleIdx);//1 byte of additional data
      }
#if defined(LUA)
      // SPort send
      if (IS_D16_MULTI(moduleIdx) && outputTelemetryBuffer.destination == TELEMETRY_ENDPOINT_SPORT && outputTelemetryBuffer.size) {
        sendSport(moduleIdx);       //8 bytes of additional data
      }
      else if (IS_HOTT_MULTI(moduleIdx)) {
        sendHott(moduleIdx);        //1 byte of additional data
      }
      else if (IS_CONFIG_MULTI(moduleIdx)) {
        sendConfig(moduleIdx);      //7 bytes of additional data
      }
      else if (IS_DSM_MULTI(moduleIdx)) {
        sendDSM(moduleIdx);         //7 bytes of additional data
      }
#endif
    }
  }
}

void setupPulsesMultiExternalModule()
{
  extmodulePulsesData.dsm2.index = 0;
  extmodulePulsesData.dsm2.ptr = extmodulePulsesData.dsm2.pulses;

  setupPulsesMulti(EXTERNAL_MODULE);
  putDsm2Flush();
}

#if defined(INTERNAL_MODULE_MULTI)
static void* multiInit(uint8_t module)
{
  (void)module;
  
  // serial port setup
  intmodulePulsesData.multi.initFrame();
  void* uart_ctx = IntmoduleSerialDriver.init(&multiSerialInitParams);

  // mixer setup
  mixerSchedulerSetPeriod(INTERNAL_MODULE, MULTIMODULE_PERIOD);
  INTERNAL_MODULE_ON();

  // reset status
  getMultiModuleStatus(INTERNAL_MODULE).failsafeChecked = false;
  getMultiModuleStatus(INTERNAL_MODULE).flags = 0;

#if defined(MULTI_PROTOLIST)
  TRACE("enablePulsesInternalModule(): trigger scan");
  MultiRfProtocols::instance(INTERNAL_MODULE)->triggerScan();
  TRACE("counter = %d", moduleState[INTERNAL_MODULE].counter);
#endif

  return uart_ctx;
}

static void multiDeInit(void* context)
{
  INTERNAL_MODULE_OFF();
  mixerSchedulerSetPeriod(INTERNAL_MODULE, 0);
  IntmoduleSerialDriver.deinit(context);
}

static void multiSetupPulses(void* context, int16_t* channels, uint8_t nChannels)
{
  (void)context;
  // TODO:
  (void)channels;
  (void)nChannels;
  
  intmodulePulsesData.multi.initFrame();
  setupPulsesMulti(INTERNAL_MODULE);
}

static void multiSendPulses(void* context)
{
  IntmoduleSerialDriver.sendBuffer(context, intmodulePulsesData.multi.getData(),
                                   intmodulePulsesData.multi.getSize());
}

static int multiGetByte(void* context, uint8_t* data)
{
  return IntmoduleSerialDriver.getByte(context, data);
}

static void multiProcessData(void* context, uint8_t data, uint8_t* buffer, uint8_t* len)
{
  processMultiTelemetryData(data, INTERNAL_MODULE);
}

#include "hal/module_driver.h"

const etx_module_driver_t MultiInternalDriver = {
  .protocol = PROTOCOL_CHANNELS_MULTIMODULE,
  .init = multiInit,
  .deinit = multiDeInit,
  .setupPulses = multiSetupPulses,
  .sendPulses = multiSendPulses,
  .getByte = multiGetByte,
  .processData = multiProcessData,
};
#endif

void sendChannels(uint8_t moduleIdx)
{
  uint32_t bits = 0;
  uint8_t bitsavailable = 0;

  // byte 4-25, channels 0..2047
  // Range for pulses (channelsOutputs) is [-1024:+1024] for [-100%;100%]
  // Multi uses [204;1843] as [-100%;100%]
  for (int i = 0; i < MULTI_CHANS; i++) {
    int channel = g_model.moduleData[moduleIdx].channelsStart + i;
    int value = channelOutputs[channel] + 2 * PPM_CH_CENTER(channel) - 2 * PPM_CENTER;

    // Scale to 80%
    value = value * 800 / 1000 + 1024;
    value = limit(0, value, 2047);

    bits |= value << bitsavailable;
    bitsavailable += MULTI_CHAN_BITS;
    while (bitsavailable >= 8) {
      sendMulti(moduleIdx, (uint8_t) (bits & 0xff));
      bits >>= 8;
      bitsavailable -= 8;
    }
  }
}

void convertMultiProtocolToEtx(int *protocol, int *subprotocol)
{
  if (*protocol == 3 && *subprotocol == 0) {
    *protocol = MODULE_SUBTYPE_MULTI_FRSKY + 1;
    *subprotocol = MM_RF_FRSKY_SUBTYPE_D8;
    return;
  }

  if (*protocol == 3 && *subprotocol == 1) {
    *protocol = MODULE_SUBTYPE_MULTI_FRSKY + 1;
    *subprotocol = MM_RF_FRSKY_SUBTYPE_D8_CLONED;
    return;
  }

  if (*protocol == 25) {
    *protocol = MODULE_SUBTYPE_MULTI_FRSKY + 1;
    *subprotocol = MM_RF_FRSKY_SUBTYPE_V8;
    return;
  }

  if (*protocol == 15) {
    *protocol = MODULE_SUBTYPE_MULTI_FRSKY + 1;

    if (*subprotocol == 0)
      *subprotocol = MM_RF_FRSKY_SUBTYPE_D16;
    else if (*subprotocol == 1)
      *subprotocol = MM_RF_FRSKY_SUBTYPE_D16_8CH;
    else if (*subprotocol == 2)
      *subprotocol = MM_RF_FRSKY_SUBTYPE_D16_LBT;
    else if (*subprotocol == 3)
      *subprotocol = MM_RF_FRSKY_SUBTYPE_D16_LBT_8CH;
    else if (*subprotocol == 4)
      *subprotocol = MM_RF_FRSKY_SUBTYPE_D16_CLONED;

    return;
  }

  if (*protocol >= 25)
    *protocol -= 1;

  if (*protocol >= 16)
    *protocol -= 1;
}

void convertEtxProtocolToMulti(int *protocol, int *subprotocol)
{
  // Special treatment for the FrSky entry...
  if (*protocol == MODULE_SUBTYPE_MULTI_FRSKY + 1) {
    if (*subprotocol == MM_RF_FRSKY_SUBTYPE_D8) {
      //D8
      *protocol = 3;
      *subprotocol = 0;
    } 
    else if (*subprotocol == MM_RF_FRSKY_SUBTYPE_D8_CLONED) {
      //D8
      *protocol = 3;
      *subprotocol = 1;
    } 
    else if (*subprotocol == MM_RF_FRSKY_SUBTYPE_V8) {
      //V8
      *protocol = 25;
      *subprotocol = 0;
    } 
    else {
      *protocol = 15;
      if (*subprotocol == MM_RF_FRSKY_SUBTYPE_D16_8CH)
        *subprotocol = 1;
      else if (*subprotocol == MM_RF_FRSKY_SUBTYPE_D16)
        *subprotocol = 0; // D16
      else if (*subprotocol == MM_RF_FRSKY_SUBTYPE_D16_LBT)
        *subprotocol = 2;
      else if (*subprotocol == MM_RF_FRSKY_SUBTYPE_D16_LBT_8CH)
        *subprotocol = 3;
      else
        *subprotocol = 4; // D16_CLONED
    }
  }
  else {
    // 15  for Multimodule is FrskyX or D16 which we map as a protocol of 3 (FrSky)
    // all protos > frskyx are therefore also off by one
    if (*protocol >= 15)
      *protocol += 1;
    // 25 is again a FrSky *protocol (FrskyV) so shift again
    if (*protocol >= 25)
      *protocol += 1;
  }
}

void sendFrameProtocolHeader(uint8_t moduleIdx, bool failsafe)
{// byte 1+2, protocol information

  // Our enumeration starts at 0
  int type = g_model.moduleData[moduleIdx].multi.rfProtocol + 1;
  int subtype = g_model.moduleData[moduleIdx].subType;
  int8_t optionValue = g_model.moduleData[moduleIdx].multi.optionValue;

  uint8_t protoByte = 0;

  uint8_t moduleMode = getModuleMode(moduleIdx);
  if (moduleMode == MODULE_MODE_SPECTRUM_ANALYSER) {
    sendMulti(moduleIdx, (uint8_t) 0x54);  // Header byte
    sendMulti(moduleIdx, (uint8_t) 54);    // Spectrum custom protocol
    sendMulti(moduleIdx, (uint8_t) 0);
    sendMulti(moduleIdx, (uint8_t) 0);
    return;
  }

#if defined(MULTI_PROTOLIST)
  if (moduleMode == MODULE_MODE_GET_HARDWARE_INFO) {
    sendMulti(moduleIdx, (uint8_t) 0x55); // Header byte
    sendMulti(moduleIdx, (uint8_t) 0);    // PROTOLIST custom protocol
    sendMulti(moduleIdx, (uint8_t) 0);

    // proto array item
    uint8_t protoIdx = MultiRfProtocols::instance(moduleIdx)->getScanProto();
    TRACE("scan [%d]", protoIdx);
    sendMulti(moduleIdx, protoIdx);
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
    if (g_model.moduleData[moduleIdx].multi.optionValue & 0x02)
      optionValue |= 0x40; // 11ms servo refresh
    optionValue |= sentModuleChannels(moduleIdx); //add number of channels
  }

  // Special treatment for the FrSky entry...
  convertEtxProtocolToMulti(&type, &subtype);

  // Set the highest bit of option byte in AFHDS2A protocol to instruct MULTI to passthrough telemetry bytes instead
  // of sending Frsky D telemetry
  if (g_model.moduleData[moduleIdx].multi.rfProtocol == MODULE_SUBTYPE_MULTI_FS_AFHDS2A)
    optionValue = optionValue | 0x80;

  // For custom protocol send unmodified type byte
  if (g_model.moduleData[moduleIdx].multi.rfProtocol == MM_RF_CUSTOM_SELECTED)
    type = g_model.moduleData[moduleIdx].multi.rfProtocol;

  uint8_t headerByte = 0x55;
  // header, byte 0,  0x55 for proto 0-31, 0x54 for proto 32-63
  if (type & 0x20)
    headerByte &= 0xFE;

  if (failsafe)
    headerByte |= 0x02;

  sendMulti(moduleIdx, headerByte);

  // protocol byte
  protoByte |= (type & 0x1f);
  if (g_model.moduleData[moduleIdx].multi.rfProtocol != MODULE_SUBTYPE_MULTI_DSM2)
    protoByte |= (g_model.moduleData[moduleIdx].multi.autoBindMode << 6);

  sendMulti(moduleIdx, protoByte);

  // byte 2, subtype, powermode, model id
  sendMulti(moduleIdx, (uint8_t) ((g_model.header.modelId[moduleIdx] & 0x0f)
                           | ((subtype & 0x7) << 4)
                           | (g_model.moduleData[moduleIdx].multi.lowPowerMode << 7))
  );

  // byte 3
  sendMulti(moduleIdx, (uint8_t) optionValue);
}

void sendD16BindOption(uint8_t moduleIdx)
{
  uint8_t bind_opt = g_model.moduleData[moduleIdx].multi.receiverTelemetryOff ? 1 : 0;
  bind_opt |= g_model.moduleData[moduleIdx].multi.receiverHigherChannels ? 2 : 0;
  sendMulti(moduleIdx, bind_opt);
}

#if defined(LUA)
void sendSport(uint8_t moduleIdx)
{
  // example: B7 30 30 0C 80 00 00 00 13
  uint8_t j=0;

  // unstuff and remove crc
  for (uint8_t i = 0; i < outputTelemetryBuffer.size - 1 && j < 8; i++, j++) {
    if (outputTelemetryBuffer.data[i] == BYTE_STUFF) {
      i++;
      sendMulti(moduleIdx, outputTelemetryBuffer.data[i] ^ STUFF_MASK);
    }
    else {
      sendMulti(moduleIdx, outputTelemetryBuffer.data[i]);
    }
  }

  outputTelemetryBuffer.reset(); // empty buffer
}

void sendHott(uint8_t moduleIdx)
{
  if (Multi_Buffer && memcmp(Multi_Buffer, "HoTT", 4) == 0 && (Multi_Buffer[5] & 0x80) && (Multi_Buffer[5] & 0x0F) >= 0x07) {
    // HoTT Lua script is running
    sendMulti(moduleIdx, Multi_Buffer[5]);
  }
}

void sendConfig(uint8_t moduleIdx)
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
      sendMulti(moduleIdx, Multi_Buffer[5 + i]);
    }
    Multi_Buffer[4] = 0x00;   // Send data only once
  }
}

void sendDSM(uint8_t moduleIdx)
{
  // Multi_Buffer[0..2]=="DSM" -> Lua script is running
  // Multi_Buffer[3]==0x70 + len -> TX to RX data ready to be sent
  // Multi_Buffer[4..9]=6 bytes of TX to RX data
  // Multi_Buffer[10..25]=16 bytes of RX to TX data
  if (Multi_Buffer && memcmp(Multi_Buffer, "DSM", 3) == 0 && (Multi_Buffer[3] & 0xF8) == 0x70) {
    for(uint8_t i = 0; i < 7; i++) {
        sendMulti(moduleIdx, Multi_Buffer[3+i]);
    }
    Multi_Buffer[3] = 0x00;    // Data sent
  }
}
#endif

