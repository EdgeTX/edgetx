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

#include "crsf_device_config.h"

#include "edgetx.h"
#include "os/task.h"
#include "telemetry.h"

#if defined(CRSF_CONFIG_MENU)

// a full ELRS TX parameter dump is ~3.5KB; running out only hides the
// fields that no longer fit
#define CRSF_CFG_ARENA_SIZE      6144

#define CRSF_CFG_MAX_WRITE       52  // largest single write payload
#define CRSF_CFG_RX_FRAMES       4   // raw frames queued for the tick
#define CRSF_CFG_RX_FRAME_SIZE   64  // maximum CRSF frame length

// all times in 10ms ticks
#define PING_INTERVAL            100  // 1s device discovery
#define REQ_TIMEOUT_LOCAL        50   // 500ms retry for the local TX module
#define REQ_TIMEOUT_REMOTE       500  // 5s retry for devices behind the RF link
#define WRITE_RELOAD_DELAY       20   // 200ms for the module EEPROM to commit
#define WRITE_SETTLE_DELAY       300  // late second re-read: some values
                                      // populate slowly (backpack boot)
#define WRITE_STAGE_DELAY        25   // coalesce rapid value edits (encoder)
#define STATUS_POLL_INTERVAL     100  // 1s ELRS link status poll
#define BIND_RESTART_INTERVAL    120  // re-trigger of the ~1s ELRS bind burst
#define TX_BIND_DELAY            100  // RX-then-TX: let the OTA link deliver

struct FieldSlot {
  uint16_t offset;
  uint16_t length;   // actual record length
  uint16_t cap;      // allocated arena capacity for this slot
  uint8_t parent;
  uint8_t type;      // low 7 bits, without hidden flag
  bool hidden;
  bool loaded;
};

struct CrsfConfigClient {
  uint8_t moduleIdx;
  CrsfConfigState state;
  uint16_t generation;

  // devices
  CrsfConfigDevice devices[CRSF_CFG_MAX_DEVICES];
  uint8_t nDevices;
  uint8_t currentDevice;  // index in devices[], 0xFF = none

  // loading
  uint8_t loadMask[(CRSF_CFG_MAX_FIELDS + 7) / 8];
  uint8_t loading;        // field id currently being fetched, 0 = none
  uint8_t chunk;          // next chunk index to request
  int16_t expectChunks;   // expected chunks_remaining of next chunk, -1 = any
  uint16_t asmLen;        // bytes assembled so far (at arena + arenaUsed)
  bool reqOutstanding;
  bool pingRequested;     // one-shot rescan (menu opened / back at root)
  tmr10ms_t reqTime;
  tmr10ms_t startTime;
  tmr10ms_t nextPing;
  tmr10ms_t nextStatusPoll;

  // deferred reload after a write
  tmr10ms_t reloadAt;     // 0 = no reload pending
  uint8_t reloadField;
  tmr10ms_t settleAt;     // one-time late re-read of the same fields
  uint8_t settleField;

  // full parameter refresh (e.g. after a device reboot)
  tmr10ms_t refreshAt;    // 0 = none

  // pending frame (single slot, sent by the next tick)
  uint8_t pendFrameType;  // 0 = empty
  uint8_t pendFrameLen;
  uint8_t pendFrame[58];  // payload incl. dest/origin

  // staged value write: rapid edits coalesce into one frame; the last
  // written value is shown until the re-read confirms (or corrects) it
  uint8_t stagedId;       // 0 = none
  uint8_t stagedShownId;  // 0 = none
  int32_t stagedValue;
  tmr10ms_t stagedAt;

  // raw frames from the telemetry task, parsed by the tick
  uint8_t rxCount;
  uint8_t rxLen[CRSF_CFG_RX_FRAMES];
  uint8_t rxFrame[CRSF_CFG_RX_FRAMES][CRSF_CFG_RX_FRAME_SIZE];

  // ExpressLRS UID (bind info) polling
  bool uidWanted;
  bool uidValid;
  uint8_t uidDevice;
  uint8_t uid[6];
  tmr10ms_t uidNextReq;

  // classic bind loop: re-trigger the module's short bind burst until
  // a receiver connects
  bool bindLoop;
  bool bindSawDisc;
  bool bindBound;
  tmr10ms_t bindNextRestart;

  // RX-then-TX bind info sequence: the TX half is sent after the OTA
  // link has delivered the RX half
  bool txBindPending;
  bool txBindIsUid;
  uint8_t txBindLen;
  uint8_t txBindData[CRSF_CFG_MAX_WRITE];
  tmr10ms_t txBindAt;

  // command in progress
  bool cmdActive;
  uint8_t cmdField;
  uint8_t cmdStatus;
  uint16_t cmdTimeout;    // 10ms units
  tmr10ms_t cmdNextPoll;

  CrsfElrsStatus elrs;

  FieldSlot slots[CRSF_CFG_MAX_FIELDS];
  uint16_t arenaUsed;
  uint8_t arena[CRSF_CFG_ARENA_SIZE];
};

// The telemetry task only queues raw frames; they are parsed by
// crsfConfigTick() on the menus task, so everything else runs on one
// task and cfgMutex only guards the frame queue and the client's
// lifetime. String pointers handed out by crsfConfigGetField() point
// into the arena: valid until the next tick, always null-terminated.
static CrsfConfigClient* client = nullptr;
static mutex_handle_t cfgMutex;
static bool cfgMutexCreated = false;

//
// helpers (cfgMutex held unless stated otherwise)
//

// 0 means "not scheduled" for reloadAt/settleAt/refreshAt
static tmr10ms_t schedTime(tmr10ms_t t) { return t ? t : 1; }

static const CrsfConfigDevice* currentDev()
{
  if (!client || client->currentDevice >= client->nDevices) return nullptr;
  return &client->devices[client->currentDevice];
}

static uint8_t currentDevAddress()
{
  auto dev = currentDev();
  return dev ? dev->address : MODULE_ADDRESS;
}

static uint8_t deviceFieldCount()
{
  auto dev = currentDev();
  if (!dev) return 0;
  return min<uint8_t>(dev->fieldCount, CRSF_CFG_MAX_FIELDS);
}

static void loadMaskSet(uint8_t id)
{
  if (id >= 1 && id <= CRSF_CFG_MAX_FIELDS)
    client->loadMask[(id - 1) / 8] |= (1 << ((id - 1) % 8));
}

static void loadMaskClear(uint8_t id)
{
  if (id >= 1 && id <= CRSF_CFG_MAX_FIELDS)
    client->loadMask[(id - 1) / 8] &= ~(1 << ((id - 1) % 8));
}

