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

#if defined(SIMU)
#define dumpPrintf(...) debugPrintf(__VA_ARGS__)
#else
#define dumpPrintf(...) dbgSerialPrintf(__VA_ARGS__)
#endif

#if defined(DEBUG) || defined(CLI)
uint8_t dumpPosition;

static void dumpStart(unsigned int size)
{
  dumpPrintf("DUMP %d bytes ...\n\r", size);
  dumpPosition = 0;
}

static void dumpBody(const void *data, unsigned int size)
{
  for (unsigned int i=0; i<size; i++) {
    dumpPrintf("%.2X ", ((uint8_t *)data)[i]);
    dumpPosition++;
    if ((dumpPosition & 31) == 0) {
      dumpPrintf(CRLF);
    }
  }
}

static void dumpEnd()
{
  dumpPrintf(CRLF);
}

void dump(const void * data, unsigned int size)
{
  dumpStart(size);
  dumpBody(data, size);
  dumpEnd();
}
#endif
