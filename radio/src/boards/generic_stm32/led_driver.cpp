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
#include "hal/led_driver.h"
#include "hal/rgbleds.h"
#include "stm32_gpio.h"
#include "boards/generic_stm32/rgb_leds.h"
#include "board.h"

#if defined(STATUS_LED_PWM)
  #include "hal.h"
  #include "stm32_hal_ll.h"
  #include "stm32_timer.h"
#endif

#define __weak __attribute__((weak))

#define GET_RED(color) (((color) & 0xFF0000) >>16)
#define GET_GREEN(color) (((color) & 0x00FF00) >> 8)
#define GET_BLUE(color) ((color) & 0x0000FF)

#if !defined(GPIO_LED_GPIO_ON)
#define GPIO_LED_GPIO_ON              gpio_set
#define GPIO_LED_GPIO_OFF             gpio_clear
#endif

#if defined(STATUS_LED_PWM)

  #if defined(LED_RED_GPIO)
    #define _LED_PWM_RED LED_RED_GPIO
  #else
    #define _LED_PWM_RED 0
  #endif
  #if defined(LED_RED2_GPIO)
    #define _LED_PWM_RED2 LED_RED2_GPIO
  #else
    #define _LED_PWM_RED2 0
  #endif
  #if defined(LED_GREEN_GPIO)
    #define _LED_PWM_GREEN LED_GREEN_GPIO
  #else
    #define _LED_PWM_GREEN 0
  #endif
  #if defined(LED_BLUE_GPIO)
    #define _LED_PWM_BLUE LED_BLUE_GPIO
  #else
    #define _LED_PWM_BLUE 0
  #endif

// No status LED pin has a timer output, so dimming toggles the GPIO from a
// fixed-rate timer interrupt counting PWM steps. A late interrupt then only
// shifts one edge, instead of stretching a whole period.
// Full brightness needs no interrupt at all.

#define LED_PWM_FREQ 250  // Hz
#define LED_PWM_LEVELS 32
#define LED_PWM_MIN_LEVEL 2  // dimmest step, the LED never goes darker
#define LED_PWM_TICK_US (1000000 / (LED_PWM_FREQ * LED_PWM_LEVELS))

static gpio_t _led_gpio[4] = {0, 0, 0, 0};  // lit LEDs, 0 = none
static uint8_t _led_bright = STATUS_LED_BRIGHT_MAX;
static uint8_t _led_level = LED_PWM_LEVELS;  // ON steps per period
static uint8_t _led_tick = 0;
static bool _led_pwm_running = false;

static void _led_pins_on()
{
  for (auto pin : _led_gpio)
    if (pin) GPIO_LED_GPIO_ON(pin);
}

static void _led_pins_off()
{
  for (auto pin : _led_gpio)
    if (pin) GPIO_LED_GPIO_OFF(pin);
}

static void _led_pwm_start()
{
  if (_led_pwm_running) return;
  _led_pwm_running = true;

  stm32_timer_enable_clock(STATUS_LED_PWM_TIMER);

  STATUS_LED_PWM_TIMER->CR1 &= ~TIM_CR1_CEN;
  STATUS_LED_PWM_TIMER->PSC = STATUS_LED_PWM_TIMER_FREQ / 1000000 - 1;
  STATUS_LED_PWM_TIMER->ARR = LED_PWM_TICK_US - 1;
  STATUS_LED_PWM_TIMER->CNT = 0;

  NVIC_SetPriority(STATUS_LED_PWM_TIMER_IRQn, 5);
  NVIC_EnableIRQ(STATUS_LED_PWM_TIMER_IRQn);

  _led_tick = 0;
  _led_pins_on();

  STATUS_LED_PWM_TIMER->SR &= ~TIM_SR_UIF;
  STATUS_LED_PWM_TIMER->DIER |= TIM_DIER_UIE;
  STATUS_LED_PWM_TIMER->CR1 |= TIM_CR1_CEN;
}

static void _led_pwm_stop()
{
  if (!_led_pwm_running) return;
  _led_pwm_running = false;

  STATUS_LED_PWM_TIMER->CR1 &= ~TIM_CR1_CEN;
  STATUS_LED_PWM_TIMER->DIER &= ~TIM_DIER_UIE;
  NVIC_DisableIRQ(STATUS_LED_PWM_TIMER_IRQn);
}

extern "C" void STATUS_LED_PWM_TIMER_IRQHandler(void)
{
  // write-only clear, a read of SR would stall on the APB every tick
  STATUS_LED_PWM_TIMER->SR = ~TIM_SR_UIF;

  if (++_led_tick >= LED_PWM_LEVELS) {
    _led_tick = 0;
    _led_pins_on();
  } else if (_led_tick >= _led_level) {
    _led_pins_off();
  }
}

static void _led_apply()
{
  if (!_led_gpio[0] && !_led_gpio[1] && !_led_gpio[2] && !_led_gpio[3]) {
    _led_pwm_stop();
    return;
  }

  if (_led_bright >= STATUS_LED_BRIGHT_MAX) {
    _led_pwm_stop();
    _led_pins_on();
  } else {
    _led_pwm_start();
  }
}

