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
 *
 * Dedicate for FlySky NV14 board.
 */

#include "opentx.h"

#include "flysky.h"
#include "telemetry/flysky_nv14.h"

#define IS_VALID_COMMAND_ID(id) ((id) < CMD_LAST)

#ifndef custom_log
#define custom_log
#endif

enum AfhdsSpecialChars {
  END = 0xC0,             //Frame end
  START = END,
  ESC_END = 0xDC,         //Escaped frame end - in case END occurs in fame then ESC ESC_END must be used
  ESC = 0xDB,             //Escaping character
  ESC_ESC = 0xDD,         //Escaping character in case ESC occurs in fame then ESC ESC_ESC  must be used
};

// 0:OFF, 1:RF frame only, 2:TX frame only, 3:Both RF and TX frame
enum DEBUG_RF_FRAME_PRINT_E {
  FRAME_PRINT_OFF = 0,
  RF_FRAME_ONLY = 1,
  TX_FRAME_ONLY = 2,
  BOTH_FRAME_PRINT = 3
};

#define DEBUG_RF_FRAME_PRINT FRAME_PRINT_OFF
//#define DEBUG_RF_FRAME_PRINT BOTH_FRAME_PRINT
#define FLYSKY_MODULE_TIMEOUT 155             /* ms */
#define FLYSKY_PERIOD 4                       /*ms*/
#define NUM_OF_NV14_CHANNELS (14)
#define VALID_CH_DATA(v) ((v) > 900 && (v) < 2100)
#define FAILSAVE_SEND_COUNTER_MAX (400)

#define gRomData g_model.moduleData[INTERNAL_MODULE].flysky
#define SET_DIRTY() storageDirty(EE_MODEL)

enum FlySkyBindState_E {
  BIND_LOW_POWER,
  BIND_NORMAL_POWER,
  BIND_EXIT,
};

enum FlySkyRxPulse_E { FLYSKY_PWM, FLYSKY_PPM };

enum FlySkyRxPort_E { FLYSKY_IBUS, FLYSKY_SBUS };

enum FlySkyFirmwareType_E { FLYSKY_RX_FIRMWARE, FLYSKY_RF_FIRMWARE };
enum FlySkyChannelDataType_E {
  FLYSKY_CHANNEL_DATA_NORMAL,
  FLYSKY_CHANNEL_DATA_FAILSAFE
};

enum FlySkyPulseModeValue_E { PWM_IBUS, PWM_SBUS, PPM_IBUS, PPM_SBUS };

enum FlySkyPulseProtocol_E {
  FLYSKY_AFHDS1,
  FLYSKY_AFHDS2,
  FLYSKY_AFHDS2A,
};

enum FlySkyModuleState_E {
  STATE_SET_TX_POWER = 0,
  STATE_INIT = 1,
  STATE_GET_FW_VERSION_INIT = 2,
  STATE_BIND = 3,
  STATE_SET_RECEIVER_ID = 4,
  STATE_SET_RX_PWM_PPM = 5,
  STATE_SET_RX_IBUS_SBUS = 6,
  STATE_SET_RX_FREQUENCY = 7,
  STATE_UPDATE_RF_FIRMWARE = 8,
  STATE_UPDATE_RX_FIRMWARE = 9,
  STATE_UPDATE_HALL_FIRMWARE = 10,
  STATE_UPDATE_RF_PROTOCOL = 11,
  STATE_GET_RECEIVER_CONFIG = 12,
  STATE_GET_RX_VERSION_INFO = 13,
  STATE_GET_RF_VERSION_INFO = 14,
  STATE_SET_RANGE_TEST = 15,
  STATE_RANGE_TEST_RUNNING = 16,
  STATE_IDLE = 17,
  STATE_DISCONNECT = 18,
  STATE_SEND_CHANNELS = 19,
};

