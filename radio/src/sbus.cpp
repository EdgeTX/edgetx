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

#include "opentx.h"
#include "sbus.h"
#include "timers_driver.h"

#define SBUS_FRAME_GAP_DELAY   1000 // 500uS

//#define SBUS_START_BYTE        0x0F
//#define SBUS_END_BYTE          0x00
//#define SBUS_FLAGS_IDX         23
#define SBUS_FRAMELOST_BIT     2
#define SBUS_FAILSAFE_BIT      3

//#define SBUS_CH_BITS           11
//#define SBUS_CH_MASK           ((1<<SBUS_CH_BITS) - 1)

#define SBUS_CH_CENTER         0x3E0

static bool (*_sbusAuxGetByte)(void*, uint8_t*) = nullptr;
static void* _sbusAuxGetByteCtx = nullptr;

void sbusSetAuxGetByte(void* ctx, bool (*fct)(void*, uint8_t*))
{
  _sbusAuxGetByte = nullptr;
  _sbusAuxGetByteCtx = ctx;
  _sbusAuxGetByte = fct;
}

bool sbusAuxGetByte(uint8_t* byte)
{
  auto _getByte = _sbusAuxGetByte;
  auto _ctx = _sbusAuxGetByteCtx;

  if (_getByte) {
    return _getByte(_ctx, byte);
  }

  return false;
}

static bool (*sbusGetByte)(uint8_t*) = nullptr;

void sbusSetGetByte(bool (*fct)(uint8_t*))
{
  sbusGetByte = fct;
}

namespace  SBus {
    struct Servo {
        using SBus = Trainer::Protocol::SBus;
        using MesgType = SBus::MesgType;
        static_assert(std::tuple_size<MesgType>::value == (SBUS_FRAME_SIZE - 2), "consistency check");        
        
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
                        ++mPackages;
                    }
                }
                else {
                    mState = State::Undefined;
                }
                break;
            }
        } 
        static inline void convert(int16_t* const pulses) {
            static_assert(MAX_TRAINER_CHANNELS == 16);
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
            
            for(size_t i = 0; i < MAX_TRAINER_CHANNELS; ++i) {
                pulses[i] = convertSbusToPuls(pulses[i]);
            }
        }
    private:
        static State mState;
        static MesgType mData; 
        static uint8_t mIndex;
        static uint16_t mPackages;
        static uint8_t mPauseCounter;
    };
    Servo::State Servo::mState{Servo::State::Undefined};
    Servo::MesgType Servo::mData; 
    uint8_t Servo::mIndex{0};
    uint16_t Servo::mPackages{0};
    uint8_t Servo::mPauseCounter{Servo::mPauseCount}; // 2 ms
}

void sbusTrainerPauseCheck() {
#if !defined(SIMU)
# if defined(AUX_SERIAL) || defined(AUX2_SERIAL) || defined(TRAINER_MODULE_SBUS)
    if (hasSerialMode(UART_MODE_SBUS_TRAINER) >= 0) {
        SBus::Servo::tick1ms();
        processSbusInput();    
    }
# endif
#endif    
}

//void processSbusInput()
//{

//  // TODO: place this outside of the function
//  static uint8_t SbusIndex = 0;
//  static uint16_t SbusTimer;
//  static uint8_t SbusFrame[SBUS_FRAME_SIZE];

//  uint32_t active = 0;

//  // Drain input first (if existing)
//  uint8_t rxchar;
//  auto _getByte = sbusGetByte;
//  while (_getByte && (_getByte(&rxchar) > 0)) {
//    active = 1;
//    if (SbusIndex > SBUS_FRAME_SIZE - 1) {
//      SbusIndex = SBUS_FRAME_SIZE - 1;
//    }
//    SbusFrame[SbusIndex++] = rxchar;
//  }

//  // Data has been received
//  if (active) {
//    SbusTimer = getTmr2MHz();
//    return;
//  }

//  // Check if end-of-frame is detected
//  if (SbusIndex) {
//    if ((uint16_t)(getTmr2MHz() - SbusTimer) > SBUS_FRAME_GAP_DELAY) {
//      processSbusFrame(SbusFrame, ppmInput, SbusIndex);
//      SbusIndex = 0;
//    }
//  }

void processSbusInput() {
#if !defined(SIMU)
  uint8_t rxchar;
  
  while (sbusAuxGetByte(&rxchar)) {
      SBus::Servo::process(rxchar, [&](){
          SBus::Servo::convert(ppmInput);
          ppmInputValidityTimer = PPM_IN_VALID_TIMEOUT;        
      });
  }
#endif    
}

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
