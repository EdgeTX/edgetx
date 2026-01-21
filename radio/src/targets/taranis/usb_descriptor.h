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

#if defined(RADIO_TX12) || defined(RADIO_TX12MK2)
  #define USB_NAME                     "Radiomaster TX12"
  #define USB_MANUFACTURER             'R', 'M', '_', 'T', 'X', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'R', 'M', ' ', 'T', 'X', '1', '2', ' '  /* 8 Bytes */
#elif defined(RADIO_BOXER)
  #define USB_NAME                     "Radiomaster Boxer"
  #define USB_MANUFACTURER             'R', 'M', '_', 'T', 'X', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'R', 'M', ' ', 'B', 'o', 'x', 'e', 'r'  /* 8 Bytes */
#elif defined(RADIO_ZORRO)
  #define USB_NAME                     "Radiomaster Zorro"
  #define USB_MANUFACTURER             'R', 'M', '_', 'T', 'X', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'R', 'M', ' ', 'Z', 'o', 'r', 'r', 'o'  /* 8 Bytes */
#elif defined(RADIO_MT12)
  #define USB_NAME                     "Radiomaster MT12"
  #define USB_MANUFACTURER             'R', 'M', '_', 'T', 'X', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'R', 'M', ' ', 'M', 'T', '1', '2', ' '  /* 8 Bytes */
#elif defined(RADIO_GX12)
  #define USB_NAME                     "Radiomaster GX12"
  #define USB_MANUFACTURER             'R', 'M', '_', 'T', 'X', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'R', 'M', ' ', 'G', 'X', '1', '2', ' '  /* 8 Bytes */
#elif defined(RADIO_POCKET)
  #define USB_NAME                     "Radiomaster Pocket"
  #define USB_MANUFACTURER             'R', 'M', '_', 'T', 'X', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'R', 'M', 'P', 'o', 'c', 'k', 'e', 't'  /* 8 Bytes */
#elif defined(RADIO_T8)
  #define USB_NAME                     "Radiomaster T8"
  #define USB_MANUFACTURER             'R', 'M', '_', 'T', 'X', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'R', 'M', ' ', 'T', '8', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_LR3PRO)
  #define USB_NAME                     "BETAFPV LR3PRO"
  #define USB_MANUFACTURER             'B', 'E', 'T', 'A', 'F', 'P', 'V', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'L', 'R', '3', 'P', 'R', 'O', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_TLITE)
  #define USB_NAME                     "Jumper TLite"
  #define USB_MANUFACTURER             'J', 'U', 'M', 'P', 'E', 'R', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '-', 'L', 'i', 't', 'e', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_TPRO)
  #define USB_NAME                     "Jumper TPro"
  #define USB_MANUFACTURER             'J', 'U', 'M', 'P', 'E', 'R', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '-', 'P', 'r', 'o', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_TPROV2)
  #define USB_NAME                     "Jumper TPro V2"
  #define USB_MANUFACTURER             'J', 'U', 'M', 'P', 'E', 'R', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '-', 'P', 'r', 'o', ' ', 'V', '2'  /* 8 Bytes */
#elif defined(RADIO_T12MAX)
#define USB_NAME                       "Jumper T12 MAX"
  #define USB_MANUFACTURER             'J', 'U', 'M', 'P', 'E', 'R', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '1', '2', ' ', 'M', 'A', 'X', ' '  /* 8 Bytes */
#elif defined(RADIO_TPROS)
  #define USB_NAME                     "Jumper T-Pro S"
  #define USB_MANUFACTURER             'J', 'U', 'M', 'P', 'E', 'R', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '-', 'P', 'r', 'o', ' ', 'S', ' '  /* 8 Bytes */
#elif defined(RADIO_BUMBLEBEE)
  #define USB_NAME                     "Jumper Bumblebee"
  #define USB_MANUFACTURER             'J', 'U', 'M', 'P', 'E', 'R', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'B', 'u', 'm', 'b', 'l', 'e', 'b', 'e'  /* 8 Bytes */
#elif defined(RADIO_T14)
  #define USB_NAME                     "Jumper T14"
  #define USB_MANUFACTURER             'J', 'U', 'M', 'P', 'E', 'R', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '1', '4', ' ', ' ', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_T20)
  #define USB_NAME                     "Jumper T20"
  #define USB_MANUFACTURER             'J', 'U', 'M', 'P', 'E', 'R', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '2', '0', ' ', ' ', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_T20V2)
  #define USB_NAME                     "Jumper T20 V2"
  #define USB_MANUFACTURER             'J', 'U', 'M', 'P', 'E', 'R', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '2', '0', 'V', '2', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_COMMANDO8)
  #define USB_NAME                     "iFlight Commando 8"
  #define USB_MANUFACTURER             'i', 'F', 'l', 'i', 'g', 'h', 't', '-'  /* 8 bytes */
  #define USB_PRODUCT                  'C', 'o', 'm', 'm', 'a', 'n', 'd', 'o'  /* 8 Bytes */
#elif defined(RADIO_V12)
  #define USB_NAME                     "HelloRadioSky V12"
  #define USB_MANUFACTURER             'H', 'R', 'S', 'k', 'y', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'V', '1', '2', ' ', ' ', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_V14)
  #define USB_NAME                     "HelloRadioSky V14"
  #define USB_MANUFACTURER             'H', 'R', 'S', 'k', 'y', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'V', '1', '4', ' ', ' ', ' ', ' ', ' '  /* 8 Bytes */
#else
  #define USB_NAME                     "FrSky Taranis"
  #define USB_MANUFACTURER             'F', 'r', 'S', 'k', 'y', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', 'a', 'r', 'a', 'n', 'i', 's', ' '  /* 8 Bytes */
#endif