static uint8_t loadMaskNext()
{
  uint8_t count = deviceFieldCount();
  for (uint8_t i = 0; i < count; i++) {
    if (client->loadMask[i / 8] & (1 << (i % 8))) return i + 1;
  }
  return 0;
}

static void resetAssembly()
{
  client->loading = 0;
  client->chunk = 0;
  client->expectChunks = -1;
  client->asmLen = 0;
  client->reqOutstanding = false;
}

static void clearFields()
{
  memclear(client->slots, sizeof(client->slots));
  memclear(client->loadMask, sizeof(client->loadMask));
  client->arenaUsed = 0;
  client->stagedShownId = 0;
  resetAssembly();

  uint8_t count = deviceFieldCount();
  for (uint8_t id = 1; id <= count; id++) loadMaskSet(id);
  client->state = count ? CRSF_STATE_LOADING : CRSF_STATE_IDLE;
}

// re-read a written field and its editable siblings (options and
// values may have changed as a side effect, like elrs.lua does)
static void reloadSiblings(uint8_t fieldId)
{
  uint8_t count = deviceFieldCount();
  if (fieldId < 1 || fieldId > count) return;
  uint8_t parent = client->slots[fieldId - 1].parent;
  for (uint8_t id = 1; id <= count; id++) {
    FieldSlot& slot = client->slots[id - 1];
    if (!slot.loaded) continue;
    if (id == fieldId ||
        (slot.parent == parent && slot.type != CRSF_FIELD_FOLDER)) {
      loadMaskSet(id);
    }
  }
}

static void checkLoadComplete()
{
  if (client->state == CRSF_STATE_LOADING && loadMaskNext() == 0) {
    client->state = CRSF_STATE_IDLE;
    client->generation++;
  }
}

// build a full CRSF extended frame into the shared output buffer, the
// same way luaCrossfireTelemetryPush() does; it is injected in place of
// a channels frame by setupPulsesCrossfire()
static bool sendFrame(uint8_t type, const uint8_t* payload, uint8_t len)
{
  if (!outputTelemetryBuffer.isAvailable()) return false;

  // COMMAND frames carry an extra inner CRC (poly 0xBA)
  uint8_t lenAdjust = (type == COMMAND_ID) ? 1 : 0;

  outputTelemetryBuffer.pushByte(MODULE_ADDRESS);
  outputTelemetryBuffer.pushByte(2 + len + lenAdjust);
  outputTelemetryBuffer.pushByte(type);
  for (uint8_t i = 0; i < len; i++)
    outputTelemetryBuffer.pushByte(payload[i]);
  if (type == COMMAND_ID)
    outputTelemetryBuffer.pushByte(
        crc8_BA(outputTelemetryBuffer.data + 2, 1 + len));
  outputTelemetryBuffer.pushByte(
      crc8(outputTelemetryBuffer.data + 2, 1 + len + lenAdjust));
  outputTelemetryBuffer.setDestination(
      client->moduleIdx == INTERNAL_MODULE ? 0 : TELEMETRY_ENDPOINT_SPORT);
  return true;
}

static bool sendPing()
{
  uint8_t payload[] = {BROADCAST_ADDRESS, RADIO_ADDRESS};
  return sendFrame(PING_DEVICES_ID, payload, sizeof(payload));
}

static bool sendFieldRequest(uint8_t fieldId, uint8_t chunkIdx)
{
  uint8_t payload[] = {currentDevAddress(), RADIO_ADDRESS, fieldId, chunkIdx};
  return sendFrame(PARAM_READ_ID, payload, sizeof(payload));
}

static bool sendPendingFrame()
{
  if (!sendFrame(client->pendFrameType, client->pendFrame,
                 client->pendFrameLen))
    return false;
  client->pendFrameType = 0;
  client->pendFrameLen = 0;
  return true;
}

static bool queueFrame(uint8_t type, const uint8_t* payload, uint8_t len)
{
  if (!client || len > sizeof(client->pendFrame)) return false;
  client->pendFrameType = type;
  client->pendFrameLen = len;
  memcpy(client->pendFrame, payload, len);
  return true;
}

static bool sendStatusRequest()
{
  // ELRS convention: write to pseudo field 0 requests an ELRS_STATUS frame
  uint8_t payload[] = {MODULE_ADDRESS, RADIO_ADDRESS, 0x00, 0x00};
  return sendFrame(PARAM_WRITE_ID, payload, sizeof(payload));
}

static void queueWrite(uint8_t id, const uint8_t* data, uint8_t len)
{
  if (len > CRSF_CFG_MAX_WRITE) return;
  uint8_t payload[3 + CRSF_CFG_MAX_WRITE];
  payload[0] = currentDevAddress();
  payload[1] = RADIO_ADDRESS;
  payload[2] = id;
  memcpy(payload + 3, data, len);
  queueFrame(PARAM_WRITE_ID, payload, 3 + len);
}

static bool queueMspConfigWrite(uint8_t destAddr, uint8_t subcmd,
                                const uint8_t* data, uint8_t len)
{
  if (len > CRSF_BIND_PHRASE_MAXLEN) return false;
  uint8_t payload[6 + CRSF_BIND_PHRASE_MAXLEN];
  payload[0] = destAddr;
  payload[1] = RADIO_ADDRESS;
  payload[2] = 0x30;  // encapsulated MSP: v1, start of frame, seq 0
  payload[3] = 1 + len;
  payload[4] = MSP_ELRS_RXTX_CONFIG;
  payload[5] = subcmd;
  memcpy(payload + 6, data, len);
  return queueFrame(MSP_WRITE_ID, payload, 6 + len);
}

static bool sendBindCommandImpl(uint8_t destAddr)
{
  // to the TX module: enter bind mode; to the receiver (over the RF
  // link): put the connected RX into bind mode ("unbind")
  uint8_t payload[] = {destAddr, RADIO_ADDRESS, SUBCOMMAND_CRSF,
                       SUBCOMMAND_CRSF_BIND};
  return queueFrame(COMMAND_ID, payload, sizeof(payload));
}

//
// record parsing
//

static int32_t readBE(const uint8_t* p, uint8_t size, bool sign)
{
  uint32_t v = 0;
  for (uint8_t i = 0; i < size; i++) v = (v << 8) | p[i];
  if (sign) {
    if (size == 1) return (int8_t)v;
    if (size == 2) return (int16_t)v;
  }
  return (int32_t)v;
}

