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
#include "afhds3_config.h"

#include "pulses.h"

#include "../debug.h"
#include "../definitions.h"

#include "telemetry/telemetry.h"
#include "mixer_scheduler.h"
#include "hal/module_driver.h"
#include "hal/module_port.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

#define checkDirtyFlag(dirtyCmd) (cfg->others.dirtyFlag & ((uint32_t) 1 << dirtyCmd))
#define clearDirtyFlag(dirtyCmd) (cfg->others.dirtyFlag &= ~((uint32_t) 1 << dirtyCmd))

#define FAILSAFE_HOLD 1
#define FAILSAFE_CUSTOM 2

#define FAILSAFE_HOLD_VALUE         0x8000
#define FAILSAFE_NOPULSES_VALUE     0x8001

#define MAX_NO_OF_MODELS            20

//get channel value outside of afhds3 namespace
int32_t getChannelValue(uint8_t channel);
void processFlySkyAFHDS3Sensor(const uint8_t * packet, uint8_t type);
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

// enum COMMAND_DIRECTION
// {
//   RADIO_TO_MODULE = 0,
//   MODULE_TO_RADIO = 1
// };

// enum DATA_TYPE
// {
//   READY_DT,  // 8 bytes 0x01 Not ready 0x02 Ready
//   STATE_DT,  // See MODULE_STATE
//   MODE_DT,
//   MOD_CONFIG_DT,
//   CHANNELS_DT,
//   TELEMETRY_DT,
//   MODULE_POWER_DT,
//   MODULE_VERSION_DT,
//   EMPTY_DT,
// };

//Product number
#define PN_FRM301       ( (0x1234<<16) | 0x0003 )
#define PN_FRM004       ( (0x1234<<16) | 0x0007 )
#define PN_FRM005       ( (0x1234<<16) | 0x0009 )

#define PN_FTR10 	      ( (0x1234<<16) | 0x0002 )
#define PN_FGR4  	      ( (0x1234<<16) | 0x0004 )

#define PN_FTR4 	      ( (0x1234<<16) | 0x0006 )
#define PN_FGR4S 	      ( (0x1234<<16) | 0x0006 )
#define PN_FGR4P 	      ( (0x1234<<16) | 0x0006 )

#define PN_FTR16S       ( (0x0001<<16) | 0x0103 )
//#define PN_MINIZ        ( (0x0001<<16) | 0x0104 )

#define PN_FTR8B        ( (0x0001<<16) | 0x0105 )
#define PN_FTR12B       ( (0x0001<<16) | 0x0106 )
#define PN_FGR8B        ( (0x0001<<16) | 0x0107 )
#define PN_FGR12B       ( (0x0001<<16) | 0x0108 )
#define PN_GMR          ( (0x0001<<16) | 0x0109 )
#define PN_TMR          ( (0x0001<<16) | 0x010A )
#define PN_INR4_GYB     ( (0x0001<<16) | 0x010B )
#define PN_INR6_HS      ( (0x0001<<16) | 0x010C )
#define PN_FTR4B        ( (0x0001<<16) | 0x010D )
#define PN_FGR4B        ( (0x0001<<16) | 0x010E )
#define PN_FBR12        ( (0x0001<<16) | 0x010F )
#define PN_INR6_FC      ( (0x0001<<16) | 0x0110 )
#define PN_TR8B         ( (0x0001<<16) | 0x0111 )
#define PN_FBR8         ( (0x0001<<16) | 0x0112 )
#define PN_FBR4         ( (0x0001<<16) | 0x0113 )

// enum used by command response -> translate to ModuleState
enum MODULE_READY_E {
  MODULE_STATUS_UNKNOWN = 0x00,
  MODULE_STATUS_NOT_READY = 0x01,
  MODULE_STATUS_READY = 0x02
};

