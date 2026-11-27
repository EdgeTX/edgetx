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
#include "telemetry/flysky_ibus2.h"
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

extern uint16_t  sns_RFCurrentPower;

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

#if defined(AFHDS3_FRM301_EXT)
// FRM301: MODULE_MODE values of the closed (FlySkyOS) protocol - different from
// the open protocol! 3 starts the RF hardware test (LED blue/green/red).
enum CLOSED_MODE : uint8_t {
  CLOSED_STANDBY = 0x01,
  CLOSED_RUN = 0x02,
  CLOSED_HW_TEST = 0x03,  // RF hardware test (LED red/green/blue, state 0xFF) - never sent
};
#endif

enum DeviceAddress {
  TRANSMITTER = 0x01,
  FRM301 = 0x02,  // FlySky FRM301 (Paladin EV module bay)
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

static constexpr uint16_t rfpowerTable[7] = {14*4, 17*4, 20*4, 25*4, 27*4, 30*4, 33*4 };

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
   void sendFrame()
   {
#if defined(AFHDS3_FRM301_EXT)
     if (skipSend) { skipSend = false; return; }
#endif
     trsp.sendBuffer();
   }

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

   bool fifoFull() { return trsp.fifoFull(); }
   uint16_t RFCurrentPower;

  protected:

    void resetConfig(uint8_t version);

  private:
    //friendship declaration - use for passing telemetry
    friend void processTelemetryData(void* ctx, uint8_t data, uint8_t* buffer, uint8_t* len);

    void processTelemetryData(uint8_t data, uint8_t* buffer, uint8_t* len);

    void parseData(uint8_t* rxBuffer, uint8_t rxBufferCount);

    void setState(ModuleState state);

    bool syncSettings();

    bool sensorCalibration();

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

#if defined(AFHDS3_FRM301_EXT)
    /*
     * FlySky FRM301 (Paladin EV module bay). Same port as the FRM303, but it
     * speaks the FlySkyOS ("closed") variant of AFHDS3 with device address 2:
     * the module also sends requests to the radio (config parts, receiver
     * block), MODULE_MODE values differ and it keeps no bindings itself.
     * Auto detection: without a reply the address is toggled until the
     * module answers.
     */
  public:
    bool isFRM301() const
    {
      return (trsp.getAddress() >> 4) != DeviceAddress::FRM303;
    }
    bool isClosedMode() const { return trsp.closedMode; }
    // called from the UI task: store receiver block / options in the model
    void storeInModel()
    {
      if (rxBlockSave) { rxBlockSave = false; saveRxBlock(); }
      if (optionsSave) { optionsSave = false; saveClosedOptions(); }
    }

  private:
    uint8_t getDevAddress() const { return trsp.getAddress() >> 4; }
    void handleNoReply();
    void setDevAddress(uint8_t dev);
    uint8_t selectedDev() const;
    bool handleModuleRequest(bool immediate = false);
    void modulePowerCycle();
    uint8_t buildClosedConfig(uint8_t part, uint8_t* out);
    void loadRxBlock();
    void saveRxBlock();
    void saveClosedOptions();

    bool addrConfirmed;
    uint8_t noReplyCount;
    uint8_t autoIdx;
    uint8_t pollToggle;
    bool skipSend;            // keep the line free in this tick
    tmr10ms_t lastReqTime;    // last request from the module
    uint8_t quietToggle;
    uint8_t cfgPush;          // 1..5 = next config push step, 0 = idle
    uint8_t cfgKind;          // 1 = bind, 2 = settings only
    uint8_t bindStep;         // bind running
    tmr10ms_t bindTime;       // last bind push
    uint8_t postBind;         // after bind: 1 = push settings, 3 = wait
    tmr10ms_t postBindTime;
    uint8_t pwrPhase;         // bind restart: 1 = module off, 2 = booting
    tmr10ms_t pwrTime;
    bool pwrReady;            // module reported state 3/5 after the restart
    bool closedLinkUp;        // module up: send channel data continuously
    uint8_t chPoll;
    uint8_t lastPart;         // last config part requested by the module
    uint8_t p2rep;            // repeated requests of config part 2
    bool p2NoEndByte;         // part 2 without the FRM301 end byte
    bool cmd7030Pending;      // RX command 0x7030 after connect
    bool rxBlockNew;          // receiver block received during this bind
    volatile bool rxBlockSave;
    volatile bool optionsSave;
    bool rxBlockValid;
    uint8_t rxBlock[168];     // receiver block (bind result incl. RX ID)
#endif

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
#if defined(AFHDS3_FRM301_EXT)
  if (trsp.closedMode && (uint8_t)state == 0xFF) {
    strcpy(buffer, "HW test");
    return;
  }
#endif
  strcpy(buffer, state <= ModuleState::STATE_READY ? moduleStateText[state]
                                                   : "Unknown");
}