void crsfConfigFormatNumber(int32_t value, uint8_t prec, const char* unit,
                            char* dest, uint8_t size)
{
  if (prec == 0) {
    snprintf(dest, size, "%ld%s", (long)value, unit);
    return;
  }
  if (prec > 3) prec = 3;
  int32_t div = 1;
  for (uint8_t i = 0; i < prec; i++) div *= 10;
  long ip = value / div;
  long fp = (value < 0 ? -value : value) % div;
  // the sign is lost by the division when -1 < value/div < 0
  const char* sign = (value < 0 && ip == 0) ? "-" : "";
  const char* fmt = (prec == 1)   ? "%s%ld.%01ld%s"
                    : (prec == 2) ? "%s%ld.%02ld%s"
                                  : "%s%ld.%03ld%s";
  snprintf(dest, size, fmt, sign, ip, fp, unit);
}

uint8_t crsfConfigOptionCount(const char* options)
{
  if (!options || !*options) return 0;
  uint8_t n = 1;
  for (const char* p = options; *p; p++)
    if (*p == ';') n++;
  return n;
}

bool crsfConfigGetOption(const char* options, uint8_t index, char* dest,
                         uint8_t destSize)
{
  if (!options || destSize == 0) return false;
  const char* p = options;
  while (index > 0) {
    while (*p && *p != ';') p++;
    if (!*p) return false;
    p++;  // skip ';'
    index--;
  }
  uint8_t i = 0;
  while (*p && *p != ';' && i < destSize - 1) {
    char c = *p++;
    // 0xC0/0xC1 are OpenTX arrow glyphs used by ELRS option lists
    if ((uint8_t)c == 0xC0) c = '+';
    else if ((uint8_t)c == 0xC1) c = '-';
    else if ((uint8_t)c > 0x7F) c = '?';
    dest[i++] = c;
  }
  dest[i] = '\0';
  return true;
}

static bool optionEmpty(const char* options, uint8_t index)
{
  char buf[2];
  if (!crsfConfigGetOption(options, index, buf, sizeof(buf))) return true;
  return buf[0] == '\0';
}

// leading number of an option text ("250mW" -> 250), -1 if none ("Never")
static int32_t optionNumericValue(const char* options, uint8_t index)
{
  char buf[24];
  if (!crsfConfigGetOption(options, index, buf, sizeof(buf))) return -1;
  if (buf[0] < '0' || buf[0] > '9') return -1;
  int32_t v = 0;
  for (const char* p = buf; *p >= '0' && *p <= '9'; p++) v = v * 10 + (*p - '0');
  return v;
}

static bool getFieldImpl(uint8_t id, CrsfConfigField& out)
{
  memclear(&out, sizeof(out));
  out.name = "";
  out.unit = "";
  out.options = "";
  out.svalue = "";
  if (!client || id < 1 || id > deviceFieldCount()) return false;

  FieldSlot& slot = client->slots[id - 1];
  out.id = id;
  out.loaded = slot.loaded;
  out.hidden = slot.hidden;
  if (!slot.loaded || slot.length < 3) return slot.loaded;

  const uint8_t* r = client->arena + slot.offset;
  const uint8_t* end = r + slot.length;

  out.parent = slot.parent;
  out.type = slot.type;
  out.name = (const char*)(r + 2);

  // skip the name
  const uint8_t* p = r + 2;
  while (p < end && *p) p++;
  if (p >= end) return true;
  p++;  // skip null

  switch (out.type) {
    case CRSF_FIELD_UINT8:
    case CRSF_FIELD_INT8:
    case CRSF_FIELD_UINT16:
    case CRSF_FIELD_INT16: {
      uint8_t size = out.type / 2 + 1;
      bool sign = out.type & 1;
      if (p + 4 * size > end) return true;
      out.value = readBE(p, size, sign);
      out.min = readBE(p + size, size, sign);
      out.max = readBE(p + 2 * size, size, sign);
      p += 4 * size;  // value, min, max, default
      if (p < end) out.unit = (const char*)p;
      break;
    }

    case CRSF_FIELD_FLOAT: {
      if (p + 17 > end) return true;
      out.value = readBE(p, 4, true);
      out.min = readBE(p + 4, 4, true);
      out.max = readBE(p + 8, 4, true);
      out.prec = min<uint8_t>(p[16], 3);
      if (p + 21 <= end) out.step = readBE(p + 17, 4, false);
      if (p + 21 < end) out.unit = (const char*)(p + 21);
      break;
    }

    case CRSF_FIELD_TEXT_SELECTION: {
      if (p >= end) return true;
      out.options = (const char*)p;
      while (p < end && *p) p++;
      if (p >= end) return true;
      p++;  // skip null
      if (p < end) out.value = *p;
      // min/max/default bytes are unreliable on selections: derive from
      // the option list (like elrs.lua does)
      out.min = 0;
      out.max = crsfConfigOptionCount(out.options);
      if (out.max > 0) out.max--;
      if (p + 4 < end) out.unit = (const char*)(p + 4);
      break;
    }

    case CRSF_FIELD_STRING: {
      if (p >= end) return true;
      out.svalue = (const char*)p;
      while (p < end && *p) p++;
      if (p + 1 < end) out.maxStrLen = p[1];
      break;
    }

    case CRSF_FIELD_INFO:
      if (p < end) out.svalue = (const char*)p;
      break;

    case CRSF_FIELD_COMMAND: {
      if (p + 2 > end) return true;
      out.cmdStatus = p[0];
      out.cmdTimeout = p[1];
      if (p + 2 < end) out.svalue = (const char*)(p + 2);
      break;
    }

    default:
      break;
  }

  // show the value the user just wrote until the re-read confirms it
  // (devices do not echo writes)
  if (id == client->stagedShownId &&
      (out.type <= CRSF_FIELD_INT16 || out.type == CRSF_FIELD_FLOAT ||
       out.type == CRSF_FIELD_TEXT_SELECTION))
    out.value = client->stagedValue;

  return true;
}

