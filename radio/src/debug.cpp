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

#include "edgetx.h"
#include "stamp.h"
#include <stdarg.h>

#if defined(SIMU)
traceCallbackFunc traceCallback = 0;
#endif

#if defined(SIMU)
#define PRINTF_BUFFER_SIZE     1024
void debugPrintf(const char * format, ...)
{
  va_list arglist;
  char tmp[PRINTF_BUFFER_SIZE];

  va_start(arglist, format);
  vsnprintf(tmp, PRINTF_BUFFER_SIZE, format, arglist);
  va_end(arglist);
  fputs(tmp, stdout);
  fflush(stdout);
  if (traceCallback) {
    traceCallback(tmp);
  }
}
#endif

#if defined(DEBUG_INTERRUPTS)

#if defined(PCBHORUS)
  const char * const interruptNames[INT_LAST] = {
    "Tick ",   // INT_TICK,
    "1ms  ",   // INT_1MS,
    "Ser2 ",   // INT_SER2,
    "TelDm",   // INT_TELEM_DMA,
    "TelUs",   // INT_TELEM_USART,
    "Sdio ",   // INT_SDIO,
    "SdDma",   // INT_SDIO_DMA,
    "D2S7 ",   // INT_DMA2S7,
    "Tim1 ",   // INT_TIM1CC,
    "Tim2 ",   // INT_TIM2,
    "Tim3 ",   // INT_TIM3,
    "BlueT",   // INT_BLUETOOTH,
    "USB  ",  // INT_OTG_FS,
#if defined(DEBUG_USB_INTERRUPTS)
    " spur",  // INT_OTG_FS_SPURIOUS,
    "  out",  // INT_OTG_FS_OUT_EP,
    "   in",  // INT_OTG_FS_IN_EP,
    " miss",  // INT_OTG_FS_MODEMISMATCH,
    " wake",  // INT_OTG_FS_WAKEUP,
    " susp",  // INT_OTG_FS_SUSPEND,
    "  sof",  // INT_OTG_FS_SOF,
    " rxst",  // INT_OTG_FS_RX_STAT,
    "  rst",  // INT_OTG_FS_RESET,
    " enum",  // INT_OTG_FS_ENUM,
    " inci",  // INT_OTG_FS_INCOMPLETE_IN,
    " inco",  // INT_OTG_FS_INCOMPLETE_OUT,
    " sess",  // INT_OTG_FS_SESSION,
    "  otg",  // INT_OTG_FS_OTG,
    " notd",  // INT_OTG_FS_RX_NOT_DEVICE,
#endif // #if defined(DEBUG_USB_INTERRUPTS)
  };
#elif defined(PCBTARANIS)
  const char * const interruptNames[INT_LAST] = {
    "Tick ",   // INT_TICK,
    "5ms  ",   // INT_5MS,
    "Audio",   // INT_AUDIO,
    "BlueT",   // INT_BLUETOOTH,
    "Lcd  ",   // INT_LCD,
    "T1CC ",   // INT_TIM1CC,
    "Tim1 ",   // INT_TIM1,
    "Tim8 ",   // INT_TIM8,
    "Ser2 ",   // INT_SER2,
    "TelDm",   // INT_TELEM_DMA,
    "TelUs",   // INT_TELEM_USART,
    "Train",   // INT_TRAINER,
    "Usb  ",   // INT_OTG_FS,
#if defined(DEBUG_USB_INTERRUPTS)
    " spur",  // INT_OTG_FS_SPURIOUS,
    "  out",  // INT_OTG_FS_OUT_EP,
    "   in",  // INT_OTG_FS_IN_EP,
    " miss",  // INT_OTG_FS_MODEMISMATCH,
    " wake",  // INT_OTG_FS_WAKEUP,
    " susp",  // INT_OTG_FS_SUSPEND,
    "  sof",  // INT_OTG_FS_SOF,
    " rxst",  // INT_OTG_FS_RX_STAT,
    "  rst",  // INT_OTG_FS_RESET,
    " enum",  // INT_OTG_FS_ENUM,
    " inci",  // INT_OTG_FS_INCOMPLETE_IN,
    " inco",  // INT_OTG_FS_INCOMPLETE_OUT,
    " sess",  // INT_OTG_FS_SESSION,
    "  otg",  // INT_OTG_FS_OTG,
    " notd",  // INT_OTG_FS_RX_NOT_DEVICE,
#endif // #if defined(DEBUG_USB_INTERRUPTS)
  };
#endif

struct InterruptCounters interruptCounters;
#endif //#if defined(DEBUG_INTERRUPTS)

#if defined(DEBUG_TIMERS)

void DebugTimer::start()
{
  _start_hiprec = getTmr2MHz();
  _start_loprec = get_tmr10ms();
}

void DebugTimer::stop()
{
  // getTmr2MHz is 16 bit timer, resolution 0.5us, max measurable value 32.7675 milli seconds
  // tmr10ms_t tmr10ms = get_tmr10ms(); 32 bit timer, resolution 10ms, max measurable value: 42949672.95 s = 1.3 years
  // if time difference is bigger than 30ms, then use low resolution timer
  // otherwise use high resolution
  if ((_start_hiprec == 0) && (_start_loprec == 0)) return;

  last = get_tmr10ms() - _start_loprec;  //use low precision timer
  if (last < 3) {
    //use high precision
    last = (uint16_t)(getTmr2MHz() - _start_hiprec) / 2;
  }
  else {
    last *= 10000ul; //adjust unit to 1us
  }
  evalStats();
}

DebugTimer debugTimers[DEBUG_TIMERS_COUNT];

const char * const debugTimerNames[DEBUG_TIMERS_COUNT] = {
   "Pulses int."   // debugTimerIntPulses
  ,"Pulses dur."   // debugTimerIntPulsesDuration
  ,"10ms dur.  "   // debugTimerPer10ms
  ,"10ms period"   // debugTimerPer10msPeriod
  ,"Rotary enc."   // debugTimerRotEnc
  ,"Haptic     "   // debugTimerHaptic
  ,"Mixer calc "   // debugTimerMixer
  ,"Tel. wakeup"   // debugTimerTelemetryWakeup
  ,"perMain dur"   // debugTimerPerMain
  ," perMain s1"   // debugTimerPerMain1
  ," guiMain   "   // debugTimerGuiMain
  ,"  LUA      "   // debugTimerLua
  ,"  LCD wait "   // debugTimerLcdRefreshWait
  ,"  LCD refr."   // debugTimerLcdRefresh
  ,"  Menus    "   // debugTimerMenus
  ,"   Menu hnd"   // debugTimerMenuHandlers
  ,"Menu Vers. "   // debugTimerVersion
  ,"Menu simple"   // debugTimerSimpleMenu
  ,"Menu drawte"   // debugTimerDrawText
  ,"Menu drawt1"   // debugTimerDrawText1
  ,"Mix ADC    "   // debugTimerGetAdc
  ,"Mix getsw  "   // debugTimerGetSwitches
  ,"Mix eval   "   // debugTimerEvalMixes
  ,"Mix 10ms   "   // debugTimerMixes10ms
  ,"ADC read   "   // debugTimerAdcRead
  ,"mix-pulses "   // debugTimerMixerCalcToUsage
  ,"mix-int.   "   // debugTimerMixerIterval
  ,"Audio int. "   // debugTimerAudioIterval
  ,"Audio dur. "   // debugTimerAudioDuration
  ," A. consume"   // debugTimerAudioConsume
  ,"SpaceMouse "   // debugTimerSpaceMouseWakeup
};

#endif