void ProtoState::processTelemetryData(uint8_t byte, uint8_t* buffer, uint8_t* len)
{
  uint8_t maxSize = TELEMETRY_RX_PACKET_SIZE;
  if (!trsp.processTelemetryData(byte, buffer, *len, maxSize)) {
#if defined(AFHDS3_FRM301_EXT)
    // FRM301: answer module requests at once (FlySkyOS does, the module
    // only waits a short time); if the port is busy the next tick answers
    if (*len == 0 && trsp.closedMode && trsp.modReq.valid && trsp.txIdle())
      handleModuleRequest(true);
#endif
    return;
  }

  parseData(buffer, *len);
  *len = 0;
#if defined(AFHDS3_FRM301_EXT)
  // FRM301: SET requests of the module (e.g. receiver block 0x05) too
  if (trsp.closedMode && trsp.modReq.valid && trsp.txIdle())
    handleModuleRequest(true);
#endif
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

uint8_t ibus_type[SES_NPT_NB_MAX_PORTS] = {SES_NPT_IBUS1_IN};
void setIbusType(uint8_t* ibus_type_buf) 
{
  for(uint8_t i = 0; i< SES_NPT_NB_MAX_PORTS; i++) {
   if (ibus_type_buf[i] == afhds3::SES_NPT_IBUS2 || ibus_type_buf[i] == afhds3::SES_NPT_IBUS2_HUB_PORT) {
    ibus_type[i] = afhds3::SES_NPT_IBUS2;
   } else {
    ibus_type[i] = afhds3::SES_NPT_IBUS1_IN;
   }
  }
}

#if defined(AFHDS3_FRM301_EXT)
// called from perMain() (UI task): store the receiver block / options in the
// model - never from the mixer task
void frmBackgroundTask()
{
  protoState[EXTERNAL_MODULE].storeInModel();
}

// switch the module bay off; setupFrame() switches it on again after 300 ms
void ProtoState::modulePowerCycle()
{
  modulePortSetPower(module_index, 0);
  pwrPhase = 1;
  pwrReady = false;
  pwrTime = get_tmr10ms();
  closedLinkUp = false;
  cfgPush = 0;
  lastPart = 0;
  lastReqTime = 0;
  trsp.modReq.valid = false;
  state = ModuleState::STATE_NOT_READY;
  clearFrameData();
}
#endif

void ProtoState::setupFrame()
{
#if defined(AFHDS3_FRM301_EXT)
  if (module_index == EXTERNAL_MODULE) {
    uint8_t want = selectedDev();
    if (want && want != getDevAddress()) {
      setDevAddress(want);
      this->state = ModuleState::STATE_NOT_READY;
      clearFrameData();
    }
  }
  // FRM301: answer pending requests of the module first
  if (trsp.closedMode && handleModuleRequest()) return;
  // while the module itself is asking (config phase) keep the line free on
  // every second tick, so its requests and our own commands both get through
  if (trsp.closedMode && lastReqTime &&
      (tmr10ms_t)(get_tmr10ms() - lastReqTime) < 5) {
    quietToggle ^= 1;
    if (quietToggle) {
      skipSend = true;
      return;
    }
  }
  // FRM301: push the config like FlySkyOS: 0x04 <kind> part 0..3, for a
  // bind (kind 1) followed by 0x05
  if (trsp.closedMode && cfgPush > 0 && !trsp.isAwaiting()) {
    static uint8_t out[184];
    if (cfgPush <= 4) {
      uint8_t part = cfgPush - 1;
      out[0] = cfgKind;
      uint8_t n;
      if (part == 0) { out[1] = 0; n = 2; }
      else n = 1 + buildClosedConfig(part, out + 1);
      trsp.putFrame(COMMAND::MODULE_SET_CONFIG,
                    FRAME_TYPE::REQUEST_SET_EXPECT_DATA, out, n);
    } else {
      trsp.putFrame((COMMAND)0x05, FRAME_TYPE::REQUEST_SET_EXPECT_DATA);
    }
    cfgPush = (cfgPush >= 5 || (cfgKind == 2 && cfgPush >= 4)) ? 0 : cfgPush + 1;
    return;
  }
  // FRM301 bind restart: module bay off for 300 ms, then on again - the
  // module boots and asks for its config like after power-on
  if (pwrPhase == 1) {
    if ((tmr10ms_t)(get_tmr10ms() - pwrTime) < 30) {
      skipSend = true;
      return;
    }
    modulePortSetPower(module_index, 1);
    pwrPhase = 2;
    pwrTime = get_tmr10ms();
    state = ModuleState::STATE_NOT_READY;
    clearFrameData();
    skipSend = true;
    return;
  }
#endif
  bool trsp_error = false;
  if (trsp.handleRetransmissions(trsp_error)) return;

  if (trsp_error) {
#if defined(AFHDS3_FRM301_EXT)
    // FRM301: the module is busy with its own requests, our poll just was
    // not answered - keep the state it reported
    if (!(trsp.closedMode && lastReqTime &&
          (tmr10ms_t)(get_tmr10ms() - lastReqTime) < 50))
#endif
    this->state = ModuleState::STATE_NOT_READY;
    clearFrameData();
#if defined(AFHDS3_FRM301_EXT)
    handleNoReply();
#endif
  }

  if (this->state == ModuleState::STATE_NOT_READY) {
//     TRACE("AFHDS3 [GET MODULE READY]");
#if defined(AFHDS3_FRM301_EXT)
    // FRM301: FlySkyOS starts with "get module state" (0x02) instead of 0x01
    if (trsp.closedMode && (pollToggle ^= 1)) {
      trsp.putFrame(COMMAND::MODULE_STATE, FRAME_TYPE::REQUEST_GET_DATA);
      return;
    }
#endif
    trsp.putFrame(COMMAND::MODULE_READY, FRAME_TYPE::REQUEST_GET_DATA);
    return;
  }

  // Process backlog, not check states
  if (trsp.processQueue()) return;

#if defined(AFHDS3_FRM301_EXT)
  if (trsp.closedMode && cmd7030Pending && (uint8_t)state == 4 && !cfgPush) {
    cmd7030Pending = false;
    uint8_t d[4] = {0x30, 0x70, 0x01, 0x01};
    trsp.putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA,
                  d, 4);
    return;
  }
#endif

  ::ModuleSettingsMode moduleMode = getModuleMode(module_index);

#if defined(AFHDS3_FRM301_EXT)
  if (trsp.closedMode && moduleMode == ::ModuleSettingsMode::MODULE_MODE_BIND &&
      bindStep == 1 &&
      ((uint8_t)state == 4 || (rxBlockNew && (uint8_t)state != 2))) {
    // receiver bound (module sent the receiver block): leave bind mode,
    // the NORMAL branch then hands the block to the module
    setModuleMode(module_index, ::ModuleSettingsMode::MODULE_MODE_NORMAL);
    moduleMode = ::ModuleSettingsMode::MODULE_MODE_NORMAL;
  }
#endif
  if (moduleMode == ::ModuleSettingsMode::MODULE_MODE_BIND) {
    if (state != STATE_BINDING) {
//       TRACE("AFHDS3 [BIND]");
      applyConfigFromModel();
#if defined(AFHDS3_FRM301_EXT)
      if (trsp.closedMode) {
        // FRM301 bind like FlySkyOS: push the whole config as kind 1
        // (0x04 <1> part 0..3) followed by 0x05 - this makes the module
        // bind (state 2, LED fast green). Repeated every 3 s until the
        // module reports binding (2) or connected (4).
        uint8_t st = (uint8_t)state;
        if (bindStep == 0) {
          // restart the module first (power off/on): it only starts a
          // bind reliably right after power-on
          rxBlockNew = false;
          postBind = 0;
          bindStep = 1;
          modulePowerCycle();
          skipSend = true;
          return;
        }
        if (pwrPhase == 2) {
          // module booting: it fetches its config (part 1..3) and reports
          // state 3 or 5 - bind 1.5 s later (at the latest after 6 s)
          bool up = (lastPart == 3 && cfgPush == 0 && (st == 3 || st == 5));
          if (up && !pwrReady) {
            pwrReady = true;
            pwrTime = get_tmr10ms();
          } else if ((pwrReady && (tmr10ms_t)(get_tmr10ms() - pwrTime) >= 150) ||
                     (!pwrReady && (tmr10ms_t)(get_tmr10ms() - pwrTime) >= 600)) {
            pwrPhase = 0;
            bindTime = get_tmr10ms() - 300;  // push now
          }
        }
        if (pwrPhase == 0 && st != 2 && st != 4 && cfgPush == 0 &&
            (tmr10ms_t)(get_tmr10ms() - bindTime) >= 300) {
          bindTime = get_tmr10ms();
          cfgKind = 1;
          cfgPush = 1;
          return;
        }
        // otherwise fall through to the normal polling below - NOT into the
        // open protocol bind (its config is garbage for the closed module
        // and its MODE 2 = BIND means RUN here: it stopped the binding)
      }
      if (!trsp.closedMode) {
#endif

      trsp.putFrame(COMMAND::MODULE_SET_CONFIG,
                     FRAME_TYPE::REQUEST_SET_EXPECT_DATA, cfg.buffer,
                     cfg.version == 0 ? sizeof(cfg.v0) : sizeof(cfg.v1));

      trsp.enqueue(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, true,
                   (uint8_t)MODULE_MODE_E::BIND);
      return;
#if defined(AFHDS3_FRM301_EXT)
      }
#endif
    }
  }
  else if (moduleMode == ::ModuleSettingsMode::MODULE_MODE_RANGECHECK) {
    TRACE("AFHDS3 [RANGE CHECK] not supported");
  }
  else if (moduleMode == ::ModuleSettingsMode::MODULE_MODE_NORMAL) {
#if defined(AFHDS3_FRM301_EXT)
    if (trsp.closedMode && bindStep) {
      // bind ended
      bindStep = 0;
      if (pwrPhase == 2) pwrPhase = 0;
      if (rxBlockNew) {
        // bound: give the module the new receiver block (config part 3)
        postBind = 1;
      } else {
        uint8_t mode = CLOSED_RUN;
        trsp.putFrame(COMMAND::MODULE_MODE,
                      FRAME_TYPE::REQUEST_SET_EXPECT_DATA, &mode, 1);
        return;
      }
    }
    if (trsp.closedMode && postBind) {
      // after bind: FlySkyOS only stores the receiver block and treats the
      // module as connected - it sends nothing. Wait up to 5 s for state 4
      // (Connected), otherwise MODE 2 (run) once.
      if (postBind == 1) {
        // the new receiver block (with ID) as settings, kind 2 (no 0x05)
        cfgKind = 2;
        cfgPush = 1;
        postBind = 3;
        postBindTime = get_tmr10ms();
        return;
      } else if (cfgPush) {
      } else if ((uint8_t)state == 4) {
        postBind = 0;
      } else if ((tmr10ms_t)(get_tmr10ms() - postBindTime) >= 500) {
        postBind = 0;
        uint8_t mode = CLOSED_RUN;
        trsp.putFrame(COMMAND::MODULE_MODE,
                      FRAME_TYPE::REQUEST_SET_EXPECT_DATA, &mode, 1);
        return;
      }
    }
#endif

    // If module is ready but not started
    if (this->state == ModuleState::STATE_READY) {
      trsp.putFrame(MODULE_STATE, FRAME_TYPE::REQUEST_GET_DATA);
      return;
    }

    // ModelID change detection, if changed => STATE_STANDBY => Set ModelID => RUN mode => STATE_SYNC_XXX
    uint8_t newModelID = g_model.header.modelId[module_index] % MAX_NO_OF_MODELS;  // Rotate the model ID when exceed no. of stored models
#if defined(AFHDS3_FRM301_EXT)
    // FRM301: the FlySkyOS protocol has no MODEL_ID command (0x2F)
    if (trsp.closedMode) modelID = newModelID;
#endif
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
#if defined(AFHDS3_FRM301_EXT)
      if (trsp.closedMode) mode = CLOSED_RUN;
#endif
      trsp.putFrame(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, &mode, 1);
      return;
    }

    // Exit bind
/*    if (this->state == STATE_BINDING) {
      TRACE("AFHDS3 [EXIT BIND]");
//      modelcfgGet = true;
      auto mode = (uint8_t)MODULE_MODE_E::RUN;
#if defined(AFHDS3_FRM301_EXT)
      if (trsp.closedMode) mode = CLOSED_RUN;
#endif
      trsp.putFrame(COMMAND::MODULE_MODE, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, &mode, 1);
      trsp.enqueue(COMMAND::MODULE_GET_CONFIG, FRAME_TYPE::REQUEST_GET_DATA);
      return;
    } */
  }