static bool selectionAllowedImpl(uint8_t fieldId, uint8_t index)
{
  CrsfConfigField f;
  if (!getFieldImpl(fieldId, f) || !f.loaded) return false;
  if (optionEmpty(f.options, index)) return false;

  // ELRS: the fan threshold cannot exceed the highest power the
  // hardware offers (the top entry of the Max Power option list);
  // prefix-matched as field names may change between ELRS releases,
  // and skipped gracefully when either field is absent
  auto dev = currentDev();
  if (dev && dev->isELRS_TX && strncasecmp(f.name, "Fan Thresh", 10) == 0) {
    int32_t v = optionNumericValue(f.options, index);
    if (v >= 0) {  // entries without a number ("Never") stay allowed
      uint8_t count = deviceFieldCount();
      for (uint8_t id = 1; id <= count; id++) {
        CrsfConfigField m;
        if (!getFieldImpl(id, m) || !m.loaded) continue;
        if (m.parent == f.parent && m.type == CRSF_FIELD_TEXT_SELECTION &&
            strncasecmp(m.name, "Max Power", 9) == 0) {
          int32_t hwMax = -1;
          uint8_t nOptions = crsfConfigOptionCount(m.options);
          for (uint8_t i = 0; i < nOptions; i++) {
            int32_t p = optionNumericValue(m.options, i);
            if (p > hwMax) hwMax = p;
          }
          if (hwMax >= 0 && v > hwMax) return false;
          break;
        }
      }
    }
  }
  return true;
}

static uint8_t elrsBindFieldIdImpl()
{
  auto dev = currentDev();
  if (!dev || !dev->isELRS_TX) return 0;
  uint8_t count = deviceFieldCount();
  for (uint8_t id = 1; id <= count; id++) {
    CrsfConfigField f;
    if (!getFieldImpl(id, f) || !f.loaded) continue;
    // field names may change between ELRS releases: match the stable
    // prefix, and only a COMMAND at the root ("Bind" since 3.x)
    if (f.parent == 0 && f.type == CRSF_FIELD_COMMAND &&
        strncasecmp(f.name, "Bind", 4) == 0)
      return id;
  }
  return 0;
}

//
// record assembly / commit
//

static void commitRecord(uint8_t fieldId)
{
  FieldSlot& slot = client->slots[fieldId - 1];
  uint8_t* record = client->arena + client->arenaUsed;
  uint16_t len = client->asmLen;

  // shortest valid record: parent, type, empty name terminator
  if (len < 3) {
    resetAssembly();
    return;
  }

  uint8_t type = record[1] & 0x7F;
  if (type == CRSF_FIELD_OUT_OF_RANGE) {
    // field id does not exist right now - either the device advertised
    // too many fields, or it is rebuilding its parameter set (e.g. a
    // band change): trim, and re-query the device info to self-heal
    if (client->currentDevice < client->nDevices)
      client->devices[client->currentDevice].fieldCount = fieldId - 1;
    for (uint8_t id = fieldId; id <= CRSF_CFG_MAX_FIELDS; id++)
      loadMaskClear(id);
    resetAssembly();
    checkLoadComplete();
    client->pingRequested = true;
    client->nextPing = get_tmr10ms() + 100;
    return;
  }

  // every string walk in getFieldImpl() terminates within the record
  if (record[len - 1] != '\0') {
    if (client->arenaUsed + len < CRSF_CFG_ARENA_SIZE)
      record[len++] = '\0';
    else
      record[len - 1] = '\0';
  }

  if (slot.loaded && len <= slot.cap) {
    // reloaded field: overwrite in place
    memcpy(client->arena + slot.offset, record, len);
    slot.length = len;
  } else {
    // new (or grown) record: commit where it was assembled
    slot.offset = client->arenaUsed;
    slot.length = len;
    slot.cap = len;
    client->arenaUsed += len;
  }

  // a re-read may reshape the menu (fields hidden or moved)
  bool hidden = (record[1] & 0x80) != 0;
  if (slot.loaded && (slot.parent != record[0] || slot.type != type ||
                      slot.hidden != hidden))
    client->generation++;

  slot.parent = record[0];
  slot.type = type;
  slot.hidden = hidden;
  slot.loaded = true;
  loadMaskClear(fieldId);

  // the re-read value replaces the shown staged one
  if (fieldId == client->stagedShownId && client->stagedId != fieldId)
    client->stagedShownId = 0;

  // command status tracking
  if (client->cmdActive && fieldId == client->cmdField) {
    CrsfConfigField f;
    getFieldImpl(fieldId, f);
    client->cmdStatus = f.cmdStatus;
    if (f.cmdTimeout > 0) client->cmdTimeout = f.cmdTimeout;
    if (f.cmdStatus == CRSF_CMD_READY) {
      client->cmdActive = false;
      if (!client->bindLoop) {
        // finished: reload everything to pick up any changed values
        client->reloadAt = schedTime(get_tmr10ms());
        client->reloadField = 0;
      }
    }
    client->generation++;
  }

  resetAssembly();
  checkLoadComplete();
}

static void handleSettingsEntry(const uint8_t* rx, uint8_t pktLen)
{
  if (pktLen < 9) return;

  uint8_t origin = rx[4];
  uint8_t fieldId = rx[5];
  uint8_t chunks = rx[6];
  const uint8_t* data = rx + 7;
  uint16_t dataLen = pktLen - 8;

  if (origin != currentDevAddress()) return;

  if (client->loading == 0 || fieldId != client->loading) {
    // unsolicited entry: accept only status updates of the running
    // command (a retransmitted final chunk of a regular field would
    // otherwise commit a mid-record fragment as a whole record)
    if (client->cmdActive && fieldId == client->cmdField && chunks == 0 &&
        dataLen >= 3 &&
        client->arenaUsed + dataLen < CRSF_CFG_ARENA_SIZE) {
      resetAssembly();
      memcpy(client->arena + client->arenaUsed, data, dataLen);
      client->asmLen = dataLen;
      commitRecord(fieldId);
      client->generation++;
    }
    return;
  }

  // duplicate/garbled chunk protection: ELRS may retransmit a chunk;
  // like elrs.lua, ignore it and keep the assembly in progress
  if (client->expectChunks >= 0 && chunks != client->expectChunks) return;

  if (client->arenaUsed + client->asmLen + dataLen >= CRSF_CFG_ARENA_SIZE) {
    // no room for this record: hide the field and carry on with the
    // rest instead of looping over a load that can never fit
    FieldSlot& slot = client->slots[fieldId - 1];
    slot.loaded = true;
    slot.hidden = true;
    slot.length = 0;
    slot.cap = 0;
    loadMaskClear(fieldId);
    resetAssembly();
    checkLoadComplete();
    return;
  }

  memcpy(client->arena + client->arenaUsed + client->asmLen, data, dataLen);
  client->asmLen += dataLen;

  if (chunks > 0) {
    client->expectChunks = chunks - 1;
    client->chunk++;
    client->reqOutstanding = false;  // request the next chunk immediately
  } else {
    commitRecord(fieldId);
  }
}

//
// device discovery
//