enum ModuleState {
  STATE_NOT_READY = 0x00,     // virtual, module not ready
  STATE_HW_ERROR = 0x01,
  STATE_BINDING = 0x02,
  STATE_SYNC_RUNNING = 0x03,  // sync state, ready to sync settings, receiver not connected
  STATE_SYNC_DONE = 0x04,     // sync state, ready to sync settings, receiver connected
  STATE_STANDBY = 0x05,       // standby state, ready to update modelID, and switch to RUN mode
  STATE_UPDATING_WAIT = 0x06,
  STATE_UPDATING_MOD = 0x07,
  STATE_UPDATING_RX = 0x08,
  STATE_UPDATING_RX_FAILED = 0x09,
  STATE_RF_TESTING = 0x0a,
  STATE_READY = 0x0b,         // virtual, module ready awaiting to query module state
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

enum CMD_RESULT {
  FAILURE = 0x01,
  SUCCESS = 0x02,
};

enum CHANNELS_DATA_MODE {
  CHANNELS = 0x01,
  FAIL_SAFE = 0x02,
};

PACK(struct ChannelsData {
  uint8_t mode;
  uint8_t channelsNumber;
  int16_t data[AFHDS3_MAX_CHANNELS];
});

union ChannelsData_u {
  ChannelsData data;
  uint8_t buffer[sizeof(ChannelsData)];
};

PACK(struct TelemetryData {
  uint8_t sensorType;
  uint8_t length;
  uint8_t type;
  uint8_t semsorID;
  uint8_t data[8];
});

enum MODULE_POWER_SOURCE {
  INTERNAL = 0x01,
  EXTERNAL = 0x02,
};

enum DeviceAddress {
  TRANSMITTER = 0x01,
  FRM303 = 0x04,
  IRM301 = 0x05,
};

PACK(struct ModuleVersion
{
  uint16_t companyNumber;
  uint32_t txID;
  uint32_t rxID;
  uint32_t productNumber;
  uint32_t hardwareVersion;
  uint32_t bootloaderVersion;
  uint32_t firmwareVersion;
  uint32_t rfVersion;
});


PACK(struct ReceiverVersion
{
uint32_t ProductNumber;
uint16_t MainboardVersion;
uint16_t RFModuleVersion;
uint16_t BootloaderVersion;
uint16_t FirmwareVersion;
uint16_t RFLibraryVersion;
} );

PACK(struct CommandResult_s {
  uint16_t command;
  uint8_t result;
  uint8_t respLen;
});

union AfhdsFrameData {
  uint8_t value;
  // Config_s Config;
  ChannelsData Channels;
  TelemetryData Telemetry;
  ModuleVersion Version;
  CommandResult_s CommandResult;
};

#define FRM302_STATUS 0x56

uint8_t receiver_type( unsigned long productnumber );
class ProtoState
{
  public:
    /**
    * Initialize class for operation
    * @param moduleIndex index of module one of INTERNAL_MODULE, EXTERNAL_MODULE
    * @param resetFrameCount flag if current frame count should be reseted
    */
   void init(uint8_t moduleIndex, void* buffer, etx_module_state_t* mod_st,
             uint8_t fAddr);

   /**
    * Fills DMA buffers with frame to be send depending on actual state
    */
   void setupFrame();

   /**
    * Sends prepared buffers
    */
   void sendFrame() { trsp.sendBuffer(); }

   /**
    * Gets actual module status into provided buffer
    * @param statusText target buffer for status
    */
   void getStatusString(char* statusText) const;

   /**
    * Sends stop command to prevent any further module operations
    */
   void stop();

   Config_u* getConfig() { return &cfg; }

   void applyConfigFromModel();

  protected:

    void resetConfig(uint8_t version);

  private:
    //friendship declaration - use for passing telemetry
    friend void processTelemetryData(void* ctx, uint8_t data, uint8_t* buffer, uint8_t* len);

    void processTelemetryData(uint8_t data, uint8_t* buffer, uint8_t* len);

    void parseData(uint8_t* rxBuffer, uint8_t rxBufferCount);

    void setState(ModuleState state);

    bool syncSettings();

  //  void requestInfoAndRun(bool send = false);

    uint8_t setFailSafe(int16_t* target, uint8_t rfchannelcount=AFHDS3_MAX_CHANNELS);

    inline int16_t convert(int channelValue);

    void sendChannelsData();

    void clearFrameData();

    bool isConnected();
    bool hasTelemetry();

    Transport trsp;

    /**
     * Index of the module
     */
    uint8_t module_index;

    /**
     * Reported state of the HF module
     */
    ModuleState state;

    //bool modelIDSet;
    //bool modelcfgGet;
    uint8_t modelID;
    bool rx_state; //false:disconnect; true:connect

    /**
     * Command count used for counting actual number of commands sent in run mode
     */
    uint32_t cmdCount;

    /**
     * Command index of command to be send when cmdCount reached necessary value
     */
    uint32_t cmdIndex;

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
    ReceiverVersion rx_version;
};

static const char* const moduleStateText[] =
{
  "Not ready",
  "HW Error",
  "Binding",
  "Disconnected",
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

static const COMMAND periodicRequestCommands[] =
{
  COMMAND::MODULE_STATE,
  // COMMAND::MODULE_GET_CONFIG,
  COMMAND::VIRTUAL_FAILSAFE // One way failsafe
};

static const uint16_t AFHDS3_POWER[] = {56, 68, 80, 96, 108, 120, 132};

//Static collection of afhds3 object instances by module
static ProtoState protoState[MAX_MODULES];

void getStatusString(uint8_t module, char* buffer)
{
  return protoState[module].getStatusString(buffer);
}

//friends function that can access telemetry parsing method
void processTelemetryData(void* ctx, uint8_t data, uint8_t* buffer, uint8_t* len)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto p_state = (ProtoState*)mod_st->user_data;
  p_state->processTelemetryData(data, buffer, len);
}

void ProtoState::getStatusString(char* buffer) const
{
  strcpy(buffer, state <= ModuleState::STATE_READY ? moduleStateText[state]
                                                   : "Unknown");
}

void ProtoState::processTelemetryData(uint8_t byte, uint8_t* buffer, uint8_t* len)
{
  uint8_t maxSize = TELEMETRY_RX_PACKET_SIZE;
  if (!trsp.processTelemetryData(byte, buffer, *len, maxSize))
    return;

  parseData(buffer, *len);
  *len = 0;
}

bool ProtoState::isConnected()
{
  return this->state == ModuleState::STATE_SYNC_DONE;
}

bool ProtoState::hasTelemetry()
{
  if (cfg.version == 0)
    return cfg.v0.IsTwoWay;
  else
    return cfg.v1.IsTwoWay;
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
//     TRACE("AFHDS3 [GET MODULE READY]");
    trsp.putFrame(COMMAND::MODULE_READY, FRAME_TYPE::REQUEST_GET_DATA);
    return;
  }

