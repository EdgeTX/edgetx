#include "sumd.h"

void processSumdInput() {
#if !defined(SIMU)
  uint8_t rxchar;

  while (sbusAuxGetByte(&rxchar)) {
      SumDV3::Servo<0>::process(rxchar, [&](){
          SumDV3::Servo<0>::convert(trainerInput);
          trainerInputValidityTimer = TRAINER_IN_VALID_TIMEOUT;
      });
  }
#endif
}
