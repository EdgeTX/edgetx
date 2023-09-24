#include "ibus.h"

void processIbusInput() {
#if !defined(SIMU)
  uint8_t rxchar;

  while (sbusAuxGetByte(&rxchar)) {
      IBus::Servo<0>::process(rxchar, [&](){
          IBus::Servo<0>::convert(trainerInput);
          trainerInputValidityTimer = TRAINER_IN_VALID_TIMEOUT;
      });
  }
#endif
}

