#pragma once

#include "opentx.h"
#include "trainer.h"

#include <algorithm>
#include <limits>

void processIbusInput();

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic error "-Wswitch" // unfortunately the project uses -Wnoswitch
#endif

namespace IBus {
    static constexpr uint32_t baudrate{115200};
    
    struct CheckSum final {
        inline void reset() {
            mSum = std::numeric_limits<uint16_t>::max();
        }
        inline uint8_t operator+=(const uint8_t b) {
            mSum -= b;
            return b;
        }
        inline uint8_t highByte() const {
            return uint8_t(mSum >> 8);
        }
        inline uint8_t lowByte() const {
            return uint8_t(mSum);
        }
        inline void highByte(const uint8_t hb) {
            mH = hb;
        }
        inline void lowByte(const uint8_t lb){
            mL = lb;
        }
        inline explicit operator bool() const {
            return ((mSum & 0xff) == mL) && (((mSum >> 8) & 0xff) == mH);
        }
    private:
        uint8_t mH{};
        uint8_t mL{};
        uint16_t mSum = std::numeric_limits<uint16_t>::max();
    };

    template<uint8_t Instance>    
    struct Servo {
        using IBus = Trainer::Protocol::IBus;
        using MesgType = IBus::MesgType;
        
        enum class State : uint8_t {Undefined, GotStart20, Data, CheckL, CheckH};
        
        static inline int16_t convertIbusToPuls(uint16_t const ibusValue) {
            return Trainer::clamp(ibusValue - IBus::CenterValue);
        }
        
        static inline void process(const uint8_t b, const std::function<void()> f) {
            ++mBytesCounter;
            switch(mState) { // enum-switch -> no default (intentional)
            case State::Undefined:
                csum.reset();
                if (b == IBus::StartByte1) {
                    csum += b;
                    mState = State::GotStart20;
                }
                break;
            case State::GotStart20:
                if (b == IBus::StartByte2) {
                    csum += b;
                    mState = State::Data;
                    mIndex = 0;
                }
                else {
                    mState = State::Undefined;
                }
                break;
            case State::Data:
                ibusFrame[mIndex] = b;
                csum += b;
                if (mIndex >= (ibusFrame.size() - 1)) {
                    mState = State::CheckL;
                }
                else {
                    ++mIndex;
                }
                break;
            case State::CheckL:
                csum.lowByte(b);
                mState = State::CheckH;
                break;
            case State::CheckH:
                csum.highByte(b);
                mState = State::Undefined;
                if (csum) {
                    ++mPackagesCounter;
                    f();
                }
                break;
            }            
        }        
        static inline void convert(int16_t* const pulses) {
            for (size_t chi{0}; chi < MAX_TRAINER_CHANNELS; chi++) {
                if (chi < 14) {
                    const uint8_t h = ibusFrame[2 * chi + 1] & 0x0f;
                    const uint8_t l = ibusFrame[2 * chi];
                    const uint16_t  v = (uint16_t(h) << 8) + uint8_t(l);
                    pulses[chi] = convertIbusToPuls(v);
                }
                else if (chi < 18) {
                    const uint8_t h1 = ibusFrame[6 * (chi - 14) + 1] & 0xf0;
                    const uint8_t h2 = ibusFrame[6 * (chi - 14) + 3] & 0xf0;
                    const uint8_t h3 = ibusFrame[6 * (chi - 14) + 5] & 0xf0;
                    const uint16_t v = (uint8_t(h1) >> 4) + uint8_t(h2) + (uint16_t(h3) << 4);
                    pulses[chi] = convertIbusToPuls(v);
                }
            }
        }
        static inline uint16_t packages() {
            return mPackagesCounter;
        }
        static inline uint16_t getBytes() {
            return mBytesCounter;
        }
    private:
        static CheckSum csum;
        static State mState;
        static MesgType ibusFrame;
        static uint8_t mIndex;
        static uint16_t mPackagesCounter;
        static uint16_t mBytesCounter;
    };
    // inline static member definitions not until c++17
    template<uint8_t Instance>        
    CheckSum Servo<Instance>::csum;
    template<uint8_t Instance>
    typename Servo<Instance>::State Servo<Instance>::mState{Servo::State::Undefined};
    template<uint8_t Instance>
    typename Servo<Instance>::MesgType Servo<Instance>::ibusFrame; 
    template<uint8_t Instance>
    uint8_t Servo<Instance>::mIndex{};
    template<uint8_t Instance>
    uint16_t Servo<Instance>::mPackagesCounter{};
    template<uint8_t Instance>
    uint16_t Servo<Instance>::mBytesCounter{};
}

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