#define AfhdsPwmMode (gRomData.mode & 2 ? FLYSKY_PPM : FLYSKY_PWM)
#define AfhdsIbusMode (gRomData.mode & 1 ? FLYSKY_SBUS : FLYSKY_IBUS)

struct rf_info_t {
  uint8_t bind_power;
  uint8_t protocol;
  // 0: normal, COMMAND_ID0C_UPDATE_RF_FIRMWARE or CMD_UPDATE_FIRMWARE_END
  uint8_t fw_state;
};

// static STRUCT_HALL rxBuffer = {0};
static uint32_t rfRxCount = 0;
static uint8_t lastState = STATE_IDLE;
static uint32_t set_loop_cnt = 0;

uint32_t NV14internalModuleFwVersion = 0;

static rf_info_t rf_info = {
    .bind_power = BIND_LOW_POWER,
    .protocol = FLYSKY_AFHDS2A,  // never updated currently
    .fw_state = 0,
};

typedef struct __attribute__((packed)) rxParams {
  uint8_t outputMode;
  uint8_t outputSerialMode;
  uint16_t outputFreq;
} rxParams;

typedef struct __attribute__((packed)) fwVersion {
  uint8_t target;
  uint32_t version;
} fwVersion;

struct __attribute__((packed)) afhds2Resp {
  uint8_t startByte;
  uint8_t frame_number;
  uint8_t frame_type;
  uint8_t command_id;
  uint8_t value;
};

uint8_t emptyRxID[] = {0, 0, 0, 0};

void setFlyskyState(uint8_t state);
void onFlySkyBindReceiver();
void onFlySkyModuleSetPower(bool isPowerOn);
void afhds2Command(uint8_t type, uint8_t cmd);
void onFlySkyGetVersionInfoStart(uint8_t isRfTransfer);
// void usbDownloadTransmit(uint8_t *buffer, uint32_t size);

// extern uint32_t NV14internalModuleFwVersion;

#define END 0xC0
#define ESC 0xDB
#define ESC_END 0xDC
#define ESC_ESC 0xDD

#define FRAME_TYPE_REQUEST_ACK 0x01
#define FRAME_TYPE_REQUEST_NACK 0x02
#define FRAME_TYPE_ANSWER 0x10

enum FlySkyModuleCommandID {
  CMD_NONE=0,
  CMD_RF_INIT=1,
  CMD_BIND=2,
  CMD_SET_RECEIVER_ID=3,
  CMD_RF_GET_CONFIG=4,
  CMD_SEND_CHANNEL_DATA=5,
  CMD_RX_SENSOR_DATA=6,
  CMD_SET_RX_PWM_PPM=7,
  CMD_SET_RX_SERVO_FREQ=8,
  CMD_GET_VERSION_INFO=9,
  CMD_SET_RX_IBUS_SBUS=0xA,
  CMD_SET_RX_IBUS_SERVO_EXT=0xB,
  CMD_UPDATE_RF_FIRMWARE = 0x0C,
  CMD_SET_TX_POWER = 0x0D,
  CMD_SET_RF_PROTOCOL,
  CMD_TEST_RANGE,
  CMD_TEST_RF_RESERVED,
  CMD_UPDATE_RX_FIRMWARE = 0x20,
  CMD_LAST
};

bool isFlySkyUsbDownload(void) { return rf_info.fw_state != 0; }

// void usbSetFrameTransmit(uint8_t packetID, uint8_t *dataBuf, uint32_t nBytes)
// {
//     // send to host via usb
//     uint8_t *pt = (uint8_t*)&rxBuffer;
//    // rxBuffer.head = HALL_PROTOLO_HEAD;
//     rxBuffer.hallID.hall_Id.packetID = packetID;//0x08;
//     rxBuffer.hallID.hall_Id.senderID = 0x03;
//     rxBuffer.hallID.hall_Id.receiverID = 0x02;

