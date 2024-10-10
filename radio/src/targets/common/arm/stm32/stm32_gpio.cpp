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

#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "stm32_exti_driver.h"
#include "stm32_gpio_driver.h"
#include "stm32_hal.h"
#include "stm32_hal_ll.h"

static inline GPIO_TypeDef* _port(gpio_t pin)
{
  return (GPIO_TypeDef*)(pin & ~(0x0f));
}

static inline int _port_num(gpio_t pin)
{
  return (((pin - GPIOA_BASE) >> 10) & 0x0f);
}

static inline void _enable_clock(GPIO_TypeDef* port)
{
  uint32_t reg_idx = (((uint32_t)port) - GPIOA_BASE) / 0x0400UL;
#if defined(RCC_AHB4ENR_GPIOAEN)
  uint32_t reg_msk = RCC_AHB4ENR_GPIOAEN << reg_idx;
  LL_AHB4_GRP1_EnableClock(reg_msk);
#elif defined(RCC_AHB1ENR_GPIOAEN)
  uint32_t reg_msk = RCC_AHB1ENR_GPIOAEN << reg_idx;
  LL_AHB1_GRP1_EnableClock(reg_msk);
#else
  #error "Unsupported GPIO clock"
#endif
}

static inline int _pin_num(gpio_t pin)
{
  return (pin & 0x0f);
}

static inline void _set_mode(GPIO_TypeDef* port, int pin_num, unsigned mode)
{
  uint32_t tmp = port->MODER;
  tmp &= ~(0x3 << (2 * pin_num));
  tmp |=  ((mode & 0x3) << (2 * pin_num));
  port->MODER = tmp;
}

static inline unsigned _get_mode(GPIO_TypeDef* port, int pin_num)
{
  uint32_t tmp = port->MODER;
  tmp >>= (2 * pin_num);
  return tmp & 0x03;
}


void gpio_init(gpio_t pin, gpio_mode_t mode, gpio_speed_t speed)
{
  GPIO_TypeDef* port = _port(pin);
  int pin_num = _pin_num(pin);

  _enable_clock(port);
  _set_mode(port, pin, mode);

  // pull up/down
  port->PUPDR &= ~(0x3 << (2 * pin_num));
  port->PUPDR |=  (((mode >> 2) & 0x3) << (2 * pin_num));

  // output mode
  port->OTYPER &= ~(1 << pin_num);
  port->OTYPER |=  (((mode >> 4) & 0x1) << pin_num);

  // pin speed to max
  port->OSPEEDR |= (speed << (2 * pin_num));
}

void gpio_init_int(gpio_t pin, gpio_mode_t mode, gpio_flank_t flank, gpio_cb_t cb)
{
  int port_num = _port_num(pin);
  int pin_num = _pin_num(pin);

  gpio_init(pin, mode, GPIO_PIN_SPEED_LOW);

  // enable specific pin as exti sources
#if defined(LL_APB4_GRP1_PERIPH_SYSCFG)
  LL_APB4_GRP1_EnableClock(LL_APB4_GRP1_PERIPH_SYSCFG);
#elif defined(LL_APB2_GRP1_PERIPH_SYSCFG)
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
#elif defined(LL_APB4_GRP1_PERIPH_SBS)
# define SYSCFG SBS
  LL_APB4_GRP1_EnableClock(LL_APB4_GRP1_PERIPH_SBS);
#else
  #error "Unsupported SYSCFG clock"
#endif

  SYSCFG->EXTICR[pin_num >> 2] &= ~(0xf << ((pin_num & 0x03) * 4));
  SYSCFG->EXTICR[pin_num >> 2] |= (port_num << ((pin_num & 0x03) * 4));

  stm32_exti_enable(1 << pin_num, (uint8_t)flank, cb);
}

void gpio_int_disable(gpio_t pin)
{
  int pin_num = _pin_num(pin);  
  stm32_exti_disable(1 << pin_num);
}

void gpio_init_af(gpio_t pin, gpio_af_t af, gpio_speed_t speed)
{
  GPIO_TypeDef* port = _port(pin);
  int pin_num = _pin_num(pin);

  _enable_clock(port);

  // Clear PUPDR
  port->PUPDR &= ~(0x3 << (2 * pin_num));

  // set selected function
  port->AFR[(pin_num > 7) ? 1 : 0] &= ~(0xf << ((pin_num & 0x07) * 4));
  port->AFR[(pin_num > 7) ? 1 : 0] |= (af << ((pin_num & 0x07) * 4));
  // pin speed to max
  port->OSPEEDR |= (speed << (2 * pin_num));

  _set_mode(port, pin_num, 2);
}

void gpio_set_af(gpio_t pin, gpio_af_t af)
{
  GPIO_TypeDef* port = _port(pin);
  int pin_num = _pin_num(pin);

  // set selected function
  port->AFR[(pin_num > 7) ? 1 : 0] &= ~(0xf << ((pin_num & 0x07) * 4));
  port->AFR[(pin_num > 7) ? 1 : 0] |= (af << ((pin_num & 0x07) * 4));

  _set_mode(port, pin_num, 2);
}

void gpio_init_analog(gpio_t pin)
{
  GPIO_TypeDef* port = _port(pin);
  int pin_num = _pin_num(pin);

  _enable_clock(port);
  _set_mode(port, pin_num, 3);

  // PUPD has to be 0b00
  port->PUPDR &= ~(0x3 << (2 * pin_num));
}

gpio_mode_t gpio_get_mode(gpio_t pin)
{
  return _get_mode(_port(pin), _pin_num(pin));
}

int gpio_read(gpio_t pin)
{
  return (_port(pin)->IDR & (1 << _pin_num(pin)));
}

void gpio_set(gpio_t pin)
{
  _port(pin)->BSRR = (1 << _pin_num(pin));
}

void gpio_clear(gpio_t pin)
{
  _port(pin)->BSRR = (1 << (_pin_num(pin) + 16));
}

void gpio_toggle(gpio_t pin)
{
  if (gpio_read(pin)) {
    gpio_clear(pin);
  } else {
    gpio_set(pin);
  }
}

void gpio_write(gpio_t pin, int value)
{
  if (value) {
    gpio_set(pin);
  } else {
    gpio_clear(pin);
  }
}
