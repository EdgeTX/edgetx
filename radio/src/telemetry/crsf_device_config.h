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

#include <inttypes.h>

#include "crossfire.h"
#include "edgetx_types.h"

//
// Native client for the CRSF device configuration protocol (parameter
// browsing / editing over DEVICE_PING / DEVICE_INFO / PARAMETER_*
// extended frames). The protocol is self-describing: devices advertise
// their parameters (name, type, range, value), so this works with any
// CRSF device implementing it: ExpressLRS, TBS Crossfire, CRSF
// receivers, VTXs, ...
//

#define CRSF_CFG_MAX_DEVICES           8
#define CRSF_CFG_MAX_FIELDS            80

// Additional extended frame types (0x28/0x29 are in crossfire.h)
#define PARAM_SETTINGS_ENTRY_ID        0x2B
#define PARAM_READ_ID                  0x2C
#define PARAM_WRITE_ID                 0x2D
#define ELRS_STATUS_ID                 0x2E

// MSP over CRSF (used by ExpressLRS >= 4.1 for bind phrase / UID config)
#define MSP_REQ_ID                     0x7A
#define MSP_RESP_ID                    0x7B
#define MSP_WRITE_ID                   0x7C
#define MSP_ELRS_RXTX_CONFIG           45
#define MSP_ELRS_SUBCMD_UID            0x00
#define MSP_ELRS_SUBCMD_BIND_PHRASE    0x01

#define CRSF_BIND_PHRASE_MAXLEN        52

enum CrsfFieldType : uint8_t {
  CRSF_FIELD_UINT8 = 0,
  CRSF_FIELD_INT8 = 1,
  CRSF_FIELD_UINT16 = 2,
  CRSF_FIELD_INT16 = 3,
  CRSF_FIELD_FLOAT = 8,  // fixed point i32 with decimal places, not IEEE
  CRSF_FIELD_TEXT_SELECTION = 9,
  CRSF_FIELD_STRING = 10,
  CRSF_FIELD_FOLDER = 11,
  CRSF_FIELD_INFO = 12,
  CRSF_FIELD_COMMAND = 13,
  CRSF_FIELD_OUT_OF_RANGE = 127,
};

enum CrsfCommandStatus : uint8_t {
  CRSF_CMD_READY = 0,                // command idle / finished
  CRSF_CMD_START = 1,                // (write) start executing
  CRSF_CMD_PROGRESS = 2,             // running
  CRSF_CMD_CONFIRMATION_NEEDED = 3,  // device asks the user to confirm
  CRSF_CMD_CONFIRM = 4,              // (write) user confirmed
  CRSF_CMD_CANCEL = 5,               // (write) user cancelled
  CRSF_CMD_POLL = 6,                 // (write) query current status
};

// ELRS_STATUS flags
#define CRSF_ELRS_FLAG_CONNECTED       0x01
#define CRSF_ELRS_FLAG_MODEL_MATCH     0x04
#define CRSF_ELRS_FLAG_ARMED           0x08
#define CRSF_ELRS_FLAG_WARNING_MASK    0xFC  // any of these: show flag info
#define CRSF_ELRS_FLAG_CRITICAL_MASK   0xE0  // critical: needs user ack

enum CrsfConfigState : uint8_t {
  CRSF_STATE_INACTIVE,
  CRSF_STATE_PING,      // discovering devices
  CRSF_STATE_LOADING,   // fetching parameters of the selected device
  CRSF_STATE_IDLE,      // all parameters loaded
  CRSF_STATE_ELRS_V1,   // incompatible ELRS 1.x module detected
};

struct CrsfConfigDevice {
  uint8_t address;
  uint8_t fieldCount;
  uint8_t paramVersion;
  bool isELRS;     // ExpressLRS device (TX module or receiver)
  bool isELRS_TX;  // the ExpressLRS TX module itself
  uint8_t fwMajor;
  uint8_t fwMinor;
  char name[CRSF_NAME_MAXSIZE];
};

// Parsed, UI-facing view of one parameter. Pointers point into the
// client's storage: valid until the next tick.
struct CrsfConfigField {
  uint8_t id;
  uint8_t parent;  // field id of parent folder, 0 = root
  uint8_t type;    // CrsfFieldType
  bool hidden;
  bool loaded;
  const char* name;
  const char* unit;     // "" if none
  const char* options;  // TEXT_SELECTION: ';' separated list
  const char* svalue;   // STRING / INFO value, COMMAND info text
  int32_t value;
  int32_t min;
  int32_t max;
  int32_t step;         // FLOAT only
  uint8_t prec;         // FLOAT only: decimal places (0..3)
  uint8_t cmdStatus;    // COMMAND only
  uint8_t cmdTimeout;   // COMMAND only, in 10ms units
  uint8_t maxStrLen;    // STRING only
};

struct CrsfElrsStatus {
  uint8_t badPkts;
  uint16_t goodPkts;
  uint8_t flags;
  char flagInfo[32];
};

// lifecycle; all calls below are for the menus task - incoming frames
// are queued by the telemetry task and parsed by the tick
void crsfConfigStart(uint8_t moduleIdx);
void crsfConfigStop();
bool crsfConfigActive();
CrsfConfigState crsfConfigState();

// periodic driver: call from the UI at each refresh (10-50 ms)
void crsfConfigTick();

// frame sink, called from processCrossfireTelemetryFrame()
void crsfConfigFrameReceived(uint8_t module, const uint8_t* rxBuffer,
                             uint8_t length);