//     if ( packetID == 0x08 ) {
//       uint8_t fwVerision[40];
//       for(uint32_t idx = 40; idx > 0; idx--)
//       {
//           if ( idx <= nBytes ) {
//               fwVerision[idx-1] = dataBuf[idx-1];
//           }
//           else fwVerision[idx-1] = 0;
//       }
//       dataBuf = fwVerision;
//       nBytes = 40;
//     }

//     rxBuffer.length = nBytes;

//     TRACE_NOCRLF("\r\nToUSB: 55 %02X %02X ", rxBuffer.hallID.ID, nBytes);
//     for ( uint32_t idx = 0; idx < nBytes; idx++ )
//     {
//         rxBuffer.data[idx] = dataBuf[idx];
//         TRACE_NOCRLF("%02X ", rxBuffer.data[idx]);
//     }
// #if !defined(SIMU)
//     uint16_t checkSum = calc_crc16(pt, rxBuffer.length+3);
//     TRACE(" CRC:%04X;", checkSum);

//     pt[rxBuffer.length + 3] = checkSum & 0xFF;
//     pt[rxBuffer.length + 4] = checkSum >> 8;

//     usbDownloadTransmit(pt, rxBuffer.length + 5);
// #endif
// }

void onFlySkyModuleSetPower(bool isPowerOn)
{
  if (isPowerOn) {
    INTERNAL_MODULE_ON();
    resetPulsesAFHDS2();
  } else {
    moduleState[INTERNAL_MODULE].mode = MODULE_MODE_NORMAL;
    INTERNAL_MODULE_OFF();
  }
}

void setFlyskyState(uint8_t state) { intmodulePulsesData.flysky.state = state; }

void onFlySkyUsbDownloadStart(uint8_t fw_state) { rf_info.fw_state = fw_state; }

void onFlySkyGetVersionInfoStart(uint8_t isRfTransfer)
{
  lastState = intmodulePulsesData.flysky.state;
  setFlyskyState(isRfTransfer ? STATE_GET_RF_VERSION_INFO
                              : STATE_GET_RX_VERSION_INFO);
}

inline void initFlySkyArray()
{
  intmodulePulsesData.flysky.ptr = intmodulePulsesData.flysky.pulses;
  intmodulePulsesData.flysky.crc = 0;
}

inline void putFlySkyByte(uint8_t byte)
{
  if (END == byte) {
    *intmodulePulsesData.flysky.ptr++ = ESC;
    *intmodulePulsesData.flysky.ptr++ = ESC_END;
  } else if (ESC == byte) {
    *intmodulePulsesData.flysky.ptr++ = ESC;
    *intmodulePulsesData.flysky.ptr++ = ESC_ESC;
  } else {
    *intmodulePulsesData.flysky.ptr++ = byte;
  }
}

inline void putFlySkyFrameByte(uint8_t byte)
{
  intmodulePulsesData.flysky.crc += byte;
  putFlySkyByte(byte);
}

inline void putFlySkyFrameCmd(uint8_t type, uint8_t cmd)
{
  intmodulePulsesData.flysky.crc += type + cmd;
  putFlySkyByte(type);
  putFlySkyByte(cmd);
}

inline void putFlySkyFrameBytes(uint8_t* data, int length)
{
  for (int i = 0; i < length; i++) {
    intmodulePulsesData.flysky.crc += data[i];
    putFlySkyByte(data[i]);
  }
}

inline void putFlySkyFrameHeader()
{
  initFlySkyArray();
  *intmodulePulsesData.flysky.ptr++ = END;
  putFlySkyFrameByte(intmodulePulsesData.flysky.frame_index);
}

inline void putFlySkyFrameFooter()
{
  if (++intmodulePulsesData.flysky.frame_index == 0) {
    intmodulePulsesData.flysky.frame_index = 1;
  }
  putFlySkyByte(intmodulePulsesData.flysky.crc ^ 0xff);
  *intmodulePulsesData.flysky.ptr++ = END;
}

