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

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include <string>

#if __wasm__
#define WASM_EXPORT_AS(name) __attribute__((export_name(name)))
#define WASM_EXPORT(symbol) WASM_EXPORT_AS(#symbol) symbol
#define WASM_IMPORT_AS(name) __attribute__((import_name(name)))
#define WASM_IMPORT(symbol) WASM_IMPORT_AS(#symbol) symbol
#else
#define WASM_EXPORT_AS(name)
#define WASM_EXPORT(symbol) symbol
#define WASM_IMPORT_AS(name)
#define WASM_IMPORT(symbol) symbol
#endif

// -- WASM exports (called by host) --

// Lifecycle: call simuInit() once, then simuFatfsSetPaths() + simuStart().
// Poll simuIsRunning() periodically. Call simuStop() to shut down.
void WASM_EXPORT(simuInit)();
void WASM_EXPORT(simuStart)(bool tests = true);
void WASM_EXPORT(simuStop)();
bool WASM_EXPORT(simuIsRunning)();

// Set SD card and settings paths before simuStart() to avoid STORAGE WARNING.
void WASM_EXPORT(simuFatfsSetPaths)(const char * sdPath, const char * settingsPath);

// Input: keys use Board::Keys enum, switches use Board switch indices,
// trims use Board::TrimSwitches enum (momentary press, not value).
void WASM_EXPORT(simuSetKey)(uint8_t key, bool state);
void WASM_EXPORT(simuSetTrim)(uint8_t trim, bool state);
void WASM_EXPORT(simuSetSwitch)(uint8_t swtch, int8_t state);

// Touch: call simuTouchDown(x,y) on press and repeatedly during drag
// (continuous position updates), then simuTouchUp() on release.
// The firmware detects slides from successive position changes.
void WASM_EXPORT(simuTouchDown)(int16_t x, int16_t y);
void WASM_EXPORT(simuTouchUp)();

// LCD: the firmware calls simuLcdNotify() (import) when a new frame is ready.
// The host waits for this notification, then calls simuLcdCopy() to read the
// framebuffer and simuLcdFlushed() to signal that the buffer can be reused.
// simuLcdChanged() is kept for backward compatibility (Companion polling).
// Depth is bits per pixel (1, 4, or 16).
bool     WASM_EXPORT(simuLcdChanged)();
uint32_t WASM_EXPORT(simuLcdCopy)(uint8_t* buf, uint32_t maxLen);
uint32_t WASM_EXPORT(simuLcdGetWidth)();
uint32_t WASM_EXPORT(simuLcdGetHeight)();
uint32_t WASM_EXPORT(simuLcdGetDepth)();

// Rotary encoder: positive steps = clockwise. The firmware handles
// mode inversion and key translation internally.
void WASM_EXPORT(simuRotaryEncoderEvent)(int32_t steps);

// Capabilities: cap values match SimulatorInterface::Capability enum
// (0=LUA, 1=ROTARY_ENC, 2=ROTARY_ENC_NAV, 3=TELEM_FRSKY_SPORT,
//  4=SERIAL_AUX1, 5=SERIAL_AUX2). Returns 0 or 1.
int32_t WASM_EXPORT(simuGetCapability)(uint8_t cap);

int WASM_EXPORT(simuAudioGetVolume)();

// Backlight state: returns true if backlight is currently enabled.
bool WASM_EXPORT(simuGetBacklightState)();

// Custom (function) switches: LED state and RGB color.
// idx is the custom switch index (0-based), not the global switch index.
// Use simuGetNumCustomSwitches() to get the count.
uint8_t  WASM_EXPORT(simuGetNumCustomSwitches)();
uint8_t  WASM_EXPORT(simuGetCustomSwitchIndex)(uint8_t cfsIdx);
bool     WASM_EXPORT(simuGetCustomSwitchState)(uint8_t idx);
uint32_t WASM_EXPORT(simuGetCustomSwitchColor)(uint8_t idx);

// Value-based trim setting: idx is mapped through inputMappingConvertMode.
void WASM_EXPORT(simuSetTrimValue)(uint8_t idx, int32_t value);

