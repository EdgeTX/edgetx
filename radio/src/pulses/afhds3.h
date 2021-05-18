/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "libopenui/src/bitfield.h"
#include "definitions.h"
#include "dataconstants.h"
#include "opentx_types.h"
#include "myeeprom.h"
#include "opentx_helpers.h"
#include "pulses_common.h"
#include <cstring>
#include "fifo.h"

#define AFHDS_MAX_PULSES 72
//max number of transitions measured so far 290 + 10%
#define AFHDS_MAX_PULSES_TRANSITIONS 320
//#define AFHDS3_SLOW

#if defined(EXTMODULE_USART) && defined(EXTMODULE_TX_INVERT_GPIO)
#define AFHDS3_BAUDRATE        1500000
#define AFHDS3_COMMAND_TIMEOUT 5
#elif !defined(AFHDS3_SLOW)
// 1s = 1 000 000 us
// 1000000/115200 = 8,68 us
// actual timer is ticking in 0,5 us  = 8,68*2 = 17,36 because it must be integer take 17
// difference 1000000 / x = 8.5 -->  x = 117 647 = difference = 1.2 %
// allowed half a bit difference on the last bit -- should be fine
#define AFHDS3_BAUDRATE        115200
//Because timer is ticking with 0.5us
#define BITLEN_AFHDS           (17)

#define AFHDS3_COMMAND_TIMEOUT 15
#else
// 1000000/57600 = 17,36 us
#define AFHDS3_BAUDRATE        57600
// 64* 86 = 11 110
#define AFHDS3_COMMAND_TIMEOUT 20
#define BITLEN_AFHDS           (35)
#endif

#define AFHDS3_FRAME_HALF_US AFHDS3_COMMAND_TIMEOUT * 2000

//get channel value outside of afhds3 namespace
int32_t getChannelValue(uint8_t channel);

//use uint16_t instead of pulse_duration_t
namespace afhds3
{
enum AfhdsSpecialChars
{
  END = 0xC0,             //Frame end
  START = END,
  ESC_END = 0xDC,         //Escaped frame end - in case END occurs in fame then ESC ESC_END must be used
  ESC = 0xDB,             //Escaping character
  ESC_ESC = 0xDD,         //Escaping character in case ESC occurs in fame then ESC ESC_ESC  must be used
};

struct Data
{
  uint32_t pulsesSize;
#if defined(EXTMODULE_USART) && defined(EXTMODULE_TX_INVERT_GPIO)
  uint8_t pulses[AFHDS_MAX_PULSES];
  uint8_t * ptr;
#else
  uint16_t pulses[AFHDS_MAX_PULSES_TRANSITIONS];
  uint32_t total;
#endif

  uint8_t frame_index;
  uint8_t crc;
  uint8_t state;
  uint8_t timeout;
  uint8_t esc_state;

  void reset()
  {
    pulsesSize = 0;
  }

#if defined(EXTMODULE_USART) && defined(EXTMODULE_TX_INVERT_GPIO)
  void sendByte(uint8_t b)
  {
    *ptr++ = b;
  }

  const uint8_t* getData()
  {
    return pulses;
  }

  void flush()
  {
  }
#else
  inline void _send_level(uint16_t v)
  {
    if (pulsesSize >= AFHDS_MAX_PULSES_TRANSITIONS) {
      return;
    }
    pulses[pulsesSize++] = v;
  }
  void sendByte(uint8_t b)
  {
    if (pulsesSize >= AFHDS_MAX_PULSES_TRANSITIONS) {
      return;
    }
    //use 8n1
    //parity: If the parity is enabled, then the MSB bit of the data to be transmitted is changed by the parity bit
    //start is always 0
    bool level = 0;
    uint16_t length = BITLEN_AFHDS; //start bit
    for (uint8_t i = 0; i <= 8; i++)
    { //8 data bits + Stop=1
      bool next_level = b & 1;
      if (level == next_level) {
        length += BITLEN_AFHDS;
      }
      else {
        _send_level(length);
        length = BITLEN_AFHDS;
        level = next_level;
      }
      b = (b >> 1) | 0x80; // shift left to get next bit, fill msb with stop bit - needed just once
    }
    _send_level(length); //last bit (stop)

  }
  //add remaining time of frame
  void flush()
  {
    pulses[pulsesSize - 1] = 60000;
  }

