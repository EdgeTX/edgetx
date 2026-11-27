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

// CRSF SD bulk session: a CRSF module gets file access to /ELRS/ on the SD
// card over its own UART. The module is the client, the radio a dumb file
// server. See elrs-rx-ota/docs/sd-bulk-session.md for the protocol.

#pragma once

#include <stdint.h>
#include <functional>

// In-band CRSF extended frame (dest, origin, subtype, ...). Provisional id,
// not registered with TBS.
#define CRSF_SD_SESSION_ID 0x3E

enum CrsfSdSubtype : uint8_t {
  CRSF_SD_REQUEST = 0x01,  // module -> radio: please offer me a session
  CRSF_SD_OFFER = 0x02,    // radio -> module
  CRSF_SD_ACCEPT = 0x03,   // module -> radio: switching to bulk now
  CRSF_SD_REJECT = 0x04,   // module -> radio
};

enum CrsfSdKind : uint8_t {
  CRSF_SD_KIND_RX_UPDATE = 0x01,
  CRSF_SD_KIND_IMAGE_SYNC = 0x02,
};

#define CRSF_SD_PROTO_VERSION 1
#define CRSF_SD_MAX_PAYLOAD 512

// telemetry task: a CRSF_SD_SESSION_ID frame arrived from the module
void crsfSdProcessFrame(uint8_t module, const uint8_t* frame, uint8_t len);

// mixer task (pulses): returns the length of an OFFER frame written to
// `frame` if one is due, else 0
uint8_t crsfSdSetupOffer(uint8_t module, uint8_t* frame);

// UI: ask the module for a session of the given kind
bool crsfSdStartOffer(uint8_t module, uint8_t kind);

// telemetry poll: true while the session owns the module port
bool crsfSdPortOwned(uint8_t module);

// telemetry alarms: true during a session and for a few seconds after
bool crsfSdSuppressAlarms();

// Set from the telemetry side when an accepted session or a module's refusal needs
// the UI. perMain() checks only this, and calls crsfSdRunDue() when it's set.
extern volatile bool crsfSdUiDue;
// UI task: shows the refusal, or runs the accepted session to completion (blocking)
void crsfSdRunDue();

struct CrsfSdUi {
  // stage text (from the module), done/total (total 0 = unknown)
  std::function<void(const char* text, uint32_t done, uint32_t total)> progress;
  // polled every loop; true once the user wants out
  std::function<bool()> cancelled;
};

// implemented by each UI flavor; called from crsfSdRunDue()
void crsfSdRunWithUi(uint8_t module, uint8_t kind);
// a notice that must not block the UI task (color radios' POPUP_WARNING is modal)
void crsfSdNotify(const char* title, const char* text);

// the session itself; returns the END message (or an error) for a result popup
const char* crsfSdRunSession(uint8_t module, const CrsfSdUi& ui, bool& ok);
