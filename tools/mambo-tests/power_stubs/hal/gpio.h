// SPDX-License-Identifier: GPL-2.0-only
#pragma once
typedef unsigned gpio_t;
struct GPIO_TypeDef;
void gpio_init(unsigned pin, unsigned mode, unsigned speed);
int gpio_read(unsigned pin);
void gpio_set(unsigned pin);
void gpio_clear(unsigned pin);
