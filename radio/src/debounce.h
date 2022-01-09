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

#ifndef _DEBOUNCE_H_
#define _DEBOUNCE_H_

#include "board.h"

template <class T>
class Debounce
{
  public:
    T debounce(T state)
    {
      if (state == lastState)
        debouncedState = state;
      else
        lastState = state;
      return debouncedState;
    }

  private:
    T lastState;
    T debouncedState;
};

class PinDebounce: public Debounce<uint8_t>
{
  public:
    uint8_t debounce(GPIO_TypeDef * GPIOx, uint16_t GPIO_Pin)
    {
      return Debounce<uint8_t>::debounce(GPIO_ReadInputDataBit(GPIOx, GPIO_Pin));
    }
};

#endif // _DEBOUNCE_H_