/*  if (modelcfgGet) {
    trsp.enqueue(COMMAND::MODULE_GET_CONFIG, FRAME_TYPE::REQUEST_GET_DATA);
    return;
  }*/

#if defined(AFHDS3_FRM301_EXT)
  // FRM301: no open protocol extras (virtual failsafe, RX commands 0x0C with
  // open protocol codes) - FlySkyOS sends only channels and state polls
  if (trsp.closedMode) cmdCount = 0;
#endif
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

  // Sync settings when dirty flag is set
  auto *cfg = this->getConfig();
#if defined(AFHDS3_FRM301_EXT)
  if (trsp.closedMode) {
    clearDirtyFlag(DC_RX_CMD_TX_PWR);
    // module options changed in the options page (bus type, PWM ...):
    // store them in the model and hand them to the module as settings
    // (config kind 2), like FlySkyOS after a settings change
    const uint32_t optMask =
        (1u << DC_RX_CMD_FAILSAFE_VALUE) | (1u << DC_RX_CMD_FAILSAFE_TIME) |
        (1u << DC_RX_CMD_RSSI_CHANNEL_SETUP) |
        (1u << DC_RX_CMD_OUT_PWM_PPM_MODE) | (1u << DC_RX_CMD_FREQUENCY_V0) |
        (1u << DC_RX_CMD_PORT_TYPE_V1) | (1u << DC_RX_CMD_FREQUENCY_V1) |
        (1u << DC_RX_CMD_BUS_TYPE_V0) | (1u << DC_RX_CMD_IBUS_SETUP) |
        (1u << DC_RX_CMD_BUS_FAILSAFE);
    bool optChanged = (cfg->others.dirtyFlag & optMask) != 0;
    cfg->others.dirtyFlag = 0;  // the open protocol RX commands are not used
    if (optChanged) {
      optionsSave = true;
      uint8_t st = (uint8_t)state;
      if (!cfgPush && !bindStep && (st == 3 || st == 4 || st == 5)) {
        cfgKind = 2;
        cfgPush = 1;
        return;
      }
    }
  }
