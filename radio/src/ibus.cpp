#include "ibus.h"
#include "opentx.h"

#include <algorithm>
#include <limits>

#define IBUS_FRAME_GAP_DELAY   1000 // 500uS

#define IBUS_VALUE_MIN 988
#define IBUS_VALUE_MAX 2011
#define IBUS_VALUE_CENTER 1500

//namespace  {
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
    
    void processIbusFrame(const uint8_t* const ibusFrame, int16_t* const pulses, uint8_t size) {
        if (size != IBUS_FRAME_SIZE) return;
        if (ibusFrame[0] != 0x20) return;
        if (ibusFrame[1] != 0x40) return;
        CheckSum cs;
        cs += 0x20;
        cs += 0x40;
        for(size_t i = 0; i < 28; ++i) {
            cs += ibusFrame[2 + i];
        }
        cs.lowByte(ibusFrame[2 + 28]);
        cs.highByte(ibusFrame[2 + 28 + 1]);
        if (!cs) return;
        
        for (size_t chi{0}; chi < MAX_TRAINER_CHANNELS; chi++) {
            if (chi < 14) {
                const uint8_t h = ibusFrame[2 * chi + 1 + 2] & 0x0f;
                const uint8_t l = ibusFrame[2 * chi + 2];
                const uint16_t  v = (uint16_t(h) << 8) + uint8_t(l);
                pulses[chi] = convertIbusToPuls(v);
            }
            else if (chi < 18) {
                const uint8_t h1 = ibusFrame[6 * (chi - 14) + 1 + 2] & 0xf0;
                const uint8_t h2 = ibusFrame[6 * (chi - 14) + 3 + 2] & 0xf0;
                const uint8_t h3 = ibusFrame[6 * (chi - 14) + 5 + 2] & 0xf0;
                const uint16_t v = (uint8_t(h1) >> 4) + uint8_t(h2) + (uint16_t(h3) << 4);
                pulses[chi] = convertIbusToPuls(v);
            }
        }
        ppmInputValidityTimer = PPM_IN_VALID_TIMEOUT;        
    }
//}

void processIbusInput() {
#if !defined(SIMU)
  uint8_t rxchar;
  bool active = false;
  static uint8_t IbusIndex = 0;
  static uint16_t IbusTimer;
  static uint8_t IbusFrame[IBUS_FRAME_SIZE];

  while (sbusGetByte(&rxchar)) {
    active = true;
    IbusIndex = std::min(IbusIndex, (uint8_t)(IBUS_FRAME_SIZE - 1));
    IbusFrame[IbusIndex++] = rxchar;
  }
  if (active) {
    IbusTimer = getTmr2MHz();
    return;
  }
  else {
    if (IbusIndex) {
      if ((uint16_t) (getTmr2MHz() - IbusTimer) > IBUS_FRAME_GAP_DELAY) {
        processIbusFrame(IbusFrame, ppmInput, IbusIndex);
        IbusIndex = 0;
      }
    }
  }
#endif
    
}