void afhds2Command(uint8_t type, uint8_t cmd)
{
  putFlySkyFrameHeader();
  putFlySkyFrameCmd(type, cmd);
  putFlySkyFrameFooter();
}

inline void putFlySkySendChannelData()
{
  uint16_t pulseValue = 0;
  uint8_t channels_start = g_model.moduleData[INTERNAL_MODULE].channelsStart;
  uint8_t channels_last =
      channels_start + 8 + g_model.moduleData[INTERNAL_MODULE].channelsCount;
  putFlySkyFrameCmd(FRAME_TYPE_REQUEST_NACK, CMD_SEND_CHANNEL_DATA);

  if (moduleState[INTERNAL_MODULE].counter-- == 0) {
    moduleState[INTERNAL_MODULE].counter = FAILSAVE_SEND_COUNTER_MAX;
    putFlySkyFrameByte(FLYSKY_CHANNEL_DATA_FAILSAFE);
    putFlySkyFrameByte(channels_last - channels_start);
    for (uint8_t channel = channels_start; channel < channels_last; channel++) {
      if (g_model.moduleData[INTERNAL_MODULE].failsafeMode == FAILSAFE_CUSTOM) {
        int16_t failsafeValue = g_model.failsafeChannels[channel];
        pulseValue =
            limit<uint16_t>(0, 988 + ((failsafeValue + 1024) / 2), 0xfff);
      } else if (g_model.moduleData[INTERNAL_MODULE].failsafeMode ==
                 FAILSAFE_HOLD) {
        // protocol uses hold by default
        pulseValue = 0xfff;
      } else {
        int16_t failsafeValue =
            -1024 + 2 * PPM_CH_CENTER(channel) - 2 * PPM_CENTER;
        pulseValue =
            limit<uint16_t>(0, 988 + ((failsafeValue + 1024) / 2), 0xfff);
      }
      putFlySkyFrameByte(pulseValue & 0xff);
      putFlySkyFrameByte(pulseValue >> 8);
    }
    if (DEBUG_RF_FRAME_PRINT & RF_FRAME_ONLY) {
      TRACE("------FAILSAFE------");
    }
  } else {
    putFlySkyFrameByte(FLYSKY_CHANNEL_DATA_NORMAL);
    putFlySkyFrameByte(channels_last - channels_start);
    for (uint8_t channel = channels_start; channel < channels_last; channel++) {
      int channelValue =
          channelOutputs[channel] + 2 * PPM_CH_CENTER(channel) - 2 * PPM_CENTER;
      pulseValue = limit<uint16_t>(0, 988 + ((channelValue + 1024) / 2), 0xfff);
      putFlySkyFrameByte(pulseValue & 0xff);
      putFlySkyFrameByte(pulseValue >> 8);
    }
  }
}

void putFlySkyUpdateFirmwareStart(uint8_t cmd)
{
  putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, cmd);
}

bool checkFlySkyFrameCrc(const uint8_t* ptr, uint8_t size)
{
  uint8_t crc = 0;

  for (uint8_t i = 0; i < size; i++) {
    crc += ptr[i];
  }

  if (DEBUG_RF_FRAME_PRINT & RF_FRAME_ONLY) {
    if (ptr[2] != 0x06 || (set_loop_cnt++ % 50 == 0)) {
      TRACE_NOCRLF("RF(%0d): C0", INTMODULE_USART_AFHDS2_BAUDRATE);
      for (int idx = 0; idx <= size; idx++) {
        TRACE_NOCRLF(" %02X", ptr[idx]);
      }
      TRACE_NOCRLF(" C0;" CRLF);

      if ((crc ^ 0xff) != ptr[size]) {
        TRACE("ErrorCRC %02X especting %02X", crc ^ 0xFF, ptr[size]);
      }
    }
  }

  return (crc ^ 0xff) == ptr[size];
}