static void _led_on(gpio_t pin, gpio_t pin2)
{
  ledOff();
  _led_gpio[0] = pin;
  _led_gpio[1] = pin2;
  _led_apply();
}

static void _led_on_all()
{
  ledOff();
  _led_gpio[0] = _LED_PWM_RED;
  _led_gpio[1] = _LED_PWM_RED2;
  _led_gpio[2] = _LED_PWM_GREEN;
  _led_gpio[3] = _LED_PWM_BLUE;
  _led_apply();
}

// all status LEDs, until the radio signals it is ready
void ledBoot()
{
  _led_on_all();
}

void ledSetBrightness(uint8_t bright)
{
  if (bright > STATUS_LED_BRIGHT_MAX) bright = STATUS_LED_BRIGHT_MAX;
  if (bright == _led_bright) return;

  // squared over what is left above the floor, so the low end stays usable
  uint32_t level =
      LED_PWM_MIN_LEVEL +
      ((uint32_t)bright * bright * (LED_PWM_LEVELS - LED_PWM_MIN_LEVEL)) /
          (STATUS_LED_BRIGHT_MAX * STATUS_LED_BRIGHT_MAX);

  _led_level = level;
  _led_bright = bright;
  _led_apply();
}

#endif  // STATUS_LED_PWM

#if defined(FUNCTION_SWITCHES) && !defined(FUNCTION_SWITCHES_RGB_LEDS)
static const uint32_t fsLeds[] = {FSLED_GPIO_PIN_1, FSLED_GPIO_PIN_2,
				  FSLED_GPIO_PIN_3, FSLED_GPIO_PIN_4,
				  FSLED_GPIO_PIN_5, FSLED_GPIO_PIN_6};
#endif

__weak void ledInit()
{
#if defined(LED_GREEN_GPIO)
  gpio_init(LED_GREEN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

#if defined(LED_RED_GPIO)
  gpio_init(LED_RED_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

#if defined(LED_RED2_GPIO)
  gpio_init(LED_RED2_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

#if defined(LED_BLUE_GPIO)
  gpio_init(LED_BLUE_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

#if defined(FUNCTION_SWITCHES) && !defined(FUNCTION_SWITCHES_RGB_LEDS)
  for (size_t i = 0; i < DIM(fsLeds); i++) {
    gpio_init(fsLeds[i], GPIO_OUT, GPIO_PIN_SPEED_LOW);
  }
#endif
}

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
__weak void fsLedRGB(uint8_t index, uint32_t color)
{
  rgbSetLedColor(index, GET_RED(color), GET_GREEN(color), GET_BLUE(color));
  rgbLedColorApply();
}

uint8_t getRGBColorIndex(uint32_t color)
{
  for (uint8_t i = 0; i < (sizeof(colorTable) / sizeof(colorTable[0])); i++) {
    if (color == colorTable[i])
      return(i + 1);
  }
  return 0; // Custom value set with Companion
}
#elif defined(FUNCTION_SWITCHES)
__weak void fsLedOff(uint8_t index)
{
  gpio_clear(fsLeds[index]);
}

__weak void fsLedOn(uint8_t index)
{
  gpio_set(fsLeds[index]);
}

__weak bool fsLedState(uint8_t index)
{
  return gpio_read(fsLeds[index]) ? true : false;
}
#endif

__weak void ledOff()
{
#if defined(STATUS_LED_PWM)
  _led_pwm_stop();
  for (auto& pin : _led_gpio) pin = 0;
#endif
#if defined(LED_RED_GPIO)
  GPIO_LED_GPIO_OFF(LED_RED_GPIO);
#endif
#if defined(LED_RED2_GPIO)
  GPIO_LED_GPIO_OFF(LED_RED2_GPIO);
#endif
#if defined(LED_BLUE_GPIO)
  GPIO_LED_GPIO_OFF(LED_BLUE_GPIO);
#endif
#if defined(LED_GREEN_GPIO)
  GPIO_LED_GPIO_OFF(LED_GREEN_GPIO);
#endif
}

__weak void ledRed()
{
#if defined(STATUS_LED_PWM)
  _led_on(_LED_PWM_RED, _LED_PWM_RED2);
#else
  ledOff();
#if defined(LED_RED_GPIO)
  GPIO_LED_GPIO_ON(LED_RED_GPIO);
#endif
#if defined(LED_RED2_GPIO)
  GPIO_LED_GPIO_ON(LED_RED2_GPIO);
#endif
#endif
}

__weak void ledGreen()
{
#if defined(STATUS_LED_PWM)
  _led_on(_LED_PWM_GREEN, 0);
#else
  ledOff();
#if defined(LED_GREEN_GPIO)
  GPIO_LED_GPIO_ON(LED_GREEN_GPIO);
#endif
#endif
}

__weak void ledBlue()
{
#if defined(STATUS_LED_PWM)
  _led_on(_LED_PWM_BLUE, 0);
#else
  ledOff();
#if defined(LED_BLUE_GPIO)
  GPIO_LED_GPIO_ON(LED_BLUE_GPIO);
#endif
#endif
}

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
__weak uint32_t fsGetLedRGB(uint8_t index)
{
  return rgbGetLedColor(index + CFS_LED_STRIP_START);
}
#endif