  // Process backlog, not check states
  if (trsp.processQueue()) return;

  ::ModuleSettingsMode moduleMode = getModuleMode(module_index);

  if (moduleMode == ::ModuleSettingsMode::MODULE_MODE_BIND) {
    if (state != STATE_BINDING) {
//       TRACE("AFHDS3 [BIND]");
      applyConfigFromModel();

      trsp.putFrame(COMMAND::MODULE_SET_CONFIG,
                     FRAME_TYPE::REQUEST_SET_EXPECT_DATA, cfg.buffer,
                     cfg.version == 0 ? sizeof(cfg.v0) : sizeof(cfg.v1));

      trsp.enqueue(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, true,
                   (uint8_t)MODULE_MODE_E::BIND);
      return;
    }
  }
  else if (moduleMode == ::ModuleSettingsMode::MODULE_MODE_RANGECHECK) {
    TRACE("AFHDS3 [RANGE CHECK] not supported");
  }
  else if (moduleMode == ::ModuleSettingsMode::MODULE_MODE_NORMAL) {

    // If module is ready but not started
    if (this->state == ModuleState::STATE_READY) {
      trsp.putFrame(MODULE_STATE, FRAME_TYPE::REQUEST_GET_DATA);
      return;
    }

    // ModelID change detection, if changed => STATE_STANDBY => Set ModelID => RUN mode => STATE_SYNC_XXX
    uint8_t newModelID = g_model.header.modelId[module_index] % MAX_NO_OF_MODELS;  // Rotate the model ID when exceed no. of stored models
    if (modelID != newModelID)
    {
      if (this->state != ModuleState::STATE_STANDBY) {
//         TRACE("AFHDS3 [Model ID Changed] Switch to STATE_STANDBY");
        auto mode = (uint8_t)MODULE_MODE_E::STANDBY;
        trsp.putFrame(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, &mode, 1);
        return;
      } else {
//         TRACE("AFHDS3 [Model ID Changed] Set ModelID to %d", newModelID);
        modelID = newModelID;
        trsp.putFrame(COMMAND::MODEL_ID, FRAME_TYPE::REQUEST_SET_EXPECT_DATA,
                       &modelID, 1);
        return;
      }
    }

    // If standby or exit bind =>  RUN mode => STATE_SYNC_XXX
    if (this->state == ModuleState::STATE_STANDBY || this->state == ModuleState::STATE_BINDING) {
      cmdCount = 0;
//      requestInfoAndRun(true);
      auto mode = (uint8_t)MODULE_MODE_E::RUN;
      trsp.putFrame(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, &mode, 1);
      return;
    }

    // Exit bind
/*    if (this->state == STATE_BINDING) {
      TRACE("AFHDS3 [EXIT BIND]");
//      modelcfgGet = true;
      auto mode = (uint8_t)MODULE_MODE_E::RUN;
      trsp.putFrame(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, &mode, 1);
      trsp.enqueue(COMMAND::MODULE_GET_CONFIG, FRAME_TYPE::REQUEST_GET_DATA);
      return;
    } */
  }

/*  if (modelcfgGet) {
    trsp.enqueue(COMMAND::MODULE_GET_CONFIG, FRAME_TYPE::REQUEST_GET_DATA);
    return;
  }*/

  if (cmdCount++ >= 150) {

    cmdCount = 0;
    if (cmdIndex >= sizeof(periodicRequestCommands)) {
      cmdIndex = 0;
    }
    COMMAND cmd = periodicRequestCommands[cmdIndex++];

    if (cmd == COMMAND::VIRTUAL_FAILSAFE) {
      Config_u* cfg = this->getConfig();
      uint8_t len =_phyMode_channels[cfg->v0.PhyMode];
      if (!hasTelemetry()) {
          uint16_t failSafe[AFHDS3_MAX_CHANNELS + 1] = {
          ((AFHDS3_MAX_CHANNELS << 8) | CHANNELS_DATA_MODE::FAIL_SAFE), 0};
          setFailSafe((int16_t*)(&failSafe[1]), len);
//           TRACE("AFHDS ONE WAY FAILSAFE");
          trsp.putFrame(COMMAND::CHANNELS_FAILSAFE_DATA,
                   FRAME_TYPE::REQUEST_SET_NO_RESP, (uint8_t*)failSafe,
                   AFHDS3_MAX_CHANNELS * 2 + 2);
      }
      else if( isConnected() ){
          uint8_t data[AFHDS3_MAX_CHANNELS*2 + 3] = { (uint8_t)(RX_CMD_FAILSAFE_VALUE&0xFF), (uint8_t)((RX_CMD_FAILSAFE_VALUE>>8)&0xFF), (uint8_t)(2*len)};
          int16_t failSafe[18];
          setFailSafe(&failSafe[0], len);
          std::memcpy( &data[3], failSafe, 2*len );
          trsp.putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, 2*len+3);
      }
    } else {
      trsp.putFrame(cmd, FRAME_TYPE::REQUEST_GET_DATA);
    }
    return;
  }

  if (isConnected()) {
    // Sync config, with commands
    if (syncSettings()) { return; }

    // Send channels data
    sendChannelsData();
  } else {
    //default frame - request state
    trsp.putFrame(MODULE_STATE, FRAME_TYPE::REQUEST_GET_DATA);
  }
}


