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

#include "edgetx.h"

#include "flysky.h"
#include "telemetry/flysky_nv14.h"

#define IS_VALID_COMMAND_ID(id) ((id) < CMD_LAST)

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

static uint8_t lastState = STATE_IDLE;
static uint32_t set_loop_cnt = 0;

static uint8_t _flysky_state;
static uint8_t _flysky_frame_index;
static uint8_t _flysky_crc;
static uint8_t _flysky_timeout;
static uint8_t _esc_state;

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
void afhds2Command(uint8_t*& p_buf, uint8_t type, uint8_t cmd);

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

void setFlyskyState(uint8_t state)
{
  _flysky_state = state;
}

inline void initFlySkyCRC()
{
  _flysky_crc = 0;
}

inline void putFlySkyByte(uint8_t*& p_buf, uint8_t byte)
{
  if (END == byte) {
    *p_buf++ = ESC;
    *p_buf++ = ESC_END;
  } else if (ESC == byte) {
    *p_buf++ = ESC;
    *p_buf++ = ESC_ESC;
  } else {
    *p_buf++ = byte;
  }
}

inline void putFlySkyFrameByte(uint8_t*& p_buf, uint8_t byte)
{
  _flysky_crc += byte;
  putFlySkyByte(p_buf, byte);
}

inline void putFlySkyFrameCmd(uint8_t*& p_buf, uint8_t type, uint8_t cmd)
{
  _flysky_crc += type + cmd;
  putFlySkyByte(p_buf, type);
  putFlySkyByte(p_buf, cmd);
}

inline void putFlySkyFrameBytes(uint8_t*& p_buf, uint8_t* data, int length)
{
  for (int i = 0; i < length; i++) {
    _flysky_crc += data[i];
    putFlySkyByte(p_buf, data[i]);
  }
}

inline void putFlySkyFrameHeader(uint8_t*& p_buf)
{
  initFlySkyCRC();
  *p_buf++ = END;
  putFlySkyFrameByte(p_buf, _flysky_frame_index);
}

inline void putFlySkyFrameFooter(uint8_t*& p_buf)
{
  if (++_flysky_frame_index == 0) {
    _flysky_frame_index = 1;
  }
  putFlySkyByte(p_buf, _flysky_crc ^ 0xff);
  *p_buf++ = END;
}

void afhds2Command(uint8_t*& p_buf, uint8_t type, uint8_t cmd)
{
  putFlySkyFrameHeader(p_buf);
  putFlySkyFrameCmd(p_buf, type, cmd);
  putFlySkyFrameFooter(p_buf);
}

inline void putFlySkySendChannelData(uint8_t*& p_buf)
{
  uint16_t pulseValue = 0;
  uint8_t channels_start = g_model.moduleData[INTERNAL_MODULE].channelsStart;
  uint8_t channels_last =
      channels_start + 8 + g_model.moduleData[INTERNAL_MODULE].channelsCount;
  putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_NACK, CMD_SEND_CHANNEL_DATA);

  if (moduleState[INTERNAL_MODULE].counter-- == 0) {
    moduleState[INTERNAL_MODULE].counter = FAILSAVE_SEND_COUNTER_MAX;
    putFlySkyFrameByte(p_buf, FLYSKY_CHANNEL_DATA_FAILSAFE);
    putFlySkyFrameByte(p_buf, channels_last - channels_start);
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
      putFlySkyFrameByte(p_buf, pulseValue & 0xff);
      putFlySkyFrameByte(p_buf, pulseValue >> 8);
    }
    if (DEBUG_RF_FRAME_PRINT & RF_FRAME_ONLY) {
      TRACE("------FAILSAFE------");
    }
  } else {
    putFlySkyFrameByte(p_buf, FLYSKY_CHANNEL_DATA_NORMAL);
    putFlySkyFrameByte(p_buf, channels_last - channels_start);
    for (uint8_t channel = channels_start; channel < channels_last; channel++) {
      int channelValue =
          channelOutputs[channel] + 2 * PPM_CH_CENTER(channel) - 2 * PPM_CENTER;
      pulseValue = limit<uint16_t>(0, 988 + ((channelValue + 1024) / 2), 0xfff);
      putFlySkyFrameByte(p_buf, pulseValue & 0xff);
      putFlySkyFrameByte(p_buf, pulseValue >> 8);
    }
  }
}

