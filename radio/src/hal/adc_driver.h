/*
 * Copyright (C) EdgeTx
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

struct etx_hal_adc_channel {
  const uint8_t adc_channel;
  const uint8_t rank;
  const uint8_t sample_time;
};

typedef const etx_hal_adc_channel* (*etx_hal_adc_get_channels)();
typedef uint8_t (*etx_hal_adc_get_nconv)();

struct etx_hal_adc {
  ADC_TypeDef* adc;
  const etx_hal_adc_get_nconv    get_nconv;
  const etx_hal_adc_get_channels get_channels;
};