static void handleDeviceInfo(const uint8_t* rx, uint8_t pktLen)
{
  uint8_t origin = rx[4];
  const uint8_t* payload = rx + 5;
  uint16_t payloadLen = pktLen - 6;

  // name, then serial/hw/fw (12 bytes), field count, param version
  uint16_t nameLen = 0;
  while (nameLen < payloadLen && payload[nameLen]) nameLen++;
  if (nameLen + 15 > payloadLen) return;

  uint32_t serial = readBE(payload + nameLen + 1, 4, false);
  uint8_t fieldCount = payload[nameLen + 13];
  uint8_t paramVersion = payload[nameLen + 14];

  // find or create the device entry
  uint8_t idx = 0xFF;
  for (uint8_t i = 0; i < client->nDevices; i++) {
    if (client->devices[i].address == origin) {
      idx = i;
      break;
    }
  }
  if (idx == 0xFF) {
    if (client->nDevices >= CRSF_CFG_MAX_DEVICES) return;
    idx = client->nDevices++;
    client->generation++;
  }

  CrsfConfigDevice& dev = client->devices[idx];
  bool fieldCountChanged = dev.fieldCount != fieldCount;
  dev.address = origin;
  dev.fieldCount = fieldCount;
  dev.paramVersion = paramVersion;
  dev.isELRS = (serial == 0x454C5253 /* 'ELRS' */);
  dev.isELRS_TX = dev.isELRS && (origin == MODULE_ADDRESS);
  // firmware version: bytes 1..2 of the firmware id
  dev.fwMajor = payload[nameLen + 10];
  dev.fwMinor = payload[nameLen + 11];
  strncpy(dev.name, (const char*)payload,
          min<uint16_t>(nameLen, CRSF_NAME_MAXSIZE - 1));
  dev.name[min<uint16_t>(nameLen, CRSF_NAME_MAXSIZE - 1)] = '\0';

  if (client->currentDevice == 0xFF) {
    // auto select the TX module; other devices (e.g. the receiver, over
    // the slower RF link) may answer first and must not capture the menu
    // (the tick falls back to the first device if no module answers)
    if (origin == MODULE_ADDRESS) {
      client->currentDevice = idx;
      clearFields();
      client->generation++;
    }
  } else if (idx == client->currentDevice && fieldCountChanged) {
    clearFields();
    client->generation++;
  }
}

static void handleElrsStatus(const uint8_t* rx, uint8_t pktLen)
{
  if (pktLen < 11) return;

  client->elrs.badPkts = rx[5];
  client->elrs.goodPkts = (rx[6] << 8) | rx[7];
  client->elrs.flags = rx[8];

  uint16_t infoLen = pktLen - 10;
  if (infoLen >= sizeof(client->elrs.flagInfo))
    infoLen = sizeof(client->elrs.flagInfo) - 1;
  memcpy(client->elrs.flagInfo, rx + 9, infoLen);
  client->elrs.flagInfo[infoLen] = '\0';
  // no generation bump: status is polled live by both UIs, and a full
  // page rebuild on every connect/arm transition would break open edits
}

//
// staged value writes / command steps (cfgMutex held)
//

static void flushStagedWrite()
{
  uint8_t id = client->stagedId;
  if (!id) return;
  client->stagedId = 0;

  CrsfConfigField f;
  if (!getFieldImpl(id, f) || !f.loaded) return;

  int32_t value = client->stagedValue;
  uint8_t data[4];
  uint8_t len = 0;
  switch (f.type) {
    case CRSF_FIELD_UINT8:
    case CRSF_FIELD_INT8:
    case CRSF_FIELD_TEXT_SELECTION:
      data[len++] = value & 0xFF;
      break;
    case CRSF_FIELD_UINT16:
    case CRSF_FIELD_INT16:
      data[len++] = (value >> 8) & 0xFF;
      data[len++] = value & 0xFF;
      break;
    case CRSF_FIELD_FLOAT:
      data[len++] = (value >> 24) & 0xFF;
      data[len++] = (value >> 16) & 0xFF;
      data[len++] = (value >> 8) & 0xFF;
      data[len++] = value & 0xFF;
      break;
    default:
      return;
  }
  queueWrite(id, data, len);

  // devices do not echo writes: re-read the field and its editable
  // siblings once the EEPROM has committed (like elrs.lua does)
  client->reloadAt = schedTime(get_tmr10ms() + WRITE_RELOAD_DELAY);
  client->reloadField = id;
}

static void commandStartImpl(uint8_t id)
{
  CrsfConfigField f;
  if (!getFieldImpl(id, f) || f.type != CRSF_FIELD_COMMAND) return;

  flushStagedWrite();
  uint8_t step = CRSF_CMD_START;
  queueWrite(id, &step, 1);
  client->cmdActive = true;
  client->cmdField = id;
  client->cmdStatus = CRSF_CMD_START;
  client->cmdTimeout = f.cmdTimeout ? f.cmdTimeout : 50;
  client->cmdNextPoll = get_tmr10ms() + client->cmdTimeout;
  loadMaskSet(id);  // fetch the initial command status right away
  client->generation++;
}

static void commandCancelImpl()
{
  if (!client->cmdActive) return;
  uint8_t step = CRSF_CMD_CANCEL;
  queueWrite(client->cmdField, &step, 1);
  client->cmdActive = false;
  loadMaskSet(client->cmdField);  // refresh the command's idle state
  client->generation++;
}

//
// periodic driver
//

static void parseFrame(const uint8_t* rx, uint8_t length);

