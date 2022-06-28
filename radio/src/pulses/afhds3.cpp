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

#include "afhds3.h"
#include "afhds3_transport.h"
#include "pulses.h"

#include "../debug.h"
#include "../definitions.h"
// #include <cstdio>

#include "telemetry/telemetry.h"
#include "mixer_scheduler.h"
#include "hal/module_driver.h"

#if defined(INTMODULE_USART)
#include "intmodule_serial_driver.h"
#endif

#if defined(AFHDS3_EXT_UART)
#include "extmodule_serial_driver.h"
#else
#include "extmodule_driver.h"
#endif

#define FAILSAFE_HOLD 1
#define FAILSAFE_CUSTOM 2

#define FAILSAFE_KEEP_LAST 0x8000

//get channel value outside of afhds3 namespace
int32_t getChannelValue(uint8_t channel);

void processFlySkySensor(const uint8_t * packet, uint8_t type);

namespace afhds3
{

static uint8_t _phyMode_channels[] = {
  18, // CLASSIC_FLCR1_18CH
  10, // CLASSIC_FLCR6_10CH
  18, // ROUTINE_FLCR1_18CH
  8,  // ROUTINE_FLCR6_8CH
  12, // ROUTINE_LORA_12CH
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

// used for set command
enum MODULE_MODE_E {
  STANDBY = 0x01,
  BIND = 0x02,  // after bind module will enter run mode
  RUN = 0x03,
  RX_UPDATE = 0x04,  // after successful update module will enter standby mode,
                     // otherwise hw error will be raised
  MODULE_MODE_UNKNOWN = 0xFF
};

enum CMD_RESULT
{
  FAILURE = 0x01,
  SUCCESS = 0x02,
};

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

// enum EMI_STANDARD
// {
//   FCC = 0x00,
//   CE = 0x01
// };

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

// Old format for FRM302
// PACK(struct Config_s {
//   uint8_t bindPower;
//   uint8_t runPower;
//   uint8_t emiStandard;
//   uint8_t telemetry;
//   uint16_t pwmFreq;
//   uint8_t pulseMode;
//   uint8_t serialMode;
//   uint8_t channelCount;
//   uint16_t failSafeTimout;
//   int16_t failSafeMode[AFHDS3_MAX_CHANNELS];
// });

PACK(struct sSES_PWMFrequencyV0 {
  uint16_t Frequency : 15;    // From 50 to 400Hz
  uint16_t Synchronized : 1;  // 1=Synchronize the PWM output to the RF cycle
                              // (lower latency but unstable frequency)
});

enum eSES_PA_SetAnalogOutput {
  SES_ANALOG_OUTPUT_PWM = 0,
  SES_ANALOG_OUTPUT_PPM
};

enum eEB_BusType {
  EB_BT_IBUS1=0,
  EB_BT_IBUS2,
  EB_BT_SBUS1
};

// 48 bytes
PACK(struct sDATA_ConfigV0 {
  uint8_t Version;     // =0
  uint8_t EMIStandard; // eLNK_EMIStandard
  uint8_t IsTwoWay;
  uint8_t PhyMode;     // eDATA_PHYMODE
  uint8_t SignalStrengthRCChannelNb; // 0xFF if not used, 0`18
  uint16_t FailsafeTimeout;  // in unit of ms
  int16_t FailSafe[AFHDS3_MAX_CHANNELS];
  uint8_t FailsafeOutputMode; //TRUE Or FALSE
  sSES_PWMFrequencyV0 PWMFrequency;
  uint8_t AnalogOutput; // eSES_PA_SetAnalogOutput
  uint8_t ExternalBusType; // eEB_BusType
});

#define SES_NB_MAX_CHANNELS (32)
#define SES_NPT_NB_MAX_PORTS (4)

enum eSES_NewPortType {
  SES_NPT_PWM,
  SES_NPT_PPM,
  SES_NPT_SBUS,
  SES_NPT_IBUS1_IN,
  SES_NPT_IBUS1_OUT,
  SES_NPT_IBUS2,
  SES_NPT_IBUS2_HUB_PORT,
  SES_NPT_WSTX,
  SES_NPT_WSRX,
  SES_NPT_NONE=0xFF
};

PACK(struct sSES_PWMFrequenciesAPPV1 {
  // One unsigned short per
  // channel, From 50 to 400Hz,
  // 1:1000Hz,2:833Hz
  uint16_t PWMFrequencies[SES_NB_MAX_CHANNELS];

  // 1 bit per channel, 32 channels total
  uint32_t Synchronized;
});

// 116 bytes
PACK(struct sDATA_ConfigV1 {
  uint8_t Version;      // =1
  uint8_t EMIStandard;  // eLNK_EMIStandard
  uint8_t IsTwoWay;
  uint8_t PhyMode;                    // eDATA_PHYMODE
  uint8_t SignalStrengthRCChannelNb;  // 0xFF if not used , 0`18
  uint16_t FailsafeTimeout;           // in unit of ms
  int16_t FailSafe[AFHDS3_MAX_CHANNELS];
  uint8_t FailsafeOutputMode;                  // TRUE Or FALSE
  uint8_t NewPortTypes[SES_NPT_NB_MAX_PORTS];  // eSES_NewPortType
  sSES_PWMFrequenciesAPPV1 PWMFrequenciesV1;
});

union Config_u
{
  uint8_t version;
  sDATA_ConfigV0 v0;
  sDATA_ConfigV1 v1;
  uint8_t buffer[sizeof(sDATA_ConfigV1)];
};

enum CHANNELS_DATA_MODE
{
  CHANNELS = 0x01,
  FAIL_SAFE = 0x02,
};

PACK(struct ChannelsData {
  uint8_t mode;
  uint8_t channelsNumber;
  int16_t data[AFHDS3_MAX_CHANNELS];
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
  // Config_s Config;
  ChannelsData Channels;
  TelemetryData Telemetry;
  ModuleVersion Version;
  CommandResult_s CommandResult;
};

#define FRM302_STATUS 0x56

class ProtoState
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

    uint8_t* getBuffer();
    uint32_t getBufferSize();

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

    void parseData(uint8_t* rxBuffer, uint8_t rxBufferCount);

    void setState(ModuleState state);

    bool syncSettings();

    void requestInfoAndRun(bool send = false);

    uint8_t setFailSafe(int16_t* target);

    inline int16_t convert(int channelValue);

    void sendChannelsData();

    void clearFrameData();

    void processTelemetryData(uint8_t byte, uint8_t* rxBuffer, uint8_t& rxBufferCount, uint8_t maxSize);

    //friendship declaration - use for passing telemetry
    friend void processTelemetryData(uint8_t data, uint8_t module);

    /**
    * Returns max power that currently can be set - use it to validate before synchronization of settings
    */
    RUN_POWER getMaxRunPower() const;

    RUN_POWER getRunPower() const;

    bool isConnectedUnicast();

    bool isConnectedMulticast();

    Transport trsp;

    /**
     * Index of the module
     */
    uint8_t module_index;

    /**
     * Reported state of the HF module
     */
    ModuleState state;

    /**
     * Target mode to be set to the module one of MODULE_MODE_E
     */
    uint8_t requestedModuleMode;

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
  
static const char* const moduleStateText[] =
{
  "Not ready",
  "HW Error",
  "Binding",
  "Connecting",
  "Connected",
  "Standby",
  "Waiting for update",
  "Updating",
  "Updating RX",
  "Updating RX failed",
  "Testing",
  "Ready",
  "HW test"
};

static const char* const powerSourceText[] =
{
  "Unknown",
  "Internal",
  "External"
};

static const COMMAND periodicRequestCommands[] =
{
  COMMAND::MODULE_STATE,
  // COMMAND::MODULE_POWER_STATUS,
  COMMAND::MODULE_GET_CONFIG,
  COMMAND::VIRTUAL_FAILSAFE
};

//Static collection of afhds3 object instances by module
static ProtoState protoState[NUM_MODULES];

void getStatusString(uint8_t module, char* buffer)
{
  return protoState[module].getStatusString(buffer);
}

void getPowerStatus(uint8_t module, char* buffer)
{
  return protoState[module].getPowerStatus(buffer);
}

RUN_POWER getActualRunPower(uint8_t module)
{
  // return protoState[module].actualRunPower();
  return (RUN_POWER)0;
}

//friends function that can access telemetry parsing method
void processTelemetryData(uint8_t data, uint8_t module)
{
  uint8_t* rxBuffer = getTelemetryRxBuffer(module);
  uint8_t& rxBufferCount = getTelemetryRxBufferCount(module);
  uint8_t maxSize = TELEMETRY_RX_PACKET_SIZE;

  protoState[module].processTelemetryData(data, rxBuffer, rxBufferCount,
                                          maxSize);
}

void ProtoState::getStatusString(char* buffer) const
{
  strcpy(buffer, state <= ModuleState::STATE_READY ? moduleStateText[state]
                                                   : "Unknown");
}

void ProtoState::getPowerStatus(char* buffer) const
{
  strcpy(buffer, this->powerSource <= MODULE_POWER_SOURCE::EXTERNAL ? powerSourceText[this->powerSource] : "Unknown");
}

void ProtoState::processTelemetryData(uint8_t byte, uint8_t* rxBuffer, uint8_t& rxBufferCount, uint8_t maxSize)
{
  if (!trsp.processTelemetryData(byte, rxBuffer, rxBufferCount, maxSize))
    return;

  parseData(rxBuffer, rxBufferCount);
  rxBufferCount = 0;
}

bool ProtoState::isConnectedUnicast()
{
  // return cfg.config.telemetry == TELEMETRY::TELEMETRY_ENABLED &&
  return this->state == ModuleState::STATE_SYNC_DONE;
}

bool ProtoState::isConnectedMulticast()
{
  // return cfg.config.telemetry == TELEMETRY::TELEMETRY_DISABLED &&
  //        this->state == ModuleState::STATE_SYNC_RUNNING;
  return false;
}

void ProtoState::setupFrame()
{
  bool trsp_error = false;
  if (trsp.handleRetransmissions(trsp_error)) return;

  if (trsp_error) {
    this->state = ModuleState::STATE_NOT_READY;
    clearFrameData();
  }

  if (this->state == ModuleState::STATE_NOT_READY) {
    TRACE("AFHDS3 [GET MODULE READY]");
    trsp.sendFrame(COMMAND::MODULE_READY, FRAME_TYPE::REQUEST_GET_DATA);
    return;
  }

  // process backlog
  if (trsp.processQueue()) return;

  // config should be loaded already
  if (syncSettings()) { return; }

  ::ModuleSettingsMode moduleMode = getModuleMode(module_index);

  if (moduleMode == ::ModuleSettingsMode::MODULE_MODE_BIND) {
    switch (state) {
      case STATE_BINDING:
        // TODO: poll status? handle timeout?
        // requestedModuleMode = MODULE_MODE_E::BIND;
        // trsp.sendFrame(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA,
        //          &requestedModuleMode, 1);
        // return;
        break;

      default:
        // case STATE_STANDBY:
        TRACE("AFHDS3 [BIND]");
        setConfigFromModel();

        trsp.sendFrame(COMMAND::MODULE_SET_CONFIG,
                 FRAME_TYPE::REQUEST_SET_EXPECT_DATA, cfg.buffer,
                 cfg.version == 0 ? sizeof(cfg.v0) : sizeof(cfg.v1));

        requestedModuleMode = MODULE_MODE_E::BIND;
        trsp.enqueue(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, true,
                     requestedModuleMode);
        return;

        // default:
        //   // switch to standby mode first
        //   stop();
        //   return;
    }
  }
  // else if (moduleMode == ::ModuleSettingsMode::MODULE_MODE_RANGECHECK) {
  //   if (cfg.config.runPower != RUN_POWER::RUN_POWER_FIRST) {
  //     TRACE("AFHDS3 [RANGE CHECK]");
  //     cfg.config.runPower = RUN_POWER::RUN_POWER_FIRST;
  //     uint8_t data[] = { 0x13, 0x20, 0x02, cfg.config.runPower, 0 };
  //     TRACE("AFHDS3 SET TX POWER %d", moduleData->afhds3.runPower);
  //     trsp.sendFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA,
  //              data, sizeof(data));
  //     return;
  //   }
  // }
  else if (moduleMode == ::ModuleSettingsMode::MODULE_MODE_NORMAL) {

    // if module is ready but not started
    if (this->state == ModuleState::STATE_READY ||
        this->state == ModuleState::STATE_STANDBY) {

      cmdCount = 0;
      requestInfoAndRun(true);
      return;
    }

    // exit bind
    if (state == STATE_BINDING) {
      TRACE("AFHDS3 [EXIT BIND]");
      requestedModuleMode = MODULE_MODE_E::RUN;
      trsp.sendFrame(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA,
               &requestedModuleMode, 1);
      return;
    }
  }

  bool isConnected = isConnectedUnicast() || isConnectedMulticast();

  if (cmdCount++ >= 150) {
    cmdCount = 0;
    if (cmdIndex >= sizeof(periodicRequestCommands)) {
      cmdIndex = 0;
    }
    COMMAND cmd = periodicRequestCommands[cmdIndex++];

    if (cmd == COMMAND::VIRTUAL_FAILSAFE) {
      if (isConnected) {
        if (isConnectedMulticast()) {
          TRACE("AFHDS ONE WAY FAILSAFE");
          uint16_t failSafe[AFHDS3_MAX_CHANNELS + 1] = {
              ((AFHDS3_MAX_CHANNELS << 8) | CHANNELS_DATA_MODE::FAIL_SAFE), 0};
          setFailSafe((int16_t*)(&failSafe[1]));
          trsp.sendFrame(COMMAND::CHANNELS_FAILSAFE_DATA,
                   FRAME_TYPE::REQUEST_SET_NO_RESP, (uint8_t*)failSafe,
                   AFHDS3_MAX_CHANNELS * 2 + 2);
        } else {
          TRACE("AFHDS TWO WAYS FAILSAFE");
          uint8_t failSafe[3 + AFHDS3_MAX_CHANNELS * 2] = {
              0x11, 0x60, AFHDS3_MAX_CHANNELS * 2};
          setFailSafe((int16_t*)(failSafe + 3));
          trsp.sendFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA,
                   failSafe, 3 + AFHDS3_MAX_CHANNELS * 2);
        }
      } else {
        trsp.sendFrame(COMMAND::MODULE_STATE, FRAME_TYPE::REQUEST_GET_DATA);
      }
    } else {
      trsp.sendFrame(cmd, FRAME_TYPE::REQUEST_GET_DATA);
    }
  } else if (isConnected) {
    sendChannelsData();
  } else {
    //default frame - request state
    trsp.sendFrame(MODULE_STATE, FRAME_TYPE::REQUEST_GET_DATA);
  }
}

uint8_t* ProtoState::getBuffer()
{
  return trsp.getFrameBuffer();
}

uint32_t ProtoState::getBufferSize()
{
  return trsp.getFrameSize();
}

void ProtoState::init(uint8_t moduleIndex, bool resetFrameCount)
{
  module_index = moduleIndex;
#if defined(INTERNAL_MODULE_AFHDS3)
  if (moduleIndex == INTERNAL_MODULE) {
    trsp.init(ByteTransport::Serial, &intmodulePulsesData.afhds3);
  } else
#endif
  {
#if defined(AFHDS3_EXT_UART)
    trsp.init(ByteTransport::Serial, &extmodulePulsesData.afhds3);
#else
    trsp.init(ByteTransport::Pulses, &extmodulePulsesData.afhds3);
#endif
  }

  //clear local vars because it is member of union
  moduleData = &g_model.moduleData[module_index];
  state = ModuleState::STATE_NOT_READY;
  clearFrameData();
}

void ProtoState::clearFrameData()
{
  TRACE("AFHDS3 clearFrameData");
  trsp.clear();

  cmdCount = 0;
  cmdIndex = 0;
}

bool containsData(enum FRAME_TYPE frameType)
{
  return (frameType == FRAME_TYPE::RESPONSE_DATA ||
      frameType == FRAME_TYPE::REQUEST_SET_EXPECT_DATA ||
      frameType == FRAME_TYPE::REQUEST_SET_EXPECT_ACK ||
      frameType == FRAME_TYPE::REQUEST_SET_EXPECT_DATA ||
      frameType == FRAME_TYPE::REQUEST_SET_NO_RESP);
}

void ProtoState::setState(ModuleState state)
{
  if (state == this->state) {
    return;
  }
  uint8_t oldState = this->state;
  this->state = state;
  if (oldState == ModuleState::STATE_BINDING) {
    setModuleMode(module_index, ::ModuleSettingsMode::MODULE_MODE_NORMAL);
  }
  if (state == ModuleState::STATE_NOT_READY) {
    trsp.clear();
  }
}

void ProtoState::requestInfoAndRun(bool send)
{
  if (!send) {
    trsp.enqueue(COMMAND::MODULE_VERSION, FRAME_TYPE::REQUEST_GET_DATA);
  }

  // trsp.enqueue(COMMAND::MODULE_POWER_STATUS, FRAME_TYPE::REQUEST_GET_DATA);

  requestedModuleMode = MODULE_MODE_E::RUN;
  trsp.enqueue(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, true,
          (uint8_t)requestedModuleMode);

  if (send) {
    trsp.sendFrame(COMMAND::MODULE_VERSION, FRAME_TYPE::REQUEST_GET_DATA);
  }
}

void ProtoState::parseData(uint8_t* rxBuffer, uint8_t rxBufferCount)
{
  AfhdsFrame* responseFrame = reinterpret_cast<AfhdsFrame*>(rxBuffer);
  if (containsData((enum FRAME_TYPE) responseFrame->frameType)) {
    switch (responseFrame->command) {
      case COMMAND::MODULE_READY:
        TRACE("AFHDS3 [MODULE_READY] %02X", responseFrame->value);
        if (responseFrame->value == MODULE_STATUS_READY) {
          setState(ModuleState::STATE_READY);
          requestInfoAndRun();
        }
        else {
          setState(ModuleState::STATE_NOT_READY);
        }
        break;
      case COMMAND::MODULE_GET_CONFIG: {
        size_t len = min<size_t>(sizeof(cfg.buffer), rxBufferCount);
        std::memcpy((void*) cfg.buffer, &responseFrame->value, len);
        // TRACE(
        //     "AFHDS3 [MODULE_GET_CONFIG] bind power %d run power %d mode %d "
        //     "pwm/ppm %d ibus/sbus %d",
        //     cfg.config.bindPower, cfg.config.runPower, cfg.config.telemetry,
        //     cfg.config.pulseMode, cfg.config.serialMode);
      } break;
      case COMMAND::MODULE_VERSION:
        std::memcpy((void*) &version, &responseFrame->value, sizeof(version));
        TRACE("AFHDS3 [MODULE_VERSION] Product %d, HW %d, BOOT %d, FW %d",
              version.productNumber, version.hardwereVersion,
              version.bootloaderVersion, version.firmwareVersion);
        break;
      // case COMMAND::MODULE_POWER_STATUS:
      //   powerSource = (enum MODULE_POWER_SOURCE) responseFrame->value;
      //   TRACE("AFHDS3 [MODULE_POWER_STATUS], %d", powerSource);
      //   break;
      case COMMAND::MODULE_STATE:
        TRACE("AFHDS3 [MODULE_STATE] %02X", responseFrame->value);
        setState((ModuleState)responseFrame->value);
        break;
      case COMMAND::MODULE_MODE:
        TRACE("AFHDS3 [MODULE_MODE] %02X", responseFrame->value);
        if (responseFrame->value != CMD_RESULT::SUCCESS) {
          setState(ModuleState::STATE_NOT_READY);
        }
        else {
          if (requestedModuleMode == MODULE_MODE_E::RUN) {
            trsp.enqueue(COMMAND::MODULE_GET_CONFIG, FRAME_TYPE::REQUEST_GET_DATA);
            trsp.enqueue(COMMAND::MODULE_STATE, FRAME_TYPE::REQUEST_GET_DATA);
          }
          requestedModuleMode = MODULE_MODE_UNKNOWN;
        }
        break;
      case COMMAND::MODULE_SET_CONFIG:
        if (responseFrame->value != CMD_RESULT::SUCCESS) {
          setState(ModuleState::STATE_NOT_READY);
        }
        TRACE("AFHDS3 [MODULE_SET_CONFIG], %02X", responseFrame->value);
        break;
      case COMMAND::TELEMETRY_DATA:
        {
        uint8_t* telemetry = &responseFrame->value;

        if (telemetry[0] == 0x22) {
          telemetry++;
          while (telemetry < rxBuffer + rxBufferCount) {
            uint8_t length = telemetry[0];
            uint8_t id = telemetry[1];
            if (id == 0xFE) {
              id = 0xF7;  //use new id because format is different
            }
            if (length == 0 || telemetry + length > rxBuffer + rxBufferCount) {
              break;
            }
            if (length == 4) {
              //one byte value fill missing byte
              uint8_t data[] = { id, telemetry[2], telemetry[3], 0 };
              ::processFlySkySensor(data, 0xAA);
            }
            if (length == 5) {
              if (id == 0xFA) {
                telemetry[1] = 0xF8; //remap to afhds3 snr
              }
              ::processFlySkySensor(telemetry + 1, 0xAA);
            }
            else if (length == 6 && id == FRM302_STATUS) {
              //convert to ibus
              uint16_t t = (uint16_t) (((int16_t) telemetry[3] * 10) + 400);
              uint8_t dataTemp[] = { ++id, telemetry[2], (uint8_t) (t & 0xFF), (uint8_t) (t >> 8) };
              ::processFlySkySensor(dataTemp, 0xAA);
              uint8_t dataVoltage[] = { ++id, telemetry[2], telemetry[4], telemetry[5] };
              ::processFlySkySensor(dataVoltage, 0xAA);
            }
            else if (length == 7) {
              ::processFlySkySensor(telemetry + 1, 0xAC);
            }
            telemetry += length;
          }
        }
      }
        break;
      case COMMAND::COMMAND_RESULT: {
        // AfhdsFrameData* respData = responseFrame->GetData();
        // TRACE("COMMAND RESULT %02X result %d datalen %d",
        // respData->CommandResult.command, respData->CommandResult.result,
        // respData->CommandResult.respLen);
      } break;
    }
  }

  if (responseFrame->frameType == FRAME_TYPE::REQUEST_GET_DATA ||
      responseFrame->frameType == FRAME_TYPE::REQUEST_SET_EXPECT_DATA) {
    TRACE("Command %02X NOT IMPLEMENTED!", responseFrame->command);
  }
}

inline bool isSbus(uint8_t mode)
{
  return (mode & 1);
}

inline bool isPWM(uint8_t mode)
{
  return !(mode & 2);
}

RUN_POWER ProtoState::getMaxRunPower() const
{
  if (powerSource == MODULE_POWER_SOURCE::EXTERNAL) {
    return RUN_POWER::PLUS_33dBm;
  }

  return RUN_POWER::PLUS_27dbm;
}

RUN_POWER ProtoState::actualRunPower() const
{
  // uint8_t actualRfPower = cfg.config.runPower;
  // if (getMaxRunPower() < actualRfPower) {
  //   actualRfPower = getMaxRunPower();
  // }
  // return (RUN_POWER) actualRfPower;
  return (RUN_POWER)0;
}

RUN_POWER ProtoState::getRunPower() const
{
  // RUN_POWER targetPower = (RUN_POWER) moduleData->afhds3.runPower;
  // if (getMaxRunPower() < targetPower) {
  //   targetPower = getMaxRunPower();
  // }
  // return targetPower;
  return (RUN_POWER)0;
}

bool ProtoState::syncSettings()
{
  // RUN_POWER targetPower = getRunPower();

  // /*not sure if we need to prevent them in bind mode*/
  // if (getModuleMode(module_index) != ::ModuleSettingsMode::MODULE_MODE_BIND &&
  //     targetPower != cfg.config.runPower) {
  //   cfg.config.runPower = moduleData->afhds3.runPower;
  //   uint8_t data[] = {0x13, 0x20, 0x02, moduleData->afhds3.runPower, 0};
  //   TRACE("AFHDS3 SET TX POWER %d", moduleData->afhds3.runPower);
  //   trsp.sendFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data,
  //            sizeof(data));
  //   return true;
  // }

  // // other settings only in 2 way mode (state must be synchronized)
  // if (this->state != ModuleState::STATE_SYNC_DONE) {
  //   return false;
  // }

  // if (moduleData->afhds3.rxFreq() != cfg.config.pwmFreq) {
  //   cfg.config.pwmFreq = moduleData->afhds3.rxFreq();
  //   uint8_t data[] = {0x17, 0x70, 0x02,
  //                     (uint8_t)(moduleData->afhds3.rxFreq() & 0xFF),
  //                     (uint8_t)(moduleData->afhds3.rxFreq() >> 8)};
  //   TRACE("AFHDS3 SET RX FREQ");
  //   trsp.sendFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data,
  //            sizeof(data));
  //   return true;
  // }

  // PULSE_MODE modelPulseMode = isPWM(moduleData->afhds3.mode)
  //                                 ? PULSE_MODE::PWM_MODE
  //                                 : PULSE_MODE::PPM_MODE;
  // if (modelPulseMode != cfg.config.pulseMode) {
  //   cfg.config.pulseMode = modelPulseMode;
  //   TRACE("AFHDS3 PWM/PPM %d", modelPulseMode);
  //   uint8_t data[] = {0x16, 0x70, 0x01, (uint8_t)(modelPulseMode)};
  //   trsp.sendFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data,
  //            sizeof(data));
  //   return true;
  // }

  // SERIAL_MODE modelSerialMode = isSbus(moduleData->afhds3.mode)
  //                                   ? SERIAL_MODE::SBUS_MODE
  //                                   : SERIAL_MODE::IBUS;
  // if (modelSerialMode != cfg.config.serialMode) {
  //   cfg.config.serialMode = modelSerialMode;
  //   TRACE("AFHDS3 IBUS/SBUS %d", modelSerialMode);
  //   uint8_t data[] = {0x18, 0x70, 0x01, (uint8_t)(modelSerialMode)};
  //   trsp.sendFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data,
  //            sizeof(data));
  //   return true;
  // }

  // if (moduleData->afhds3.failsafeTimeout != cfg.config.failSafeTimout) {
  //   moduleData->afhds3.failsafeTimeout = cfg.config.failSafeTimout;
  //   uint8_t data[] = {0x12, 0x60, 0x02,
  //                     (uint8_t)(moduleData->afhds3.failsafeTimeout & 0xFF),
  //                     (uint8_t)(moduleData->afhds3.failsafeTimeout >> 8)};
  //   trsp.sendFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data,
  //            sizeof(data));
  //   TRACE("AFHDS3 FAILSAFE TMEOUT, %d", moduleData->afhds3.failsafeTimeout);
  //   return true;
  // }

  return false;
}

void ProtoState::sendChannelsData()
{
  uint8_t channels_start = moduleData->channelsStart;
  uint8_t channelsCount = 8 + moduleData->channelsCount;
  uint8_t channels_last = channels_start + channelsCount;

  int16_t buffer[AFHDS3_MAX_CHANNELS + 1] = {0};

  uint8_t* header = (uint8_t*)buffer;
  header[0] = CHANNELS_DATA_MODE::CHANNELS;

  uint8_t channels = _phyMode_channels[cfg.v0.PhyMode];
  header[1] = channels;

  for (uint8_t channel = channels_start, index = 1; channel < channels_last;
       channel++, index++) {
    int16_t channelValue = convert(::getChannelValue(channel));
    buffer[index] = channelValue;
  }

  trsp.sendFrame(COMMAND::CHANNELS_FAILSAFE_DATA, FRAME_TYPE::REQUEST_SET_NO_RESP,
           (uint8_t*)buffer, (channels + 1) * 2);
}

void ProtoState::stop()
{
  TRACE("AFHDS3 STOP");
  requestedModuleMode = MODULE_MODE_E::STANDBY;
  trsp.sendFrame(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA,
           &requestedModuleMode, 1);
}

void ProtoState::setConfigFromModel()
{
  // cfg.config.bindPower = moduleData->afhds3.bindPower;
  // cfg.config.runPower = getRunPower();
  // cfg.config.emiStandard = EMI_STANDARD::FCC;
  // cfg.config.telemetry = moduleData->afhds3.telemetry; //always use bidirectional mode
  // cfg.config.pwmFreq = moduleData->afhds3.rxFreq();
  // cfg.config.serialMode = isSbus(moduleData->afhds3.mode)
  //                             ? SERIAL_MODE::SBUS_MODE
  //                             : SERIAL_MODE::IBUS;
  // cfg.config.pulseMode = isPWM(moduleData->afhds3.mode) ? PULSE_MODE::PWM_MODE
  //                                                       : PULSE_MODE::PPM_MODE;
  // // use max channels - because channel count can not be changed after bind
  // cfg.config.channelCount = AFHDS3_MAX_CHANNELS;
  // cfg.config.failSafeTimout = moduleData->afhds3.failsafeTimeout;
  // setFailSafe(cfg.config.failSafeMode);

  if (moduleData->afhds3.phyMode < ROUTINE_FLCR1_18CH) {
    cfg.version = 0;
  } else {
    cfg.version = 1;
  }

  if (cfg.version == 1) {
    cfg.v1.EMIStandard = moduleData->afhds3.emi;
    cfg.v1.IsTwoWay = moduleData->afhds3.telemetry;
    cfg.v1.PhyMode = moduleData->afhds3.phyMode;
    cfg.v1.SignalStrengthRCChannelNb = 0xFF;

    // Failsafe
    cfg.v1.FailsafeTimeout = 500;
    setFailSafe(cfg.v1.FailSafe);

    if (moduleData->failsafeMode != FAILSAFE_NOPULSES) {
      cfg.v1.FailsafeOutputMode = true;
    } else {
      cfg.v1.FailsafeOutputMode = false;
    }

    // Serial output
    memclear(&cfg.v1.NewPortTypes, sizeof(cfg.v1.NewPortTypes));

    // PWM settings
    for (int i = 0; i < SES_NB_MAX_CHANNELS; i++)
      cfg.v1.PWMFrequenciesV1.PWMFrequencies[i] = 50;

    cfg.v1.PWMFrequenciesV1.Synchronized = 0;

  } else {
    cfg.v0.EMIStandard = moduleData->afhds3.emi;
    cfg.v0.IsTwoWay = moduleData->afhds3.telemetry;
    cfg.v0.PhyMode = moduleData->afhds3.phyMode;
    cfg.v0.SignalStrengthRCChannelNb = 0xFF;

    // Failsafe
    cfg.v0.FailsafeTimeout = 500;
    setFailSafe(cfg.v0.FailSafe);

    if (moduleData->failsafeMode != FAILSAFE_NOPULSES) {
      cfg.v0.FailsafeOutputMode = true;
    } else {
      cfg.v0.FailsafeOutputMode = false;
    }

    // PWM settings
    cfg.v0.PWMFrequency.Frequency = 50;
    cfg.v0.PWMFrequency.Synchronized = 0;

    // PWM/PPM
    cfg.v0.AnalogOutput = 0;

    // iBUS/SBUS
    cfg.v0.ExternalBusType = EB_BT_IBUS1;
  }
}

inline int16_t ProtoState::convert(int channelValue)
{
  //pulseValue = limit<uint16_t>(0, 988 + ((channelValue + 1024) / 2), 0xfff);
  //988 - 750 = 238
  //238 * 20 = 4760
  //2250 - 2012 = 238
  //238 * 20 = 4760
  // 988   ---- 2012
  //-10240 ---- 10240
  //-1024  ---- 1024
  return ::limit<int16_t>(AFHDS3_FAILSAFE_MIN, channelValue * 10, AFHDS3_FAILSAFE_MAX);
}

uint8_t ProtoState::setFailSafe(int16_t* target)
{
  int16_t pulseValue = 0;
  uint8_t channels_start = moduleData->channelsStart;
  uint8_t channels_last = channels_start + 8 + moduleData->channelsCount;

  for (uint8_t channel = channels_start; channel < channels_last; channel++) {
    if (moduleData->failsafeMode == FAILSAFE_CUSTOM) {
      pulseValue = convert(g_model.failsafeChannels[channel]);
    }
    else if (moduleData->failsafeMode == FAILSAFE_HOLD) {
      pulseValue = FAILSAFE_KEEP_LAST;
    }
    else {
      pulseValue = convert(::getChannelValue(channel));
    }
    target[channel - channels_start] = pulseValue;
  }
  //return max channels because channel count can not be change after bind
  return (uint8_t) (AFHDS3_MAX_CHANNELS);
}

#if defined(AFHDS3_EXT_UART)

static const etx_serial_init extmoduleUsartParams = {
  .baudrate = AFHDS3_UART_BAUDRATE,
  .parity = ETX_Parity_None,
  .stop_bits = ETX_StopBits_One,
  .word_length = ETX_WordLength_8,
  .rx_enable = true,
};

static void* initExternal(uint8_t module)
{
  auto p_state = &protoState[module];

  p_state->init(module);
  void* ctx = ExtmoduleSerialDriver.init(&extmoduleUsartParams);

  mixerSchedulerSetPeriod(module, AFHDS3_UART_COMMAND_TIMEOUT * 1000 /* us */);
  EXTERNAL_MODULE_ON();

  return ctx;
}

static void deinitExternal(void* context)
{
  EXTERNAL_MODULE_OFF();
  mixerSchedulerSetPeriod(EXTERNAL_MODULE, 0);
  ExtmoduleSerialDriver.deinit(context);
}

static void sendPulsesExternal(void* context)
{
  auto p_state = &protoState[EXTERNAL_MODULE];
  ExtmoduleSerialDriver.sendBuffer(context, p_state->getBuffer(), p_state->getBufferSize());
}

#else // !defined(AFHDS3_USART)

static void* initExternal(uint8_t module)
{
  auto p_state = &protoState[module];

  p_state->init(module);
  extmoduleSerialStart();

  mixerSchedulerSetPeriod(module, AFHDS3_SOFTSERIAL_COMMAND_TIMEOUT * 1000 /* us */);
  return nullptr;
}

static void deinitExternal(void* context)
{
  (void)context;
  extmoduleStop();
  mixerSchedulerSetPeriod(EXTERNAL_MODULE, 0);
}

static void sendPulsesExternal(void* context)
{
  (void)context;
  auto p_state = &protoState[EXTERNAL_MODULE];
  extmoduleSendNextFrameSoftSerial(p_state->getBuffer(), p_state->getBufferSize(), false);
}

#endif

static void setupPulsesExternal(void* context, int16_t* channels, uint8_t nChannels)
{
  (void)context;
  (void)channels;
  (void)nChannels;

  auto p_state = &protoState[EXTERNAL_MODULE];
  p_state->setupFrame();
}

etx_module_driver_t externalDriver = {
  .protocol = PROTOCOL_CHANNELS_AFHDS3,
  .init = initExternal,
  .deinit = deinitExternal,
  .setupPulses = setupPulsesExternal,
  .sendPulses = sendPulsesExternal
};

#if defined(INTERNAL_MODULE_AFHDS3)

static const etx_serial_init intmoduleUsartParams = {
  .baudrate = AFHDS3_UART_BAUDRATE,
  .parity = ETX_Parity_None,
  .stop_bits = ETX_StopBits_One,
  .word_length = ETX_WordLength_8,
  .rx_enable = true,
};

static void* initInternal(uint8_t module)
{
  auto p_state = &protoState[module];

  p_state->init(module);
  void* ctx = IntmoduleSerialDriver.init(&intmoduleUsartParams);

  mixerSchedulerSetPeriod(module, AFHDS3_UART_COMMAND_TIMEOUT * 1000 /* us */);
  INTERNAL_MODULE_ON();

  return ctx;
}

static void deinitInternal(void* context)
{
  (void)context;

  INTERNAL_MODULE_OFF();
  mixerSchedulerSetPeriod(INTERNAL_MODULE, 0);
  intmoduleStop();
}

static void setupPulsesInternal(void* context, int16_t* channels, uint8_t nChannels)
{
  (void)context;
  (void)channels;
  (void)nChannels;

  auto p_state = &protoState[INTERNAL_MODULE];
  p_state->setupFrame();
}

static void sendPulsesInternal(void* context)
{
  auto p_state = &protoState[INTERNAL_MODULE];  
  IntmoduleSerialDriver.sendBuffer(context, p_state->getBuffer(), p_state->getBufferSize());
}

static int getByteInternal(void* context, uint8_t* data)
{
  return IntmoduleSerialDriver.getByte(context, data);
}

etx_module_driver_t internalDriver = {
  .protocol = PROTOCOL_CHANNELS_AFHDS3,
  .init = initInternal,
  .deinit = deinitInternal,
  .setupPulses = setupPulsesInternal,
  .sendPulses = sendPulsesInternal,
  .getByte = getByteInternal,
};

#endif

}


