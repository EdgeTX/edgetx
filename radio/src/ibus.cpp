#include "ibus.h"

namespace IBus {
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
}

void processIbusInput() {
#if !defined(SIMU)
  uint8_t rxchar;

  while (sbusAuxGetByte(&rxchar)) {
      IBus::Servo<0>::process(rxchar, [&](){
          IBus::Servo<0>::convert(ppmInput);
          ppmInputValidityTimer = PPM_IN_VALID_TIMEOUT;        
      });
  }
#endif
}