// Telemetry injection: protocol matches SIMU_TELEMETRY_PROTOCOL_* enum.
// module: 0=internal, 1=external.
void WASM_EXPORT(simuSendTelemetry)(uint8_t module, uint8_t protocol,
                                     const uint8_t* data, uint32_t len);

// Lua: trigger reload of permanent scripts.
void WASM_EXPORT(simuLuaReloadPermanentScripts)();

// LCD: notify firmware that host has consumed the LCD buffer.
void WASM_EXPORT(simuLcdFlushed)();

// Trainer input: bulk copy values (-512..512) into trainerInput[].
// Returns the max number of channels.
uint8_t WASM_EXPORT(simuGetMaxTrainerChannels)();
void WASM_EXPORT(simuCopyTrainerInput)(const int16_t* buf, uint8_t count);
void WASM_EXPORT(simuSetTrainerTimeout)(uint16_t ms);

// Output values: polled periodically by host (every ~50ms).

// Bulk copy channel outputs into buf (int16_t[]). Returns channel count.
uint8_t  WASM_EXPORT(simuGetNumChannels)();
uint8_t  WASM_EXPORT(simuCopyChannelOutputs)(int16_t* buf, uint8_t maxCount);
uint8_t  WASM_EXPORT(simuCopyMixOutputs)(int16_t* buf, uint8_t maxCount);

// Channel/mixer queries.
bool     WASM_EXPORT(simuIsChannelUsed)(uint8_t channel);
int      WASM_EXPORT(simuGetChannelsUsed)();
uint8_t  WASM_EXPORT(simuGetMixCount)();

// Bulk copy logical switch states into buf (uint8_t[], 0 or 1). Returns count.
uint8_t  WASM_EXPORT(simuGetNumLogicalSwitches)();
uint8_t  WASM_EXPORT(simuCopyLogicalSwitches)(uint8_t* buf, uint8_t maxCount);

// Trim values. idx 0..TRIM_AXIS_COUNT-1 (typically 8).
// Returns the trim value for the current flight mode.
int32_t  WASM_EXPORT(simuGetTrimValue)(uint8_t idx);
int16_t  WASM_EXPORT(simuGetTrimRange)();

// Flight mode: returns current flight mode index (0-based).
int32_t  WASM_EXPORT(simuGetFlightMode)();

// GVars: gv 0..N-1, fm 0..M-1. Returns encoded gVarMode_t.
uint8_t  WASM_EXPORT(simuGetNumGVars)();
uint8_t  WASM_EXPORT(simuGetNumFlightModes)();
int32_t  WASM_EXPORT(simuGetGVar)(uint8_t gv, uint8_t fm);

// -- WASM imports (provided by host) --

// simuGetAnalog: return ADC-range value for input at index idx.
// All inputs use 0..4096 range (center=2048), same as SDL simulator.
uint16_t WASM_IMPORT(simuGetAnalog)(uint8_t idx);
void WASM_IMPORT(simuQueueAudio)(const uint8_t* buf, uint32_t len);

// simuTrace: send debug/trace output text to the host for display.
// Called via traceCallback from debugPrintf().
void WASM_IMPORT(simuTrace)(const char* text);

// simuLcdNotify: signal the host that the LCD framebuffer has been updated
// and is ready to be copied.  Called from lcdRefresh() (mono) and the LVGL
// flush callback (color).  On the host side this wakes an Atomics.waitAsync
// listener so the frame can be rendered without polling.
void WASM_IMPORT(simuLcdNotify)();

// First-run helper: request default radio.yml + model creation if
// no settings file exists.  Call before simuStart().  The actual file
// I/O runs inside storageReadAll() on a worker thread.
void WASM_EXPORT(simuCreateDefaults)();

// Flag checked by storageReadAll() to silently create defaults.
extern bool simuCreateDefaultSettings;

// -- Internal (not exported) --
void simuMain();
std::string simuFatfsGetCurrentPath();
std::string simuFatfsGetRealPath(const std::string& p);

#if defined(HARDWARE_TOUCH)
  extern struct TouchState simTouchState;
  extern bool simTouchOccured;
#endif
