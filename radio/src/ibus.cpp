#include "ibus.h"
#include "opentx.h"

#include <algorithm>
#include <limits>

//#define IBUS_FRAME_GAP_DELAY   2000 // 1ms

#define IBUS_VALUE_MIN 988
#define IBUS_VALUE_MAX 2011
#define IBUS_VALUE_CENTER 1500

namespace IBus {
    struct CheckSum final {
        inline void reset() {
            mSum = std::numeric_limits<uint16_t>::max();
        }
        inline uint8_t operator+=(const uint8_t b) {
            mSum -= static_cast<uint8_t>(b);
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
    
    uint16_t clamp(uint16_t v, uint16_t lower, uint16_t upper) {
        return (v < lower) ? lower : ((v > upper) ? upper: v);        
    }
    
    int16_t convertIbusToPuls(uint16_t const ibusValue) {
        const uint16_t clamped = clamp(ibusValue, IBUS_VALUE_MIN, IBUS_VALUE_MAX);
        return (clamped - IBUS_VALUE_CENTER);        
    }
    struct Servo {
        enum class State : uint8_t {Undefined, GotStart20, Data, CheckL, CheckH};
        static inline void process(const uint8_t b, std::function<void()> f) {
            switch(mState) {
            case State::Undefined:
                csum.reset();
                if (b == 0x20) {
                    csum += b;
                    mState = State::GotStart20;
                }
                break;
            case State::GotStart20:
                if (b == 0x40) {
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
        static inline void convert(int16_t* pulses) {
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
    private:
        using MesgType = std::array<uint8_t, 28>;  // 0x20, 0x40 , 28 Bytes, checkH, checkL
        static CheckSum csum;
        static State mState;
        static MesgType ibusFrame;
        static uint8_t mIndex;
        static uint16_t mPackagesCounter;
    };
    
    CheckSum Servo::csum;
    Servo::State Servo::mState{Servo::State::Undefined};
    Servo::MesgType Servo::ibusFrame; 
    uint8_t Servo::mIndex{};
    uint16_t Servo::mPackagesCounter{};
}

void processIbusInput() {
#if !defined(SIMU)
  uint8_t rxchar;

  while (sbusGetByte(&rxchar)) {
      IBus::Servo::process(rxchar, [&](){
          IBus::Servo::convert(ppmInput);
          ppmInputValidityTimer = PPM_IN_VALID_TIMEOUT;        
      });
  }
#endif
}