  const uint16_t * getData()
  {
    return pulses;
  }
#endif

  uint32_t getSize() const
  {
#if defined(EXTMODULE_USART) && defined(EXTMODULE_TX_INVERT_GPIO)
    return ptr - pulses;
#else
    return pulsesSize;
#endif
  }
};

enum DeviceAddress
{
  TRANSMITTER = 0x01,
  MODULE = 0x03,
};

enum FRAME_TYPE: uint8_t
{
  REQUEST_GET_DATA = 0x01,  //Get data response: ACK + DATA
  REQUEST_SET_EXPECT_DATA = 0x02,  //Set data response: ACK + DATA
  REQUEST_SET_EXPECT_ACK = 0x03,  //Set data response: ACK
  REQUEST_SET_NO_RESP = 0x05,  //Set data response: none
  RESPONSE_DATA = 0x10,  //Response ACK + DATA
  RESPONSE_ACK = 0x20,  //Response ACK
  NOT_USED = 0xff
};

enum COMMAND: uint8_t
{
  MODULE_READY = 0x01,
  MODULE_STATE = 0x02,
  MODULE_MODE = 0x03,
  MODULE_SET_CONFIG = 0x04,
  MODULE_GET_CONFIG = 0x06,
  CHANNELS_FAILSAFE_DATA = 0x07,
  TELEMETRY_DATA = 0x09,
  SEND_COMMAND = 0x0C,
  COMMAND_RESULT = 0x0D,
  MODULE_POWER_STATUS = 0x0F,
  MODULE_VERSION = 0x1F,
  VIRTUAL_FAILSAFE = 0x99, // virtual command used to trigger failsafe
  UNDEFINED = 0xFF
};

enum COMMAND_DIRECTION
{
  RADIO_TO_MODULE = 0,
  MODULE_TO_RADIO = 1
};

enum DATA_TYPE
{
  READY_DT,  // 8 bytes 0x01 Not ready 0x02 Ready
  STATE_DT,  // See MODULE_STATE
  MODE_DT,
  MOD_CONFIG_DT,
  CHANNELS_DT,
  TELEMETRY_DT,
  MODULE_POWER_DT,
  MODULE_VERSION_DT,
  EMPTY_DT,
};

//enum used by command response -> translate to ModuleState
enum MODULE_READY_E
{
  MODULE_STATUS_UNKNOWN = 0x00,
  MODULE_STATUS_NOT_READY = 0x01,
  MODULE_STATUS_READY = 0x02
};

enum ModuleState
{
  STATE_NOT_READY = 0x00, //virtual
  STATE_HW_ERROR = 0x01,
  STATE_BINDING = 0x02,
  STATE_SYNC_RUNNING = 0x03,
  STATE_SYNC_DONE = 0x04,
  STATE_STANDBY = 0x05,
  STATE_UPDATING_WAIT = 0x06,
  STATE_UPDATING_MOD = 0x07,
  STATE_UPDATING_RX = 0x08,
  STATE_UPDATING_RX_FAILED = 0x09,
  STATE_RF_TESTING = 0x0a,
  STATE_READY = 0x0b,      //virtual
  STATE_HW_TEST = 0xff,
};

//used for set command
enum MODULE_MODE_E
{
  STANDBY = 0x01,
  BIND = 0x02,  //after bind module will enter run mode
  RUN = 0x03,
  RX_UPDATE = 0x04, //after successful update module will enter standby mode, otherwise hw error will be raised
  MODULE_MODE_UNKNOWN = 0xFF
};

enum CMD_RESULT
{
  FAILURE = 0x01,
  SUCCESS = 0x02,
};

#define MIN_FREQ 50
#define MAX_FREQ 400
#define MAX_CHANNELS 18
#define FAILSAFE_KEEP_LAST 0x8000
#define FAILSAFE_MIN -15000
#define FAILSAFE_MAX 15000

enum BIND_POWER
{
  MIN_16dBm = 0x00,
  BIND_POWER_FIRST = MIN_16dBm,
  MIN_5dBm = 0x01,
  MIN_0dbm = 0x02,
  PLUS_5dBm = 0x03,
  PLUS_14dBm = 0x04,
  BIND_POWER_LAST = PLUS_14dBm,
};

enum RUN_POWER
{
  PLUS_15bBm = 0x00,
  RUN_POWER_FIRST = PLUS_15bBm,
  PLUS_20bBm = 0x01,
  PLUS_27dbm = 0x02,
  PLUS_30dBm = 0x03,
  PLUS_33dBm = 0x04,
  RUN_POWER_LAST = PLUS_33dBm,
};

enum EMI_STANDARD
{
  FCC = 0x00,
  CE = 0x01
};

enum TELEMETRY
{
  TELEMETRY_DISABLED = 0x00,
  TELEMETRY_ENABLED = 0x01
};

enum PULSE_MODE
{
  PWM_MODE = 0x00,
  PPM_MODE = 0x01,
};

enum SERIAL_MODE
{
  IBUS = 0x00,
  SBUS_MODE = 0x02
};

PACK(struct Config_s {
  uint8_t bindPower;
  uint8_t runPower;
  uint8_t emiStandard;
  uint8_t telemetry;
  uint16_t pwmFreq;
  uint8_t pulseMode;
  uint8_t serialMode;
  uint8_t channelCount;
  uint16_t failSafeTimout;
  int16_t failSafeMode[MAX_CHANNELS];
});

union Config_u
{
  Config_s config;
  uint8_t buffer[sizeof(Config_s)];
};

enum CHANNELS_DATA_MODE
{
  CHANNELS = 0x01,
  FAIL_SAFE = 0x02,
};

PACK(struct ChannelsData {
  uint8_t mode;
  uint8_t channelsNumber;
  int16_t data[MAX_CHANNELS];
});

union ChannelsData_u
{
  ChannelsData data;
  uint8_t buffer[sizeof(ChannelsData)];
};

PACK(struct TelemetryData
{
  uint8_t sensorType;
  uint8_t length;
  uint8_t type;
  uint8_t semsorID;
  uint8_t data[8];
});

enum MODULE_POWER_SOURCE
{
  INTERNAL = 0x01,
  EXTERNAL = 0x02,
};

PACK(struct ModuleVersion
{
  uint32_t productNumber;
  uint32_t hardwereVersion;
  uint32_t bootloaderVersion;
  uint32_t firmwareVersion;
  uint32_t rfVersion;
});

PACK(struct CommandResult_s
{
  uint16_t command;
  uint8_t result;
  uint8_t respLen;
});

union AfhdsFrameData
{
  uint8_t value;
  Config_s Config;
  ChannelsData Channels;
  TelemetryData Telemetry;
  ModuleVersion Version;
  CommandResult_s CommandResult;
};

PACK(struct AfhdsFrame
{
  uint8_t startByte;
  uint8_t address;
  uint8_t frameNumber;
  uint8_t frameType;
  uint8_t command;
  uint8_t value;

  AfhdsFrameData * GetData()
  {
    return reinterpret_cast<AfhdsFrameData*>(&value);
  }
});

#define FRM302_STATUS 0x56

enum State
{
  UNKNOWN = 0,
  SENDING_COMMAND,
  AWAITING_RESPONSE,
  IDLE
};

// one byte frames for request queue
struct Frame
{
  enum COMMAND command;
  enum FRAME_TYPE frameType;
  uint8_t payload;
  uint8_t frameNumber;
  bool useFrameNumber;
  uint8_t payloadSize;
};

// simple fifo implementation because Pulses is used as member of union and can not be non trivial type
struct CommandFifo
{
  Frame commandFifo[8];
  volatile uint32_t setIndex;
  volatile uint32_t getIndex;

