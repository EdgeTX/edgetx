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

#include "stm32_spi_adc.h"
#include "stm32_adc.h"

void ads79xx_init(const stm32_spi_adc_t* adc);
bool ads79xx_adc_start_read(const stm32_spi_adc_t* adc, const stm32_adc_input_t* inputs);
void ads79xx_adc_wait_completion(const stm32_spi_adc_t* adc, const stm32_adc_input_t* inputs);