inline void debugFrame(const uint8_t* rxBuffer, uint8_t rxBufferCount)
{
  // debug print the content of the packet
  char buffer[64 * 3];
  char* pos = buffer;
  if (rxBufferCount * 3 > sizeof(buffer)) rxBufferCount = 64;
  for (int i = 0; i < rxBufferCount; i++) {
    pos += snprintf(pos, buffer + sizeof(buffer) - pos, "%02X ", rxBuffer[i]);
  }
  (*pos) = 0;
  TRACE("count [%d] data: %s", rxBufferCount, buffer);
}

inline void parseResponse(uint8_t* buffer, uint8_t dataLen)
{
  afhds2Resp* resp = reinterpret_cast<afhds2Resp*>(buffer);
  if (resp->startByte != END || dataLen < 2) return;

  dataLen -= 2;
  if (!checkFlySkyFrameCrc(buffer + 1, dataLen)) {
    return;
  }

  if ((moduleState[INTERNAL_MODULE].mode != MODULE_MODE_BIND) &&
      (resp->frame_type == FRAME_TYPE_ANSWER) &&
      ((intmodulePulsesData.flysky.frame_index - 1) != resp->frame_number)) {
    return;
  } else if (resp->frame_type == FRAME_TYPE_REQUEST_ACK) {
    intmodulePulsesData.flysky.frame_index = resp->frame_number;
  }

  debugFrame(&resp->command_id, dataLen - 2);
  switch (resp->command_id) {
    default:
      if (moduleState[INTERNAL_MODULE].mode == MODULE_MODE_NORMAL &&
          intmodulePulsesData.flysky.state >= STATE_IDLE) {
        setFlyskyState(STATE_SEND_CHANNELS);
        if (DEBUG_RF_FRAME_PRINT & RF_FRAME_ONLY)
          TRACE("State back to channel data");
      }
      break;
    case CMD_RF_INIT:
      if (resp->value == 0x01) {  // action only RF ready
        if (moduleState[INTERNAL_MODULE].mode == MODULE_MODE_BIND)
          setFlyskyState(STATE_BIND);
        else
          setFlyskyState(STATE_GET_FW_VERSION_INIT);  // get version first
      } else {
        // Try one more time;
        resetPulsesAFHDS2();
        setFlyskyState(STATE_INIT);
      }
      break;
    case CMD_BIND: {
      if (resp->frame_type != FRAME_TYPE_ANSWER) {
        setFlyskyState(STATE_IDLE);
        return;
      }
      if (moduleState[INTERNAL_MODULE].mode == MODULE_MODE_BIND)
        moduleState[INTERNAL_MODULE].mode = MODULE_MODE_NORMAL;
      *((uint32_t*)gRomData.rx_id) =
          *(reinterpret_cast<const uint32_t*>(&resp->value));
      g_model.header.modelId[INTERNAL_MODULE] = gRomData.rx_id[3];
      if (DEBUG_RF_FRAME_PRINT & RF_FRAME_ONLY)
        TRACE("New Rx ID: %02X %02X %02X %02X", gRomData.rx_id[0],
              gRomData.rx_id[1], gRomData.rx_id[2], gRomData.rx_id[3]);
      SET_DIRTY();
      resetPulsesAFHDS2();
      setFlyskyState(STATE_INIT);
      break;
    }
    case CMD_RF_GET_CONFIG:
      setFlyskyState(STATE_GET_RECEIVER_CONFIG);
      intmodulePulsesData.flysky.timeout = FLYSKY_MODULE_TIMEOUT;
      break;
    case CMD_RX_SENSOR_DATA:
      flySkyNv14ProcessTelemetryPacket(&resp->value, dataLen - 3);
      if (moduleState[INTERNAL_MODULE].mode == MODULE_MODE_NORMAL &&
          intmodulePulsesData.flysky.state >= STATE_IDLE) {
        setFlyskyState(STATE_SEND_CHANNELS);
      }
      break;
    case CMD_SET_RECEIVER_ID: {
      // range check seems to be not working
      // it disconnects receiver
      // if (moduleState[INTERNAL_MODULE].mode == MODULE_MODE_RANGECHECK) {
      //  setFlyskyState(STATE_SET_RANGE_TEST);
      //}
      // else
      setFlyskyState(STATE_SEND_CHANNELS);
      return;
    }
    case CMD_TEST_RANGE:
      if (moduleState[INTERNAL_MODULE].mode != MODULE_MODE_RANGECHECK)
        resetPulsesAFHDS2();
      else
        setFlyskyState(STATE_RANGE_TEST_RUNNING);
      break;
    case CMD_SET_TX_POWER:
      setFlyskyState(STATE_INIT);
      break;
    case CMD_SET_RX_PWM_PPM:
      setFlyskyState(STATE_SET_RX_IBUS_SBUS);
      break;
    case CMD_SET_RX_IBUS_SBUS:
      setFlyskyState(STATE_SET_RX_FREQUENCY);
      break;
    case CMD_SET_RX_SERVO_FREQ:
      setFlyskyState(STATE_SEND_CHANNELS);
      break;
    case CMD_UPDATE_RF_FIRMWARE: {
      rf_info.fw_state = STATE_UPDATE_RF_FIRMWARE;
      setFlyskyState(STATE_IDLE);
      break;
    }
    case CMD_GET_VERSION_INFO: {
      if (intmodulePulsesData.flysky.state == STATE_GET_FW_VERSION_INIT) {
        memcpy(&NV14internalModuleFwVersion, &resp->value + 1,
               sizeof(NV14internalModuleFwVersion));
        setFlyskyState(STATE_SET_RECEIVER_ID);
        break;
      }

      // if (dataLen > 4) {
      //   usbSetFrameTransmit(0x08, (uint8_t*) &resp->value + 1, dataLen - 4);
      // }

      if (lastState == STATE_GET_RF_VERSION_INFO ||
          lastState == STATE_GET_RX_VERSION_INFO) {
        lastState = STATE_INIT;
      }
      setFlyskyState(lastState);
      break;
    }
  }
}