void crsfConfigTick()
{
  if (!client) return;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  if (!client) return;

  if (moduleState[client->moduleIdx].protocol != PROTOCOL_CHANNELS_CROSSFIRE) {
    lock.unlock();
    crsfConfigStop();
    return;
  }

  // frames queued by the telemetry task since the last tick
  for (uint8_t i = 0; i < client->rxCount; i++)
    parseFrame(client->rxFrame[i], client->rxLen[i]);
  client->rxCount = 0;

  if (client->state == CRSF_STATE_ELRS_V1) return;

  tmr10ms_t now = get_tmr10ms();

  // deferred post-write reload
  if (client->reloadAt && (tmr10ms_t)(now - client->reloadAt) < 0x8000) {
    client->reloadAt = 0;
    if (client->reloadField == 0) {
      // refresh everything (after a finished command), in place: the
      // current values stay visible, and the menu remains usable even
      // if the module stops answering (e.g. it switched to WiFi or
      // BLE joystick mode)
      uint8_t count = deviceFieldCount();
      for (uint8_t id = 1; id <= count; id++) loadMaskSet(id);
    } else {
      reloadSiblings(client->reloadField);
      // some side effects appear only after the hardware settles,
      // e.g. the backpack version once the backpack has booted
      client->settleAt = schedTime(now + WRITE_SETTLE_DELAY);
      client->settleField = client->reloadField;
    }
    if (loadMaskNext() != 0) client->state = CRSF_STATE_LOADING;
  }

  // one-time late re-read after a write; a root-level change (packet
  // rate, band, domain) can reshape fields everywhere, so re-read all
  if (client->settleAt && (tmr10ms_t)(now - client->settleAt) < 0x8000) {
    client->settleAt = 0;
    uint8_t f = client->settleField;
    uint8_t count = deviceFieldCount();
    if (f >= 1 && f <= count && client->slots[f - 1].parent == 0) {
      for (uint8_t id = 1; id <= count; id++)
        if (client->slots[id - 1].loaded) loadMaskSet(id);
    } else {
      reloadSiblings(f);
    }
  }

  // no TX module answered, but another device did: select it
  if (client->currentDevice == 0xFF && client->nDevices > 0 &&
      (tmr10ms_t)(now - client->startTime) > 300) {
    client->currentDevice = 0;
    clearFields();
    client->generation++;
  }

  // scheduled full refresh (e.g. the module rebooted after a UID change)
  if (client->refreshAt && (tmr10ms_t)(now - client->refreshAt) < 0x8000) {
    client->refreshAt = 0;
    client->uidValid = false;
    clearFields();
    client->pingRequested = true;
    client->nextPing = now;
    // re-read the bind UID once the device is back (retries until answered)
    if (client->uidDevice) {
      client->uidWanted = true;
      client->uidNextReq = now + 100;
    }
    client->generation++;
  }

  // classic bind loop: the module's bind burst only lasts ~1s, so it is
  // re-triggered until the link status reports a connected receiver
  if (client->bindLoop) {
    if (!(client->elrs.flags & CRSF_ELRS_FLAG_CONNECTED)) {
      client->bindSawDisc = true;
    } else if (client->bindSawDisc) {
      client->bindLoop = false;
      client->bindBound = true;
      if (client->cmdActive) commandCancelImpl();
    }
    if (client->bindLoop && !client->cmdActive && !client->pendFrameType &&
        (tmr10ms_t)(now - client->bindNextRestart) < 0x8000) {
      uint8_t id = elrsBindFieldIdImpl();
      if (id)
        commandStartImpl(id);
      else
        sendBindCommandImpl(MODULE_ADDRESS);
      client->bindNextRestart = now + BIND_RESTART_INTERVAL;
    }
  }

  // staged value write: send once the encoder has settled
  if (client->stagedId && !client->pendFrameType &&
      (tmr10ms_t)(now - client->stagedAt) >= WRITE_STAGE_DELAY) {
    flushStagedWrite();
  }

  // deferred TX half of an RX-then-TX bind info change
  if (client->txBindPending && !client->pendFrameType &&
      (tmr10ms_t)(now - client->txBindAt) < 0x8000) {
    client->txBindPending = false;
    queueMspConfigWrite(MODULE_ADDRESS,
                        client->txBindIsUid ? MSP_ELRS_SUBCMD_UID
                                            : MSP_ELRS_SUBCMD_BIND_PHRASE,
                        client->txBindData, client->txBindLen);
    client->refreshAt = schedTime(now + TX_BIND_DELAY);  // reboot follows
  }

  // 1) pending frame has priority: leave the module queue free for it
  if (client->pendFrameType) {
    sendPendingFrame();
    return;
  }

  // ExpressLRS UID (bind info) polling
  if (client->uidWanted && (tmr10ms_t)(now - client->uidNextReq) < 0x8000) {
    uint8_t payload[] = {client->uidDevice, RADIO_ADDRESS,
                         0x30, 0x01, MSP_ELRS_RXTX_CONFIG,
                         MSP_ELRS_SUBCMD_UID};
    if (sendFrame(MSP_REQ_ID, payload, sizeof(payload))) {
      client->uidNextReq = now + 50;
      return;
    }
  }

  // 2) command status polling
  if (client->cmdActive && client->cmdStatus != CRSF_CMD_CONFIRMATION_NEEDED &&
      (tmr10ms_t)(now - client->cmdNextPoll) < 0x8000) {
    uint8_t payload[] = {currentDevAddress(), RADIO_ADDRESS, client->cmdField,
                         CRSF_CMD_POLL};
    if (sendFrame(PARAM_WRITE_ID, payload, sizeof(payload))) {
      loadMaskSet(client->cmdField);  // re-read status/info text
      client->cmdNextPoll = now + (client->cmdTimeout ? client->cmdTimeout : 50);
      resetAssembly();
    }
    return;
  }

  // 3) ELRS link status poll (kept ahead of field loading so connection
  // changes are noticed promptly, e.g. while binding; fires 1x per second)
  auto statusDev = currentDev();
  if (statusDev && statusDev->isELRS_TX &&
      (tmr10ms_t)(now - client->nextStatusPoll) < 0x8000) {
    if (sendStatusRequest()) {
      client->nextStatusPoll = now + STATUS_POLL_INTERVAL;
      return;
    }
  }

  // 4) field requests
  uint8_t next = client->loading ? client->loading : loadMaskNext();
  if (next != 0 && currentDev()) {
    bool local = currentDevAddress() == MODULE_ADDRESS;
    tmr10ms_t timeout = local ? REQ_TIMEOUT_LOCAL : REQ_TIMEOUT_REMOTE;

    if (!client->reqOutstanding ||
        (tmr10ms_t)(now - client->reqTime) > timeout) {
      if (client->loading != next) {
        client->loading = next;
        client->chunk = 0;
        client->expectChunks = -1;
        client->asmLen = 0;
      }
      if (client->reqOutstanding && client->chunk == 0) {
        // retry from scratch on timeout
        client->expectChunks = -1;
        client->asmLen = 0;
      }
      if (sendFieldRequest(client->loading, client->chunk)) {
        client->reqOutstanding = true;
        client->reqTime = now;
      }
    }
    return;
  }

  // 5) device discovery: ping until something answers (like the ELRS
  // Lua), then only when explicitly asked to rescan - no periodic
  // pings wasting channel slots
  if ((client->nDevices == 0 || client->pingRequested) &&
      (tmr10ms_t)(now - client->nextPing) < 0x8000) {
    if (sendPing()) {
      client->pingRequested = false;
      client->nextPing = now + PING_INTERVAL;
    }
  }
}

//
// frame sink (telemetry task): frames are queued raw and parsed by the
// next tick, so the parsing only ever runs on the menus task
//

