#include "crsf.h"

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