bool isrxBufferMsgOK(void)
{
  bool isMsgOK = (0 != rfRxCount);
  rfRxCount = 0;
  return isMsgOK && isFlySkyUsbDownload();
}

void processInternalFlySkyTelemetryData(uint8_t byte, uint8_t* buffer, uint8_t* len)
{
  // #if !defined(SIMU)
  //   parseFlyskyData(&rxBuffer, byte);
  // #endif
  // if (rxBuffer.valid) {
  //   rfRxCount++;
  //   rxBuffer.valid = 0;
  //   uint8_t *pt = (uint8_t*)&rxBuffer;
  //   //rxBuffer.head = HALL_PROTOLO_HEAD;
  //   pt[rxBuffer.length + 3] = rxBuffer.checkSum & 0xFF;
  //   pt[rxBuffer.length + 4] = rxBuffer.checkSum >> 8;

  //   if((DEBUG_RF_FRAME_PRINT & RF_FRAME_ONLY)) {
  //       TRACE("RF: %02X %02X %02X ...%04X; CRC:%04X", pt[0], pt[1], pt[2],
  //       rxBuffer.checkSum, calc_crc16(pt, rxBuffer.length+3));
  //   }

  //   //FW UPDATE done
  //   if (0x01 == rxBuffer.length && (0x05 == rxBuffer.data[0] || 0x06 ==
  //   rxBuffer.data[0])) {
  //       setFlyskyState(STATE_INIT);
  //       rf_info.fw_state = 0;
  //   }
  //   usbDownloadTransmit(pt, rxBuffer.length + 5);
  // }
  if (byte == END && *len > 0) {
    parseResponse(buffer, *len);
    *len = 0;
  } else {
    if (byte == ESC) {
      intmodulePulsesData.flysky.esc_state = 1;
    } else {
      if (intmodulePulsesData.flysky.esc_state) {
        intmodulePulsesData.flysky.esc_state = 0;
        if (byte == ESC_END)
          byte = END;
        else if (byte == ESC_ESC)
          byte = ESC;
      }
      buffer[(*len)++] = byte;
      if (*len >= TELEMETRY_RX_PACKET_SIZE) {
        // TODO buffer is full, log an error?
        *len = 0;
      }
    }
  }
}

