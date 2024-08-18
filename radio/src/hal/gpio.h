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

typedef uint32_t gpio_t;

typedef uint8_t  gpio_mode_t;
typedef uint8_t  gpio_af_t;
typedef uint8_t  gpio_speed_t;

typedef enum {
    GPIO_RISING = 1,
    GPIO_FALLING = 2,
    GPIO_BOTH = 3
} gpio_flank_t;

typedef void (*gpio_cb_t)();

void gpio_init(gpio_t pin, gpio_mode_t mode, gpio_speed_t speed);
void gpio_init_af(gpio_t pin, gpio_af_t af, gpio_speed_t speed);
void gpio_init_int(gpio_t pin, gpio_mode_t mode, gpio_flank_t flank, gpio_cb_t cb);
void gpio_init_analog(gpio_t pin);

void gpio_int_disable(gpio_t pin);
void gpio_set_af(gpio_t pin, gpio_af_t af);

gpio_mode_t gpio_get_mode(gpio_t pin);

int gpio_read(gpio_t pin);
void gpio_set(gpio_t pin);
void gpio_clear(gpio_t pin);
void gpio_write(gpio_t pin, int value);
void gpio_toggle(gpio_t pin);
