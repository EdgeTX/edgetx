#include "crsf.h"

namespace CRSF {
    
    template<uint8_t Instance>
    CRC8 Servo<Instance>::csum;
    template<uint8_t Instance>
    typename Servo<Instance>::State Servo<Instance>::mState{Servo::State::Undefined};
    template<uint8_t Instance>
    typename Servo<Instance>::MesgType Servo<Instance>::mData; 
    template<uint8_t Instance>
    uint8_t Servo<Instance>::mIndex{0};
    template<uint8_t Instance>
    uint8_t Servo<Instance>::mLength{0};
    template<uint8_t Instance>
    uint16_t Servo<Instance>::mPackages{0};
    template<uint8_t Instance>
    uint8_t Servo<Instance>::mPauseCounter{Servo::mPauseCount}; // 2 ms
}

void crsfTrainerPauseCheck() {
#if !defined(SIMU)
# if defined(AUX_SERIAL) || defined(AUX2_SERIAL)
    if (hasSerialMode(UART_MODE_CRSF_TRAINER) >= 0) {
        CRSF::Servo<0>::tick1ms();
        processCrsfInput();    
    }
# endif
#endif    
}

void processCrsfInput() {
#if !defined(SIMU)
  uint8_t rxchar;

  while (sbusAuxGetByte(&rxchar)) {
      CRSF::Servo<0>::process(rxchar, [&](){
          CRSF::Servo<0>::convert(ppmInput);
          ppmInputValidityTimer = PPM_IN_VALID_TIMEOUT;        
      });
  }
#endif
}