// devices
uint8_t crsfConfigDeviceCount();
const CrsfConfigDevice* crsfConfigGetDevice(uint8_t index);
const CrsfConfigDevice* crsfConfigCurrentDevice();
void crsfConfigSelectDevice(uint8_t address);
void crsfConfigRefreshDevices();  // re-ping the bus right away

// parameters of the selected device (ids are 1..fieldCount)
uint8_t crsfConfigFieldCount();
uint8_t crsfConfigLoadedFieldCount();
bool crsfConfigGetField(uint8_t id, CrsfConfigField& out);
// re-read the fields of a folder in place (like the ELRS Lua does when
// opening one): edits elsewhere may have changed values or options
void crsfConfigReloadFolder(uint8_t folderId);

// bumped whenever the UI should rebuild (fields loaded / device changed)
uint16_t crsfConfigGeneration();

// edits (queued; sent on the next tick)
void crsfConfigWriteValue(uint8_t id, int32_t value);
void crsfConfigWriteString(uint8_t id, const char* value);

// commands
void crsfConfigCommandStart(uint8_t id);
void crsfConfigCommandConfirm();
void crsfConfigCommandCancel();  // stop the command on the device
void crsfConfigCommandDetach();  // close the UI, leave the command running
bool crsfConfigCommandActive();
uint8_t crsfConfigCommandField();
uint8_t crsfConfigCommandStatus();  // live status incl. pending confirm/cancel

// ELRS link status (valid when the selected device is an ELRS TX)
CrsfElrsStatus crsfConfigElrsStatus();
void crsfConfigAckCriticalFlags();

// ExpressLRS binding (see the bind-phrase Lua script / RxTxEndpoint.cpp)
// - bind mode / phrase / UID target: MODULE_ADDRESS or RECEIVER_ADDRESS
// - phrase & UID configuration require ELRS >= 4.1 on the target device
// - the target reboots ~200ms after a successful phrase/UID set and the
//   parameters reload automatically afterwards
bool crsfConfigElrsCanSetBindInfo(const CrsfConfigDevice* dev);
// the ELRS TX module's own "Bind" command parameter (0 = none);
// the UIs replace it with the full native bind page
uint8_t crsfConfigElrsBindFieldId();
// the ELRS version parameter (version string as the field name)
bool crsfConfigElrsIsVersionField(const CrsfConfigField& f);
bool crsfConfigSendBindCommand(uint8_t destAddr);
// bind phrase or "a,b,c,d,e,f" UID text; rxFirst also re-binds the
// connected receiver before the TX module switches over
bool crsfConfigApplyBindInfo(const char* text, bool rxFirst);
void crsfConfigRequestUid(uint8_t destAddr);  // poll until answered
bool crsfConfigGetUid(uint8_t uid[6]);        // false until received

// classic bind: re-triggers the module's short bind burst until a
// receiver connects (or the loop is stopped)
void crsfConfigBindLoopStart();
void crsfConfigBindLoopStop();
bool crsfConfigBindLoopActive();
bool crsfConfigBindLoopBound();

// helpers shared by both UIs
// formats a numeric value with `prec` decimals (0..3) plus unit
void crsfConfigFormatNumber(int32_t value, uint8_t prec, const char* unit,
                            char* dest, uint8_t size);
uint8_t crsfConfigOptionCount(const char* options);  // number of ';' entries
bool crsfConfigGetOption(const char* options, uint8_t index, char* dest,
                         uint8_t destSize);
// whether an option of a selection field may be picked: skips empty
// placeholders, and applies cross-field rules (ELRS: the fan threshold
// cannot exceed the configured Max Power)
bool crsfConfigSelectionAllowed(uint8_t fieldId, uint8_t index);
uint8_t crsfConfigNextAllowedOption(uint8_t fieldId, uint8_t current,
                                    int8_t dir);

#if !defined(COLORLCD)
#define CRSF_CFG_FOLDER_DEPTH 8

// entries the menu can show: device switches + fields + bind
#define CRSF_CFG_MAX_ENTRIES  (CRSF_CFG_MAX_DEVICES + CRSF_CFG_MAX_FIELDS + 1)

// state of the B&W menu, kept in the reusableBuffer union
struct CrsfMenuState {
  uint8_t folder;        // current folder field id, 0 = root
  uint8_t folderStack[CRSF_CFG_FOLDER_DEPTH];
  uint8_t selStack[CRSF_CFG_FOLDER_DEPTH];  // selection to restore on back
  uint8_t folderDepth;
  uint8_t selected;      // index into the visible entry list
  uint8_t scrollOfs;
  uint8_t editId;        // field id being edited, 0 = none
  tmr10ms_t selTime;     // when the selection last moved
  bool cmdWasActive;
  bool loopWasActive;
  bool uidPrefilled;
  uint8_t uidEdit[6];
  uint8_t phraseCursor;
  uint8_t applyMode;     // 0 = TX only, 1 = RX + TX
  char phrase[CRSF_BIND_PHRASE_MAXLEN + 1];
  char cmdResult[40];    // final message of a completed command
  char info[32];         // transient status message
  tmr10ms_t infoUntil;
  int32_t editValue;
  // visible entries, rebuilt when the fields or the folder change
  uint16_t entries[CRSF_CFG_MAX_ENTRIES];
  uint16_t nEntries;
  uint8_t bindFieldId;
  coord_t valCol;
  bool entriesBuilt;
  uint16_t builtGeneration;
  uint8_t builtLoaded;
  uint8_t builtFolder;
};
#endif
