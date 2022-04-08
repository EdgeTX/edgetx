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

#include "opentx.h"
#include "trainer.h"

// Setup SBUS AUX serial input
void sbusSetAuxGetByte(void* ctx, bool (*fct)(void*, uint8_t*));

// SBUS AUX serial getter:
//  if set, it will fetch data from the handler set
//  with sbusSetAuxGetByte()
bool sbusAuxGetByte(uint8_t* byte);

//// Setup general SBUS input source
void sbusSetGetByte(bool (*fct)(uint8_t*));

void processSbusInput();
void sbusTrainerPauseCheck();

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic error "-Wswitch" // unfortunately the project uses -Wnoswitch
#endif

namespace  SBus {
    static constexpr uint32_t baudrate{100000};

    template<uint8_t Instance>
    struct Servo {
        using SBus = Trainer::Protocol::SBus;
        using MesgType = SBus::MesgType;
        
        enum class State : uint8_t {Undefined, Data, GotEnd, WaitEnd};

        static constexpr uint8_t mPauseCount{2}; // 2ms

        static inline void tick1ms() {
            if (mPauseCounter > 0) {
                --mPauseCounter;
            }
            else {
                mState = State::Undefined;
            }
        }

        static inline int16_t convertSbusToPuls(uint16_t const sbusValue) {
            const int16_t centered = sbusValue - SBus::CenterValue;
            return Trainer::clamp((centered * 5) / 8);
        }
        
        static inline void process(const uint8_t b, const std::function<void()> f) {
            ++mBytesCounter;
            mPauseCounter = mPauseCount;
            switch(mState) { // enum-switch -> no default (intentional)
            case State::Undefined:
                if (b == SBus::EndByte) {
                    mState = State::GotEnd;
                }
                else if (b == SBus::StartByte) {
                    mState = State::Data;
                    mIndex = 0;
                }
                break;
            case State::GotEnd:
                if (b == SBus::StartByte) {
                    mState = State::Data;
                    mIndex = 0;
                }
                else if (b == SBus::EndByte) {
                    mState = State::GotEnd;
                }
                else {
                    mState = State::Undefined;
                }
                break;
            case State::Data:
                mData[mIndex] = b;
                if (mIndex >= (mData.size() - 1)) { // got last byte
                    mState = State::WaitEnd;
                }
                else {
                    ++mIndex;
                }
                break;
            case State::WaitEnd:
                if (b == SBus::EndByte) {
                    mState = State::GotEnd;
                    uint8_t& statusByte = mData[mData.size() - 1]; // last byte
                    if (!((statusByte & SBus::FrameLostMask) || (statusByte & SBus::FailSafeMask))) {
                        f();
                        ++mPackagesCounter;
                    }
                }
                else {
                    mState = State::Undefined;
                }
                break;
            }
        } 

        template<uint8_t N>
        static inline void convert(int16_t (&pulses)[N]) {
            static_assert(N >= 16, "array too small");
            pulses[0]  = (uint16_t) (((mData[0]    | mData[1] << 8))                 & SBus::ValueMask);
            pulses[1]  = (uint16_t) ((mData[1]>>3  | mData[2] <<5)                   & SBus::ValueMask);
            pulses[2]  = (uint16_t) ((mData[2]>>6  | mData[3] <<2 | mData[4]<<10)  	 & SBus::ValueMask);
            pulses[3]  = (uint16_t) ((mData[4]>>1  | mData[5] <<7)                   & SBus::ValueMask);
            pulses[4]  = (uint16_t) ((mData[5]>>4  | mData[6] <<4)                   & SBus::ValueMask);
            pulses[5]  = (uint16_t) ((mData[6]>>7  | mData[7] <<1 | mData[8]<<9)   	 & SBus::ValueMask);
            pulses[6]  = (uint16_t) ((mData[8]>>2  | mData[9] <<6)                   & SBus::ValueMask);
            pulses[7]  = (uint16_t) ((mData[9]>>5  | mData[10]<<3)                   & SBus::ValueMask);
            pulses[8]  = (uint16_t) ((mData[11]    | mData[12]<<8)                   & SBus::ValueMask);
            pulses[9]  = (uint16_t) ((mData[12]>>3 | mData[13]<<5)                   & SBus::ValueMask);
            pulses[10] = (uint16_t) ((mData[13]>>6 | mData[14]<<2 | mData[15]<<10) 	 & SBus::ValueMask);
            pulses[11] = (uint16_t) ((mData[15]>>1 | mData[16]<<7)                   & SBus::ValueMask);
            pulses[12] = (uint16_t) ((mData[16]>>4 | mData[17]<<4)                   & SBus::ValueMask);
            pulses[13] = (uint16_t) ((mData[17]>>7 | mData[18]<<1 | mData[19]<<9)  	 & SBus::ValueMask);
            pulses[14] = (uint16_t) ((mData[19]>>2 | mData[20]<<6)                   & SBus::ValueMask);
            pulses[15] = (uint16_t) ((mData[20]>>5 | mData[21]<<3)                   & SBus::ValueMask);
            
            for(size_t i = 0; i < N; ++i) {
                pulses[i] = convertSbusToPuls(pulses[i]);
            }
        }
        static inline uint16_t packages() {
            return mPackagesCounter;
        }
        static inline uint16_t getBytes() {
            return mBytesCounter;
        }
    private:
        static State mState;
        static MesgType mData; 
        static uint8_t mIndex;
        static uint16_t mPackagesCounter;
        static uint16_t mBytesCounter;
        static uint8_t mPauseCounter;
    };
}

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
