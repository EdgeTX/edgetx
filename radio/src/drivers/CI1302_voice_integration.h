/*
 * Copyright (C) EdgeTX
 *
 * CI1302 voice control hooks for shared EdgeTX code (VOICE_CONTROL_SENSOR).
 * Implementation lives in drivers/CI1302.cpp (CI1302_SENSOR targets).
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined(VOICE_CONTROL_SENSOR)

#include "edgetx_constants.h"
#include "edgetx_types.h"
#include "hal/switch_driver.h"

#define VOICE_SWITCH_COUNT 2

// --- shared-code integration hooks ---

void CI1302_voiceIntegrationPer10ms();
void CI1302_voiceIntegrationOnFlightReset();

bool CI1302_voiceIntegrationMixSrcValue(mixsrc_t i, getvalue_t* val);
const char* CI1302_voiceIntegrationMixSrcName(mixsrc_t idx);

bool CI1302_voiceIntegrationMixSrcParseYaml(const char* val, uint8_t val_len, uint32_t* out);
bool CI1302_voiceIntegrationMixSrcWriteYaml(uint32_t val, const char** str);

bool CI1302_voiceIntegrationSwitchSrcParseYaml(const char* val, uint8_t val_len, int32_t* ival);

bool CI1302_voiceIntegrationModelGetSwitchType(uint8_t n, SwitchConfig* out);
bool CI1302_voiceIntegrationModelSetSwitchType(uint8_t n);

bool CI1302_voiceIntegrationIsSwitchAvailable(int swtch, int context, bool* available);
bool CI1302_voiceIntegrationIsSwitchSwitchAvailable(int swtch, bool* available);

// --- virtual voice switches (VGR / VFL) ---

uint8_t CI1302_voiceSwitchExtraCount();

bool CI1302_voiceSwitchTryGetPosition(uint8_t idx, SwitchHwPos* pos);
const char* CI1302_voiceSwitchTryGetName(uint8_t idx);
int8_t CI1302_voiceSwitchTryGetIndexFromName(const char* name);
bool CI1302_voiceSwitchTryGetHwType(uint8_t idx, SwitchHwType* type);

bool CI1302_voiceSwitchIsIndex(uint8_t idx);
bool CI1302_voiceSwitchIsPositionAvailable(uint8_t idx, uint8_t position);

bool CI1302_voiceSwitchTryIsAvailable(int swtch, int context, bool* available);
bool CI1302_voiceSwitchTryParseYamlSrc(const char* val, uint8_t val_len, int32_t* ival);

void CI1302_voiceSwitchInitBootGuard();
bool CI1302_voiceSwitchBootGuardActive();
bool CI1302_voiceSwitchSuppressMovedAudio(uint8_t idx, bool startup);

bool CI1302_voiceSwitchIsSwitchSource(swsrc_t swtch);
bool CI1302_voiceSwitchShouldSuppressFunctionAudio(swsrc_t swtch, bool risingEdge);

// --- motion control (mixer hook) ---

void CI1302_voiceMotionControlApplyToInputs(uint8_t pots_offset);

#endif