void resetPulsesAFHDS2()
{
  NV14internalModuleFwVersion = 0;
  intmodulePulsesData.flysky.frame_index = 1;
  setFlyskyState(STATE_SET_TX_POWER);
  intmodulePulsesData.flysky.timeout = 0;
  intmodulePulsesData.flysky.esc_state = 0;
  uint16_t rx_freq = gRomData.rx_freq[0];
  rx_freq += (gRomData.rx_freq[1] * 256);
  if (50 > rx_freq || 400 < rx_freq) {
    gRomData.rx_freq[0] = 50;
  }
}

void setupPulsesAFHDS2()
{
  putFlySkyFrameHeader();
  if (intmodulePulsesData.flysky.state == STATE_DISCONNECT) {
    TRACE("STATE_DISCONNECT");
    putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, CMD_SET_RECEIVER_ID);
    putFlySkyFrameBytes(emptyRxID, 4);
  } else if (intmodulePulsesData.flysky.state < STATE_SEND_CHANNELS) {
    if (++intmodulePulsesData.flysky.timeout >=
        FLYSKY_MODULE_TIMEOUT / FLYSKY_PERIOD) {
      intmodulePulsesData.flysky.timeout = 0;
      switch (intmodulePulsesData.flysky.state) {
        case STATE_INIT: {
          putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, CMD_RF_INIT);
        } break;
        case STATE_BIND: {
          putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, CMD_BIND);
          putFlySkyFrameByte(rf_info.bind_power);
          uint32_t txID =
              0;  // internal module firmware does not allow changing TX id
          putFlySkyFrameBytes((uint8_t*)(&txID), 4);
        } break;
        case STATE_SET_RECEIVER_ID: {
          putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, CMD_SET_RECEIVER_ID);
          putFlySkyFrameBytes(gRomData.rx_id, 4);
        } break;
        case STATE_GET_RECEIVER_CONFIG: {
          putFlySkyFrameCmd(FRAME_TYPE_ANSWER, CMD_RF_GET_CONFIG);
          putFlySkyFrameByte(AfhdsPwmMode);   // 00:PWM, 01:PPM
          putFlySkyFrameByte(AfhdsIbusMode);  // 00:I-BUS, 01:S-BUS
          putFlySkyFrameByte(
              gRomData.rx_freq[0] < 50
                  ? 50
                  : gRomData.rx_freq[0]);  // receiver servo freq bit[7:0]
          putFlySkyFrameByte(
              gRomData.rx_freq[1]);  // receiver servo freq bit[15:8]
          setFlyskyState(STATE_INIT);
        } break;
        case STATE_SET_TX_POWER: {
          uint8_t power =
              moduleState[INTERNAL_MODULE].mode == MODULE_MODE_RANGECHECK ? 0
              : gRomData.rfPower                                          ? 170
                                                                          : 90;
          putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, CMD_SET_TX_POWER);
          putFlySkyFrameByte(power);
        } break;
        case STATE_SET_RANGE_TEST: {
          putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, CMD_TEST_RANGE);
          putFlySkyFrameByte(moduleState[INTERNAL_MODULE].mode ==
                             MODULE_MODE_RANGECHECK);
        } break;
        case STATE_RANGE_TEST_RUNNING: {
          if (moduleState[INTERNAL_MODULE].mode != MODULE_MODE_RANGECHECK) {
            // this will send stop command
            setFlyskyState(STATE_SET_RANGE_TEST);
          }
        } break;
        case STATE_SET_RX_PWM_PPM: {
          putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, CMD_SET_RX_PWM_PPM);
          putFlySkyFrameByte(AfhdsPwmMode);  // 00:PWM, 01:PPM
        } break;
        case STATE_SET_RX_IBUS_SBUS: {
          putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, CMD_SET_RX_IBUS_SBUS);
          putFlySkyFrameByte(AfhdsIbusMode);  // 0x00:I-BUS, 0x01:S-BUS
        } break;
        case STATE_SET_RX_FREQUENCY: {
          putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, CMD_SET_RX_SERVO_FREQ);
          putFlySkyFrameBytes(gRomData.rx_freq, 2);
        } break;
        case STATE_UPDATE_RF_PROTOCOL: {
          putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, CMD_SET_RF_PROTOCOL);
          putFlySkyFrameByte(
              rf_info.protocol);  // 0x00:AFHDS1 0x01:AFHDS2 0x02:AFHDS2A
        } break;
        case STATE_UPDATE_RX_FIRMWARE: {
          putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, CMD_UPDATE_RX_FIRMWARE);
        } break;
        case STATE_UPDATE_RF_FIRMWARE: {
          putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, CMD_UPDATE_RF_FIRMWARE);
        } break;
        case STATE_GET_RX_VERSION_INFO: {
          putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, CMD_GET_VERSION_INFO);
          putFlySkyFrameByte(FLYSKY_RX_FIRMWARE);
        } break;
        case STATE_GET_FW_VERSION_INIT:
        case STATE_GET_RF_VERSION_INFO: {
          putFlySkyFrameCmd(FRAME_TYPE_REQUEST_ACK, CMD_GET_VERSION_INFO);
          putFlySkyFrameByte(FLYSKY_RF_FIRMWARE);
        } break;
        case STATE_IDLE:
          initFlySkyArray();
          break;
        default:
          setFlyskyState(STATE_INIT);
          initFlySkyArray();
          if ((DEBUG_RF_FRAME_PRINT & TX_FRAME_ONLY)) {
            TRACE("State back to INIT\r\n");
          }
          return;
      }
    } else {
      initFlySkyArray();
      return;
    }
  } else {
    if (moduleState[INTERNAL_MODULE].mode == MODULE_MODE_BIND)
      moduleState[INTERNAL_MODULE].mode = MODULE_MODE_NORMAL;
    putFlySkySendChannelData();
  }

  putFlySkyFrameFooter();

  if (intmodulePulsesData.flysky.state < STATE_SEND_CHANNELS) {
    // uint8_t size = intmodulePulsesData.flysky.ptr -
    // intmodulePulsesData.flysky.pulses;
    // debugFrame(intmodulePulsesData.flysky.pulses, size);
  }
  if ((DEBUG_RF_FRAME_PRINT & TX_FRAME_ONLY)) {
    /* print each command, except channel data by interval */
    uint8_t* data = intmodulePulsesData.flysky.pulses;
    if (data[3] != CMD_SEND_CHANNEL_DATA || (set_loop_cnt++ % 100 == 0)) {
      uint8_t size = intmodulePulsesData.flysky.ptr - data;
      TRACE_NOCRLF("TX(State%0d)%0dB:", intmodulePulsesData.flysky.state, size);
      for (int idx = 0; idx < size; idx++) {
        TRACE_NOCRLF(" %02X", data[idx]);
      }
      TRACE_NOCRLF(";" CRLF);
    }
  }
}

// void usbDownloadTransmit(uint8_t *buffer, uint32_t size)
// {
//     if (USB_SERIAL_MODE != getSelectedUsbMode()) return;
//     buffer[0] = HALL_PROTOLO_HEAD;
//     for (uint32_t idx = 0; idx < size; idx++)
//     {
// #if !defined(SIMU)
//         usbSerialPutc(buffer[idx]);
// #endif
//     }
// }
