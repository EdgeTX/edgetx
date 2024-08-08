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

#include <inttypes.h>

#if defined(EXTMODULE_TIMER_32BITS)
  typedef uint32_t pulse_duration_t;
#else
  typedef uint16_t pulse_duration_t;
#endif

typedef uint16_t trainer_pulse_duration_t;

enum ModuleSettingsMode
{
  MODULE_MODE_NORMAL,
  MODULE_MODE_SPECTRUM_ANALYSER,
  MODULE_MODE_POWER_METER,
  MODULE_MODE_GET_HARDWARE_INFO,
  MODULE_MODE_MODULE_SETTINGS,
  MODULE_MODE_RECEIVER_SETTINGS,
  MODULE_MODE_BEEP_FIRST,
  MODULE_MODE_REGISTER = MODULE_MODE_BEEP_FIRST,
  MODULE_MODE_BIND,
  MODULE_MODE_SHARE,
  MODULE_MODE_RANGECHECK,
  MODULE_MODE_RESET,
  MODULE_MODE_AUTHENTICATION,
  MODULE_MODE_OTA_UPDATE,
};

ModuleSettingsMode getModuleMode(int moduleIndex);
void setModuleMode(int moduleIndex, ModuleSettingsMode mode);

template <class T, int SIZE>
class DataBuffer {
  public:
    const T * getData()
    {
      return data;
    }

    uint8_t getSize()
    {
      return ptr - data;
    }

  protected:
    T data[SIZE];
    T * ptr;

    void initBuffer()
    {
      ptr = data;
    }
};

template <class T, int SIZE>
class PulsesBuffer: public DataBuffer<T, SIZE> {
  public:
    T getLast() {
      return *(DataBuffer<T, SIZE>::ptr - 1);
    };

    void setLast(T value) {
      *(DataBuffer<T, SIZE>::ptr - 1) = value;
    }
};