void putFlySkyUpdateFirmwareStart(uint8_t*& p_buf, uint8_t cmd)
{
  putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, cmd);
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
      ((_flysky_frame_index - 1) != resp->frame_number)) {
    return;
  } else if (resp->frame_type == FRAME_TYPE_REQUEST_ACK) {
    _flysky_frame_index = resp->frame_number;
  }

  debugFrame(&resp->command_id, dataLen - 2);
  switch (resp->command_id) {
    default:
      if (moduleState[INTERNAL_MODULE].mode == MODULE_MODE_NORMAL &&
          _flysky_state >= STATE_IDLE) {
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
      _flysky_timeout = FLYSKY_MODULE_TIMEOUT;
      break;
    case CMD_RX_SENSOR_DATA:
      flySkyNv14ProcessTelemetryPacket(&resp->value, dataLen - 3);
      if (moduleState[INTERNAL_MODULE].mode == MODULE_MODE_NORMAL &&
          _flysky_state >= STATE_IDLE) {
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
      if (_flysky_state == STATE_GET_FW_VERSION_INIT) {
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

void processInternalFlySkyTelemetryData(uint8_t byte, uint8_t* buffer, uint8_t* len)
{
  if (byte == END && *len > 0) {
    parseResponse(buffer, *len);
    *len = 0;
  } else {
    if (byte == ESC) {
      _esc_state = 1;
    } else {
      if (_esc_state) {
        _esc_state = 0;
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
  _flysky_frame_index = 1;
  setFlyskyState(STATE_SET_TX_POWER);
  _flysky_timeout = 0;
  _esc_state = 0;
  uint16_t rx_freq = gRomData.rx_freq[0];
  rx_freq += (gRomData.rx_freq[1] * 256);
  if (50 > rx_freq || 400 < rx_freq) {
    gRomData.rx_freq[0] = 50;
  }
}

void setupPulsesAFHDS2(uint8_t*& p_buf)
{
  auto buffer_start = p_buf;

  putFlySkyFrameHeader(p_buf);
  if (_flysky_state == STATE_DISCONNECT) {
    TRACE("STATE_DISCONNECT");
    putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, CMD_SET_RECEIVER_ID);
    putFlySkyFrameBytes(p_buf, emptyRxID, 4);
  } else if (_flysky_state < STATE_SEND_CHANNELS) {
    if (++_flysky_timeout >=
        FLYSKY_MODULE_TIMEOUT / FLYSKY_PERIOD) {
      _flysky_timeout = 0;
      switch (_flysky_state) {
        case STATE_INIT: {
          putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, CMD_RF_INIT);
        } break;
        case STATE_BIND: {
          putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, CMD_BIND);
          putFlySkyFrameByte(p_buf, rf_info.bind_power);
          uint32_t txID =
              0;  // internal module firmware does not allow changing TX id
          putFlySkyFrameBytes(p_buf, (uint8_t*)(&txID), 4);
        } break;
        case STATE_SET_RECEIVER_ID: {
          putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, CMD_SET_RECEIVER_ID);
          putFlySkyFrameBytes(p_buf, gRomData.rx_id, 4);
        } break;
        case STATE_GET_RECEIVER_CONFIG: {
          putFlySkyFrameCmd(p_buf, FRAME_TYPE_ANSWER, CMD_RF_GET_CONFIG);
          putFlySkyFrameByte(p_buf, AfhdsPwmMode);   // 00:PWM, 01:PPM
          putFlySkyFrameByte(p_buf, AfhdsIbusMode);  // 00:I-BUS, 01:S-BUS
          putFlySkyFrameByte(p_buf,
              gRomData.rx_freq[0] < 50
                  ? 50
                  : gRomData.rx_freq[0]);  // receiver servo freq bit[7:0]
          putFlySkyFrameByte(p_buf,
              gRomData.rx_freq[1]);  // receiver servo freq bit[15:8]
          setFlyskyState(STATE_INIT);
        } break;
        case STATE_SET_TX_POWER: {
          uint8_t power =
              moduleState[INTERNAL_MODULE].mode == MODULE_MODE_RANGECHECK ? 0
              : gRomData.rfPower                                          ? 170
                                                                          : 90;
          putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, CMD_SET_TX_POWER);
          putFlySkyFrameByte(p_buf, power);
        } break;
        case STATE_SET_RANGE_TEST: {
          putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, CMD_TEST_RANGE);
          putFlySkyFrameByte(p_buf, moduleState[INTERNAL_MODULE].mode ==
                             MODULE_MODE_RANGECHECK);
        } break;
        case STATE_RANGE_TEST_RUNNING: {
          if (moduleState[INTERNAL_MODULE].mode != MODULE_MODE_RANGECHECK) {
            // this will send stop command
            setFlyskyState(STATE_SET_RANGE_TEST);
          }
        } break;
        case STATE_SET_RX_PWM_PPM: {
          putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, CMD_SET_RX_PWM_PPM);
          putFlySkyFrameByte(p_buf, AfhdsPwmMode);  // 00:PWM, 01:PPM
        } break;
        case STATE_SET_RX_IBUS_SBUS: {
          putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, CMD_SET_RX_IBUS_SBUS);
          putFlySkyFrameByte(p_buf, AfhdsIbusMode);  // 0x00:I-BUS, 0x01:S-BUS
        } break;
        case STATE_SET_RX_FREQUENCY: {
          putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, CMD_SET_RX_SERVO_FREQ);
          putFlySkyFrameBytes(p_buf, gRomData.rx_freq, 2);
        } break;
        case STATE_UPDATE_RF_PROTOCOL: {
          putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, CMD_SET_RF_PROTOCOL);
          putFlySkyFrameByte(p_buf,
              rf_info.protocol);  // 0x00:AFHDS1 0x01:AFHDS2 0x02:AFHDS2A
        } break;
        case STATE_UPDATE_RX_FIRMWARE: {
          putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, CMD_UPDATE_RX_FIRMWARE);
        } break;
        case STATE_UPDATE_RF_FIRMWARE: {
          putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, CMD_UPDATE_RF_FIRMWARE);
        } break;
        case STATE_GET_RX_VERSION_INFO: {
          putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, CMD_GET_VERSION_INFO);
          putFlySkyFrameByte(p_buf, FLYSKY_RX_FIRMWARE);
        } break;
        case STATE_GET_FW_VERSION_INIT:
        case STATE_GET_RF_VERSION_INFO: {
          putFlySkyFrameCmd(p_buf, FRAME_TYPE_REQUEST_ACK, CMD_GET_VERSION_INFO);
          putFlySkyFrameByte(p_buf, FLYSKY_RF_FIRMWARE);
        } break;
        case STATE_IDLE:
          initFlySkyCRC();
          break;
        default:
          setFlyskyState(STATE_INIT);
          initFlySkyCRC();
          if ((DEBUG_RF_FRAME_PRINT & TX_FRAME_ONLY)) {
            TRACE("State back to INIT\r\n");
          }
          return;
      }
    } else {
      initFlySkyCRC();
      return;
    }
  } else {
    if (moduleState[INTERNAL_MODULE].mode == MODULE_MODE_BIND)
      moduleState[INTERNAL_MODULE].mode = MODULE_MODE_NORMAL;
    putFlySkySendChannelData(p_buf);
  }

  putFlySkyFrameFooter(p_buf);

  if (_flysky_state < STATE_SEND_CHANNELS) {
    // uint8_t size = p_buf - buffer_start;
    // debugFrame(buffer_start, size);
  }
  if ((DEBUG_RF_FRAME_PRINT & TX_FRAME_ONLY)) {
    /* print each command, except channel data by interval */
    uint8_t* data = buffer_start;
    if (data[3] != CMD_SEND_CHANNEL_DATA || (set_loop_cnt++ % 100 == 0)) {
      uint8_t size = p_buf - data;
      TRACE_NOCRLF("TX(State%0d)%0dB:", _flysky_state, size);
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
