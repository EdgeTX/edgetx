/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "opentx.h"
#include "serial.h"
#include <stdarg.h>
#include <stdio.h>

#define PRINTF_BUFFER_SIZE    128

void serialPutc(char c) {
#if !defined(BOOT) && defined(USB_SERIAL)
  if (getSelectedUsbMode() == USB_SERIAL_MODE)
    usbSerialPutc(c);
#endif
#if defined(AUX_SERIAL)
  if (auxSerialTracesEnabled())
    auxSerialPutc(c);
#endif
#if defined(AUX2_SERIAL)
  if (aux2SerialTracesEnabled())
    aux2SerialPutc(c);
#endif
}

void serialPrintf(const char * format, ...)
{
  va_list arglist;
  char tmp[PRINTF_BUFFER_SIZE+1];

  va_start(arglist, format);
  vsnprintf(tmp, PRINTF_BUFFER_SIZE, format, arglist);
  tmp[PRINTF_BUFFER_SIZE] = '\0';
  va_end(arglist);

  const char *t = tmp;
  while (*t) {
    serialPutc(*t++);
  }
}

void serialCrlf()
{
  serialPutc('\r');
  serialPutc('\n');
}