  void clearCommandFifo();

  inline uint32_t nextIndex(uint32_t idx) const
  {
    return (idx + 1) & (sizeof(commandFifo) / sizeof(commandFifo[0]) - 1);
  }

  inline uint32_t prevIndex(uint32_t idx) const
  {
     if (idx == 0)
     {
       return (sizeof(commandFifo) / sizeof(commandFifo[0]) - 1);
     }
     return (idx - 1);
  }

  inline bool isEmpty() const
  {
    return (getIndex == setIndex);
  }

  inline void skip()
  {
    getIndex = nextIndex(getIndex);
  }

  void enqueueACK(COMMAND command, uint8_t frameNumber);

  void enqueue(COMMAND command, FRAME_TYPE frameType, bool useData = false, uint8_t byteContent = 0);

};

void processTelemetryData(uint8_t module, uint8_t byte, uint8_t* rxBuffer, uint8_t& rxBufferCount, uint8_t maxSize);

class PulsesData: public Data, CommandFifo
{
  public:
    /**
    * Initialize class for operation
    * @param moduleIndex index of module one of INTERNAL_MODULE, EXTERNAL_MODULE
    * @param resetFrameCount flag if current frame count should be reseted
    */

    void init(uint8_t moduleIndex, bool resetFrameCount = true);

