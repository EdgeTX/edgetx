// SPDX-License-Identifier: GPL-2.0-only
#include "inputs.h"

#include "board.h"
#include "hal/adc_driver.h"
#include "hal/gpio.h"
#include "stm32_gpio.h"

void mamboEarlyInit()
{
  // RF and gimbals share SPI2. No RF startup or module power is permitted in
  // this bench target; hold the SX1281 reset and de-select both Hall sensors.
  gpio_init(MAMBO_RF_RESET_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_clear(MAMBO_RF_RESET_GPIO);
  gpio_init(GPIO_PIN(GPIOE, 3), GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_set(GPIO_PIN(GPIOE, 3));
  gpio_init(EXTMODULE_PWR_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_clear(EXTMODULE_PWR_GPIO);
  gpio_init(GPIO_PIN(GPIOD, 7), GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_set(GPIO_PIN(GPIOD, 7));
  gpio_init(GPIO_PIN(GPIOE, 8), GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_set(GPIO_PIN(GPIOE, 8));
}

uint32_t readTrims()
{
  // RAW1 is the trim resistor ladder, after four virtual sticks and two pots.
  constexpr unsigned trimInput = 6;
  return mambo::decodeTrimLadder(getAnalogValue(trimInput));
}
