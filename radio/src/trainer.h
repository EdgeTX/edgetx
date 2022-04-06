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

#ifndef _TRAINER_H_
#define _TRAINER_H_

#include "dataconstants.h"

namespace Trainer {
    static constexpr int16_t MaxValue = +512;
    static constexpr int16_t MinValue = -512;

    static inline int16_t clamp(int16_t const v) {
        return (v < MinValue) ? MinValue : ((v > MaxValue) ? MaxValue : v);        
    }

    namespace Protocol {
        struct SBus {
            using MesgType = std::array<uint8_t, 23>;
            
            static constexpr uint8_t  ValueBits = 11;
            static constexpr uint16_t ValueMask = ((1 << ValueBits) - 1);
                
            static constexpr uint8_t FrameLostBit = 2;
            static constexpr uint8_t FailSafeBit = 3;
            static constexpr uint8_t StartByte = 0x0f;
            static constexpr uint8_t EndByte = 0x00;
            static constexpr uint8_t FrameLostMask{1 << FrameLostBit};
            static constexpr uint8_t FailSafeMask{1 << FailSafeBit};
            
            static constexpr uint16_t CenterValue = 0x3e0;            
        };
        struct IBus {
            using MesgType = std::array<uint8_t, 28>;  // 0x20, 0x40 , 28 Bytes, checkH, checkL
    
            static constexpr uint8_t StartByte1 = 0x20;
            static constexpr uint8_t StartByte2 = 0x40;
            static constexpr uint16_t MaxValue = 988;            
            static constexpr uint16_t MinValue = 2011;            
            static constexpr uint16_t CenterValue = (MaxValue + MinValue) / 2;            
        };     
        struct Crsf {
//            Every frame has the structure:
//            <Device address><Frame length><Type><Payload><CRC>            
            using MesgType = std::array<uint8_t, 64>;

            static constexpr uint8_t  ValueBits = 11;
            static constexpr uint16_t ValueMask = ((1 << ValueBits) - 1);
                
            static constexpr uint8_t FcAddress = FC_ADDRESS;
            static constexpr uint8_t FrametypeChannels = CHANNELS_ID;
            static constexpr uint8_t FrametypeLink = LINK_ID;

            static constexpr uint16_t CenterValue = 0x3e0;            
            
        };
        struct SumDV3 {
            static constexpr uint8_t start_code = 0xa8;
            static constexpr uint8_t version_code1 = 0x01;
            static constexpr uint8_t version_code3 = 0x03;
    
            static constexpr uint16_t ExtendedLow = 0x1c20; // 7200
            static constexpr uint16_t MinValue = 0x2260; // 8800
            static constexpr uint16_t CenterValue = 0x2ee0; // 12000
            static constexpr uint16_t MaxValue = 0x3b60; // 15200
            static constexpr uint16_t ExtendedHigh = 0x41a0; // 16800

            static constexpr uint8_t MaxChannels = 32;
            
            using MesgType = std::array<std::pair<uint8_t, uint8_t>, MaxChannels>;
            using SwitchesType = uint64_t;
        };
    }    
}

// Trainer input channels
extern int16_t ppmInput[MAX_TRAINER_CHANNELS];

// Timer gets decremented in per10ms()
#define PPM_IN_VALID_TIMEOUT 100 // 1s
extern uint8_t ppmInputValidityTimer;

extern uint8_t currentTrainerMode;
#define IS_TRAINER_INPUT_VALID() (ppmInputValidityTimer != 0)

void checkTrainerSignalWarning();
void checkTrainerSettings();
void stopTrainer();
void forceResetTrainerSettings();

// Needs to be inlined to avoid slow function calls in ISR routines
inline void captureTrainerPulses(uint16_t capture)
{
  static uint16_t lastCapt = 0;
  static int8_t channelNumber = -1;

  uint16_t val = (uint16_t)(capture - lastCapt) / 2;
  lastCapt = capture;

  // We process ppmInput right here to make servo movement as smooth as possible
  //    while under trainee control
  //
  // G: Prioritize reset pulse. (Needed when less than 16 incoming pulses)
  //
  if (val > 4000 && val < 19000) {
    channelNumber = 0; // triggered
  }
  else {
    if (channelNumber >= 0 && channelNumber < MAX_TRAINER_CHANNELS) {
      if (val > 800 && val < 2200) {
        ppmInputValidityTimer = PPM_IN_VALID_TIMEOUT;
        ppmInput[channelNumber++] =
          // +-500 != 512, but close enough.
          (int16_t)(val - 1500) * (g_eeGeneral.PPM_Multiplier+10) / 10;
      }
      else {
        channelNumber = -1; // not triggered
      }
    }
  }
}

#endif // _TRAINER_H_