void crsfConfigFrameReceived(uint8_t module, const uint8_t* rxBuffer,
                             uint8_t length)
{
  if (!cfgMutexCreated || !client) return;

  uint8_t type = rxBuffer[2];
  if (type != DEVICE_INFO_ID && type != PARAM_SETTINGS_ENTRY_ID &&
      type != ELRS_STATUS_ID && type != PARAM_WRITE_ID &&
      type != MSP_RESP_ID)
    return;

  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  if (!client || module != client->moduleIdx) return;
  if (length < 6 || length > CRSF_CFG_RX_FRAME_SIZE) return;
  if (client->rxCount >= CRSF_CFG_RX_FRAMES) return;  // retries recover

  memcpy(client->rxFrame[client->rxCount], rxBuffer, length);
  client->rxLen[client->rxCount++] = length;
}

static void parseFrame(const uint8_t* rx, uint8_t length)
{
  switch (rx[2]) {
    case DEVICE_INFO_ID:
      handleDeviceInfo(rx, length);
      break;

    case PARAM_SETTINGS_ENTRY_ID:
      handleSettingsEntry(rx, length);
      break;

    case ELRS_STATUS_ID:
      handleElrsStatus(rx, length);
      break;

    case PARAM_WRITE_ID:
      // a device never sends PARAMETER_WRITE to the handset, except
      // ELRS 1.x firmware, which is incompatible with this protocol
      if (rx[3] == RADIO_ADDRESS && rx[4] == MODULE_ADDRESS) {
        client->state = CRSF_STATE_ELRS_V1;
        client->generation++;
      }
      break;

    case MSP_RESP_ID:
      // ExpressLRS RXTX_CONFIG response carrying the bind UID
      if (client->uidWanted && length >= 16 &&
          rx[7] == MSP_ELRS_RXTX_CONFIG && rx[8] == MSP_ELRS_SUBCMD_UID &&
          rx[4] == client->uidDevice) {
        memcpy(client->uid, rx + 9, 6);
        client->uidValid = true;
        client->uidWanted = false;
        client->generation++;
      }
      break;
  }
}

//
// public API (menus task)
//

void crsfConfigStart(uint8_t moduleIdx)
{
  if (!cfgMutexCreated) {
    mutex_create(&cfgMutex);
    cfgMutexCreated = true;
  }
  crsfConfigStop();

  auto c = new CrsfConfigClient();
  c->moduleIdx = moduleIdx;
  c->state = CRSF_STATE_PING;
  c->currentDevice = 0xFF;
  c->startTime = get_tmr10ms();
  c->nextPing = c->startTime;  // ping right away
  c->nextStatusPoll = c->startTime;

  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  client = c;
}

void crsfConfigStop()
{
  if (!client) return;
  CrsfConfigClient* c;
  {
    MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
    c = client;
    if (!c) return;
    // best effort: do not lose writes the user already made
    flushStagedWrite();
    if (c->txBindPending) {
      c->txBindPending = false;
      queueMspConfigWrite(MODULE_ADDRESS,
                          c->txBindIsUid ? MSP_ELRS_SUBCMD_UID
                                         : MSP_ELRS_SUBCMD_BIND_PHRASE,
                          c->txBindData, c->txBindLen);
    }
    if (c->pendFrameType) sendPendingFrame();
    client = nullptr;
  }
  // the frame sink can no longer reach it
  delete c;
}

bool crsfConfigActive() { return client != nullptr; }

CrsfConfigState crsfConfigState()
{
  return client ? client->state : CRSF_STATE_INACTIVE;
}

uint16_t crsfConfigGeneration() { return client ? client->generation : 0; }

uint8_t crsfConfigDeviceCount() { return client ? client->nDevices : 0; }

const CrsfConfigDevice* crsfConfigGetDevice(uint8_t index)
{
  if (!client || index >= client->nDevices) return nullptr;
  return &client->devices[index];
}

const CrsfConfigDevice* crsfConfigCurrentDevice()
{
  if (!client) return nullptr;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  return currentDev();
}

void crsfConfigRefreshDevices()
{
  if (!client) return;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  client->pingRequested = true;
  client->nextPing = get_tmr10ms();
}

void crsfConfigSelectDevice(uint8_t address)
{
  if (!client) return;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  for (uint8_t i = 0; i < client->nDevices; i++) {
    if (client->devices[i].address == address) {
      if (client->currentDevice != i) {
        client->currentDevice = i;
        client->cmdActive = false;
        client->bindLoop = false;
        client->stagedId = 0;
        client->pendFrameType = 0;
        client->pendFrameLen = 0;
        client->reloadAt = 0;
        client->nextStatusPoll = get_tmr10ms();
        clearFields();
        client->generation++;
      }
      return;
    }
  }
}

uint8_t crsfConfigFieldCount()
{
  if (!client) return 0;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  return deviceFieldCount();
}

uint8_t crsfConfigLoadedFieldCount()
{
  if (!client) return 0;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  uint8_t n = 0;
  uint8_t count = deviceFieldCount();
  for (uint8_t i = 0; i < count; i++)
    if (client->slots[i].loaded) n++;
  return n;
}

bool crsfConfigGetField(uint8_t id, CrsfConfigField& out)
{
  if (!client) {
    memclear(&out, sizeof(out));
    out.name = out.unit = out.options = out.svalue = "";
    return false;
  }
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  return getFieldImpl(id, out);
}

void crsfConfigReloadFolder(uint8_t folderId)
{
  if (!client) return;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  uint8_t count = deviceFieldCount();
  for (uint8_t id = 1; id <= count; id++) {
    FieldSlot& slot = client->slots[id - 1];
    if (slot.loaded && slot.parent == folderId) loadMaskSet(id);
  }
}

bool crsfConfigSelectionAllowed(uint8_t fieldId, uint8_t index)
{
  if (!client) return false;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  return selectionAllowedImpl(fieldId, index);
}

uint8_t crsfConfigNextAllowedOption(uint8_t fieldId, uint8_t current,
                                    int8_t dir)
{
  if (!client) return current;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  CrsfConfigField f;
  if (!getFieldImpl(fieldId, f) || !f.loaded) return current;
  uint8_t count = crsfConfigOptionCount(f.options);
  if (count == 0) return current;
  int idx = current;
  for (uint8_t i = 0; i < count; i++) {
    idx += dir;
    if (idx < 0 || idx >= count) return current;
    if (selectionAllowedImpl(fieldId, idx)) return idx;
  }
  return current;
}

void crsfConfigWriteValue(uint8_t id, int32_t value)
{
  if (!client) return;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  if (client->stagedId && client->stagedId != id) flushStagedWrite();
  client->stagedId = id;
  client->stagedShownId = id;
  client->stagedValue = value;
  client->stagedAt = get_tmr10ms();
}

