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

#include <stdint.h>
#include "hal.h"

#define AFHDS_MAX_PULSES 128

// max number of transitions measured so far 290 + 10%
// TODO: define as a function of MAX_PULSES
#define AFHDS_MAX_PULSES_TRANSITIONS 320

#if defined(EXTMODULE_USART) && defined(EXTMODULE_TX_INVERT_GPIO)
  #define AFHDS3_EXT_UART
#else
  #define AFHDS3_EXT_SOFTSERIAL
#endif

#define AFHDS3_UART_BAUDRATE        1500000
#define AFHDS3_UART_COMMAND_TIMEOUT 5

#if defined(AFHDS3_SLOW)
  #define AFHDS3_SOFTSERIAL_BAUDRATE        57600
  #define AFHDS3_SOFTSERIAL_COMMAND_TIMEOUT 20
#else
  #define AFHDS3_SOFTSERIAL_BAUDRATE        115200
  #define AFHDS3_SOFTSERIAL_COMMAND_TIMEOUT 15
#endif

#define AFHDS3_MAX_MODEL_ID 19

namespace afhds3
{

struct SerialData {
  uint8_t  pulses[AFHDS_MAX_PULSES];
  uint8_t* ptr;  
};

struct PulsesData {
  uint16_t  pulses[AFHDS_MAX_PULSES_TRANSITIONS];
  uint16_t* ptr;
};

#if defined(EXTMODULE_USART) && defined(EXTMODULE_TX_INVERT_GPIO)
typedef SerialData ExtmoduleData;
#else
typedef PulsesData ExtmoduleData;
#endif

#if defined(INTERNAL_MODULE_AFHDS3)
typedef SerialData IntmoduleData;
#endif

};
