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

#define SBUS_FRAME_GAP_DELAY   1000 // 500uS

//#define SBUS_START_BYTE        0x0F
//#define SBUS_END_BYTE          0x00
//#define SBUS_FLAGS_IDX         23
#define SBUS_FRAMELOST_BIT     2
#define SBUS_FAILSAFE_BIT      3

//#define SBUS_CH_BITS           11
//#define SBUS_CH_MASK           ((1<<SBUS_CH_BITS) - 1)

#define SBUS_CH_CENTER         0x3E0

static bool (*_sbusAuxGetByte)(void*, uint8_t*) = nullptr;
static void* _sbusAuxGetByteCtx = nullptr;

void sbusSetAuxGetByte(void* ctx, bool (*fct)(void*, uint8_t*))
{
  _sbusAuxGetByte = nullptr;
  _sbusAuxGetByteCtx = ctx;
  _sbusAuxGetByte = fct;
}

bool sbusAuxGetByte(uint8_t* byte)
{
  auto _getByte = _sbusAuxGetByte;
  auto _ctx = _sbusAuxGetByteCtx;

  if (_getByte) {
    return _getByte(_ctx, byte);
  }

  return false;
}

static bool (*sbusGetByte)(uint8_t*) = nullptr;

void sbusSetGetByte(bool (*fct)(uint8_t*))
{
  sbusGetByte = fct;
}

namespace  SBus {
    template<uint8_t Instance>
    typename Servo<Instance>::State Servo<Instance>::mState{Servo::State::Undefined};
    template<uint8_t Instance>
    typename Servo<Instance>::MesgType Servo<Instance>::mData; 
    template<uint8_t Instance>
    uint8_t Servo<Instance>::mIndex{0};
    template<uint8_t Instance>
    uint16_t Servo<Instance>::mPackages{0};
    template<uint8_t Instance>
    uint8_t Servo<Instance>::mPauseCounter{Servo::mPauseCount}; // 2 ms
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
  
  while (sbusAuxGetByte(&rxchar)) {
      SBus::Servo<0>::process(rxchar, [&](){
          SBus::Servo<0>::convert(ppmInput);
          ppmInputValidityTimer = PPM_IN_VALID_TIMEOUT;        
      });
  }
#endif    
}

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