void ProtoState::init(uint8_t moduleIndex, void* buffer,
                      etx_module_state_t* mod_st, uint8_t fAddr)
{
  module_index = moduleIndex;
  trsp.init(buffer, mod_st, fAddr);

  //clear local vars because it is member of union
  moduleData = &g_model.moduleData[module_index];
  state = ModuleState::STATE_NOT_READY;
  modelID = 0xff;  // Valid range 0-19
//  modelIDSet = false;
  clearFrameData();
}

void ProtoState::clearFrameData()
{
//   TRACE("AFHDS3 clearFrameData");
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
  else if (state == ModuleState::STATE_SYNC_RUNNING || state == ModuleState::STATE_SYNC_DONE)
  {
    // Get config when switched to STATE_SYNC_XXX
    trsp.enqueue(COMMAND::MODULE_GET_CONFIG, FRAME_TYPE::REQUEST_GET_DATA);

    // Change connected state and refresh UI
    cfg.others.isConnected = isConnected();
    cfg.others.lastUpdated = get_tmr10ms();
    cfg.others.dirtyFlag = 0U;

    if (state == ModuleState::STATE_SYNC_DONE)
    {
      // Update power config
//       TRACE("Added PWM CMD");
      DIRTY_CMD((&cfg), afhds3::DirtyConfig::DC_RX_CMD_TX_PWR);
    }
  }
}

/*
void ProtoState::requestInfoAndRun(bool send)
{
  // set model ID
  // trsp.enqueue(COMMAND::MODEL_ID, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, true,
  //              g_model.header.modelId[module_index]);

  // RUN
  trsp.enqueue(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, true,
               (uint8_t)MODULE_MODE_E::RUN);

  if (send) { trsp.processQueue(); }
}*/

