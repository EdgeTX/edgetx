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
#include "aux_serial_driver.h"

#include "telemetry.h"
#include "io/multi_protolist.h"
#include "multi.h"
#include "spektrum.h"
#include "flysky_ibus.h"
#include "hitec.h"
#include "hott.h"
#include "mlink.h"

constexpr int32_t MULTI_DESIRED_VERSION = (1 << 24) | (3 << 16) | (3 << 8)  | 0;
#define MULTI_CHAN_BITS 11

extern uint8_t g_moduleIdx;

enum MultiPacketTypes : uint8_t
{
  MultiStatus = 1,
  FrSkySportTelemetry,
  FrSkyHubTelemetry,
  SpektrumTelemetry,
  DSMBindPacket,
  FlyskyIBusTelemetry,
  ConfigCommand,
  InputSync,
  FrskySportPolling,
  HitecTelemetry,
  SpectrumScannerPacket,
  FlyskyIBusTelemetryAC,
  MultiRxChannels,
  HottTelemetry,
  MLinkTelemetry,
  ConfigTelemetry,
  MultiProtoDef
};

enum MultiBufferState : uint8_t
{
  NoProtocolDetected,
  MultiFirstByteReceived,
  ReceivingMultiProtocol,
  ReceivingMultiStatus,
  SpektrumTelemetryFallback,
  FrskyTelemetryFallback,
  FrskyTelemetryFallbackFirstByte,
  FrskyTelemetryFallbackNextBytes,
  FlyskyTelemetryFallback,
  HitecTelemetryFallback,
  MultiStatusOrFrskyData
};


#if defined(INTERNAL_MODULE_MULTI)

static MultiModuleStatus multiModuleStatus[NUM_MODULES] = {MultiModuleStatus(), MultiModuleStatus()};
static uint8_t multiBindStatus[NUM_MODULES] = {MULTI_BIND_NONE, MULTI_BIND_NONE};

static MultiBufferState multiTelemetryBufferState[NUM_MODULES];
static uint16_t multiTelemetryLastRxTS[NUM_MODULES];

MultiModuleStatus &getMultiModuleStatus(uint8_t module)
{
  return multiModuleStatus[module];
}

uint8_t getMultiBindStatus(uint8_t module)
{
  return multiBindStatus[module];
}

void setMultiBindStatus(uint8_t module, uint8_t bindStatus)
{
  multiBindStatus[module] = bindStatus;
}

MultiBufferState getMultiTelemetryBufferState(uint8_t module)
{
  return multiTelemetryBufferState[module];
}

void setMultiTelemetryBufferState(uint8_t module, MultiBufferState state)
{
  multiTelemetryBufferState[module] = state;
}

static uint16_t& getMultiTelemetryLastRxTS(uint8_t module)
{
  return multiTelemetryLastRxTS[module];
}

#else // !INTERNAL_MODULE_MULTI

static MultiModuleStatus multiModuleStatus;
static uint8_t multiBindStatus = MULTI_BIND_NONE;

static MultiBufferState multiTelemetryBufferState;
static uint16_t multiTelemetryLastRxTS;

MultiModuleStatus& getMultiModuleStatus(uint8_t)
{
  return multiModuleStatus;
}

uint8_t getMultiBindStatus(uint8_t)
{
  return multiBindStatus;
}

void setMultiBindStatus(uint8_t, uint8_t bindStatus)
{
  multiBindStatus = bindStatus;
}

MultiBufferState getMultiTelemetryBufferState(uint8_t)
{
  return multiTelemetryBufferState;
}

void setMultiTelemetryBufferState(uint8_t, MultiBufferState state)
{
  multiTelemetryBufferState = state;
}

uint16_t& getMultiTelemetryLastRxTS(uint8_t module)
{
  return multiTelemetryLastRxTS;
}

#endif // INTERNAL_MODULE_MULTI

bool isMultiModeScanning(uint8_t module)
{
  return getModuleMode(module) == MODULE_MODE_GET_HARDWARE_INFO;
}

