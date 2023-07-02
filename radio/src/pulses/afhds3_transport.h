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

#include "afhds3_module.h"
#include "definitions.h"
#include "hal/module_port.h"

namespace afhds3
{

enum FRAME_TYPE : uint8_t {
  REQUEST_GET_DATA = 0x01,         // Get data response: ACK + DATA
  REQUEST_SET_EXPECT_DATA = 0x02,  // Set data response: ACK + DATA
  REQUEST_SET_EXPECT_ACK = 0x03,   // Set data response: ACK
  REQUEST_SET_NO_RESP = 0x05,      // Set data response: none
  RESPONSE_DATA = 0x10,            // Response ACK + DATA
  RESPONSE_ACK = 0x20,             // Response ACK
  NOT_USED = 0xff
};

enum COMMAND : uint8_t {
  MODULE_READY = 0x01,
  MODULE_STATE = 0x02,
  MODULE_MODE = 0x03,
  MODULE_SET_CONFIG = 0x04,
  MODULE_GET_CONFIG = 0x06,
  CHANNELS_FAILSAFE_DATA = 0x07,
  TELEMETRY_DATA = 0x09,
  SEND_COMMAND = 0x0C,
  COMMAND_RESULT = 0x0D,
  MODULE_VERSION = 0x20,
  MODEL_ID = 0x2F,
  VIRTUAL_FAILSAFE = 0x99,  // virtual command used to trigger failsafe
  UNDEFINED = 0xFF
};

#define RX_CMD_TX_PWR                   ( 0x2013 )
#define RX_CMD_FAILSAFE_VALUE           ( 0x6011 )
#define RX_CMD_FAILSAFE_TIME            ( 0x6012 )
#define RX_CMD_RSSI_CHANNEL_SETUP       ( 0x602B )
#define RX_CMD_RANGE                    ( 0x7013 )
#define RX_CMD_GET_CAPABILITIES         ( 0x7015 )
#define RX_CMD_OUT_PWM_PPM_MODE         ( 0x7016 )   //PWM or PPM
#define RX_CMD_FREQUENCY_V0             ( 0x7017 )
#define RX_CMD_PORT_TYPE_V1             ( 0x7027 )
#define RX_CMD_FREQUENCY_V1             ( 0x7028 )
#define RX_CMD_FREQUENCY_V1_2           ( 0x7028 )
#define RX_CMD_BUS_TYPE_V0              ( 0x7018 ) //I-BUS/S-BUS
#define RX_CMD_IBUS_SETUP               ( 0x7019 )
#define RX_CMD_IBUS_DIRECTION           ( 0x7020 ) //IBUS INPUT or OUTPUT
#define RX_CMD_BUS_FAILSAFE             ( 0x702A )
#define RX_CMD_GET_VERSION              ( 0x701F )

enum RX_CMDRESULT: uint8_t
{
  RXSUCCESS=0,
  RXTIMEOUT=1,
  RXERROR=2, //not support
  RXINVALID=3,
};

// one byte frames for request queue
struct Frame {
  enum COMMAND command;
  enum FRAME_TYPE frameType;
  uint8_t payload;
  uint8_t frameNumber;
  bool useFrameNumber;
  uint8_t payloadSize;
};

union AfhdsFrameData;

PACK(struct AfhdsFrame {
  uint8_t startByte;
  uint8_t address;
  uint8_t frameNumber;
  uint8_t frameType;
  uint8_t command;
  uint8_t value;

  AfhdsFrameData* GetData()
  {
    return reinterpret_cast<AfhdsFrameData*>(&value);
  }
});

// simple fifo implementation because Pulses is used as member of union and can
// not be non trivial type
struct CommandFifo {
  Frame commandFifo[8];
  volatile uint32_t setIndex;
  volatile uint32_t getIndex;

  void clearCommandFifo();
  Frame* getCommand();

  inline uint32_t nextIndex(uint32_t idx) const
  {
    return (idx + 1) & (sizeof(commandFifo) / sizeof(commandFifo[0]) - 1);
  }

  inline uint32_t prevIndex(uint32_t idx) const
  {
    if (idx == 0) {
      return (sizeof(commandFifo) / sizeof(commandFifo[0]) - 1);
    }
    return (idx - 1);
  }

  inline bool isEmpty() const { return (getIndex == setIndex); }

  inline void skip() { getIndex = nextIndex(getIndex); }

  void enqueueACK(COMMAND command, uint8_t frameNumber);

  void enqueue(COMMAND command, FRAME_TYPE frameType, bool useData,
               uint8_t byteContent);
};

struct FrameTransport {
  uint8_t* trsp_buffer;
  uint8_t* data_ptr;

  uint8_t crc;
  uint8_t frameAddress;
  // uint8_t timeout;
  uint8_t esc_state;

  void init(void* buffer, uint8_t fAddr);
  void clear();

  void putByte(uint8_t b);
  void putBytes(uint8_t* data, int length);

  void putFrame(COMMAND command, FRAME_TYPE frameType, uint8_t* data,
                uint8_t dataLength, uint8_t frameIndex);

  uint32_t getFrameSize();

  bool processTelemetryData(uint8_t byte, uint8_t* rxBuffer,
                            uint8_t& rxBufferCount, uint8_t maxSize);
};

class Transport
{
  enum State { UNKNOWN = 0, SENDING_COMMAND, AWAITING_RESPONSE, IDLE };

  etx_module_state_t* mod_st;

  FrameTransport trsp;
  CommandFifo fifo;

  /**
   * Internal operation state one of UNKNOWN, SENDING_COMMAND,
   * AWAITING_RESPONSE, IDLE
   * Used to avoid sending commands when not allowed to
   */
  State operationState;

  /**
   * Current frame index
   */
  uint8_t frameIndex;

  /**
   * Actual repeat count for requested command/operation - incremented by every
   * attempt sending anything
   */
  uint16_t repeatCount;

  bool handleReply(uint8_t* buffer, uint8_t len);

 public:
  void init(void* buffer, etx_module_state_t* mod_st, uint8_t fAddr);

  void clear();

  void putFrame(COMMAND command, FRAME_TYPE frameType, uint8_t* data = nullptr,
                 uint8_t dataLength = 0);

  void enqueue(COMMAND command, FRAME_TYPE frameType, bool useData = false,
               uint8_t byteContent = 0);

  void sendBuffer();

  /**
   * Process retransmissions
   * error: transport is in error state
   * @return true if something was just sent
   */
  bool handleRetransmissions(bool& error);

  /**
   * Process queue backlog
   * @return true if something was just sent
   */
  bool processQueue();

  bool processTelemetryData(uint8_t byte, uint8_t* rxBuffer,
                            uint8_t& rxBufferCount, uint8_t maxSize);
};
};  // namespace afhds3