void crsfConfigWriteString(uint8_t id, const char* value)
{
  if (!client) return;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  uint8_t len = strlen(value) + 1;  // include the terminator
  if (len > CRSF_CFG_MAX_WRITE) return;
  queueWrite(id, (const uint8_t*)value, len);
  client->reloadAt = schedTime(get_tmr10ms() + WRITE_RELOAD_DELAY);
  client->reloadField = id;
}

void crsfConfigCommandStart(uint8_t id)
{
  if (!client) return;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  commandStartImpl(id);
}

void crsfConfigCommandConfirm()
{
  if (!client) return;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  if (!client->cmdActive) return;
  uint8_t step = CRSF_CMD_CONFIRM;
  queueWrite(client->cmdField, &step, 1);
  client->cmdStatus = CRSF_CMD_CONFIRM;  // resume polling
  client->cmdNextPoll = get_tmr10ms() + client->cmdTimeout;
  client->generation++;
}

void crsfConfigCommandCancel()
{
  if (!client) return;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  commandCancelImpl();
}

void crsfConfigCommandDetach()
{
  if (!client) return;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  if (!client->cmdActive) return;
  // stop polling but do not send anything: the command (e.g. WiFi,
  // BLE joystick) keeps running on the device
  client->cmdActive = false;
  loadMaskSet(client->cmdField);
  client->generation++;
}

bool crsfConfigCommandActive() { return client && client->cmdActive; }

uint8_t crsfConfigCommandField() { return client ? client->cmdField : 0; }

uint8_t crsfConfigCommandStatus()
{
  return client ? client->cmdStatus : CRSF_CMD_READY;
}

CrsfElrsStatus crsfConfigElrsStatus()
{
  if (!client) return CrsfElrsStatus();
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  return client->elrs;
}

void crsfConfigAckCriticalFlags()
{
  if (!client) return;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  uint8_t zero = 0;
  queueWrite(ELRS_STATUS_ID, &zero, 1);
  client->elrs.flags = 0;
  client->generation++;
}

//
// ExpressLRS binding
//

bool crsfConfigElrsCanSetBindInfo(const CrsfConfigDevice* dev)
{
  // bind phrase / UID configuration over MSP requires ELRS >= 4.1
  return dev && dev->isELRS &&
         (dev->fwMajor > 4 || (dev->fwMajor == 4 && dev->fwMinor >= 1));
}

uint8_t crsfConfigElrsBindFieldId()
{
  if (!client) return 0;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  return elrsBindFieldIdImpl();
}

bool crsfConfigElrsIsVersionField(const CrsfConfigField& f)
{
  // the ELRS version parameter carries the version as its NAME and the
  // commit hash as its value
  if (!client) return false;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  auto dev = currentDev();
  return dev && dev->isELRS_TX && f.parent == 0 &&
         f.type == CRSF_FIELD_INFO && f.name && f.name[0] >= '0' &&
         f.name[0] <= '9';
}

bool crsfConfigSendBindCommand(uint8_t destAddr)
{
  if (!client) return false;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  return sendBindCommandImpl(destAddr);
}

// parses "a,b,c,d,e,f" (4..6 numbers 0..255) into a left-zero-padded
// UID, the same way the official ExpressLRS bind-phrase script does
static bool parseUidText(const char* s, uint8_t uid[6])
{
  int values[6];
  uint8_t n = 0;
  const char* p = s;
  while (*p && n < 6) {
    if (*p < '0' || *p > '9') return false;
    int v = 0;
    while (*p >= '0' && *p <= '9') {
      v = v * 10 + (*p++ - '0');
      if (v > 255) return false;
    }
    values[n++] = v;
    if (*p == ',') {
      p++;
      if (!*p) return false;
    } else if (*p) {
      return false;
    }
  }
  if (*p || n < 4) return false;
  memset(uid, 0, 6);
  for (uint8_t i = 0; i < n; i++) uid[6 - n + i] = values[i];
  return true;
}

bool crsfConfigApplyBindInfo(const char* text, bool rxFirst)
{
  if (!client) return false;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  if (client->pendFrameType) return false;  // busy, try again

  uint8_t uid[6];
  bool isUid = parseUidText(text, uid);
  const uint8_t* data = isUid ? uid : (const uint8_t*)text;
  uint8_t len = isUid ? 6 : strlen(text);
  if (len > CRSF_BIND_PHRASE_MAXLEN) return false;
  uint8_t subcmd = isUid ? MSP_ELRS_SUBCMD_UID : MSP_ELRS_SUBCMD_BIND_PHRASE;

  tmr10ms_t now = get_tmr10ms();
  client->uidValid = false;  // shown again once re-read after the reboot
  if (rxFirst && len) {
    // set the receiver first, while the current link is still up; the
    // TX half follows from the tick once the OTA link has delivered
    queueMspConfigWrite(RECEIVER_ADDRESS, subcmd, data, len);
    memcpy(client->txBindData, data, len);
    client->txBindLen = len;
    client->txBindIsUid = isUid;
    client->txBindPending = true;
    client->txBindAt = now + TX_BIND_DELAY;
  } else {
    // an empty phrase clears the binding: TX only, never the RX
    queueMspConfigWrite(MODULE_ADDRESS, subcmd, data, len);
    client->refreshAt = schedTime(now + TX_BIND_DELAY + 10);
  }
  return true;
}

void crsfConfigRequestUid(uint8_t destAddr)
{
  if (!client) return;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  client->uidWanted = true;
  client->uidValid = false;
  client->uidDevice = destAddr;
  client->uidNextReq = get_tmr10ms();
}

bool crsfConfigGetUid(uint8_t uid[6])
{
  if (!client) return false;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  if (!client->uidValid) return false;
  memcpy(uid, client->uid, 6);
  return true;
}

void crsfConfigBindLoopStart()
{
  if (!client) return;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  client->bindLoop = true;
  client->bindSawDisc = false;
  client->bindBound = false;
  client->bindNextRestart = get_tmr10ms();
}

void crsfConfigBindLoopStop()
{
  if (!client) return;
  MutexLock lock = MutexLock::MakeInstance(&cfgMutex);
  if (!client->bindLoop) return;
  client->bindLoop = false;
  if (client->cmdActive) commandCancelImpl();
}

bool crsfConfigBindLoopActive() { return client && client->bindLoop; }

// true once the loop has stopped because a receiver connected
bool crsfConfigBindLoopBound() { return client && client->bindBound; }

#endif  // defined(CRSF_CONFIG_MENU)