void ProtoState::parseData(uint8_t* rxBuffer, uint8_t rxBufferCount)
{
  AfhdsFrame* responseFrame = reinterpret_cast<AfhdsFrame*>(rxBuffer);
  if (containsData((enum FRAME_TYPE) responseFrame->frameType)) {
    switch (responseFrame->command) {
      case COMMAND::MODULE_READY:
//         TRACE("AFHDS3 [MODULE_READY] %02X", responseFrame->value);
        if (responseFrame->value == MODULE_STATUS_READY) {
          setState(ModuleState::STATE_READY);
          // requestInfoAndRun();
        }
        else {
          setState(ModuleState::STATE_NOT_READY);
        }
        break;
      case COMMAND::MODULE_GET_CONFIG: {
//        modelcfgGet = false;
//         TRACE("AFHDS3 [MODULE_GET_CONFIG]");
        size_t len = min<size_t>(sizeof(cfg.buffer), rxBufferCount);
        std::memcpy((void*) cfg.buffer, &responseFrame->value, len);
        moduleData->afhds3.emi = cfg.v0.EMIStandard;
        moduleData->afhds3.telemetry = cfg.v0.IsTwoWay;
        moduleData->afhds3.phyMode = cfg.v0.PhyMode;
        cfg.others.ExternalBusType = cfg.v0.ExternalBusType;
//         TRACE("PhyMode %d, emi %d", moduleData->afhds3.phyMode, moduleData->afhds3.emi);
        SET_DIRTY();
        cfg.others.lastUpdated = get_tmr10ms();
      } break;
      case COMMAND::MODULE_VERSION:
        std::memcpy((void*) &version, &responseFrame->value, sizeof(version));
//         TRACE("AFHDS3 [MODULE_VERSION] Product %d, HW %d, BOOT %d, FW %d",
//               version.productNumber, version.hardwareVersion,
//               version.bootloaderVersion, version.firmwareVersion);
        break;
      case COMMAND::MODULE_STATE:
//        TRACE("AFHDS3 [MODULE_STATE] %02X", responseFrame->value);
        setState((ModuleState)responseFrame->value);
        if(STATE_SYNC_DONE == (ModuleState)responseFrame->value){
          if( !this->rx_state )
          {
              auto *cfg = this->getConfig();
              this->rx_state = true;
              DIRTY_CMD( cfg, DC_RX_CMD_GET_RX_VERSION );
              trsp.enqueue( COMMAND::MODULE_VERSION, FRAME_TYPE::REQUEST_GET_DATA );
//            modelcfgGet = true;
//            cfg.others.isConnected = true;
//            cfg.others.lastUpdated = get_tmr10ms();
          }
        }
        else
        {
          this->rx_state = false;
//          cfg.others.isConnected = false;
//          cfg.others.lastUpdated = get_tmr10ms();
        }
        break;
      case COMMAND::MODULE_MODE:
//         TRACE("AFHDS3 [MODULE_MODE] %02X", responseFrame->value);
        if (responseFrame->value != CMD_RESULT::SUCCESS) {
          setState(ModuleState::STATE_NOT_READY);
        }
        break;
      case COMMAND::MODULE_SET_CONFIG:
        if (responseFrame->value != CMD_RESULT::SUCCESS) {
          setState(ModuleState::STATE_NOT_READY);
        }
//         TRACE("AFHDS3 [MODULE_SET_CONFIG], %02X", responseFrame->value);
        break;
      case COMMAND::MODEL_ID:
//         TRACE("AFHDS3 [MODEL_ID]");
        if (responseFrame->value == CMD_RESULT::SUCCESS) {
//         TRACE("Enqueue get config");
//          trsp.enqueue(COMMAND::MODULE_GET_CONFIG, FRAME_TYPE::REQUEST_GET_DATA);
//          trsp.enqueue(COMMAND::MODULE_GET_CONFIG, FRAME_TYPE::REQUEST_GET_DATA);
//          modelcfgGet = true;
        }
        break;
      case COMMAND::TELEMETRY_DATA:
        {
        uint8_t* telemetry = &responseFrame->value;

        if (telemetry[0] == 0x22) {
          telemetry++;
          while (telemetry < rxBuffer + rxBufferCount) {

            uint8_t len = telemetry[0];
            if (len < 4 || telemetry + len > rxBuffer + rxBufferCount)
            {
              break;
            }
            telemetry[0] = 0;
            ::processFlySkyAFHDS3Sensor( telemetry, len-3 );
            telemetry += len;
          }
        }
      }
        break;
      case COMMAND::COMMAND_RESULT: {
        uint8_t *data = &responseFrame->value;
        uint16_t cmd_code = *data++;
        cmd_code |= (*data++)<<8;
        uint8_t result  = *data++;
        auto *cfg = this->getConfig();
        switch (cmd_code)
        {
          case RX_CMD_RSSI_CHANNEL_SETUP:
            if(RX_CMDRESULT::RXSUCCESS==result) {
              clearDirtyFlag(DC_RX_CMD_RSSI_CHANNEL_SETUP);
            } break;
          case RX_CMD_OUT_PWM_PPM_MODE:
            if(RX_CMDRESULT::RXSUCCESS==result) {
              clearDirtyFlag(DC_RX_CMD_OUT_PWM_PPM_MODE);
            } break;
          case RX_CMD_FREQUENCY_V0:
            if(RX_CMDRESULT::RXSUCCESS==result) {
              clearDirtyFlag(DC_RX_CMD_FREQUENCY_V0);
            } break;
          case RX_CMD_PORT_TYPE_V1:
            if(RX_CMDRESULT::RXSUCCESS==result) {
              clearDirtyFlag(DC_RX_CMD_PORT_TYPE_V1);
            } break;
          case RX_CMD_FREQUENCY_V1:
            if(RX_CMDRESULT::RXSUCCESS==result) {
              if (checkDirtyFlag(DC_RX_CMD_FREQUENCY_V1)) {
                clearDirtyFlag(DC_RX_CMD_FREQUENCY_V1);
              } else {
                clearDirtyFlag(DC_RX_CMD_FREQUENCY_V1_2);
              }
            } break;
          case RX_CMD_BUS_TYPE_V0:
            if(RX_CMDRESULT::RXSUCCESS==result) {
              clearDirtyFlag(DC_RX_CMD_BUS_TYPE_V0);
              clearDirtyFlag(DC_RX_CMD_BUS_TYPE_V0_2);
            } break;
          case RX_CMD_IBUS_DIRECTION:
            if(RX_CMDRESULT::RXSUCCESS==*data++) {
              clearDirtyFlag(DC_RX_CMD_BUS_DIRECTION);
              DIRTY_CMD(cfg, DC_RX_CMD_BUS_TYPE_V0_2);
            }break;
          case RX_CMD_GET_VERSION :
            if(RX_CMDRESULT::RXSUCCESS==result) {
              if(14==*data++)
              {
                std::memcpy((void*) &rx_version, data, sizeof(rx_version));
                clearDirtyFlag(DC_RX_CMD_GET_RX_VERSION);
              }
            }break;
        default:
          break;
        }
//         TRACE("AFHDS3 [CMD Result] Cmd: %X, Result: %d, DirtyFlag: %X", cmd_code, result, cfg->others.dirtyFlag);
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

bool ProtoState::syncSettings()
{

  auto *cfg = this->getConfig();

  // Handles old receivers bug
  if ( checkDirtyFlag(DC_RX_CMD_GET_RX_VERSION) )
  {
    uint8_t data[] = { (uint8_t)(RX_CMD_GET_VERSION&0xFF), (uint8_t)((RX_CMD_GET_VERSION>>8)&0xFF), 0x00 };
    trsp.putFrame( COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data) );
    return true;
  }
  // Sync settings when dirty flag is set
  if (checkDirtyFlag(DC_RX_CMD_TX_PWR))
  {
//     TRACE("AFHDS3 [RX_CMD_TX_PWR] %d", AFHDS3_POWER[moduleData->afhds3.rfPower] / 4);
    uint8_t data[] = { (uint8_t)(RX_CMD_TX_PWR&0xFF), (uint8_t)((RX_CMD_TX_PWR>>8)&0xFF), 2,
                       (uint8_t)(AFHDS3_POWER[moduleData->afhds3.rfPower]&0xFF),  (uint8_t)((AFHDS3_POWER[moduleData->afhds3.rfPower]>>8)&0xFF)};
    trsp.putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));
    clearDirtyFlag(DC_RX_CMD_TX_PWR);
    return true;
  }

  if (checkDirtyFlag(DC_RX_CMD_RSSI_CHANNEL_SETUP))
  {
//     TRACE("AFHDS3 [RX_CMD_RSSI_CHANNEL_SETUP]");
    uint8_t data[] = { (uint8_t)(RX_CMD_RSSI_CHANNEL_SETUP&0xFF), (uint8_t)((RX_CMD_RSSI_CHANNEL_SETUP>>8)&0xFF), 1, cfg->v1.SignalStrengthRCChannelNb };
    trsp.putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));
    return true;
  }

  if (checkDirtyFlag(DC_RX_CMD_OUT_PWM_PPM_MODE))
  {
//     TRACE("AFHDS3 [RX_CMD_OUT_PWM_PPM_MODE]");
    uint8_t data[] = { (uint8_t)(RX_CMD_OUT_PWM_PPM_MODE&0xFF), (uint8_t)((RX_CMD_OUT_PWM_PPM_MODE>>8)&0xFF), 1, cfg->v0.AnalogOutput };
    trsp.putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));
    return true;
  }
  if (checkDirtyFlag(DC_RX_CMD_FREQUENCY_V0))
  {
//     TRACE("AFHDS3 [RX_CMD_FREQUENCY_V0]");
    uint16_t Frequency = ((cfg->v0.PWMFrequency.Synchronized<<15)| cfg->v0.PWMFrequency.Frequency);
    uint8_t data[] = { (uint8_t)(RX_CMD_FREQUENCY_V0&0xFF), (uint8_t)((RX_CMD_FREQUENCY_V0>>8)&0xFF), 2,
                        (uint8_t)(Frequency&0xFF), (uint8_t)((Frequency>>8)&0xFF) };
    trsp.putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));
    return true;
  }

  if (checkDirtyFlag(DC_RX_CMD_PORT_TYPE_V1))
  {
//     TRACE("AFHDS3 [RX_CMD_PORT_TYPE_V1]");
    uint8_t data[] = { (uint8_t)(RX_CMD_PORT_TYPE_V1&0xFF), (uint8_t)((RX_CMD_PORT_TYPE_V1>>8)&0xFF), 4, 0, 0, 0, 0 };
    std::memcpy(&data[3], &cfg->v1.NewPortTypes, SES_NPT_NB_MAX_PORTS);
    trsp.putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));
    return true;
  }

  if (checkDirtyFlag(DC_RX_CMD_FREQUENCY_V1))
  {
//     TRACE("AFHDS3 [RX_CMD_FREQUENCY_V1]");
    uint8_t data[32 + 3 + 3] = { (uint8_t)(RX_CMD_FREQUENCY_V1&0xFF), (uint8_t)((RX_CMD_FREQUENCY_V1>>8)&0xFF), 32+3};
    data[3] = 0;
    std::memcpy(&data[4], &cfg->v1.PWMFrequenciesV1.PWMFrequencies[0], 32);
    data[36] = cfg->v1.PWMFrequenciesV1.Synchronized & 0xff;
    data[37] = (cfg->v1.PWMFrequenciesV1.Synchronized>>8) & 0xff;
    trsp.putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));
    DIRTY_CMD(cfg, DC_RX_CMD_FREQUENCY_V1_2);
    return true;
  }

  if (checkDirtyFlag(DC_RX_CMD_FREQUENCY_V1_2))
  {
//     TRACE("AFHDS3 [RX_CMD_FREQUENCY_V1_2]");
    uint8_t data[32 + 3 + 3] = { (uint8_t)(RX_CMD_FREQUENCY_V1_2&0xFF), (uint8_t)((RX_CMD_FREQUENCY_V1_2>>8)&0xFF), 32+3};
    data[3] = 1;
    std::memcpy(&data[4], &cfg->v1.PWMFrequenciesV1.PWMFrequencies[16], 32);
    data[36] = (cfg->v1.PWMFrequenciesV1.Synchronized>>16) & 0xff;
    data[37] = (cfg->v1.PWMFrequenciesV1.Synchronized>>24) & 0xff;
    trsp.putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));
    return true;
  }

  if (checkDirtyFlag(DC_RX_CMD_BUS_TYPE_V0))
  {
//     TRACE("AFHDS3 [RX_CMD_BUS_TYPE_V0]");
    bool onlySupportIBUSOut = (1==receiver_type(rx_version.ProductNumber));

    if (onlySupportIBUSOut && cfg->others.ExternalBusType == EB_BT_IBUS1_IN)
    {
        cfg->others.ExternalBusType = EB_BT_IBUS1_OUT;
    }

    DIRTY_CMD(cfg, DC_RX_CMD_BUS_DIRECTION);
    clearDirtyFlag(DC_RX_CMD_BUS_TYPE_V0);
  }

  if (checkDirtyFlag(DC_RX_CMD_BUS_TYPE_V0_2))
  {
//     TRACE("AFHDS3 [RX_CMD_BUS_TYPE_V0]");
    bool onlySupportIBUSOut = (1==receiver_type(rx_version.ProductNumber));

    if (onlySupportIBUSOut && cfg->others.ExternalBusType == EB_BT_IBUS1_IN)
    {
        cfg->others.ExternalBusType = EB_BT_IBUS1_OUT;
    }

    uint8_t data[] = { (uint8_t)(RX_CMD_BUS_TYPE_V0&0xFF), (uint8_t)((RX_CMD_BUS_TYPE_V0>>8)&0xFF), 1,
                       cfg->others.ExternalBusType == EB_BT_SBUS1 ? EB_BT_SBUS1 : EB_BT_IBUS1};
    trsp.putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));

    return true;
  }

  if (checkDirtyFlag(DC_RX_CMD_BUS_DIRECTION))
  {
    static uint8_t bus_dir;

    if( cfg->others.ExternalBusType == EB_BT_IBUS1_OUT || cfg->others.ExternalBusType == EB_BT_SBUS1 )
        bus_dir = BUS_OUT;
    else
        bus_dir = BUS_IN;
//     TRACE("AFHDS3 [RX_CMD_IBUS_DIRECTION]");
    uint8_t data[4] = { (uint8_t)(RX_CMD_IBUS_DIRECTION&0xFF), (uint8_t)((RX_CMD_IBUS_DIRECTION>>8)&0xFF), 1, bus_dir };
    trsp.putFrame( COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data) );
    return true;
  }

  // No need to sync
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

  trsp.putFrame(COMMAND::CHANNELS_FAILSAFE_DATA, FRAME_TYPE::REQUEST_SET_NO_RESP,
           (uint8_t*)buffer, (channels + 1) * 2);
}

