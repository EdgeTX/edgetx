// SPDX-License-Identifier: GPL-2.0-only
// Compiles the actual shared driver against a GPIO fake for both polarities.
#include "board.h"
#include "stm32_gpio.h"
#include <cstdio>
#include <cstdlib>

static bool level = false, latch = false;
static unsigned inputMode = 0, reads = 0;
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr,"line %d: %s\n",__LINE__,#x); std::exit(1); } } while(0)
void gpio_init(unsigned pin, unsigned mode, unsigned speed)
{
  CHECK(speed == GPIO_PIN_SPEED_LOW);
  if (pin == PWR_SWITCH_GPIO) inputMode = mode;
  else CHECK(pin == PWR_ON_GPIO && mode == GPIO_OUT);
}
int gpio_read(unsigned pin) { CHECK(pin == PWR_SWITCH_GPIO); ++reads; return level; }
void gpio_set(unsigned pin) { CHECK(pin == PWR_ON_GPIO); latch = true; }
void gpio_clear(unsigned pin) { CHECK(pin == PWR_ON_GPIO); latch = false; }

int main()
{
  pwrInit();
  CHECK(inputMode == GPIO_IN_PU); // same pull configuration as stock Mambo
#if defined(PWR_SWITCH_PIN_ACTIVE_HIGH)
  constexpr bool pressedLevel = true;
#else
  constexpr bool pressedLevel = false;
#endif
  level = !pressedLevel;
  CHECK(!pwrPressed());
#if defined(PWR_BUTTON_PRESS)
  CHECK(!pwrOffPressed());
#else
  CHECK(pwrOffPressed());
#endif
  level = pressedLevel;
  CHECK(pwrPressed());
#if defined(PWR_BUTTON_PRESS)
  CHECK(pwrOffPressed());
#else
  CHECK(!pwrOffPressed());
#endif
  level = !pressedLevel;
  CHECK(!pwrPressed());
  CHECK(reads == 5);
  pwrOn(); CHECK(latch);
  pwrOff(); CHECK(!latch);
  pwrResetHandler(); CHECK(!latch);
  std::printf("Actual power driver: %s polarity passed\n",pressedLevel ? "active-high" : "active-low");
}
