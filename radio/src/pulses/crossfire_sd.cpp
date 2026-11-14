/*
 * Copyright (C) EdgeTX
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

#include "crossfire_sd.h"

#include "edgetx.h"
#include "crossfire.h"
#include "hal/module_port.h"
#include "hal/usb_driver.h"
#include "os/sleep.h"
#include "os/time.h"
#include "tasks/mixer_task.h"
#include "telemetry/crossfire.h"

// ---------------------------------------------------------------------------
// In-band offer / accept
// ---------------------------------------------------------------------------

enum SdState : uint8_t {
  SD_IDLE,
  SD_OFFER_DUE,
  SD_OFFERED,
  SD_ACCEPTED,
  SD_RUNNING,
  SD_REJECTED,
};

#define SD_OFFER_RESEND_MS   100
#define SD_OFFER_TIMEOUT_MS  3000
#define SD_ALARM_QUIET_MS    5000

volatile bool crsfSdUiDue = false;

static volatile uint8_t _state = SD_IDLE;
static volatile uint8_t _module = 0;
static volatile uint8_t _kind = 0;
static volatile uint32_t _offerStart = 0;
static volatile uint32_t _offerLast = 0;
static volatile uint32_t _alarmQuietUntil = 0;
static volatile bool _portOwned[NUM_MODULES] = {};
static char _rejectText[33];

// A request we can't serve right now gets a REJECT with the reason, in the next
// slot an offer would have used. Separate from _state: that stays SD_IDLE.
static volatile bool _refuseDue = false;
static volatile uint8_t _refuseModule = 0;
static volatile uint8_t _refuseKind = 0;
static volatile uint8_t _refuseReason = 0;
static const char* volatile _refuseText = "";

static void _refuse(uint8_t module, uint8_t kind, uint8_t reason, const char* text)
{
  _refuseModule = module;
  _refuseKind = kind;
  _refuseReason = reason;
  _refuseText = text;
  _refuseDue = true;
}

bool crsfSdStartOffer(uint8_t module, uint8_t kind)
{
  if (_state != SD_IDLE || module >= NUM_MODULES) return false;
  _module = module;
  _kind = kind;
  _offerStart = time_get_ms();
  _state = SD_OFFER_DUE;
  return true;
}

static bool _armed(uint8_t module)
{
  ModuleData* md = &g_model.moduleData[module];
  if (md->crsf.crsfArmingMode != ARMING_MODE_SWITCH) return false;
  swsrc_t sw = md->crsf.crsfArmingTrigger;
  return sw != SWSRC_NONE && getSwitch(sw, 0);
}

void crsfSdProcessFrame(uint8_t module, const uint8_t* frame, uint8_t len)
{
  // [sync][len][type][dest][origin][subtype][kind]...[crc]
  if (len < 8 || frame[3] != RADIO_ADDRESS) return;
  uint8_t sub = frame[5];
  uint8_t kind = frame[6];

  switch (sub) {
    case CRSF_SD_REQUEST:
      // The module asking is the user asking: no prompt. Only refuse what
      // would be unsafe or impossible right now.
      // The module retries its request every 100 ms, so each one gets its own
      // refusal; the text is what the module shows after "Radio: "
      if (_armed(module)) {
        TRACE("[SD] request refused: armed");
        _refuse(module, kind, 1, "armed");
        return;
      }
      if (usbPlugged() && getSelectedUsbMode() == USB_MASS_STORAGE_MODE) {
        TRACE("[SD] request refused: USB storage");
        _refuse(module, kind, 2, "USB storage is on");
        return;
      }
      if (!sdMounted()) {
        TRACE("[SD] request refused: no card");
        _refuse(module, kind, 3, "no SD card");
        return;
      }
      if (crsfSdStartOffer(module, kind)) {
        TRACE("[SD] request kind=%d -> offer", kind);
      }
      break;

    case CRSF_SD_ACCEPT:
      if (module == _module &&
          (_state == SD_OFFERED || _state == SD_OFFER_DUE)) {
        TRACE("[SD] accepted");
        _state = SD_ACCEPTED;
        crsfSdUiDue = true;
      }
      break;

    case CRSF_SD_REJECT:
      if (module == _module &&
          (_state == SD_OFFERED || _state == SD_OFFER_DUE)) {
        // [reason][text...][crc]: the text is what's between frame[8] and the crc
        uint8_t n = 0;
        if (len > 9) {
          n = min<uint8_t>(len - 9, sizeof(_rejectText) - 1);
          memcpy(_rejectText, &frame[8], n);
        }
        _rejectText[n] = '\0';
        TRACE("[SD] rejected: %s", _rejectText);
        _state = SD_REJECTED;
        crsfSdUiDue = true;
      }
      break;
  }
}

uint8_t crsfSdSetupOffer(uint8_t module, uint8_t* frame)
{
  if (_refuseDue && module == _refuseModule) {
    // [REJECT][kind][reason][text]
    const char* text = _refuseText;
    const uint8_t n = min<size_t>(strlen(text), 32);
    uint8_t* buf = frame;
    *buf++ = UART_SYNC;
    *buf++ = 7 + n;  // type .. crc
    *buf++ = CRSF_SD_SESSION_ID;
    *buf++ = MODULE_ADDRESS;
    *buf++ = RADIO_ADDRESS;
    *buf++ = CRSF_SD_REJECT;
    *buf++ = _refuseKind;
    *buf++ = _refuseReason;
    memcpy(buf, text, n);
    buf += n;
    *buf = crc8(frame + 2, buf - frame - 2);
    buf++;
    _refuseDue = false;
    return buf - frame;
  }

  if (module != _module) return 0;

  uint32_t now = time_get_ms();
  // an offer nobody answers: give up on it here, on the frame path, rather than
  // have the UI loop watch the clock
  if ((_state == SD_OFFER_DUE || _state == SD_OFFERED) &&
      now - _offerStart > SD_OFFER_TIMEOUT_MS) {
    TRACE("[SD] offer timed out");
    _state = SD_IDLE;
    return 0;
  }
  if (_state == SD_OFFER_DUE ||
      (_state == SD_OFFERED && now - _offerLast >= SD_OFFER_RESEND_MS)) {
    uint8_t* buf = frame;
    *buf++ = UART_SYNC;
    *buf++ = 9;  // type .. crc
    *buf++ = CRSF_SD_SESSION_ID;
    *buf++ = MODULE_ADDRESS;
    *buf++ = RADIO_ADDRESS;
    *buf++ = CRSF_SD_OFFER;
    *buf++ = _kind;
    *buf++ = CRSF_SD_PROTO_VERSION;
    *buf++ = CRSF_SD_MAX_PAYLOAD & 0xFF;
    *buf++ = CRSF_SD_MAX_PAYLOAD >> 8;
    *buf++ = crc8(frame + 2, 8);
    _offerLast = now;
    _state = SD_OFFERED;
    return buf - frame;
  }
  return 0;
}

bool crsfSdPortOwned(uint8_t module)
{
  return module < NUM_MODULES && _portOwned[module];
}

bool crsfSdSuppressAlarms()
{
  return _state == SD_RUNNING || _state == SD_ACCEPTED ||
         (int32_t)(_alarmQuietUntil - time_get_ms()) > 0;
}

// ---------------------------------------------------------------------------
// Bulk framing
//   [0xEB][len u16][type][seq][payload...][crc16 u16]
//   len counts type..payload, crc16 (CCITT-FALSE) covers the same bytes
// ---------------------------------------------------------------------------

#define BULK_SYNC        0xEB
#define BULK_HDR         5   // sync, len(2), type, seq
#define BULK_OVERHEAD    7
#define BULK_RESP        0x80

#define HELLO_TIMEOUT_MS 3000
#define IDLE_TIMEOUT_MS  3000
#define CANCEL_GRACE_MS  2000

// Writes fail with NO_SPACE below this much free card space
#define SD_FREE_FLOOR    (8u * 1024 * 1024)

#define SD_ROOT          "/ELRS"
#define SD_MAX_REL_PATH  64
#define SD_MAX_DEPTH     4
#define SD_MAX_HANDLES   2

enum BulkOp : uint8_t {
  OP_HELLO = 0x01,
  OP_STAT = 0x02,
  OP_LIST = 0x03,
  OP_OPEN = 0x04,
  OP_READ = 0x05,
  OP_WRITE = 0x06,
  OP_CLOSE = 0x07,
  OP_RENAME = 0x08,
  OP_DELETE = 0x09,
  OP_MKDIR = 0x0A,
  OP_PROGRESS = 0x0B,
  OP_END = 0x0C,
};

enum BulkStatus : uint8_t {
  ST_OK = 0,
  ST_EOF = 1,
  ST_NOT_FOUND = 2,
  ST_DENIED = 3,
  ST_NO_SPACE = 4,
  ST_BAD_HANDLE = 5,
  ST_IO_ERROR = 6,
  ST_BUSY = 7,
  ST_CANCELLED = 8,
  ST_BAD_REQUEST = 9,
};

static uint16_t _crc16(const uint8_t* p, uint32_t n)
{
  uint16_t crc = 0xFFFF;
  while (n--) {
    crc ^= (uint16_t)(*p++) << 8;
    for (uint8_t i = 0; i < 8; i++)
      crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
  }
  return crc;
}

static inline uint16_t _get16(const uint8_t* p) { return p[0] | (p[1] << 8); }
static inline uint32_t _get32(const uint8_t* p)
{
  return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24);
}
static inline uint8_t* _put16(uint8_t* p, uint16_t v)
{
  *p++ = v; *p++ = v >> 8;
  return p;
}
static inline uint8_t* _put32(uint8_t* p, uint32_t v)
{
  *p++ = v; *p++ = v >> 8; *p++ = v >> 16; *p++ = v >> 24;
  return p;
}

struct SdHandle {
  FIL fil;
  bool open;
};

struct BulkSession {
  const etx_serial_driver_t* drv;
  void* ctx;

  uint16_t rxLen;
  uint16_t maxPayload;
  uint16_t txLen;
  uint8_t lastType;
  uint8_t lastSeq;
  bool helloSeen;
  bool cancelled;
  bool ended;
  bool endOk;
  uint32_t freeBytes;
  char endText[49];

  SdHandle h[SD_MAX_HANDLES];
};

static BulkSession _s;
static uint8_t _rx[CRSF_SD_MAX_PAYLOAD + BULK_OVERHEAD + 16];
static uint8_t _tx[CRSF_SD_MAX_PAYLOAD + BULK_OVERHEAD + 16] __DMA_NO_CACHE;

// Everything the module can touch lives under /ELRS/. Not about trusting the
// module: a TX firmware bug with a bad path shouldn't eat models or settings.
static uint8_t _buildPath(const uint8_t* p, uint16_t n, char* out)
{
  if (n && p[0] == '/') { p++; n--; }
  if (n > SD_MAX_REL_PATH) return ST_DENIED;

  uint8_t depth = 0;
  uint16_t compStart = 0;
  for (uint16_t i = 0; i <= n; i++) {
    if (i == n || p[i] == '/') {
      uint16_t clen = i - compStart;
      if (i == n && clen == 0 && n > 0 && p[n - 1] == '/') break;  // trailing '/'
      if (clen == 0 && n > 0) return ST_DENIED;                      // "//"
      if ((clen == 1 && p[compStart] == '.') ||
          (clen == 2 && p[compStart] == '.' && p[compStart + 1] == '.'))
        return ST_DENIED;
      if (clen && ++depth > SD_MAX_DEPTH) return ST_DENIED;
      compStart = i + 1;
      continue;
    }
    uint8_t c = p[i];
    bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-';
    if (!ok) return ST_DENIED;
  }

  char* o = strAppend(out, SD_ROOT);
  if (n) {
    *o++ = '/';
    memcpy(o, p, n);
    o += n;
    if (o[-1] == '/') o--;
  }
  *o = '\0';
  return ST_OK;
}

static uint8_t _fsStatus(FRESULT r)
{
  switch (r) {
    case FR_OK: return ST_OK;
    case FR_NO_FILE:
    case FR_NO_PATH: return ST_NOT_FOUND;
    case FR_DENIED:
    case FR_EXIST:
    case FR_WRITE_PROTECTED: return ST_DENIED;
    default: return ST_IO_ERROR;
  }
}

static uint32_t _freeCardBytes()
{
  FATFS* fs;
  DWORD fre = 0;
  if (f_getfree("", &fre, &fs) != FR_OK) return 0;
  uint64_t bytes = (uint64_t)fre * fs->csize * FF_MAX_SS;
  return bytes > 0xFFFFFFFFu ? 0xFFFFFFFFu : (uint32_t)bytes;
}

static void _closeAll()
{
  for (auto& h : _s.h) {
    if (h.open) f_close(&h.fil);
    h.open = false;
  }
}

static SdHandle* _handle(uint8_t idx)
{
  if (idx >= SD_MAX_HANDLES || !_s.h[idx].open) return nullptr;
  return &_s.h[idx];
}

// Builds the response into _tx (after the 6-byte header incl. status) and
// returns the payload length after the status byte.
static uint16_t _dispatch(uint8_t op, const uint8_t* req, uint16_t n,
                        uint8_t& st, const CrsfSdUi& ui)
{
  uint8_t* out = &_tx[BULK_HDR + 1];
  char path[sizeof(SD_ROOT) + SD_MAX_REL_PATH + 2];
  st = ST_OK;

  if (_s.cancelled && op != OP_END && op != OP_CLOSE && op != OP_HELLO) {
    st = ST_CANCELLED;
    return 0;
  }

  switch (op) {
    case OP_HELLO: {
      // [proto][max u16] -> [proto][max u16][free KB u32][flags][rtc u32]
      uint16_t want = n >= 3 ? _get16(&req[1]) : CRSF_SD_MAX_PAYLOAD;
      _s.maxPayload = min<uint16_t>(want ? want : CRSF_SD_MAX_PAYLOAD,
                                    CRSF_SD_MAX_PAYLOAD);
      if (!_s.helloSeen) _s.freeBytes = _freeCardBytes();
      _s.helloSeen = true;
      *out++ = CRSF_SD_PROTO_VERSION;
      out = _put16(out, _s.maxPayload);
      out = _put32(out, _s.freeBytes / 1024);
      *out++ = 0;
      out = _put32(out, (uint32_t)g_rtcTime);
      break;
    }

    case OP_STAT: {
      if ((st = _buildPath(req, n, path)) != ST_OK) break;
      FILINFO fno;
      FRESULT r = f_stat(path, &fno);
      if ((st = _fsStatus(r)) != ST_OK) break;
      out = _put32(out, fno.fsize);
      out = _put32(out, ((uint32_t)fno.fdate << 16) | fno.ftime);
      *out++ = (fno.fattrib & AM_DIR) ? 1 : 0;
      break;
    }

    case OP_LIST: {
      // [cookie u16][path] -> [next cookie u16, 0xFFFF = done]
      //   then entries: [flags][size u32][namelen][name]
      if (n < 2) { st = ST_BAD_REQUEST; break; }
      uint16_t cookie = _get16(req);
      if ((st = _buildPath(req + 2, n - 2, path)) != ST_OK) break;
      DIR dir;
      FRESULT r = f_opendir(&dir, path);
      if ((st = _fsStatus(r)) != ST_OK) break;

      uint8_t* next = out;
      out += 2;
      uint8_t* end = &_tx[BULK_HDR + 1 + _s.maxPayload - 1];
      uint16_t idx = 0;
      uint16_t nextCookie = 0xFFFF;
      FILINFO fno;
      while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0]) {
        if (idx++ < cookie) continue;
        uint8_t nl = strlen(fno.fname);
        if (nl > SD_MAX_REL_PATH) continue;
        if (out + 6 + nl > end) {
          nextCookie = idx - 1;
          break;
        }
        *out++ = (fno.fattrib & AM_DIR) ? 1 : 0;
        out = _put32(out, fno.fsize);
        *out++ = nl;
        memcpy(out, fno.fname, nl);
        out += nl;
      }
      f_closedir(&dir);
      _put16(next, nextCookie);
      break;
    }

    case OP_OPEN: {
      // [mode][path] -> [handle][size u32]
      if (n < 1) { st = ST_BAD_REQUEST; break; }
      uint8_t mode = req[0];
      if ((st = _buildPath(req + 1, n - 1, path)) != ST_OK) break;
      int8_t idx = -1;
      for (uint8_t i = 0; i < SD_MAX_HANDLES; i++)
        if (!_s.h[i].open) { idx = i; break; }
      if (idx < 0) { st = ST_BUSY; break; }
      BYTE fa = mode == 0   ? (FA_READ | FA_OPEN_EXISTING)
                : mode == 1 ? (FA_READ | FA_WRITE | FA_OPEN_ALWAYS)
                            : (FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
      FRESULT r = f_open(&_s.h[idx].fil, path, fa);
      if ((st = _fsStatus(r)) != ST_OK) break;
      _s.h[idx].open = true;
      *out++ = idx;
      out = _put32(out, f_size(&_s.h[idx].fil));
      break;
    }

    case OP_READ: {
      // [handle][offset u32][len u16] -> [data]
      if (n < 7) { st = ST_BAD_REQUEST; break; }
      SdHandle* h = _handle(req[0]);
      if (!h) { st = ST_BAD_HANDLE; break; }
      uint32_t off = _get32(&req[1]);
      uint16_t len = min<uint16_t>(_get16(&req[5]), _s.maxPayload - 1);
      if (off >= f_size(&h->fil)) { st = ST_EOF; break; }
      UINT got = 0;
      FRESULT r = f_lseek(&h->fil, off);
      if (r == FR_OK) r = f_read(&h->fil, out, len, &got);
      if ((st = _fsStatus(r)) != ST_OK) break;
      out += got;
      break;
    }

    case OP_WRITE: {
      // [handle][offset u32][data] -> [written u16]
      if (n < 5) { st = ST_BAD_REQUEST; break; }
      SdHandle* h = _handle(req[0]);
      if (!h) { st = ST_BAD_HANDLE; break; }
      uint32_t off = _get32(&req[1]);
      uint16_t len = n - 5;
      uint32_t grow = off + len > f_size(&h->fil) ? off + len - f_size(&h->fil) : 0;
      if (grow && _s.freeBytes < SD_FREE_FLOOR + grow) { st = ST_NO_SPACE; break; }
      UINT put = 0;
      FRESULT r = f_lseek(&h->fil, off);
      if (r == FR_OK) r = f_write(&h->fil, &req[5], len, &put);
      if ((st = _fsStatus(r)) != ST_OK) break;
      if (put < len) st = ST_NO_SPACE;
      _s.freeBytes -= min(_s.freeBytes, grow);
      out = _put16(out, put);
      break;
    }

    case OP_CLOSE: {
      if (n < 1) { st = ST_BAD_REQUEST; break; }
      SdHandle* h = _handle(req[0]);
      if (!h) { st = ST_BAD_HANDLE; break; }
      st = _fsStatus(f_close(&h->fil));
      h->open = false;
      break;
    }

    case OP_RENAME: {
      // [oldlen][old][new]
      if (n < 1 || req[0] + 1u > n) { st = ST_BAD_REQUEST; break; }
      char to[sizeof(path)];
      if ((st = _buildPath(req + 1, req[0], path)) != ST_OK) break;
      if ((st = _buildPath(req + 1 + req[0], n - 1 - req[0], to)) != ST_OK) break;
      FILINFO fno;
      if (f_stat(to, &fno) == FR_OK && !(fno.fattrib & AM_DIR)) f_unlink(to);
      st = _fsStatus(f_rename(path, to));
      break;
    }

    case OP_DELETE: {
      if ((st = _buildPath(req, n, path)) != ST_OK) break;
      if (!strcmp(path, SD_ROOT)) { st = ST_DENIED; break; }
      st = _fsStatus(f_unlink(path));
      break;
    }

    case OP_MKDIR: {
      // creates parents
      if ((st = _buildPath(req, n, path)) != ST_OK) break;
      for (char* p = path + 1;; p++) {
        if (*p == '/' || *p == '\0') {
          char c = *p;
          *p = '\0';
          FRESULT r = f_mkdir(path);
          *p = c;
          if (r != FR_OK && r != FR_EXIST) { st = _fsStatus(r); break; }
          if (!c) break;
        }
      }
      break;
    }

    case OP_PROGRESS: {
      // [stage][done u32][total u32][text]
      if (n < 9) { st = ST_BAD_REQUEST; break; }
      char text[33];
      uint8_t tl = min<uint16_t>(n - 9, sizeof(text) - 1);
      memcpy(text, &req[9], tl);
      text[tl] = '\0';
      if (ui.progress) ui.progress(text, _get32(&req[1]), _get32(&req[5]));
      break;
    }

    case OP_END: {
      // [result][text]
      uint8_t tl = n > 1 ? min<uint16_t>(n - 1, sizeof(_s.endText) - 1) : 0;
      memcpy(_s.endText, &req[1], tl);
      _s.endText[tl] = '\0';
      _s.endOk = n >= 1 && req[0] == 0 && !_s.cancelled;
      _s.ended = true;
      break;
    }

    default:
      st = ST_BAD_REQUEST;
      break;
  }

  return out - &_tx[BULK_HDR + 1];
}

static void _send(uint16_t total)
{
  if (_s.drv->waitForTxCompleted) _s.drv->waitForTxCompleted(_s.ctx);
  _s.drv->sendBuffer(_s.ctx, _tx, total);
}

static void _respond(uint8_t type, uint8_t seq, const CrsfSdUi& ui)
{
  // A retried request (same type and seq) gets the cached answer: a WRITE
  // must not be applied twice.
  if (_s.txLen && type == _s.lastType && seq == _s.lastSeq) {
    _send(_s.txLen);
    return;
  }

  uint16_t len = _get16(&_rx[1]);
  uint8_t st;
  uint16_t n = _dispatch(type, &_rx[BULK_HDR], len - 2, st, ui);

  uint16_t flen = 2 + 1 + n;  // type, seq, status, payload
  _tx[0] = BULK_SYNC;
  _put16(&_tx[1], flen);
  _tx[3] = type | BULK_RESP;
  _tx[4] = seq;
  _tx[5] = st;
  _put16(&_tx[3 + flen], _crc16(&_tx[3], flen));

  _s.lastType = type;
  _s.lastSeq = seq;
  _s.txLen = 3 + flen + 2;
  _send(_s.txLen);
}

// Feed bytes into the frame parser; returns true when _rx holds a request
static bool _parse(uint8_t b)
{
  if (_s.rxLen == 0 && b != BULK_SYNC) return false;
  _rx[_s.rxLen++] = b;
  if (_s.rxLen < 3) return false;

  uint16_t flen = _get16(&_rx[1]);
  if (flen < 2 || flen > _s.maxPayload + 2) {
    _s.rxLen = 0;
    return false;
  }
  if (_s.rxLen < 3u + flen + 2) return false;

  _s.rxLen = 0;
  if (_get16(&_rx[3 + flen]) != _crc16(&_rx[3], flen)) {
    TRACE("[SD] crc error");
    return false;
  }
  // our own echo on a half-duplex line, or garbage
  return !(_rx[3] & BULK_RESP);
}

const char* crsfSdRunSession(uint8_t module, const CrsfSdUi& ui, bool& ok)
{
  ok = false;
  auto mod = pulsesGetModuleDriver(module);
  if (!mod || mod->drv != &CrossfireDriver || !mod->ctx) return "Module not ready";
  auto mod_st = (etx_module_state_t*)mod->ctx;

  memclear(&_s, sizeof(_s));
  _s.drv = modulePortGetSerialDrv(mod_st->rx);
  _s.ctx = modulePortGetCtx(mod_st->rx);
  _s.maxPayload = CRSF_SD_MAX_PAYLOAD;
  if (!_s.drv || !_s.ctx || !_s.drv->copyRxBuffer) return "Module port busy";

  // Keep the port and its config; just stop CRSF from using it. The module
  // stays powered (pulsesStopModule would cut it).
  mixerTaskStop();
  _portOwned[module] = true;
  if (_s.drv->clearRxBuffer) _s.drv->clearRxBuffer(_s.ctx);

  f_mkdir(SD_ROOT);

  const char* result = nullptr;
  uint32_t lastRx = time_get_ms();
  uint32_t cancelAt = 0;
  uint8_t chunk[64];

  while (!_s.ended) {
    watchdogSuspend(100 /*1s*/);

    int got = _s.drv->copyRxBuffer(_s.ctx, chunk, sizeof(chunk));
    for (int i = 0; i < got && !_s.ended; i++) {
      if (_parse(chunk[i])) {
        lastRx = time_get_ms();
        _respond(_rx[3], _rx[4], ui);
      }
    }

    uint32_t now = time_get_ms();
    if (!_s.cancelled && ui.cancelled && ui.cancelled()) {
      _s.cancelled = true;
      cancelAt = now;
    }
    if (_s.cancelled && now - cancelAt > CANCEL_GRACE_MS) {
      result = "Cancelled";
      break;
    }
    if (now - lastRx > (_s.helloSeen ? IDLE_TIMEOUT_MS : HELLO_TIMEOUT_MS)) {
      result = _s.helloSeen ? "Module stopped answering" : "No answer from module";
      break;
    }
    // never sleep mid-frame: at 3.75 Mbaud a 512 B request laps a 512 B
    // RX ring in 1.4 ms
    if (got <= 0 && _s.rxLen == 0) sleep_ms(1);
  }

  // Let the last response (END's, usually) fully leave before channels start
  // again: waitForTxCompleted alone can return before a DMA send has begun.
  if (_s.drv->waitForTxCompleted) _s.drv->waitForTxCompleted(_s.ctx);
  sleep_ms(5);
  _closeAll();

  if (_s.drv->clearRxBuffer) _s.drv->clearRxBuffer(_s.ctx);
  _portOwned[module] = false;
  _state = SD_IDLE;
  // resend the model ID once channels flow again
  moduleState[module].counter = CRSF_FRAME_MODELID;
  _alarmQuietUntil = time_get_ms() + SD_ALARM_QUIET_MS;
  mixerTaskStart();

  if (_s.ended) {
    ok = _s.endOk;
    if (_s.cancelled) return "Cancelled";
    return _s.endText[0] ? _s.endText : (ok ? "Done" : "Failed");
  }
  return result;
}

__attribute__((weak)) void crsfSdNotify(const char* title, const char* text)
{
  POPUP_WARNING(title, text);
}

// Fallback for UI flavors without a session screen: no progress, EXIT cancels
__attribute__((weak)) void crsfSdRunWithUi(uint8_t module, uint8_t kind)
{
  CrsfSdUi ui;
  ui.cancelled = []() { return keysGetState(KEY_EXIT); };
  bool ok;
  const char* msg = crsfSdRunSession(module, ui, ok);
  if (ok) POPUP_INFORMATION(msg);
  else POPUP_WARNING("Module SD session", msg);
}

void crsfSdRunDue()
{
  crsfSdUiDue = false;
  switch (_state) {
    case SD_REJECTED:
      _state = SD_IDLE;
      crsfSdNotify("Module refused", _rejectText);
      break;

    case SD_ACCEPTED:
      _state = SD_RUNNING;
      crsfSdRunWithUi(_module, _kind);
      break;

    default:
      break;
  }
}
