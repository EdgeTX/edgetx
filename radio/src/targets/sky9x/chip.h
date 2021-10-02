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

//------------------------------------------------------------------------------
/// \unit
/// !Purpose
///
/// Definition of AT91SAM3S4 characteristics and features
///
/// !Usage
/// -# For ARM core feature, see "AT91SAM3S4 - ARM core features".
/// -# For IP features, see "AT91SAM3S4 - IP features".
/// -# For misc, see "AT91SAM3S4 - Misc".
//------------------------------------------------------------------------------
 
#ifndef _CHIP_H_
#define _CHIP_H_

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// \page "AT91SAM3S4 - ARM core features"
/// This page lists several characteristics related to the ARM core
///

//ARM core features

/// ARM core definition.
#define cortexm3

/// family definition.
#define at91sam3s

//------------------------------------------------------------------------------
/// \page "AT91SAM3S4 - IP features"
/// This page lists several characteristics related to the embedded IP
///

//IP FEATURES

// EFC GPNVM number
#define CHIP_EFC_NUM_GPNVMS    3

/// Indicates chip has an Enhanced EFC. 
#define CHIP_FLASH_EEFC 

// DMA channels number
#define CHIP_DMA_CHANNEL_NUM   4

// Indicate chip's MCI interface. 
#define MCI2_INTERFACE

// Indicate chip SSC has DMA interface. 
#define CHIP_SSC_DMA

// Indicate chip SPI has DMA interface.
#define CHIP_SPI_DMA

/// Indicates chip has an UDP Full Speed. 
#define CHIP_USB_UDP 

/// Indicates chip has an internal pull-up. 
#define CHIP_USB_PULLUP_INTERNAL 

/// Number of USB endpoints 
#define CHIP_USB_NUMENDPOINTS 8 

/// Endpoints max paxcket size 
#define CHIP_USB_ENDPOINTS_MAXPACKETSIZE(i) \
   ((i == 0) ? 64 : \
   ((i == 1) ? 64 : \
   ((i == 2) ? 64 : \
   ((i == 3) ? 64 : \
   ((i == 4) ? 512 : \
   ((i == 5) ? 512 : \
   ((i == 6) ? 64 : \
   ((i == 7) ? 64 : 0 ))))))))

/// Endpoints Number of Bank 
#define CHIP_USB_ENDPOINTS_BANKS(i) \
   ((i == 0) ? 1 : \
   ((i == 1) ? 2 : \
   ((i == 2) ? 2 : \
   ((i == 3) ? 1 : \
   ((i == 4) ? 2 : \
   ((i == 5) ? 2 : \
   ((i == 6) ? 2 : \
   ((i == 7) ? 2 : 0 ))))))))

//------------------------------------------------------------------------------
/// \page "AT91SAM3S4 - Misc "
/// This page lists misc features
///

//Misc 

#endif // _CHIP_H_

