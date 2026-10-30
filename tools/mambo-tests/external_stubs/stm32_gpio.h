// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#define GPIOE 0x40021000u
#define GPIOB 0x40020400u
#define GPIO_PIN(port, number) (uintptr_t(port) | (number))
inline uint32_t __get_PRIMASK() { return 0; }
inline void __disable_irq() {}
inline void __set_PRIMASK(uint32_t) {}
