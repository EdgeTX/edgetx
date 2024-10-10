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

#if defined(PCBX12S)
  #define USB_NAME                     "FrSky Horus"
  #define USB_MANUFACTURER             'F', 'r', 'S', 'k', 'y', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'H', 'o', 'r', 'u', 's', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_T15)
  #define USB_NAME                     "Jumper T15"
  #define USB_MANUFACTURER             'J', 'u', 'm', 'p', 'e', 'r', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '1', '5', ' ', ' ', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_T16)
  #define USB_NAME                     "Jumper T16"
  #define USB_MANUFACTURER             'J', 'u', 'm', 'p', 'e', 'r', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '1', '6', ' ', ' ', ' ', ' ', ' '  /* 8 Bytes */  
#elif defined(RADIO_T18)
  #define USB_NAME                     "Jumper T18"
  #define USB_MANUFACTURER             'J', 'u', 'm', 'p', 'e', 'r', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '1', '8', ' ', ' ', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_TX16S)
  #define USB_NAME                     "Radiomaster TX16S"
  #define USB_MANUFACTURER             'R', 'M', '_', 'T', 'X', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'R', 'M', ' ', 'T', 'X', '1', '6', 'S'  /* 8 Bytes */
#elif defined(RADIO_F16)
  #define USB_NAME                     "Fatfish F16"
  #define USB_MANUFACTURER             'F', 'A', 'T', 'F', 'I', 'S', 'H', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'F', 'F', ' ', 'F', '1', '6', ' ', ' '  /* 8 Bytes */
#elif defined(PCBX10)
  #define USB_NAME                     "FrSky X10"
  #define USB_MANUFACTURER             'F', 'r', 'S', 'k', 'y', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'X', '1', '0', ' ', ' ', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_V16)
  #define USB_NAME                     "HelloRadioSky V16"
  #define USB_MANUFACTURER             'H', 'R', 'S', 'k', 'y', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'V', '1', '6', ' ', ' ', ' ', ' ', ' '  /* 8 Bytes */
#endif