    /**
    * Fills DMA buffers with frame to be send depending on actual state
    */
    void setupFrame();

    /**
    * Gets actual module status into provided buffer
    * @param statusText target buffer for status
    */
    void getStatusString(char * statusText) const;

    /**
    * Gets actual power source and voltage
    */
    void getPowerStatus(char* buffer) const;

    RUN_POWER actualRunPower() const;

    /**
    * Sends stop command to prevent any further module operations
    */
    void stop();

  protected:

    void setConfigFromModel();

  private:

    inline void putBytes(uint8_t* data, int length);

    inline void putFrame(COMMAND command, FRAME_TYPE frameType, uint8_t* data = nullptr, uint8_t dataLength = 0, uint8_t* frame_index = nullptr);

    void parseData(uint8_t* rxBuffer, uint8_t rxBufferCount);

    void setState(uint8_t state);

    bool syncSettings();

    void requestInfoAndRun(bool send = false);

    uint8_t setFailSafe(int16_t* target);

    inline int16_t convert(int channelValue);

    void sendChannelsData();

    void clearFrameData();

    void processTelemetryData(uint8_t byte, uint8_t* rxBuffer, uint8_t& rxBufferCount, uint8_t maxSize);

    //friendship declaration - use for passing telemetry
    friend void processTelemetryData(uint8_t module, uint8_t byte, uint8_t* rxBuffer, uint8_t& rxBufferCount, uint8_t maxSize);

    /**
    * Returns max power that currently can be set - use it to validate before synchronization of settings
    */
    RUN_POWER getMaxRunPower() const;

    RUN_POWER getRunPower() const;

    bool isConnectedUnicast();

    bool isConnectedMulticast();

    /**
    * Index of the module
    */
    uint8_t module_index;
    /**
     * Target mode to be set to the module one of MODULE_MODE_E
     */
    uint8_t requestedModuleMode;
    /**
     * Internal operation state one of UNKNOWN, SENDING_COMMAND, AWAITING_RESPONSE, IDLE
     * Used to avoid sending commands when not allowed to
     */
    State operationState;
    /**
     * Actual repeat count for requested command/operation - incremented by every attempt sending anything
     */
    uint16_t repeatCount;
    /**
     * Command count used for counting actual number of commands sent in run mode
     */
    uint32_t cmdCount;
    /**
     * Command index of command to be send when cmdCount reached necessary value
     */
    uint32_t cmdIndex;
    /**
     * Actual power source of the module - should be requested time to time
     * Currently requested once
     */
    enum MODULE_POWER_SOURCE powerSource;
    /**
     * Pointer to module config - it is making operations easier and faster
     */
    ModuleData* moduleData;
    /**
     * Actual module configuration - must be requested from module
     */
    Config_u cfg;

    /**
     * Actual module version - must be requested from module
     */
    ModuleVersion version;
};
} /* Namespace ahfds3 */

