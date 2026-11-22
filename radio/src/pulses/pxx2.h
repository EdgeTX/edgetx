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

#include "pxx.h"
#include "hal/module_driver.h"

#include "dataconstants.h"
#include "edgetx_types.h"

#define PXX2_TYPE_C_MODULE                  0x01
  #define PXX2_TYPE_ID_REGISTER             0x01
  #define PXX2_TYPE_ID_BIND                 0x02
  #define PXX2_TYPE_ID_CHANNELS             0x03
  #define PXX2_TYPE_ID_TX_SETTINGS          0x04
  #define PXX2_TYPE_ID_RX_SETTINGS          0x05
  #define PXX2_TYPE_ID_HW_INFO              0x06
  #define PXX2_TYPE_ID_SHARE                0x07
  #define PXX2_TYPE_ID_RESET                0x08
  #define PXX2_TYPE_ID_AUTHENTICATION       0x09
  #define PXX2_TYPE_ID_TELEMETRY            0xFE

#define PXX2_TYPE_C_POWER_METER     0x02
  #define PXX2_TYPE_ID_POWER_METER  0x01
  #define PXX2_TYPE_ID_SPECTRUM     0x02

#define PXX2_TYPE_C_OTA             0xFE
  #define PXX2_TYPE_ID_OTA          0x02

#define PXX2_CHANNELS_FLAG0_FAILSAFE         (1 << 6)
#define PXX2_CHANNELS_FLAG0_RANGECHECK       (1 << 7)
#define PXX2_CHANNELS_FLAG1_RACING_MODE      (1 << 3)

#define PXX2_TX_SETTINGS_FLAG0_WRITE               (1 << 6)
#define PXX2_TX_SETTINGS_FLAG1_EXTERNAL_ANTENNA    (1 << 3)

#define PXX2_HW_INFO_TX_ID                         0xFF

#define SPECTRUM_ANALYSER_POWER_FLOOR   -120 /*dBm*/

#define PXX2_AUTH_REFUSED_FLAG          0xA5

#define PXX2_MAX_CHANNELS               24

// Receiver options are set by this Lua tool, from the SD card
#define PXX2_RX_OPTIONS_SCRIPT          SCRIPTS_TOOLS_PATH "/AccessRx.lua"

enum PXX2ModuleModelID {
  PXX2_MODULE_NONE,
  PXX2_MODULE_XJT,
  PXX2_MODULE_ISRM,
  PXX2_MODULE_ISRM_PRO,
  PXX2_MODULE_ISRM_S,
  PXX2_MODULE_R9M,
  PXX2_MODULE_R9M_LITE,
  PXX2_MODULE_R9M_LITE_PRO,
  PXX2_MODULE_ISRM_N,
  PXX2_MODULE_ISRM_S_X9,
  PXX2_MODULE_ISRM_S_X10E,
  PXX2_MODULE_XJT_LITE,
  PXX2_MODULE_ISRM_S_X10S,
  PXX2_MODULE_ISRM_X9LITES,
};

const char * getPXX2ModuleName(uint8_t modelId);

enum {
  MODULE_OPTION_EXTERNAL_ANTENNA,
  MODULE_OPTION_POWER,
  MODULE_OPTION_SPECTRUM_ANALYSER,
  MODULE_OPTION_POWER_METER,
};

uint8_t getPXX2ModuleOptions(uint8_t modelId);
bool isPXX2ModuleOptionAvailable(uint8_t modelId, uint8_t option);

enum ModuleCapabilities {
  MODULE_CAPABILITY_COUNT
};

const char * getPXX2ReceiverName(uint8_t modelId);

enum {
  RECEIVER_OPTION_24G,
  RECEIVER_OPTION_900M,
  RECEIVER_OPTION_OTA_TO_UPDATE_SELF,
  RECEIVER_OPTION_OTA_TO_UPDATE_OTHER,
  RECEIVER_OPTION_TANDEM,
  RECEIVER_OPTION_TWIN,
  RECEIVER_OPTION_D_TELE_PORT,
};

uint8_t getPXX2ReceiverOptions(uint8_t modelId);
bool isPXX2ReceiverOptionAvailable(uint8_t modelId, uint8_t option);

enum PXX2Variant {
  PXX2_VARIANT_NONE,
  PXX2_VARIANT_FCC,
  PXX2_VARIANT_EU,
  PXX2_VARIANT_FLEX
};

enum PXX2RegisterSteps {
  REGISTER_INIT,
  REGISTER_RX_NAME_RECEIVED,
  REGISTER_RX_NAME_SELECTED,
  REGISTER_OK
};

