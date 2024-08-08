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
#include <string.h>
#include "timers_driver.h"

#define TELEMETRY_AVERAGE_COUNT   3     // we actually average one more reading!
#define RAW_FRSKY_MINMAX(v)       v.values[TELEMETRY_AVERAGE_COUNT-1]
#define MAX_CELLS                 8

class TelemetryValue {
  public:
    uint8_t value()
    {
      return _value;
    }

    void set(uint8_t value)
    {
      _value = value;
    }

    void reset()
    {
      set(0);
    }

  protected:
    uint8_t _value;
};

template <class T>
class TelemetryFilterDecorator: public T {
  public:
    void set(uint8_t value)
    {
      if (value == 0 || this->_value == 0) {
        memset(values, value, TELEMETRY_AVERAGE_COUNT);
        this->_value = value;
      }
      else {
        // calculate the average from values[] and value
        // also shift readings in values [] array
        unsigned int sum = values[0];
        for (int i=0; i<TELEMETRY_AVERAGE_COUNT-1; i++) {
          uint8_t tmp = values[i+1];
          values[i] = tmp;
          sum += tmp;
        }
        values[TELEMETRY_AVERAGE_COUNT-1] = value;
        sum += value;
        this->_value = sum / (TELEMETRY_AVERAGE_COUNT+1);
      }
    }

    void reset()
    {
      memclear(this, sizeof(*this));
    }

  protected:
    uint8_t values[TELEMETRY_AVERAGE_COUNT];
};

template <class T>
class TelemetryExpiringDecorator: public T {
  public:
    void set(uint8_t value)
    {
      T::set(value);
      expirationTime = get_tmr10ms() + 1000/*10s*/;
    }

    void reset()
    {
      memclear(this, sizeof(*this));
    }

    bool isFresh()
    {
      return get_tmr10ms() < expirationTime;
    }

  protected:
    tmr10ms_t expirationTime;
};

template <class T>
class TelemetryMinDecorator: public T {
  public:
    void set(uint8_t value)
    {
      T::set(value);
      if (!_min || value < _min) {
        _min = value;
      }
    }

    uint8_t min()
    {
      return _min;
    }

    void reset()
    {
      memclear(this, sizeof(*this));
    }

  protected:
    uint8_t _min;
};
