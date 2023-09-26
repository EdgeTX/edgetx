/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "sbus.h"
#include "timers_driver.h"

static int (*_sbusAuxGetByte)(void*, uint8_t*) = nullptr;
static void* _sbusAuxGetByteCtx = nullptr;

void sbusSetAuxGetByte(void* ctx, int (*fct)(void*, uint8_t*))
{
  _sbusAuxGetByteCtx = ctx;
  _sbusAuxGetByte = fct;
}

int sbusAuxGetByte(uint8_t* byte)
{
  auto _getByte = _sbusAuxGetByte;
  auto _ctx = _sbusAuxGetByteCtx;

  if (_getByte) {
    return _getByte(_ctx, byte);
  }

  return -1;
}

static int (*sbusGetByte)(uint8_t*) = nullptr;

void sbusSetGetByte(int (*fct)(uint8_t*))
{
  sbusGetByte = fct;
}

void sbusTrainerPauseCheck() {
#if !defined(SIMU)
# if defined(AUX_SERIAL) || defined(AUX2_SERIAL) || defined(TRAINER_MODULE_SBUS)
    if (hasSerialMode(UART_MODE_SBUS_TRAINER) >= 0) {
        SBus::Servo<0>::tick1ms();
        processSbusInput();    
    }
# endif
#endif    
}

void processSbusInput() {
#if !defined(SIMU)
  uint8_t rxchar;
  
  while (sbusAuxGetByte(&rxchar) > 0) {
      SBus::Servo<0>::process(rxchar, [&](){
          SBus::Servo<0>::convert(trainerInput);
          trainerInputValidityTimer = TRAINER_IN_VALID_TIMEOUT;
      });
  }
#endif    
}

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