enum PXX2BindSteps {
  BIND_MODULE_TX_INFORMATION_REQUEST = -2,
  BIND_MODULE_TX_SETTINGS_REQUEST = -1,
  BIND_INIT,
  BIND_RX_NAME_SELECTED,
  BIND_INFO_REQUEST,
  BIND_START,
  BIND_WAIT,
  BIND_OK
};

enum PXX2OtaUpdateSteps {
  OTA_UPDATE_START = BIND_OK + 1,
  OTA_UPDATE_START_ACK,
  OTA_UPDATE_TRANSFER,
  OTA_UPDATE_TRANSFER_ACK,
  OTA_UPDATE_EOF,
  OTA_UPDATE_EOF_ACK
};

enum PXX2ReceiverStatus {
  PXX2_HARDWARE_INFO,
  PXX2_SETTINGS_READ,
  PXX2_SETTINGS_WRITE,
  PXX2_SETTINGS_OK
};

PACK(struct PXX2Version {
  uint8_t major;
  uint8_t revision:4;
  uint8_t minor:4;
});

PACK(struct PXX2HardwareInformation {
  uint8_t modelID;
  PXX2Version hwVersion;
  PXX2Version swVersion;
  uint8_t variant;
  uint32_t capabilities; // variable length
  uint8_t capabilityNotSupported;
});

bool isPXX2PowerAvailable(const PXX2HardwareInformation& info, int value);

PACK(struct ModuleInformation {
  int8_t current;
  int8_t maximum;
  uint8_t timeout;
  PXX2HardwareInformation information;
  struct {
    PXX2HardwareInformation information;
    tmr10ms_t timestamp;
  } receivers[PXX2_MAX_RECEIVERS_PER_MODULE];
});

class ModuleSettings {
  public:
    uint8_t state;  // 0x00 = READ 0x40 = WRITE
    tmr10ms_t timeout;
    uint8_t externalAntenna;
    int8_t txPower;
    uint8_t dirty;
};

struct PXX2ModuleSetup {
  union {
    uint8_t registerStep;
    uint8_t resetStep;
  };
  uint8_t registerPopupVerticalPosition;
  uint8_t registerPopupHorizontalPosition;
  int8_t registerPopupEditMode;
  char registerRxName[PXX2_LEN_RX_NAME];
  uint8_t registerLoopIndex; // will be removed later
  union {
    uint8_t shareReceiverIndex;
    uint8_t resetReceiverIndex;
  };
  uint8_t resetReceiverFlags;
  ModuleInformation moduleInformation;
  ModuleSettings moduleSettings;
};

class BindInformation {
  public:
    int8_t step;
    uint32_t timeout;
    char candidateReceiversNames[PXX2_MAX_RECEIVERS_PER_MODULE][PXX2_LEN_RX_NAME + 1];
    uint8_t candidateReceiversCount;
    uint8_t selectedReceiverIndex;
    uint8_t rxUid;
    uint8_t lbtMode;
    uint8_t flexMode;
    PXX2HardwareInformation receiverInformation;
};

struct PXX2HardwareAndSettings {
  ModuleInformation modules[MAX_MODULES];
  uint32_t updateTime;
  ModuleSettings moduleSettings;
  char msg[64];
};

// Raw PXX2 frames exchanged with Lua scripts (accessFramePush / accessFramePop)
//
// Output: a single slot, filled by the script and sent by the mixer instead of
// one channels frame while the module is in normal mode.
//
// Input: every frame received from a module, except telemetry and
// authentication, is copied into a queue as
//   [length] [module] [TYPE_C] [TYPE_ID] [payload...]
// where length counts TYPE_C, TYPE_ID and the payload. The queue is created by
// the first accessFramePop() call.

// HEAD (2) + TYPE_C + TYPE_ID + CRC (2)
#define PXX2_LUA_FRAME_MAX_PAYLOAD  (PXX2_FRAME_MAXLENGTH - 6)

bool pxx2LuaFrameAvailable();
bool pxx2LuaFramePush(uint8_t module, uint8_t typeC, uint8_t typeId,
                      const uint8_t* payload, uint8_t length);

PXX2ModuleSetup& getPXX2ModuleSetupBuffer();
BindInformation& getPXX2BindInformationBuffer();
PXX2HardwareAndSettings& getPXX2HardwareAndSettingsBuffer();

extern const etx_proto_driver_t Pxx2Driver;
