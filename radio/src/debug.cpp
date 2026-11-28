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

