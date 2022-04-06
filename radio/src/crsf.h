#pragma once

#include "opentx.h"
#include "trainer.h"
#include "crc.h"

#include <algorithm>
#include <limits>

void processCrsfInput();
void crsfTrainerPauseCheck();

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic error "-Wswitch" // unfortunately the project uses -Wnoswitch
#endif

namespace CRSF {
    static constexpr uint32_t baudrate{420000};
    
    template<uint8_t Instance>
    struct Servo {
        using Crsf = Trainer::Protocol::Crsf;
        using MesgType = Crsf::MesgType;
        
        static constexpr uint8_t mPauseCount{2}; // 2ms
        
        static constexpr uint8_t CRSFChannels{16};
        
        enum class State : uint8_t {Undefined, GotFCAddress, GotLength, Channels, Data, AwaitCRC, AwaitCRCAndDecode};
        
        static inline int16_t convertCrsfToPuls(uint16_t const value) {
            const int16_t centered = value - Crsf::CenterValue;
            return Trainer::clamp((centered * 5) / 8);
        }
        
        static inline void tick1ms() {
            if (mPauseCounter > 0) {
                --mPauseCounter;
            }
            else {
                mState = State::Undefined;
            }
        }
        
        static inline void process(const uint8_t b, const std::function<void()> f) {
            mPauseCounter = mPauseCount;
            switch(mState) { // enum-switch -> no default (intentional)
            case State::Undefined:
                csum.reset();
                if (b == Crsf::FcAddress) {
                    mState = State::GotFCAddress;
                }
                break;
            case State::GotFCAddress:
                if ((b > 2) && (b <= mData.size())) {
                    mLength = b - 2; // only payload (not including type and crc)
                    mIndex = 0;
                    mState = State::GotLength;
                }
                else {
                    mState = State::Undefined;
                }
                break;
            case State::GotLength:
                csum += b;
                if ((b == Crsf::FrametypeChannels) && (mLength == 22)) {
                    mState = State::Channels;
                }
                else {
                    mState = State::Data;
                }
                break;
            case State::Data:
                csum += b;
                if (++mIndex >= mLength) {
                    mState = State::AwaitCRC;
                }
                break;
            case State::Channels:
                csum += b;
                mData[mIndex] = b;
                if (++mIndex >= mLength) {
                    mState = State::AwaitCRCAndDecode;
                }
                break;
            case State::AwaitCRC:
                if (csum == b) {
                    // only channel data is decoded, nothing todo here
                } 
                mState = State::Undefined;
                break;
            case State::AwaitCRCAndDecode:
                if (csum == b) {
                    ++mPackages;
                    f();
                } 
                mState = State::Undefined;
                break;
            }            
        }        
        template<uint8_t N>
        static inline void convert(int16_t (&pulses)[N]) {
            static_assert(N >= 16, "array too small");
            pulses[0]  = (uint16_t) (((mData[0]    | mData[1] << 8))                 & Crsf::ValueMask);
            pulses[1]  = (uint16_t) ((mData[1]>>3  | mData[2] <<5)                   & Crsf::ValueMask);
            pulses[2]  = (uint16_t) ((mData[2]>>6  | mData[3] <<2 | mData[4]<<10)  	 & Crsf::ValueMask);
            pulses[3]  = (uint16_t) ((mData[4]>>1  | mData[5] <<7)                   & Crsf::ValueMask);
            pulses[4]  = (uint16_t) ((mData[5]>>4  | mData[6] <<4)                   & Crsf::ValueMask);
            pulses[5]  = (uint16_t) ((mData[6]>>7  | mData[7] <<1 | mData[8]<<9)   	 & Crsf::ValueMask);
            pulses[6]  = (uint16_t) ((mData[8]>>2  | mData[9] <<6)                   & Crsf::ValueMask);
            pulses[7]  = (uint16_t) ((mData[9]>>5  | mData[10]<<3)                   & Crsf::ValueMask);
            pulses[8]  = (uint16_t) ((mData[11]    | mData[12]<<8)                   & Crsf::ValueMask);
            pulses[9]  = (uint16_t) ((mData[12]>>3 | mData[13]<<5)                   & Crsf::ValueMask);
            pulses[10] = (uint16_t) ((mData[13]>>6 | mData[14]<<2 | mData[15]<<10) 	 & Crsf::ValueMask);
            pulses[11] = (uint16_t) ((mData[15]>>1 | mData[16]<<7)                   & Crsf::ValueMask);
            pulses[12] = (uint16_t) ((mData[16]>>4 | mData[17]<<4)                   & Crsf::ValueMask);
            pulses[13] = (uint16_t) ((mData[17]>>7 | mData[18]<<1 | mData[19]<<9)  	 & Crsf::ValueMask);
            pulses[14] = (uint16_t) ((mData[19]>>2 | mData[20]<<6)                   & Crsf::ValueMask);
            pulses[15] = (uint16_t) ((mData[20]>>5 | mData[21]<<3)                   & Crsf::ValueMask);

            for(size_t i = 0; i < N; ++i) {
                pulses[i] = convertCrsfToPuls(pulses[i]);
            }
        }
        static inline uint16_t packages() {
            return mPackages;
        }
    private:
        static CRC8 csum;
        static State mState;
        static MesgType mData; 
        static uint8_t mIndex;
        static uint8_t mLength;
        static uint16_t mPackages;
        static uint8_t mPauseCounter;
    };
}
#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