void ProtoState::stop()
{
//   TRACE("AFHDS3 STOP");
  auto mode = (uint8_t)MODULE_MODE_E::STANDBY;
  trsp.putFrame(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, &mode, 1);
}

void ProtoState::resetConfig(uint8_t version)
{
  memclear(&cfg, sizeof(cfg));
  cfg.version = version;

  if (cfg.version == 1) {
    cfg.v1.SignalStrengthRCChannelNb = 0xFF;
    cfg.v1.FailsafeTimeout = 500;
    for (int i = 0; i < SES_NB_MAX_CHANNELS; i++)
      cfg.v1.PWMFrequenciesV1.PWMFrequencies[i] = 50;
  } else {
    cfg.v0.SignalStrengthRCChannelNb = 0xFF;
    cfg.v0.FailsafeTimeout = 500;
    cfg.v0.PWMFrequency.Frequency = 50;
  }
}

void ProtoState::applyConfigFromModel()
{
  uint8_t version = 0;
#if defined(SIMU)
  // TODO: work out why this is not initialised in some cases
  if (moduleData == nullptr) return;
#endif
  if (moduleData->afhds3.phyMode >= ROUTINE_FLCR1_18CH) {
    version = 1;
  }

  if (version != cfg.version) {
    resetConfig(version);
  }

  if (cfg.version == 1) {
    cfg.v1.EMIStandard = moduleData->afhds3.emi;
    cfg.v1.IsTwoWay = moduleData->afhds3.telemetry;
    cfg.v1.PhyMode = moduleData->afhds3.phyMode;

    // Failsafe
    setFailSafe(cfg.v1.FailSafe);
    if (moduleData->failsafeMode != FAILSAFE_NOPULSES) {
      cfg.v1.FailsafeOutputMode = true;
    } else {
      cfg.v1.FailsafeOutputMode = false;
    }
  } else {
    cfg.v0.EMIStandard = moduleData->afhds3.emi;
    cfg.v0.IsTwoWay = moduleData->afhds3.telemetry;
    cfg.v0.PhyMode = moduleData->afhds3.phyMode;
    cfg.v0.ExternalBusType = cfg.others.ExternalBusType==EB_BT_SBUS1 ? EB_BT_SBUS1 : EB_BT_IBUS1;
    // Failsafe
    setFailSafe(cfg.v0.FailSafe);
    if (moduleData->failsafeMode != FAILSAFE_NOPULSES) {
      cfg.v0.FailsafeOutputMode = true;
    } else {
      cfg.v0.FailsafeOutputMode = false;
    }
  }
}

