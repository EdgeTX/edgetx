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

#ifndef _CI1302_H_
#define _CI1302_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void voiceControlPowerOn(void);
void voiceControlPowerOff(void);
void voiceSerialPutc(char c);
int voiceGetByte(uint8_t* byte);
int GetVoiceInput(uint8_t *rxchar);
int dbgGetByte(uint8_t* byte);
void voiceSerialPutstr(uint8_t* byte, uint32_t len);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

extern bool VoiceGearStatus;
extern uint8_t VoiceFlapStatus;
extern bool VoicePowerStatus;
extern bool VoiceRunStatus;
extern bool MotionControlStatus;

void processUpdataInput(void);
void processVoiceInput(void);
void voiceControlInit(void);

#endif

#endif