#endif
  if (checkDirtyFlag(DC_RX_CMD_TX_PWR))
  {
//     TRACE("AFHDS3 [RX_CMD_TX_PWR] %d", AFHDS3_POWER[moduleData->afhds3.rfPower] / 4);
    uint8_t data[] = { (uint8_t)(RX_CMD_TX_PWR&0xFF), (uint8_t)((RX_CMD_TX_PWR>>8)&0xFF), 2,
                       (uint8_t)(AFHDS3_POWER[moduleData->afhds3.rfPower]&0xFF),  (uint8_t)((AFHDS3_POWER[moduleData->afhds3.rfPower]>>8)&0xFF)};
    trsp.putFrame(COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, sizeof(data));
    clearDirtyFlag(DC_RX_CMD_TX_PWR);
    RFCurrentPower = (AFHDS3_POWER[moduleData->afhds3.rfPower]&0xFF);
    return;
  }
  else if( EXTERNAL_MODULE == module_index )
  {
    if( !RFCurrentPower )
    {
      RFCurrentPower = sns_RFCurrentPower;
    }
  }

  if (isConnected()) {
    // Sync config, with commands
#if defined(AFHDS3_FRM301_EXT)
    if (!trsp.closedMode) {
#endif
    if (syncSettings()) { return; }

    if (sensorCalibration()) { return; }
#if defined(AFHDS3_FRM301_EXT)
    }
#endif

    // Send channels data
    sendChannelsData();
  } else {
#if defined(AFHDS3_FRM301_EXT)
    // FRM301: FlySkyOS sends the channel data in every mixer cycle as soon
    // as the module is up - also while binding and while it searches its
    // receiver - and asks the state only about once per second
    if (trsp.closedMode && closedLinkUp && pwrPhase == 0) {
      if (++chPoll >= 100) {
        chPoll = 0;
        trsp.putFrame(MODULE_STATE, FRAME_TYPE::REQUEST_GET_DATA);
      } else {
        sendChannelsData();
      }
      return;
    }
#endif
    //default frame - request state
    trsp.putFrame(MODULE_STATE, FRAME_TYPE::REQUEST_GET_DATA);
  }
}

