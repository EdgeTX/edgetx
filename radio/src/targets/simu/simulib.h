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

// exports
void WASM_EXPORT(simuInit)();
void WASM_EXPORT(simuStart)(bool tests = true);
void WASM_EXPORT(simuStop)();
bool WASM_EXPORT(simuIsRunning)();
void WASM_EXPORT(simuSetKey)(uint8_t key, bool state);
void WASM_EXPORT(simuSetTrim)(uint8_t trim, bool state);
void WASM_EXPORT(simuSetSwitch)(uint8_t swtch, int8_t state);
int  WASM_EXPORT(simuAudioGetVolume)();

// callbacks
uint16_t WASM_IMPORT(simuGetAnalog)(uint8_t idx);
void WASM_IMPORT(simuQueueAudio)(const uint8_t* buf, uint32_t len);

void simuMain();

void simuFatfsSetPaths(const char * sdPath, const char * settingsPath);
std::string simuFatfsGetCurrentPath();
std::string simuFatfsGetRealPath(const std::string& p);

#if defined(HARDWARE_TOUCH)
  extern struct TouchState simTouchState;
  extern bool simTouchOccured;
#endif
