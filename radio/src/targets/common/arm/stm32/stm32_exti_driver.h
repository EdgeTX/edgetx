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

// Prior to use this driver, please make sure the proper macros are defined in hal.h:
//
//  - If 'USE_EXTIm_n_IRQ' is defined,
//    'EXTIm_n_IRQHandler' will be handled by this driver.
//
//  - If 'USE_EXTIn_IRQ' is defined,
//    'EXTIn_IRQHandler' will be handled by this driver.

typedef void (*stm32_exti_handler_t)();

// Set callback and enable IRQ
void stm32_exti_enable(uint32_t line, uint8_t trigger, stm32_exti_handler_t cb);

// Reset callback and disable IRQ if no more handlers
void stm32_exti_disable(uint32_t line);