uint8_t get_current_rfpower_level( uint8_t module )
{
  int16_t diff_min = protoState[module].RFCurrentPower-rfpowerTable[0];
  uint8_t power_level = 0;
  for( uint8_t i=1; i<7; i++ )
  {
    int16_t diff = protoState[module].RFCurrentPower-rfpowerTable[i];

    if( abs(diff) < abs(diff_min))
    {
      diff_min = diff;
      power_level = i;
    }
  }
  return power_level;
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
#if defined(AFHDS3_FRM301_EXT)
  addrConfirmed = false;
  noReplyCount = 0;
  autoIdx = 0;
  pollToggle = 0;
  skipSend = false;
  lastReqTime = 0;
  quietToggle = 0;
  cfgPush = 0;
  cfgKind = 1;
  bindStep = 0;
  bindTime = 0;
  postBind = 0;
  postBindTime = 0;
  pwrPhase = 0;
  pwrTime = 0;
  pwrReady = false;
  closedLinkUp = false;
  chPoll = 0;
  lastPart = 0;
  p2rep = 0;
  p2NoEndByte = false;
  cmd7030Pending = false;
  rxBlockNew = false;
  rxBlockSave = false;
  optionsSave = false;
  rxBlockValid = false;
  memclear(rxBlock, sizeof(rxBlock));
  loadRxBlock();
  if (module_index == EXTERNAL_MODULE) {
    uint8_t dev = selectedDev();
    setDevAddress(dev ? dev : DeviceAddress::FRM303);
  }
#endif
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
#if defined(AFHDS3_FRM301_EXT)
  if (trsp.closedMode) {
    uint8_t s = (uint8_t)state;
    closedLinkUp = (s >= 2 && s <= 5);
  }
  // FRM301: the module may leave state 2 without a bound receiver, FlySkyOS
  // then simply triggers the bind again - so do not end bind here
  if (oldState == ModuleState::STATE_BINDING && !trsp.closedMode)
#else
  if (oldState == ModuleState::STATE_BINDING)
#endif
  {
    setModuleMode(module_index, ::ModuleSettingsMode::MODULE_MODE_NORMAL);
  }
  if (state == ModuleState::STATE_NOT_READY) {
    trsp.clear();
  }
#if defined(AFHDS3_FRM301_EXT)
  if (trsp.closedMode) {
    // FRM301: reactions of FlySkyOS to the state reported by the module
    switch ((uint8_t)state) {
      case 1:  // module wants its configuration
        cfgPush = 1;
        cfgKind = 1;
        break;
      case 4:  // connected: FlySkyOS sends RX command 0x7030 = 1, the
               // module answers with 0x0D later
        cmd7030Pending = true;
        break;
      case 5:  // standby: FlySkyOS asks 0x10
        trsp.enqueue((COMMAND)0x10, FRAME_TYPE::REQUEST_GET_DATA);
        break;
      default:
        break;
    }
  }
#endif
  else if (state == ModuleState::STATE_SYNC_RUNNING || state == ModuleState::STATE_SYNC_DONE)
  {
    // Get config when switched to STATE_SYNC_XXX
    trsp.enqueue(COMMAND::MODULE_GET_CONFIG, FRAME_TYPE::REQUEST_GET_DATA);

    // Change connected state and refresh UI
    cfg.others.isConnected = isConnected();
    cfg.others.lastUpdated = get_tmr10ms();
    cfg.others.dirtyFlag = 0U;
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

#if defined(AFHDS3_FRM301_EXT)
/*
 * FRM301 (FlySkyOS closed protocol): the module requests its configuration
 * from the radio with "GET 0x04 <part>". Layout as sent by FlySkyOS
 * (PL18EV firmware 1.0.9, verified with a serial capture):
 *
 * part 1 (V1 = Routine/LoRa): 01 01 freq[32]*u16 sync:u32 portTypes[4]
 *                             flag 0 0                       (77 bytes)
 * part 1 (V0 = Classic):      01 00 freq[32]*u16 0:u32 analog freq:u16
 *                             bus flag                       (75 bytes)
 * part 2: 02 0 0 0 0 0C 0*11 0 timeout:u32 chCount failsafe[32]*i16
 *         chMode[32] FF*32 0                                 (152 bytes)
 * part 3: 03 + 168 byte receiver block (mostly zero)         (169 bytes)
 */
// PHY constants FlySkyOS writes to receiver block offset 0x92..0x9c
static const uint8_t closedPhyConst[3][11] = {
  {0x03, 0x70, 0x04, 0x10, 0x00, 0x00, 0x30, 0x08, 0xc7, 0x00, 0x00},  // 18ch
  {0x01, 0x0c, 0x80, 0x00, 0x00, 0x40, 0x00, 0x00, 0x50, 0x18, 0x05},  // LoRa 12ch
  {0x03, 0x70, 0x04, 0x10, 0x00, 0x00, 0x30, 0x08, 0x45, 0x00, 0x00},  // 8ch
};

// receiver block FlySkyOS sends for a new bind (Classic 18CH, two way),
// taken from a serial capture
static const uint8_t closedDefaultBlock[168] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0xff, 0x00, 0x03, 0x0a, 0x0a, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
  0x00, 0xe6, 0x00, 0x04, 0x00, 0x21, 0x00, 0x15, 0x00, 0x13, 0x00, 0x0e,
  0x00, 0x0d, 0x00, 0x0c, 0x00, 0x24, 0x00, 0x01, 0x00, 0x18, 0x00, 0x19,
  0x00, 0x03, 0x00, 0x25, 0x00, 0x08, 0x00, 0x14, 0x00, 0x1d, 0x00, 0x05,
  0x00, 0x22, 0x00, 0x16, 0x00, 0x14, 0x00, 0x0f, 0x00, 0x10, 0x00, 0x12,
  0x00, 0x27, 0x00, 0x02, 0x00, 0x1a, 0x00, 0x1b, 0x00, 0x11, 0x00, 0x26,
  0x00, 0x0b, 0x00, 0x19, 0x00, 0x07, 0x00, 0x38, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x02, 0x03, 0x70, 0x04, 0x10, 0x00, 0x00, 0x30, 0x08, 0xc7, 0x00,
  0x00, 0x0a, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

uint8_t ProtoState::buildClosedConfig(uint8_t part, uint8_t* out)
{
  applyConfigFromModel();
  uint8_t phy = moduleData->afhds3.phyMode;
  bool v1 = phy >= ROUTINE_FLCR1_18CH;
  uint8_t chCount = _phyMode_channels[phy];
  uint16_t n = 0;
  auto put8 = [&](uint8_t v) { out[n++] = v; };
  auto put16 = [&](uint16_t v) { out[n++] = v & 0xFF; out[n++] = v >> 8; };
  auto put32 = [&](uint32_t v) { put16(v & 0xFFFF); put16(v >> 16); };

  if (part == 1) {
    put8(1);
    put8(v1 ? 1 : 0);
    for (uint8_t i = 0; i < 32; i++) put16(50);  // PWM frequency
    put32(0);                                    // synchronised PWM
    if (v1) {
      for (uint8_t i = 0; i < 4; i++) put8(cfg.v1.NewPortTypes[i]);
      put8(0);
      put8(0);
      put8(0);
    } else {
      put8(0);    // analog output PWM
      put16(50);  // PWM frequency
      put8(cfg.v0.ExternalBusType == EB_BT_SBUS1 ? 2 : 0);
      put8(1);
    }
  } else if (part == 2) {
    put8(2);
    for (uint8_t i = 0; i < 4; i++) put8(0);
    put32(0x0C);
    for (uint8_t i = 0; i < 9; i++) put8(0);
    put32(500);  // failsafe timeout
    put8(chCount);
    int16_t fs[32];
    for (uint8_t i = 0; i < 32; i++) fs[i] = FAILSAFE_NOPULSES_VALUE;
    setFailSafe(fs, chCount);
    if (moduleData->failsafeMode == FAILSAFE_NOT_SET ||
        moduleData->failsafeMode == FAILSAFE_NOPULSES) {
      for (uint8_t i = 0; i < 32; i++) fs[i] = FAILSAFE_NOPULSES_VALUE;
    }
    int16_t last = fs[0];
    for (uint8_t i = 0; i < 32; i++) {
      if (i >= 30) last = FAILSAFE_HOLD_VALUE;
      else if (i < chCount) last = fs[i];
      put16((uint16_t)last);
    }
    for (uint8_t i = 0; i < 32; i++) put8(0x1C);  // channel modes
    for (uint8_t i = 0; i < 32; i++) put8(0xFF);
    // end byte: sent by FlySkyOS and expected by current FRM301 firmware,
    // older firmware only accepts part 2 without it
    if (!p2NoEndByte) put8(0);
  } else if (part == 3) {
    put8(3);
    uint8_t* blk = out + n;
    memcpy(blk, closedDefaultBlock, 168);
    n += 168;
    if (rxBlockValid && !bindStep) {
      // receiver block the module sent after binding (cmd 0x05). FlySkyOS
      // overwrites the fields below in the stored block before sending.
      for (uint8_t i = 0; i < 168; i++) blk[i] = rxBlock[i];
    }
    // 3 = two way (telemetry), 0 = one way
    blk[0x44] = moduleData->afhds3.telemetry ? 3 : 0;
    uint16_t t = 500 - 230;  // failsafe time as stored by FlySkyOS (300 ms -> 0x46)
    blk[0x34] = t & 0xFF;
    blk[0x35] = t >> 8;
    blk[0x36] = 0xFF;  // no RSSI channel
    uint16_t pwr = AFHDS3_POWER[moduleData->afhds3.rfPower % 7];
    blk[0x8b] = pwr & 0xFF;
    blk[0x8c] = pwr >> 8;
    if (v1) {
      const uint8_t* c = closedPhyConst[phy == ROUTINE_LORA_12CH  ? 1
                                        : phy == ROUTINE_FLCR6_8CH ? 2
                                                                   : 0];
      for (uint8_t i = 0; i < 11; i++) blk[0x92 + i] = c[i];
    }
  }
  return n;
}

// The receiver block (bind result incl. receiver ID) and the module options
// are stored in the model as user data (hex strings). The FRM303 keeps both
// inside the module; the FRM301 does not, so the radio hands them over on
// every start, like FlySkyOS does.
#define FRM301_UD_RXBLOCK "FRM301|rxBlock"
#define FRM301_UD_OPTIONS "FRM301|options"
#define FRM301_CFG_LEN (sizeof(sDATA_ConfigV1) + 1)  // config + ExternalBusType

static void hexEncode(char* dst, const uint8_t* src, size_t n)
{
  static const char hx[] = "0123456789ABCDEF";
  for (size_t i = 0; i < n; i++) {
    *dst++ = hx[src[i] >> 4];
    *dst++ = hx[src[i] & 0x0F];
  }
  *dst = 0;
}

static int hexNibble(char c)
{
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;
}

// true if the user data entry holds exactly n bytes as hex
static bool hexDecode(uint8_t* dst, size_t n, const char* key)
{
  auto ud = g_model.getUserData(key);
  if (!ud || ud->value.size() != 2 * n) return false;
  const char* src = ud->value.c_str();
  for (size_t i = 0; i < n; i++) {
    int h = hexNibble(src[2 * i]), l = hexNibble(src[2 * i + 1]);
    if (h < 0 || l < 0) return false;
    dst[i] = (uint8_t)(h << 4 | l);
  }
  return true;
}

void ProtoState::loadRxBlock()
{
  if (module_index != EXTERNAL_MODULE) return;
  uint8_t tmp[sizeof(rxBlock)];
  if (hexDecode(tmp, sizeof(tmp), FRM301_UD_RXBLOCK)) {
    memcpy(rxBlock, tmp, sizeof(rxBlock));
    rxBlockValid = true;
  }
  // module options (bus type, PWM ...): only if they match the PHY version
  uint8_t c[FRM301_CFG_LEN];
  if (hexDecode(c, sizeof(c), FRM301_UD_OPTIONS)) {
    bool v1 = moduleData->afhds3.phyMode >= ROUTINE_FLCR1_18CH;
    if (c[0] == (v1 ? 1 : 0)) memcpy(cfg.buffer, c, FRM301_CFG_LEN);
  }
}

void ProtoState::saveRxBlock()
{
  if (module_index != EXTERNAL_MODULE) return;
  char hex[2 * sizeof(rxBlock) + 1];
  hexEncode(hex, rxBlock, sizeof(rxBlock));
  g_model.setUserData(FRM301_UD_RXBLOCK, hex);
}

void ProtoState::saveClosedOptions()
{
  if (module_index != EXTERNAL_MODULE) return;
  char hex[2 * FRM301_CFG_LEN + 1];
  hexEncode(hex, cfg.buffer, FRM301_CFG_LEN);
  g_model.setUserData(FRM301_UD_OPTIONS, hex);
}

bool ProtoState::handleModuleRequest(bool immediate)
{
  auto& req = trsp.modReq;
  if (!req.valid) return false;
  lastReqTime = get_tmr10ms();

  // the module is talking to us: keep (or learn) its address, stop searching
  addrConfirmed = true;
  noReplyCount = 0;
  if (module_index == EXTERNAL_MODULE && !selectedDev() &&
      (req.address >> 4) == DeviceAddress::TRANSMITTER) {
    uint8_t src = req.address & 0x0F;
    if (src && src != DeviceAddress::TRANSMITTER && src != getDevAddress())
      setDevAddress(src);
  }

  static uint8_t resp[180];
  uint8_t len = 0;
  bool answer = true;
  FRAME_TYPE rtype = FRAME_TYPE::RESPONSE_DATA;

  if (req.frameType == FRAME_TYPE::REQUEST_SET_EXPECT_ACK) {
    rtype = FRAME_TYPE::RESPONSE_ACK;  // e.g. receiver block 0x05: plain ACK
    len = 0;
  } else if (req.frameType == FRAME_TYPE::REQUEST_SET_EXPECT_DATA) {
    len = 0;  // FlySkyOS answers with an empty RESPONSE_DATA
  } else if (req.command == COMMAND::MODULE_READY) {
    resp[0] = 0x02;  // radio ready, RF on
    len = 1;
  } else if (req.command == COMMAND::MODULE_SET_CONFIG) {
    uint8_t part = req.len ? req.data[0] : 1;
    if (part != lastPart) {
      lastPart = part;
      p2rep = 0;
    } else if (part == 2 && ++p2rep >= 3) {
      // part 2 not accepted (asked 3x) - try the other layout
      p2rep = 0;
      p2NoEndByte = !p2NoEndByte;
    }
    len = buildClosedConfig(part, resp);
    answer = len > 0;
  } else {
    answer = false;  // unknown request (e.g. firmware update)
  }

  if (answer) {
    if (immediate) {
      static uint8_t respFrame[MODULE_BUFFER_SIZE] __DMA;
      trsp.sendResponseNow(respFrame, req.command, req.frameNumber, resp, len,
                           rtype);
    } else {
      trsp.putResponse(req.command, req.frameNumber, resp, len, rtype);
    }
  }
  req.valid = false;
  return answer;
}

// model setting (afhds3.reserved): 0 = auto, 1 = FRM303, 2 = FRM301
uint8_t ProtoState::selectedDev() const
{
  switch (moduleData->afhds3.reserved) {
    case 1: return DeviceAddress::FRM303;
    case 2: return DeviceAddress::FRM301;
    default: return 0;
  }
}

void ProtoState::setDevAddress(uint8_t dev)
{
  trsp.setAddress(dev << 4 | DeviceAddress::TRANSMITTER);
  // FRM301: FlySkyOS (closed) protocol, the module also sends requests
  trsp.closedMode = (dev != DeviceAddress::FRM303);
  // FlySkyOS sends a frame every 2.5 ms to the FRM301, the FRM303 uses 5 ms
  if (module_index == EXTERNAL_MODULE)
    mixerSchedulerSetPeriod(module_index, trsp.closedMode
                                              ? 2500
                                              : AFHDS3_UART_COMMAND_TIMEOUT * 1000);
}

void ProtoState::handleNoReply()
{
  if (module_index != EXTERNAL_MODULE) return;
  // module is restarting / binding (it is silent for a while then): keep
  // its address
  if (pwrPhase || bindStep || postBind) return;
  if (trsp.closedMode && lastReqTime &&
      (tmr10ms_t)(get_tmr10ms() - lastReqTime) < 3000)
    return;  // FRM301 talked to us in the last 30 s

  if (addrConfirmed) {
    // module answered before: give it one more round with the same address
    // (e.g. short disturbance), then start searching again
    addrConfirmed = false;
    noReplyCount = 0;
    return;
  }

  if (selectedDev()) return;  // fixed module type

  // a few rounds per address (module may still be booting), then switch
  if (++noReplyCount < 3) return;
  noReplyCount = 0;

  static const uint8_t autoList[] = {DeviceAddress::FRM303,
                                     DeviceAddress::FRM301,
                                     DeviceAddress::IRM301};
  autoIdx = (autoIdx + 1) % sizeof(autoList);
  setDevAddress(autoList[autoIdx]);
}
#endif

void ProtoState::parseData(uint8_t* rxBuffer, uint8_t rxBufferCount)
{
  AfhdsFrame* responseFrame = reinterpret_cast<AfhdsFrame*>(rxBuffer);
#if defined(AFHDS3_FRM301_EXT)
  if (trsp.closedMode && responseFrame->command == 0x05) {
    // the module sends its receiver block after binding as a request;
    // FlySkyOS stores it in the model and returns it as config part 3.
    // The response to our own 0x05 is not the receiver block.
    if (responseFrame->frameType != FRAME_TYPE::RESPONSE_DATA &&
        responseFrame->frameType != FRAME_TYPE::RESPONSE_ACK &&
        rxBufferCount >= 7 + 168) {
      // receiver ID at block offset 0x40 = data offset 0x41
      const uint8_t* idp = rxBuffer + 5 + 1 + 0x40;
      uint32_t rxId = (uint32_t)idp[0] | ((uint32_t)idp[1] << 8) |
                 ((uint32_t)idp[2] << 16) | ((uint32_t)idp[3] << 24);
      // one way has no receiver answer, so the block comes with ID 0 and
      // is still the bind result. Two way: only with a receiver ID.
      bool oneWay = !moduleData->afhds3.telemetry;
      if (oneWay || (rxId != 0 && rxId != 0xFFFFFFFF)) {
        for (uint8_t i = 0; i < 167; i++) rxBlock[i] = rxBuffer[5 + 1 + i];
        rxBlockValid = true;
        rxBlockSave = true;
        if (bindStep) rxBlockNew = true;
      }
    }
  }
  addrConfirmed = true;
  noReplyCount = 0;
  if (module_index == EXTERNAL_MODULE) {
    // address byte from the module: high nibble = destination (TRANSMITTER),
    // low nibble = source = the module's own address -> talk to that one
    uint8_t src = rxBuffer[1] & 0x0F;
    uint8_t dst = rxBuffer[1] >> 4;
    if (dst == DeviceAddress::TRANSMITTER && src != getDevAddress() &&
        src != DeviceAddress::TRANSMITTER && src != 0 && !selectedDev()) {
      setDevAddress(src);
    }
  }
#endif
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
      case COMMAND::MODULE_RFPOWER:
        {  uint8_t* value = &responseFrame->value;
          RFCurrentPower = (value[1]<<8) + value[0];
        }
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
              setIbusType(cfg->v1.NewPortTypes);
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
        else if( !RFCurrentPower && INTERNAL_MODULE==module_index )
        {
          trsp.enqueue( COMMAND::MODULE_RFPOWER, FRAME_TYPE::REQUEST_GET_DATA );
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
            uint8_t ibus_version = SES_NPT_IBUS1_IN;
            for(uint8_t i = 0; i < SES_NPT_NB_MAX_PORTS; i++) {
              // If ibus2 is configured, ignore ibus1.
              if (ibus_type[i] == SES_NPT_IBUS2 || ibus_type[i] == SES_NPT_IBUS2_HUB_PORT) {
                ibus_version = SES_NPT_IBUS2;
                break;
              }
            }
            if (ibus_version == afhds3::SES_NPT_IBUS2) {
              ::processFlySkyIbus2AFHDS3Sensor(telemetry, len-3);
            } else {
              ::processFlySkyAFHDS3Sensor(telemetry, len-3);
            }
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
          case RX_CMD_CODE_IBUS2_SET_PARAM:
            {
              uint8_t len = *data++;            
              ::Ibus2ParamCheck(data, len);
            }
            break;
          case RX_CMD_CODE_IBUS2_GET_PARAM:
            {
              uint8_t len = *data++;            
              ::Ibus2ParamCheck(data, len);
              // cfg->others.calibData[IBUS2_SENSOR_IBC] = getIbus2IbcState();
              // DIRTY_CMD(cfg, DC_RX_CMD_CLEAR_IBC);
            }
            break;
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

bool ProtoState::sensorCalibration() {
  auto *cfg = this->getConfig();

  static uint8_t data[30] = {0};
  uint8_t len = 0;

  uint8_t sensor_online = flyskyIbus2SensorOnLine();
  cfg->others.sensorOnLine = sensor_online;

  if (checkDirtyFlag(DC_RX_CMD_CALIB_GYRO)) {

    ::flySkyIbus2CalGpsGyro(data, &len);
    trsp.putFrame( COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, len);
    clearDirtyFlag(DC_RX_CMD_CALIB_GYRO);
    return true;
  }

  if (checkDirtyFlag(DC_RX_CMD_CALIB_ALT)) {
    ::flySkyIbus2CalGpsAlt();
    clearDirtyFlag(DC_RX_CMD_CALIB_ALT);
    return true;
  }

  if (checkDirtyFlag(DC_RX_CMD_CALIB_DIST) ){
    ::flySkyIbus2CalGpsDist();
    clearDirtyFlag(DC_RX_CMD_CALIB_DIST);
    return true;
  }

  static uint32_t ibc_update_tick = 0;
  static short last_ibc_v = cfg->others.calibData[IBUS2_SENSOR_IBC];
  if (last_ibc_v != cfg->others.calibData[IBUS2_SENSOR_IBC] ) {
    if (timersGetMsTick() - ibc_update_tick > 2000) { // 2-second check
      ibc_update_tick = timersGetMsTick();  
      last_ibc_v = cfg->others.calibData[IBUS2_SENSOR_IBC]; 
      ::flySkyIbus2CalibIBC(data, &len, cfg->others.calibData[IBUS2_SENSOR_IBC]);
      trsp.putFrame( COMMAND::SEND_COMMAND, FRAME_TYPE::REQUEST_SET_EXPECT_DATA, data, len);
      return true;
    }
  }

  return false;
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
    setIbusType(cfg->v1.NewPortTypes);
    // If pure is upgraded from multiple ibus2 to ibus2 hub
    uint8_t tempPortTypes[SES_NPT_NB_MAX_PORTS] = {0};
    std::memcpy(tempPortTypes, cfg->v1.NewPortTypes, SES_NPT_NB_MAX_PORTS);

    uint8_t ibus2Count = 0;
    for (uint8_t i = 0; i < SES_NPT_NB_MAX_PORTS; i++) {
        if (tempPortTypes[i] == afhds3::SES_NPT_IBUS2) {
            ibus2Count++;
        }
    }
    if (ibus2Count >= 2) {
        for (uint8_t i = 0; i < SES_NPT_NB_MAX_PORTS; i++) {
            if (tempPortTypes[i] == afhds3::SES_NPT_IBUS2) {
                tempPortTypes[i] = afhds3::SES_NPT_IBUS2_HUB_PORT;
            }
        }
    }
    
    std::memcpy(&data[3], tempPortTypes, SES_NPT_NB_MAX_PORTS);
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

#if defined(AFHDS3_FRM301_EXT)
bool isFRM301(uint8_t module)
{
  return module == EXTERNAL_MODULE && protoState[module].isFRM301();
}

#endif

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

#if defined(AFHDS3_FRM301_EXT)
  // FRM301 (closed protocol) runs with 2.5 ms like FlySkyOS
  if (module == EXTERNAL_MODULE && p_state->isClosedMode()) period = 2500;
#endif
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

static bool txCompleted(void* ctx)
{
  auto mod_st = (etx_module_state_t*)ctx;
  auto p_state = (ProtoState*)mod_st->user_data;
  return !p_state->fifoFull();
}

etx_proto_driver_t ProtoDriver = {
    .protocol = PROTOCOL_CHANNELS_AFHDS3,
    .init = initModule,
    .deinit = deinitModule,
    .sendPulses = sendPulses,
    .processData = processTelemetryData,
    .processFrame = nullptr,
    .onConfigChange = nullptr,
    .txCompleted = txCompleted,
};

}  // namespace afhds3
