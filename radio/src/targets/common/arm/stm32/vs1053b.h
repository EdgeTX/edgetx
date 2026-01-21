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

#include "hal/gpio.h"
#include "stm32_spi.h"

typedef struct {
  stm32_spi_t* spi;

  gpio_t       XDCS;
  gpio_t       DREQ;

  void (*set_rst_pin)(bool set);
  void (*set_mute_pin)(bool set);

  uint32_t     mute_delay_ms;
  uint32_t     unmute_delay_ms;
} vs1053b_t;

void vs1053b_init(const vs1053b_t* dev);
