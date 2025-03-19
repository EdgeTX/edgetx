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

//void voiceaiSetGetByte(void* crx, int (*cb)(void*, uint8_t*));
//void voiceaiSetPutByte(void* ctx, void (*cb)(void*, uint8_t*));

//int voiceGetByte(uint8_t* byte);

#ifdef __cplusplus
extern "C" {
#endif
    extern "C" void voiceSerialPutc(char c);
    extern "C" int voiceGetByte(uint8_t* byte);
    extern "C" int GetVoiceInput(uint8_t *rxchar);
    extern "C" int dbgGetByte(uint8_t* byte);
    extern "C" void voiceSerialPutstr(uint8_t* byte,uint32_t len);

    extern bool VoiceGearStatus;
    extern uint8_t VoiceFlapStatus;
    extern bool VoicePowerStatus;

    extern bool VoiceRunStatus;
    extern bool MotionControlStatus;

#ifdef __cplusplus
}
#endif

void processUpdataInput(void);
void processVoiceInput(void);
//extern bool VoicePowerStatus;
