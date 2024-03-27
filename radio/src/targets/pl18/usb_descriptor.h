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

#pragma once

#if defined(RADIO_NB4P)
  #define USB_NAME                        "FlySky NB4+"
  #define USB_MANUFACTURER                'F', 'l', 'y', 'S', 'k', 'y', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                     'N', 'B', '4', '+', ' ', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_PL18EV)
  #define USB_NAME                        "FlySky PL18EV"
  #define USB_MANUFACTURER                'F', 'l', 'y', 'S', 'k', 'y', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                     'P', 'L', '1', '8', 'E', 'V', ' ', ' '  /* 8 Bytes */
#else 
  #define USB_NAME                        "FlySky PL18"
  #define USB_MANUFACTURER                'F', 'l', 'y', 'S', 'k', 'y', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                     'P', 'L', '1', '8', ' ', ' ', ' ', ' '  /* 8 Bytes */
#endif
