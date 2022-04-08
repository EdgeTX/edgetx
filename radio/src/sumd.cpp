#include "sumd.h"

namespace SumDV3 {
    template<uint8_t Instance>
    uint8_t Servo<Instance>::nChannels{};
    template<uint8_t Instance>
    Crc16 Servo<Instance>::csum{};
    template<uint8_t Instance>
    uint8_t Servo<Instance>::crcH{};
    
    template<uint8_t Instance>
    typename Servo<Instance>::State Servo<Instance>::mState{Servo::State::Undefined};
    template<uint8_t Instance>
    typename Servo<Instance>::MesgType Servo<Instance>::sumdFrame; 
    template<uint8_t Instance>
    uint8_t Servo<Instance>::mIndex{};

    template<uint8_t Instance>
    uint16_t Servo<Instance>::mPackagesCounter{};
    template<uint8_t Instance>
    uint16_t Servo<Instance>::mBytesCounter{};

    template<uint8_t Instance>
    typename Servo<Instance>::Frame Servo<Instance>::frame{Servo::Frame::Undefined};
    template<uint8_t Instance>
    uint8_t Servo<Instance>::reserved{};
    template<uint8_t Instance>
    uint8_t Servo<Instance>::mode_cmd{};
    template<uint8_t Instance>
    uint8_t Servo<Instance>::sub_cmd{};
    template<uint8_t Instance>
    typename Servo<Instance>::Command_t Servo<Instance>::mCommand{};
}

void processSumdInput() {
#if !defined(SIMU)
  uint8_t rxchar;

  while (sbusAuxGetByte(&rxchar)) {
      SumDV3::Servo<0>::process(rxchar, [&](){
          SumDV3::Servo<0>::convert(ppmInput);
          ppmInputValidityTimer = PPM_IN_VALID_TIMEOUT;        
      });
  }
#endif
}