uint8_t receiver_type( unsigned long productnumber )
{
  if(PN_FTR10 == productnumber || PN_FGR4 == productnumber || PN_FTR16S == productnumber)
  {
    return 1; // This type of RX can be set to IBUS-OUT/SBUS
  }
  else if(PN_FTR4 == productnumber)
  {
    return 2; // This type of RX can be set to IBUS-IN/IBUS-OUT/SBUS
  }
  else if( (PN_FTR8B&0xFFFF) <= productnumber)
  {
      return 3;   //V1 RX
  }
  else
  {
      return 0;
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

uint8_t ProtoState::setFailSafe(int16_t* target, uint8_t rfchannelsCount )
{
  int16_t pulseValue = 0;
  uint8_t channels_start = moduleData->channelsStart;
  uint8_t channelsCount = 8 + moduleData->channelsCount;
  uint8_t channels_last = channels_start + channelsCount;
  std::memset(target, 0, 2*rfchannelsCount );
  for (uint8_t channel = channels_start, i=0; i<rfchannelsCount && channel < channels_last; channel++, i++) {
    if (moduleData->failsafeMode == FAILSAFE_CUSTOM) {
      if(FAILSAFE_CHANNEL_HOLD==g_model.failsafeChannels[channel]){
        pulseValue = FAILSAFE_HOLD_VALUE;
      }else if(FAILSAFE_CHANNEL_NOPULSE==g_model.failsafeChannels[channel]){
        pulseValue = FAILSAFE_NOPULSES_VALUE;
      }
      else{
        pulseValue = convert(g_model.failsafeChannels[channel]);
      }
    }
    else if (moduleData->failsafeMode == FAILSAFE_HOLD) {
      pulseValue = FAILSAFE_HOLD_VALUE;
    }
    else if (moduleData->failsafeMode == FAILSAFE_NOPULSES) {
      pulseValue = FAILSAFE_NOPULSES_VALUE;
    }
    else {
      pulseValue = FAILSAFE_NOPULSES_VALUE;
    }
    target[i] = pulseValue;
  }
  //return max channels because channel count can not be change after bind
  return (uint8_t) (AFHDS3_MAX_CHANNELS);
}

Config_u* getConfig(uint8_t module)
{
  auto p_state = &protoState[module];
  return p_state->getConfig();
}

void applyModelConfig(uint8_t module)
{
  auto p_state = &protoState[module];
  p_state->applyConfigFromModel();
}

static const etx_serial_init _uartParams = {
  .baudrate = 0, //AFHDS3_UART_BAUDRATE,
  .encoding = ETX_Encoding_8N1,
  .direction = ETX_Dir_TX_RX,
  .polarity = ETX_Pol_Normal,
};

static void* initModule(uint8_t module)
{
  etx_module_state_t* mod_st = nullptr;
  etx_serial_init params(_uartParams);
  uint16_t period = AFHDS3_UART_COMMAND_TIMEOUT * 1000;
  uint8_t fAddr = (module == INTERNAL_MODULE ? DeviceAddress::IRM301
                                             : DeviceAddress::FRM303)
                      << 4 |
                  DeviceAddress::TRANSMITTER;

  params.baudrate = AFHDS3_UART_BAUDRATE;
  params.polarity =
    module == INTERNAL_MODULE ? ETX_Pol_Normal : ETX_Pol_Inverted;
  mod_st = modulePortInitSerial(module, ETX_MOD_PORT_UART, &params, false);

#if defined(CONFIGURABLE_MODULE_PORT)
  if (!mod_st && module == EXTERNAL_MODULE) {
    // Try Connect using aux serial mod
    params.polarity = ETX_Pol_Normal;
    mod_st = modulePortInitSerial(module, ETX_MOD_PORT_UART, &params, false);
  }
#endif

  if (!mod_st && module == EXTERNAL_MODULE) {
    // soft-serial fallback
    params.baudrate = AFHDS3_SOFTSERIAL_BAUDRATE;
    params.direction = ETX_Dir_TX;
    period = AFHDS3_SOFTSERIAL_COMMAND_TIMEOUT * 1000 /* us */;
    mod_st = modulePortInitSerial(module, ETX_MOD_PORT_SOFT_INV, &params, false);
    // TODO: telemetry RX ???
  }

  if (!mod_st) return nullptr;

  auto p_state = &protoState[module];
  p_state->init(module, pulsesGetModuleBuffer(module), mod_st, fAddr);
  mod_st->user_data = (void*)p_state;

  mixerSchedulerSetPeriod(module, period);

  return mod_st;
}

static void deinitModule(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;
  modulePortDeInit(mod_st);
}

static void sendPulses(void* ctx, uint8_t* buffer, int16_t* channels,
                       uint8_t nChannels)
{
  (void)buffer;
  (void)channels;
  (void)nChannels;

  auto mod_st = (etx_module_state_t*)ctx;
  auto p_state = (ProtoState*)mod_st->user_data;
  p_state->setupFrame();
  p_state->sendFrame();
}

etx_proto_driver_t ProtoDriver = {
    .protocol = PROTOCOL_CHANNELS_AFHDS3,
    .init = initModule,
    .deinit = deinitModule,
    .sendPulses = sendPulses,
    .processData = processTelemetryData,
    .processFrame = nullptr,
    .onConfigChange = nullptr,
};

}  // namespace afhds3