static MultiBufferState guessProtocol(uint8_t module)
{
  uint32_t moduleIdx = EXTERNAL_MODULE;
#if defined(INTERNAL_MODULE_MULTI)
  if (isModuleMultimodule(INTERNAL_MODULE)) {
    moduleIdx = INTERNAL_MODULE;
  }
#endif

  if (g_model.moduleData[moduleIdx].multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2)
    return SpektrumTelemetryFallback;
  else if (g_model.moduleData[module].multi.rfProtocol == MODULE_SUBTYPE_MULTI_FS_AFHDS2A)
    return FlyskyTelemetryFallback;
  else
    return FrskyTelemetryFallback;
}

static void processMultiScannerPacket(const uint8_t *data, const uint8_t moduleIdx)
{
  uint8_t cur_channel = data[0];
  if (moduleState[moduleIdx].mode == MODULE_MODE_SPECTRUM_ANALYSER) {
    for (uint8_t channel = 0; channel <5; channel++) {
      uint8_t power = max<int>(0,(data[channel+1] - 34) >> 1); // remove everything below -120dB

#if LCD_W == 480
      coord_t x = cur_channel*2;
      if (x < LCD_W) {
        reusableBuffer.spectrumAnalyser.bars[x] = power;
        reusableBuffer.spectrumAnalyser.bars[x+1] = power;
        if (power > reusableBuffer.spectrumAnalyser.max[x]) {
          reusableBuffer.spectrumAnalyser.max[x] = power;
          reusableBuffer.spectrumAnalyser.max[x+1] = power;
        }
#elif LCD_W == 212
        coord_t x = cur_channel;
      if (x <= LCD_W) {
        reusableBuffer.spectrumAnalyser.bars[x] = power;
        if (power > reusableBuffer.spectrumAnalyser.max[x]) {
          reusableBuffer.spectrumAnalyser.max[x] = power;
        }
#else
      coord_t x = cur_channel/2 + 1;
      if (x <= LCD_W) {
        reusableBuffer.spectrumAnalyser.bars[x] = power;
        if (power > reusableBuffer.spectrumAnalyser.max[x]) {
          reusableBuffer.spectrumAnalyser.max[x] = power;
        }
#endif
      }
      if (++cur_channel > MULTI_SCANNER_MAX_CHANNEL)
        cur_channel = 0;
    }
  }
}

static void processMultiStatusPacket(const uint8_t * data, uint8_t module, uint8_t len)
{
  MultiModuleStatus &status = getMultiModuleStatus(module);

  // At least two status packets without bind flag
  bool wasBinding = status.isBinding();

  status.flags = data[0];
  status.major = data[1];
  status.minor = data[2];
  status.revision = data[3];
  status.patch = data[4];
  if (len < 6) {
    status.ch_order = 0xFF;
  }
  else {
    status.ch_order = data[5];
    if (len >= 24) {
      status.protocolNext = data[6];
      status.protocolPrev = data[7];
      memcpy(status.protocolName, &data[8], 7);
      status.protocolName[7] = 0;
      status.protocolSubNbr = data[15] & 0x0F;
      memcpy(status.protocolSubName, &data[16], 8);
      status.protocolSubName[8] = 0;
      status.optionDisp = data[15] >> 4;
    }
    else {
      status.protocolName[0] = 0;
    }
  }

  if (!getMultiModuleStatus(module).failsafeChecked) {
    getMultiModuleStatus(module).requiresFailsafeCheck = true;
    getMultiModuleStatus(module).failsafeChecked = true;
  }
  
  if (wasBinding && !status.isBinding() && getMultiBindStatus(module) == MULTI_BIND_INITIATED)
    setMultiBindStatus(module, MULTI_BIND_FINISHED);

  // Dirty RX protocol detection
  size_t proto_len = strnlen(status.protocolName, 8);
  if (proto_len >= 2 &&
      status.protocolName[proto_len - 2] == 'R' &&
      status.protocolName[proto_len - 1] == 'X') {
    status.isRXProto = true;
  } else {
    status.isRXProto = false;
  }

  // update timestamp last to avoid race conditions
  status.lastUpdate = get_tmr10ms();
}

static void processMultiSyncPacket(const uint8_t * data, uint8_t module)
{
  ModuleSyncStatus &status = getModuleSyncStatus(module);

  uint16_t refreshRate = data[0] << 8 | data[1];
  int16_t  inputLag    = data[2] << 8 | data[3];

  status.update(refreshRate, inputLag);
#if defined(DEBUG)
  dbgSerialPrint("MP ADJ: R %d, L %04d", refreshRate, inputLag);
#endif
}

#if defined(PCBTARANIS) || defined(PCBHORUS)
static void processMultiRxChannels(const uint8_t * data, uint8_t len)
{
  if (g_model.trainerData.mode != TRAINER_MODE_MULTI)
    return;

  //uint8_t pps  = data[0];
  //uint8_t rssi = data[1];
  int ch    = max(data[2], (uint8_t)0);
  int maxCh = min(ch + data[3], MAX_TRAINER_CHANNELS);

  uint32_t bits = 0;
  uint8_t  bitsavailable = 0;
  uint8_t  byteIdx = 4;

  while (ch < maxCh) {
    while (bitsavailable < MULTI_CHAN_BITS && byteIdx < len) {
      bits |= (uint32_t)(data[byteIdx++]) << (uint32_t)bitsavailable;
      bitsavailable += 8;
    }

    int value = bits & ((1 << MULTI_CHAN_BITS) - 1);
    bitsavailable -= MULTI_CHAN_BITS;
    bits >>= MULTI_CHAN_BITS;

    ppmInput[ch] = (value - 1024) * 500 / 800;
    ch++;

    if (byteIdx >= len)
      break;
  }

  if (ch == maxCh)
    ppmInputValidityTimer = PPM_IN_VALID_TIMEOUT;
}
#endif

#if defined(LUA)

static void processConfigPacket(const uint8_t * packet, uint8_t len)
{
  // Multi_Buffer[0..3]=="Conf" -> Lua script is running
  // Multi_Buffer[4]==0x01 -> TX to Module data ready to be sent
  // Multi_Buffer[4]==0xFF -> Clear buffer data
  // Multi_Buffer[5..11]=7 bytes of TX to Module data
  // Multi_Buffer[12] -> Current page
  // Multi_Buffer[13..172]=8*20=160 bytes of Module to TX data
  if (Multi_Buffer && memcmp(Multi_Buffer, "Conf", 4) == 0) {
    // HoTT Lua script is running
    if (Multi_Buffer[4] == 0xFF) {
      // Init
      memset(&Multi_Buffer[4], 0x00, 1 + 7 + 1 + 160);           // Clear the buffer
    }
    if ((packet[0] >> 4) != Multi_Buffer[12]) {// page change
      memset(&Multi_Buffer[13], 0x00, 160);                      // Clear the buffer
      Multi_Buffer[12] = (packet[0] >> 4);                       //Save the page number
    }
    memcpy(&Multi_Buffer[13 + (packet[0] & 0x0F) * 20], &packet[1], 20); // Store the received page in the buffer
  }
}
#endif

#if defined(MULTI_PROTOLIST)
static void processMultiProtoDef(uint8_t module, const uint8_t * packet, uint8_t len)
{
  /*
    data[0]     = protocol number, 0xFF is an invalid list entry
                  (Option value too large) and nothing sent after
    data[1..n]  = protocol name null terminated
    data[n+1]   = flags
                   flags>>4 Option text number to be displayed
                            (check multi status for description)
                   flags&0x01 failsafe supported
                   flags&0x02 Channel Map Disabled supported
    data[n+2]   = number of sub protocols
    data[n+3]   = sub protocols text length, only sent if nbr_sub != 0
    data[n+4..] = sub protocol names, only sent if nbr_sub != 0
   */

  MultiRfProtocols::instance(module)->scanReply(packet, len);
}
#endif

 
static void processMultiTelemetryPaket(const uint8_t * packet, uint8_t module)
{
  uint8_t type = packet[0];
  uint8_t len = packet[1];
  const uint8_t * data = packet + 2;

  // Switch type
  switch (type) {
    case MultiStatus:
      if (len >= 5)
        processMultiStatusPacket(data, module, len);
      break;

    case DSMBindPacket:
      if (len >= 10)
        processDSMBindPacket(module, data);
      break;

    case SpektrumTelemetry:
      // processSpektrumPacket expects data[0] to be the telemetry indicator 0xAA but does not check it,
      // just send one byte of our header instead
      if (len >= 17)
        processSpektrumPacket(data - 1);
      else
        TRACE("[MP] Received spektrum telemetry len %d < 17", len);
      break;

    case FlyskyIBusTelemetry:
      if (len >= 28)
        processFlySkyPacket(data);
      else
        TRACE("[MP] Received IBUS telemetry len %d < 28", len);
      break;

    case FlyskyIBusTelemetryAC:
      if (len >= 28)
        processFlySkyPacketAC(data);
      else
        TRACE("[MP] Received IBUS telemetry AC len %d < 28", len);
      break;

    case HitecTelemetry:
      if (len >= 8)
        processHitecPacket(data);
      else
        TRACE("[MP] Received Hitec telemetry len %d < 8", len);
      break;

    case HottTelemetry:
      if (len >= 14)
        processHottPacket(data);
      else
        TRACE("[MP] Received HoTT telemetry len %d < 14", len);
      break;

    case MLinkTelemetry:
      if (len > 6)
        processMLinkPacket(data);
      else
        TRACE("[MP] Received M-Link telemetry len %d <= 6", len);
      break;

#if defined(LUA)
    case ConfigTelemetry:
      if (len >= 21)
        processConfigPacket(data, len);
      else
        TRACE("[MP] Received Config telemetry len %d < 20", len);
      break;
#endif

    case FrSkyHubTelemetry:
      if (len >= 4)
        frskyDProcessPacket(data);
      else
        TRACE("[MP] Received Frsky HUB telemetry len %d < 4", len);
      break;

    case FrSkySportTelemetry:
      if (len >= 4) {
        if (sportProcessTelemetryPacket(data) && len >= 8) {
          uint8_t primId = data[1];
          uint16_t dataId = *((uint16_t *)(data+2));
          if (primId == DATA_FRAME && dataId == RSSI_ID) {
            // fetch MPM special TX_RSSI & TX_LQI sensors
            uint8_t physicalId = data[0] & 0x1F;
            uint8_t instance = physicalId + (TELEMETRY_ENDPOINT_SPORT << 5);
            sportProcessTelemetryPacket(TX_RSSI_ID, 0, instance, data[5] >> 1u, UNIT_DB);
            sportProcessTelemetryPacket(TX_LQI_ID, 0, instance, data[7], UNIT_RAW);
          }
        }
      }
      else
        TRACE("[MP] Received sport telemetry len %d < 4", len);
      break;

    case InputSync:
      if (len >= 6)
        processMultiSyncPacket(data, module);
      else
        TRACE("[MP] Received input sync len %d < 6", len);
      break;

    case ConfigCommand:
      // Just an ack to our command, ignore for now
      break;

#if defined(LUA)
    case FrskySportPolling:
      if (len >= 1 && outputTelemetryBuffer.destination == TELEMETRY_ENDPOINT_SPORT && data[0] == outputTelemetryBuffer.sport.physicalId) {
        TRACE("MP Sending sport data out.");
        sportSendBuffer(outputTelemetryBuffer.data, outputTelemetryBuffer.size);
      }
      break;
#endif
    case SpectrumScannerPacket:
      if (len == 6)
        processMultiScannerPacket(data, module);
      else
        TRACE("[MP] Received spectrum scanner len %d != 6", len);
      break;

#if defined(PCBTARANIS) || defined(PCBHORUS)
    case MultiRxChannels:
      if (len >= 4)
        processMultiRxChannels(data, len);
      else
        TRACE("[MP] Received RX channels len %d < 4", len);
      break;
#endif

#if defined(MULTI_PROTOLIST)
    case MultiProtoDef:
      if (len >= 1)
        processMultiProtoDef(module, data, len);
      break;
#endif

    default:
      TRACE("[MP] Unkown multi packet type 0x%02X, len %d", type, len);
      break;
  }
}

void MultiModuleStatus::getStatusString(char * statusText) const
{
  if (!isValid()) {
#if defined(PCBFRSKY)
#if !defined(INTERNAL_MODULE_MULTI)
    if (isSportLineUsedByInternalModule())
      strcpy(statusText, STR_DISABLE_INTERNAL);
    else
#endif
#endif
    strcpy(statusText, STR_MODULE_NO_TELEMETRY);
    return;
  }
  if (!protocolValid()) {
    strcpy(statusText, STR_PROTOCOL_INVALID);
    return;
  }
  else if (!serialMode()) {
    strcpy(statusText, STR_MODULE_NO_SERIAL_MODE);
    return;
  }
  else if (!inputDetected()) {
    strcpy(statusText, STR_MODULE_NO_INPUT);
    return;
  }
  else if (isWaitingforBind()) {
    strcpy(statusText, STR_MODULE_WAITFORBIND);
    return;
  }

  if ((((major << 24) | (minor << 16) | (revision << 8) | patch) < MULTI_DESIRED_VERSION) && SLOW_BLINK_ON_PHASE) {
    strcpy(statusText, STR_MODULE_UPGRADE);
  }
  else {
    char * tmp = statusText;
    *tmp++ = 'V';
    tmp = strAppendUnsigned(tmp, major);
    *tmp++ = '.';
    tmp = strAppendUnsigned(tmp, minor);
    *tmp++ = '.';
    tmp = strAppendUnsigned(tmp, revision);
    *tmp++ = '.';
    tmp = strAppendUnsigned(tmp, patch);

    if (isBinding()) {
      strcpy(tmp, " " TR_MODULE_BINDING);
    }
    else if (ch_order != 0xFF) {
      uint8_t temp = ch_order;
      *tmp++ = ' ';
      *(tmp + (temp & 0x03)) = 'A';
      temp >>= 2;
      *(tmp + (temp & 0x03)) = 'E';
      temp >>= 2;
      *(tmp + (temp & 0x03)) = 'T';
      temp >>= 2;
      *(tmp + (temp & 0x03)) = 'R';
      *(tmp + 4) = '\0';
    }
  }
}

static void processMultiTelemetryByte(const uint8_t data, uint8_t module)
{
  uint8_t * rxBuffer = getTelemetryRxBuffer(module);
  uint8_t &rxBufferCount = getTelemetryRxBufferCount(module);

  if (rxBufferCount < TELEMETRY_RX_PACKET_SIZE) {
    rxBuffer[rxBufferCount++] = data;
  }
  else {
    TRACE("[MP] array size %d error", rxBufferCount);
    setMultiTelemetryBufferState(module, NoProtocolDetected);
  }

  // Length field does not count the header
  if (rxBufferCount >= 2 && rxBuffer[1] == rxBufferCount - 2) {
    // debug print the content of the packet
#if 0
    debugPrintf("[MP] Packet type %02X len 0x%02X: ",
                rxBuffer[0], rxBuffer[1]);
    for (int i=0; i<(rxBufferCount+3)/4; i++) {
      debugPrintf("[%02X%02X %02X%02X] ", rxBuffer[i*4+2], rxBuffer[i*4 + 3],
                  rxBuffer[i*4 + 4], rxBuffer[i*4 + 5]);
    }
    debugPrintf(CRLF);
#endif
    // Packet is complete, process it
    processMultiTelemetryPaket(rxBuffer, module);
    setMultiTelemetryBufferState(module, NoProtocolDetected);
  }
}

void processMultiTelemetryData(uint8_t data, uint8_t module)
{
  uint8_t * rxBuffer = getTelemetryRxBuffer(module);
  uint8_t &rxBufferCount = getTelemetryRxBufferCount(module);

  uint16_t &lastRxTS = getMultiTelemetryLastRxTS(module);
  uint16_t nowMs = (uint16_t)RTOS_GET_MS();
  if (nowMs - lastRxTS > 15)
    setMultiTelemetryBufferState(module, NoProtocolDetected);
  lastRxTS = nowMs;
  
  // debugPrintf("State: %d, byte received %02X, buflen: %d\r\n", getMultiTelemetryBufferState(module), data, rxBufferCount);
  
  switch (getMultiTelemetryBufferState(module)) {
    case NoProtocolDetected:
      if (data == 'M') {
        setMultiTelemetryBufferState(module, MultiFirstByteReceived);
      }
      else if (data == 0xAA || data == 0x7e) {
        setMultiTelemetryBufferState(module, guessProtocol(module));

        // Process the first byte by the protocol
        processMultiTelemetryData(data, module);
      }
      else {
        TRACE("[MP] invalid start byte 0x%02X", data);
      }
      break;

    case FrskyTelemetryFallback:
      setMultiTelemetryBufferState(module, FrskyTelemetryFallbackFirstByte);
      processFrskyTelemetryData(data);
      break;

    case FrskyTelemetryFallbackFirstByte:
      if (data == 'M') {
        setMultiTelemetryBufferState(module, MultiStatusOrFrskyData);
      }
      else {
        processFrskyTelemetryData(data);
        if (data != 0x7e)
          setMultiTelemetryBufferState(module, FrskyTelemetryFallbackNextBytes);
      }

      break;

    case FrskyTelemetryFallbackNextBytes:
      processFrskyTelemetryData(data);
      if (data == 0x7e) {
        // end of packet or start of new packet
        setMultiTelemetryBufferState(module, FrskyTelemetryFallbackFirstByte);
      }
      break;

    case FlyskyTelemetryFallback:
      processFlySkyTelemetryData(data, rxBuffer, rxBufferCount);
      if (rxBufferCount == 0) {
        setMultiTelemetryBufferState(module, NoProtocolDetected);
      }
      break;

    case SpektrumTelemetryFallback:
      processSpektrumTelemetryData(module, data, rxBuffer, rxBufferCount);
      if (rxBufferCount == 0) {
        setMultiTelemetryBufferState(module, NoProtocolDetected);
      }
      break;

    case MultiFirstByteReceived:
      rxBufferCount = 0;
      if (data == 'P') {
        setMultiTelemetryBufferState(module, ReceivingMultiProtocol);
      }
      else if (data >= 5 && data <= 10) {
        // Protocol indented for er9x/ersky9, accept only 5-10 as packet length to have
        // a bit of validation
        setMultiTelemetryBufferState(module, ReceivingMultiStatus);
        processMultiTelemetryData(data, module);
      }
      else {
        TRACE("[MP] invalid second byte 0x%02X", data);
        setMultiTelemetryBufferState(module, NoProtocolDetected);
      }
      break;

    case ReceivingMultiProtocol:
      processMultiTelemetryByte(data, module);
      break;

    case MultiStatusOrFrskyData:
      // Check len byte if it makes sense for multi
      if (data >= 5 && data <= 10) {
        setMultiTelemetryBufferState(module, ReceivingMultiStatus);
        rxBufferCount = 0;
      }
      else {
        setMultiTelemetryBufferState(module, FrskyTelemetryFallbackNextBytes);
        processMultiTelemetryData('M', module);
      }
      processMultiTelemetryData(data, module);
      break;

    case ReceivingMultiStatus:
      if (rxBufferCount < TELEMETRY_RX_PACKET_SIZE) {
        rxBuffer[rxBufferCount++] = data;
        if (rxBufferCount > 5 && rxBuffer[0] == rxBufferCount - 1) {
          processMultiStatusPacket(rxBuffer + 1, module, rxBuffer[0]);
          rxBufferCount = 0;
          setMultiTelemetryBufferState(module, NoProtocolDetected);
        }
        if (rxBufferCount > 24) {
          // too long ignore
          TRACE("Overlong multi status packet detected ignoring, wanted %d", rxBuffer[0]);
          rxBufferCount = 0;
          setMultiTelemetryBufferState(module, NoProtocolDetected);
        }
      }
      else {
        TRACE("[MP] array size %d error", rxBufferCount);
        setMultiTelemetryBufferState(module, NoProtocolDetected);
      }
      break;
  }
}

bool isMultiTelemReceiving(uint8_t module)
{
  return getMultiTelemetryBufferState(module) != NoProtocolDetected;
}

void checkFailsafeMulti()
{
  for (int i=0; i<NUM_MODULES; i++) {
    if (isModuleMultimodule(i) &&
        getMultiModuleStatus(i).requiresFailsafeCheck) {
      getMultiModuleStatus(i).requiresFailsafeCheck = false;
      if (getMultiModuleStatus(i).supportsFailsafe() &&
          g_model.moduleData[i].failsafeMode == FAILSAFE_NOT_SET) {
        ALERT(STR_FAILSAFEWARN, STR_NO_FAILSAFE, AU_ERROR);
      }
    }
  }
}
