// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <cstdint>
void gpio_write(uintptr_t pin, bool value);
void gpio_clear(uintptr_t pin);
bool gpio_read(uintptr_t pin);
