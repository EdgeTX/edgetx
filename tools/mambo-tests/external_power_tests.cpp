// SPDX-License-Identifier: GPL-2.0-only
#include "edgetx.h"
#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "targets/mambo/external_rf.h"
#include <cstdlib>
#include <cstdio>
TestModel g_model;
static bool powered = false;
void gpio_write(uintptr_t pin, bool value)
{
  if (pin != EXTMODULE_PWR_GPIO) std::abort();
  powered = value;
}
void gpio_clear(uintptr_t pin) { gpio_write(pin, false); }
bool gpio_read(uintptr_t pin)
{
  if (pin == GPIO_PIN(GPIOB, 2) || pin == GPIO_PIN(GPIOB, 11) ||
      pin == GPIO_PIN(GPIOB, 13)) return false;
  if (pin != EXTMODULE_PWR_GPIO) std::abort();
  return powered;
}
int main()
{
  // Test actual power control against every protocol, including a persisted
  // unsupported model. Off must always cut power after an enabled CRSF model.
  for (unsigned type = 0; type < 256; ++type) {
    g_model.moduleData[EXTERNAL_MODULE].type = MODULE_TYPE_CROSSFIRE;
    EXTERNAL_MODULE_PWR_ON();
    if (!powered) return 1;
    g_model.moduleData[EXTERNAL_MODULE].type = type;
    EXTERNAL_MODULE_PWR_ON();
    if (powered != (type == MODULE_TYPE_CROSSFIRE)) return 1;
    EXTERNAL_MODULE_PWR_OFF();
    if (powered) return 1;
  }
  g_model.moduleData[EXTERNAL_MODULE].type = MODULE_TYPE_CROSSFIRE;
  EXTERNAL_MODULE_PWR_ON();
  if (!powered) return 1;
  if (!mamboExternalDiagnostics().powered) return 1;
  mamboExternalModuleShutdown();
  if (powered) return 1;
  if (mamboExternalDiagnostics().powered) return 1;
  // A late protocol-start request after emergency shutdown cannot undo it.
  EXTERNAL_MODULE_PWR_ON();
  if (powered) return 1;
  std::puts("All 256 persisted module types checked; power-off always succeeds.");
}
