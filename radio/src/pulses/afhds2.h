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

#ifndef PULSES_AFHDS2_H_
#define PULSES_AFHDS2_H_

#include <inttypes.h>
#include <functional>
#include <map>
#include <list>

#define AFHDS2_PERIOD (2 * 1000) /* us */

struct FlySkySerialPulsesData {
  uint8_t  pulses[64];
  uint8_t  * ptr;
  uint8_t  frame_index;
  uint8_t  crc;
  uint8_t  state;
  uint8_t  timeout;
  uint8_t  esc_state;
  uint8_t  telemetry[64];
  uint8_t  telemetry_index;
} __attribute__((__packed__));

enum AfhdsSpecialChars {
  END = 0xC0,             //Frame end
  START = END,
  ESC_END = 0xDC,         //Escaped frame end - in case END occurs in fame then ESC ESC_END must be used
  ESC = 0xDB,             //Escaping character
  ESC_ESC = 0xDD,         //Escaping character in case ESC occurs in fame then ESC ESC_ESC  must be used
};


class afhds2 {
public:
  afhds2();
  virtual ~afhds2();
};

extern etx_serial_init afhds2SerialInitParams;

#endif /* PULSES_AFHDS2_H_ */
