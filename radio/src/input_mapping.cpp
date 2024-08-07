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

#include "input_mapping.h"
#include "hal/adc_driver.h"

#include "edgetx.h"

#define _CHANNEL_ORDER(a,b,c,d) \
  (((a) & 0x3) | (((b) & 0x3) << 2) | (((c) & 0x3) << 4) | (((d) & 0x3) << 6))

#if defined(SURFACE_RADIO)

#define ST 0
#define TH 1

const uint8_t _channel_order_lut[] = {
  _CHANNEL_ORDER(ST,TH,0,0),
  _CHANNEL_ORDER(TH,ST,0,0),
};

#else

#define RUD 0
#define ELE 1
#define THR 2
#define AIL 3

const uint8_t _channel_order_lut[] = {
  _CHANNEL_ORDER(RUD,ELE,THR,AIL),
  _CHANNEL_ORDER(RUD,ELE,AIL,THR),
  _CHANNEL_ORDER(RUD,THR,ELE,AIL),
  _CHANNEL_ORDER(RUD,THR,AIL,ELE),
  _CHANNEL_ORDER(RUD,AIL,ELE,THR),
  _CHANNEL_ORDER(RUD,AIL,THR,ELE),
  _CHANNEL_ORDER(ELE,RUD,THR,AIL),
  _CHANNEL_ORDER(ELE,RUD,AIL,THR),
  _CHANNEL_ORDER(ELE,THR,RUD,AIL),
  _CHANNEL_ORDER(ELE,THR,AIL,RUD),
  _CHANNEL_ORDER(ELE,AIL,RUD,THR),
  _CHANNEL_ORDER(ELE,AIL,THR,RUD),
  _CHANNEL_ORDER(THR,RUD,ELE,AIL),
  _CHANNEL_ORDER(THR,RUD,AIL,ELE),
  _CHANNEL_ORDER(THR,ELE,RUD,AIL),
  _CHANNEL_ORDER(THR,ELE,AIL,RUD),
  _CHANNEL_ORDER(THR,AIL,RUD,ELE),
  _CHANNEL_ORDER(THR,AIL,ELE,RUD),
  _CHANNEL_ORDER(AIL,RUD,ELE,THR),
  _CHANNEL_ORDER(AIL,RUD,THR,ELE),
  _CHANNEL_ORDER(AIL,ELE,RUD,THR),
  _CHANNEL_ORDER(AIL,ELE,THR,RUD),
  _CHANNEL_ORDER(AIL,THR,RUD,ELE),
  _CHANNEL_ORDER(AIL,THR,ELE,RUD),
};
#endif

#if !defined(SURFACE_RADIO)

/*
mode1 rud ele thr ail
mode2 rud thr ele ail
mode3 ail ele thr rud
mode4 ail thr ele rud
*/

const uint8_t _input_mode_lut[]  = {
    RUD, ELE, THR, AIL,
    RUD, THR, ELE, AIL,
    AIL, ELE, THR, RUD,
    AIL, THR, ELE, RUD
};

#endif

bool inputMappingModesUsed()
{
#if defined(SURFACE_RADIO)
  return false;
#else
  return true;
#endif
}

uint8_t inputMappingGetThrottle()
{
#if defined(SURFACE_RADIO)
  return TH;
#else
  return THR;
#endif
}

uint8_t inputMappingConvertMode(uint8_t mode, uint8_t ch)
{
#if !defined(SURFACE_RADIO)
  if (ch < adcGetMaxInputs(ADC_INPUT_MAIN)) {
    mode = min(mode, uint8_t(MAX_INPUT_MODES - 1));
    return _input_mode_lut[mode * MAX_STICKS + ch];
  }
#endif
  return ch;
}

uint8_t inputMappingConvertMode(uint8_t ch)
{
  return inputMappingConvertMode(g_eeGeneral.stickMode, ch);
}

uint8_t inputMappingChannelOrder(uint8_t order, uint8_t ch)
{
  ch = min(ch, uint8_t(MAX_STICKS - 1));
  order = min(order, uint8_t(DIM(_channel_order_lut) - 1));
  return (_channel_order_lut[order] >> (ch * 2)) & 0x3;
}

uint8_t inputMappingChannelOrder(uint8_t ch)
{
  return inputMappingChannelOrder(g_eeGeneral.templateSetup, ch);
}

uint8_t inputMappingGetChannelOrder(uint8_t order)
{
  order = min(order, uint8_t(DIM(_channel_order_lut) - 1));
  return _channel_order_lut[order];
}

uint8_t inputMappingGetMaxChannelOrder()
{
  return DIM(_channel_order_lut);
}

